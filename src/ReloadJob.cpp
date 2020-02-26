/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// ReloadJob.cpp: implementation of the ReloadJob class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ReloadJob.h"
#include "PhotoInfoStorage.h"
#include "ImageScanner.h"
#include "Database/ImageDatabase.h"
#include "CatchAll.h"
#include "ImgDb.h"
#include "Exception.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

namespace {
	const DWORD SYNCHRO_DELAY= 400; // ms
}

struct ReloadJob::Impl
{
	Impl(PhotoInfoStorage& photos, std::auto_ptr<ImageScanner> scanner, bool readOnlyPhotosWithEXIF,
					   bool recursiveScan, HWND parent_wnd, uint32 dbFileLengthLimitMB, ImageDatabase& dbImages);

	std::auto_ptr<ImageScanner> scanner_;
	bool scanning_;
	bool recursiveScan_;
	bool readOnlyPhotosWithEXIF_;
	HANDLE thread_handle_;
	HWND parentWnd_;
	ImageDatabase& dbImages_;
	uint32 dbFileLengthLimitMB_;
	PhotoInfoStorage& photos_;
	DWORD lastSyncTime_;		// last synchro time, to avoid sending notification too frequently
	size_t lastCountReported_;

	static UINT AFX_CDECL WorkerProc(LPVOID param);
	int LoadPhotos();
	void NotifyClient();
	void Quit();
};


ReloadJob::Impl::Impl(PhotoInfoStorage& photos, std::auto_ptr<ImageScanner> scanner,
					   bool readOnlyPhotosWithEXIF, bool recursiveScan, HWND parent_wnd,
					   uint32 dbFileLengthLimitMB, ImageDatabase& dbImages)
 : scanner_(scanner), recursiveScan_(recursiveScan), parentWnd_(parent_wnd), scanning_(false),
   readOnlyPhotosWithEXIF_(readOnlyPhotosWithEXIF), dbFileLengthLimitMB_(dbFileLengthLimitMB),
   dbImages_(dbImages), photos_(photos)
{
	thread_handle_ = 0;	// handle to thread (will be used to test thread completion)

	// create worker thread
	CWinThread* thread= ::AfxBeginThread(WorkerProc, this, THREAD_PRIORITY_LOWEST /*THREAD_PRIORITY_IDLE*/, 0, CREATE_SUSPENDED);

	if (thread == 0)
	{
		ASSERT(false);	// thread not created
		THROW_EXCEPTION(L"Thread creation failed", L"Attempt to create a thread failed.");
	}

	// if thread created duplicate it's handle (cause CWinThread will close it on exit)
	if (!::DuplicateHandle(::GetCurrentProcess(), *thread, ::GetCurrentProcess(), &thread_handle_, 0, false, DUPLICATE_SAME_ACCESS))
	{
		thread->ExitInstance();
		thread = 0;
		ASSERT(false);
		THROW_EXCEPTION(L"Thread creation failed", L"Attempt to duplicate thread handle failed.");
	}

	TRACE(_T("Reloading thread started\n"));
	thread->ResumeThread();
}


ReloadJob::ReloadJob(PhotoInfoStorage& photos, std::auto_ptr<ImageScanner> scanner, bool readOnlyPhotosWithEXIF,
					   bool recursiveScan, HWND parent_wnd, uint32 dbFileLengthLimitMB, ImageDatabase& dbImages)
 : pImpl_(new Impl(photos, scanner, readOnlyPhotosWithEXIF, recursiveScan, parent_wnd, dbFileLengthLimitMB, dbImages))
{
}


ReloadJob::~ReloadJob()
{
	Quit();
}


void ReloadJob::Quit()
{
	pImpl_->Quit();
}


void ReloadJob::Impl::Quit()	// break an execution and exit
{
	scanner_->CancelScan();	// signal break

	if (thread_handle_)
	{
		// boost thread priority so it can reach the break flag
		// note: this may not help if thread group/pool is hard at work
		::SetThreadPriority(thread_handle_, THREAD_PRIORITY_NORMAL);

		DWORD exit= ::WaitForSingleObject(thread_handle_, 10000);
		if (exit == WAIT_OBJECT_0)
		{
			// thread quit
			TRACE(_T("Reloading thread closed\n"));
		}
		else if (exit == WAIT_TIMEOUT)
		{
			ASSERT(false);		// deadlock?
			::TerminateThread(thread_handle_, -1);

			if (dbImages_.IsOpen())
				dbImages_.Flush();

			if (CWnd* wnd= AfxGetMainWnd())
			{
				::PostMessage(parentWnd_, RELOADING_THREAD_MSG, 0, RELOADING_CANCELLED);
				wnd->EnableWindow();
				wnd->SetActiveWindow();
			}
		}

		// release thread handle
		::CloseHandle(thread_handle_);
		thread_handle_ = 0;
	}

	if (parentWnd_)
		::PostMessage(parentWnd_, RELOADING_THREAD_MSG, -1, RELOADING_QUIT);
}


void ReloadJob::Impl::NotifyClient()
{
	// notify parent window if enough time has passed; notifications should be sparse
	// since PostMsg takes a long time due to the thread synchronization

	if (labs(static_cast<long>(::GetTickCount() - lastSyncTime_)) > SYNCHRO_DELAY)
	{
		lastSyncTime_ = ::GetTickCount();
		const size_t count= photos_.size();

		if (lastCountReported_ < count)
		{
			// let parent window know about new photo
			if (parentWnd_)
				::PostMessage(parentWnd_, RELOADING_THREAD_MSG, count, RELOADING_PENDING);

			lastCountReported_ = count;
		}
	}
}


UINT AFX_CDECL ReloadJob::Impl::WorkerProc(LPVOID param)
{
	ReloadJob::Impl* impl= reinterpret_cast<ReloadJob::Impl*>(param);

	int ret= -1;
	try
	{
		ret = impl->LoadPhotos();
	}
	CATCH_ALL_W(AfxGetMainWnd())

	if (impl->parentWnd_)
		::PostMessage(impl->parentWnd_, RELOADING_THREAD_MSG, 0, RELOADING_QUIT);

	return ret;
}


int ReloadJob::Impl::LoadPhotos()
{
	// reloading photos: post messages before and after scanning the path
	scanning_ = true;
	if (parentWnd_)
		::PostMessage(parentWnd_, RELOADING_THREAD_MSG, 0, RELOADING_START);

	// find image db location and open db
	//String path= dbImages_.CreateDbFolderIfNeeded();
	String path= GetConfiguredDbFileAndPath();
	if (!path.empty())
	{
		if (dbImages_.IsOpen())
			dbImages_.Close();

		uint32 limit= dbFileLengthLimitMB_;
#ifdef _UNICODE
		if (dbFileLengthLimitMB_ >= 4096)		// currently db is limited to 4 GB in size
			limit = 4095;	// to prevent it from crossing 4 GB barrier limit to 1 MB less than 4 GB
#else
		if (dbFileLengthLimitMB_ >= 2048)		// currently db is limited to 2 GB in size
			limit = 2047;	// to prevent it from crossing 2 GB barrier limit to 1 MB less than 2 GB
#endif
		dbImages_.LimitFileLength(path, uint64(limit) << 20);
		dbImages_.OpenDb(path, false);
	}

	lastSyncTime_ = ::GetTickCount() + SYNCHRO_DELAY / 5;
	lastCountReported_ = 0;

	scanner_->SetNotificationCallback(boost::bind(&Impl::NotifyClient, this));
	scanner_->ReadPhotosWithExifOnly(readOnlyPhotosWithEXIF_);
	scanner_->SetImageDatabase(dbImages_);

	uint32 dir_id= 0;

	bool fin= scanner_->Scan(recursiveScan_, dir_id, photos_);

	if (dbImages_.IsOpen())
		dbImages_.Flush();
		//dbImages_.Close();

	scanning_ = false;
	if (parentWnd_)
		::PostMessage(parentWnd_, RELOADING_THREAD_MSG, 0, fin ? RELOADING_FINISHED : RELOADING_CANCELLED);

	return 0;
}


// is scanning operation still pending?
//
bool ReloadJob::IsPending() const
{
	if (this == 0)
		return false;

	if (pImpl_->thread_handle_ == 0)
		return false;

	DWORD exit= ::WaitForSingleObject(pImpl_->thread_handle_, 0);

	if (exit == WAIT_TIMEOUT)
		return true;	// thread is working

	return false;
}


// is scanning operation still pending?
//
bool ReloadJob::IsScanning() const
{
	if (this == 0)
		return false;

	return pImpl_->scanning_;
}


// log of image load/scan problems
const ImgLogger& ReloadJob::GetImageErrorLogger() const
{
	return pImpl_->scanner_->GetLogger();
}

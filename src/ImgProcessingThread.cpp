/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// ImgProcessingThread.cpp: simple working thread wrapper
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ImgProcessingThread.h"
#include "JPEGException.h"
#include "Exception.h"
#include "PhotoQueue.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


ImgProcessingThread::ImgProcessingThread(size_t file_count)
 : file_count_(file_count), status_wnd_(0)
{
	index_ = nullptr;
	break_ = false;
	thread_handle_ = 0;
	pending_ = true;
	cur_file_index_ = ~0;
//	file_begin_ = 0;
//	file_step_ = 1;
	thread_ = ::AfxBeginThread(WorkerProc, this, THREAD_PRIORITY_BELOW_NORMAL, 0, CREATE_SUSPENDED);

	if (thread_ == 0)
		THROW_EXCEPTION(L"未能启动图像处理", L"图像处理线程创建失败")

	thread_handle_ = *thread_;
}


ImgProcessingThread::~ImgProcessingThread()
{
	Quit();
}


void ImgProcessingThread::Start()
{
	if (thread_)
		thread_->ResumeThread();
}


void ImgProcessingThread::Start(PhotoQueue* index)
{
//	if (from < file_count_ && step > 0)
	{
//		file_begin_ = from;
		index_ = index;
//		file_step_ = step;

		if (thread_)
			thread_->ResumeThread();
	}
//	else
//	{ ASSERT(false); }
}


UINT AFX_CDECL ImgProcessingThread::WorkerProc(LPVOID param)
{
	reinterpret_cast<ImgProcessingThread*>(param)->Exec();
	return 0;
}


void ImgProcessingThread::Quit()	// break an execution and exit
{
	break_ = true;

	Break();	// signal break

	if (thread_handle_ && pending_)
	{
		DWORD exit= ::WaitForSingleObject(thread_handle_, 5000);
		if (exit == WAIT_OBJECT_0)
			return;	  // thread quit
		ASSERT(false);	// something's wrong: thread locked?
		if (exit == WAIT_TIMEOUT)
			::TerminateThread(thread_handle_, -1);
	}

	thread_handle_ = 0;
}


void ImgProcessingThread::Exec()
{
	bool completed= false;
	String msg;

	cur_file_index_ = 0;

	try
	{
		for (;;)
		{
			LONG index= -1;

			if (index_)
			{
				index = index_->GetNext();

				if (index < 0)
					break;	// done, no more items available

				cur_file_index_ = static_cast<size_t>(index);
			}
			else
			{
				if (cur_file_index_ >= file_count_)
					break;
			}


			if (break_)
			{
				completed = false;
				break;
			}

			if (status_wnd_)
				status_wnd_->PostMessage(IPM_NEXT_PHOTO, cur_file_index_, 0);

			Process(cur_file_index_);

			if (status_wnd_)
				status_wnd_->PostMessage(IPM_NEXT_PHOTO, cur_file_index_, 1);

			++cur_file_index_;
		}

		completed = true;
	}
	catch (CException* ex)
	{
		TCHAR error_message[512];
		if (ex->GetErrorMessage(error_message, array_count(error_message)))
			msg = error_message;
		else
			msg = _T("MFC 遇到异常情况");
		ex->Delete();
	}
	catch (std::exception& ex)
	{
		msg = CString(ex.what());
	}
	catch (JPEGException& ex)
	{
		msg = ex.GetMessage();
	}
	catch (String& str)
	{
		msg = str;
	}
	catch (const TCHAR* ex)
	{
		msg = ex;
	}
	catch (Exception& ex)
	{
		msg = ex.GetTitle();
		msg += L"\n";
		msg += ex.GetDescription();
	}
#ifndef _DEBUG
	catch (...)
	{
		msg = _T("遭遇致命错误.");
	}
#endif

	if (!completed && status_wnd_)
	{
		if (cur_file_index_ < file_count_)
			msg += L"\n文件: " + GetSourceFileName(cur_file_index_);
		status_wnd_->PostMessage(IPM_EXCEPTION, cur_file_index_, reinterpret_cast<LPARAM>(new String(msg)));
	}

	if (status_wnd_)
		status_wnd_->PostMessage(IPM_NEXT_PHOTO, completed ? IPM_COMPLETION_STATUS_OK : IPM_COMPLETION_STATUS_ERR);

	pending_ = false;
}


// decoder progress
bool ImgProcessingThread::LinesDecoded(int lines_ready, int img_height, bool finished)
{
	if (break_)
		return false;	// stop

	if (lines_ready % 10 == 0)	// every 10 scan lines report decoding progress
	{
		// progress value: 0..50%
		int progress= lines_ready * 50 / img_height;
		if (status_wnd_)
			status_wnd_->PostMessage(IPM_PROGRESS, progress, 1);
	}

	return true;		// continue
}

// encoder progress
bool ImgProcessingThread::LinesEncoded(int lines_ready, int img_height, bool finished)
{
	if (break_)
		return false;	// stop

	if (lines_ready % 10 == 0)	// every 10 scan lines report encoding progress
	{
		// progress value: 50..100%
		int progress= 50 + lines_ready * 50 / img_height;
		if (status_wnd_)
			status_wnd_->PostMessage(IPM_PROGRESS, progress, 2);
	}

	return true;		// continue
}


void ImgProcessingThread::Break()
{
	break_ = true;
}


void ImgProcessingThread::SetOperationLabel(int str_rsrc_id)
{
	if (status_wnd_)
		status_wnd_->PostMessage(IPM_NEXT_OPERATION, str_rsrc_id);
}


void ImgProcessingThread::SendPhotoStatus(int status) const
{
	if (status_wnd_)
		status_wnd_->PostMessage(IPM_PHOTO_STATUS, cur_file_index_, status);
}

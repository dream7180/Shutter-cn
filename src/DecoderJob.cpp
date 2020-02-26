/*____________________________________________________________________________

   EXIF Image Viewer

   Copyright (C) 2000-2011 Michael Kowalski
____________________________________________________________________________*/

// DecoderJob.cpp: implementation of the DecoderJob class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DecoderJob.h"
#include "BmpFunc.h"
#include "Config.h"
#include "FileDataSource.h"
#include "ColorProfile.h"
#include "JPEGException.h"
#include "ErrorDlg.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


struct DecoderJob::Impl : DecoderProgress
{
	Impl(const PhotoInfo& photo, CSize photo_size, bool auto_rotate, double scale_requested, CSize requested_size, CWnd* display_wnd, unsigned int orientation)
	 : bmp_(new Dib()), working_copy_(new Dib()), file_(photo.GetDisplayPath()), requested_size_(requested_size), scale_requested_(scale_requested)
	{
		photo_ = &photo;
		requested_orientation_ = orientation & 3;
		pending_ = false;
		rotation_dir_ = 0;
		display_wnd_ = display_wnd;
		orientation_ = photo.GetOrientation();
		exif_bmp_size_.cx = photo_size.cx;
		exif_bmp_size_.cy = photo_size.cy;
		auto_rotate_ = auto_rotate;
	//	ICC_transform_ = trans;
		decoding_status_ = IS_NO_IMAGE;

		decoder_ = photo.GetDecoder();
		decoder_->SetProgressClient(this);	// send progress info here (to the LinesDecoded)
		StartThread();
	}
	virtual ~Impl()
	{}

	CImageDecoderPtr decoder_;
	String file_;
	CCriticalSection lock_;
	DibPtr bmp_;
	DibPtr temp_;
	DibPtr working_copy_;	// use this bitmap  when starting decoder, then swap it with bmp_
							// that way, viewer either sees no valid bitmap, or already created one;
							// since CDid is not synchronized for thread access, this is a way to ensure data integrity
	HANDLE thread_;
	bool pending_;
	unsigned int requested_orientation_;
	int rotation_dir_;
	int lines_ready_;
	int last_line_ready_;
	CSize exif_bmp_size_;		// bmp size as recorded in EXIF block (may not reflect real size)
	CSize requested_size_;		// requested bmp size (dictated by current preview window size and scale),
								// this size may have different ratio then photo's ratio
	double scale_requested_;	// 0.0 or scaling requested (that is if it's 0.5 then 2 times reduction will be used)
//	bool rescale_;			// if true loaded image will be rescalled to requested size (useful for thumbnails)
	PhotoInfo::ImgOrientation orientation_;
	bool auto_rotate_;
	CWnd* display_wnd_;
	bool break_;
	ConstPhotoInfoPtr photo_;
	ICMTransform ICC_transform_;
	ImageStat decoding_status_;

	enum State { START, PROGRESS, FINISHED };

	static UINT AFX_CDECL WorkerProc(LPVOID param);
	int Decode();

//	friend class CPhotoDecoder;		// grant access to LinesDecoded fn

	void LinesDecoded(State state, int lines_from, int lines_to);
	virtual bool LinesDecoded(int lines_ready, bool finished);	// progress info from the decoder

	void Restart(CSize requested_size);
	void StartThread();			// start worker thread
	void Quit();				// stop decoding and exit

	UINT_PTR GetUniqueId() const;
};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

DecoderJob::DecoderJob(const PhotoInfo& photo, CSize photo_size, bool auto_rotate,
	double scale_requested, CSize requested_size, CWnd* display_wnd, unsigned int orientation)
 : impl_(new Impl(photo, photo_size, auto_rotate, scale_requested, requested_size, display_wnd, orientation))
{
}


void DecoderJob::Restart(CSize requested_size)
{
	impl_->Restart(requested_size);
}

void DecoderJob::Impl::Restart(CSize requested_size)
{
	// stop if it's pending
	Quit();

	// new size & gamma
	requested_size_ = requested_size;
//	ICC_transform_ = trans;

	// restart worker thread
	StartThread();
}


void DecoderJob::Impl::StartThread()
{
	break_ = false;
	pending_ = true;
	lines_ready_ = 0;
	last_line_ready_ = 0;
	thread_ = 0;

	if (CWinThread* thread= ::AfxBeginThread(WorkerProc, this,
		THREAD_PRIORITY_LOWEST/*THREAD_PRIORITY_IDLE*//*THREAD_PRIORITY_NORMAL*/, 0, CREATE_SUSPENDED))
	{
		thread_ = *thread;
		thread->ResumeThread();
	}
	else
		AfxThrowMemoryException();
}


DecoderJob::~DecoderJob()
{
	Quit();
}


UINT AFX_CDECL DecoderJob::Impl::WorkerProc(LPVOID param)
{
	return reinterpret_cast<DecoderJob::Impl*>(param)->Decode();
}


int DecoderJob::Impl::Decode()
{
	int ret= 0;
	ImageStat stat= IS_DECODING_FAILED;
	decoding_status_ = IS_NO_IMAGE;

	const wchar_t* err_caption= L"Image Decoding Error";
	CWnd* wnd= display_wnd_ ? display_wnd_ : AfxGetMainWnd();

	try
	{
		// because of auto rotation there's no way of knowing which size (cx or cy) should
		// be a limiting factor in resizing a photograph; for safety both sizes are made equal to the bigger one
		int max= std::max(requested_size_.cx, requested_size_.cy);
		CSize requested(max, max);

		if (scale_requested_ != 0.0)
		{
			if (scale_requested_ < 1.0)
			{
				requested.cx = static_cast<int>(exif_bmp_size_.cx * scale_requested_);
				requested.cy = static_cast<int>(exif_bmp_size_.cy * scale_requested_);
			}
			else
				requested = exif_bmp_size_;
		}

		if (ICC_transform_.IsValid())
			decoder_->SetICCProfiles(photo_->HasColorProfile() ? photo_->GetColorProfile() : ICC_transform_.in_,
			ICC_transform_.out_, ICC_transform_.rendering_intent_);

		stat = decoder_->DecodeImg(*working_copy_, requested, false);

//		if (rescale_ && requested_size_.cx != 0 && requested_size_.cy != 0)
//			bmp->Resize(img_size, rgb_back, false);

		pending_ = false;
		ret = stat == IS_OK ? 1 : 3;
	}
	catch (CMemoryException* ex)
	{
		pending_ = false;
		wnd->MessageBox(_T("Out of memory."));
		ex->Delete();
		stat = IS_OUT_OF_MEM;
		ret = 3;
	}
	catch (CException* ex)
	{
		pending_ = false;
		{
			TCHAR error_message[512];
			if (ex->GetErrorMessage(error_message, array_count(error_message)))
				DisplayErrorDialog(err_caption, wnd, error_message, 0, false);
			else
				DisplayErrorDialog(L"MFC Exception Encountered", wnd, error_message, 0, false);
		}
		ex->Delete();
		stat = IS_OUT_OF_MEM;
		ret = 3;
	}
	catch (JPEGException& ex)
	{
		pending_ = false;
		std::wstring ttl= L"Error decoding \"" + file_ + L'"';
		std::wstring details;
		if (const wchar_t* msg= ex.GetMessage())
		{
			details += L"\nJPEG decoding error: ";
			details += msg;
		}
		DisplayErrorDialog(err_caption, wnd, ttl.c_str(), details.c_str(), false);
		stat = IS_DECODING_FAILED;
		ret = 4;
	}
	catch (...)		// fatal error
	{
		pending_ = false;
		String ttl= L"Error reading " + file_;
		DisplayErrorDialog(err_caption, wnd, ttl.c_str(), 0, false);
		stat = IS_DECODING_FAILED;
		ret = 4;
	}

	decoding_status_ = stat;

	if (!break_ && display_wnd_)
	{
		UINT_PTR job_id= GetUniqueId();
		if (::IsWindow(*display_wnd_))
			display_wnd_->PostMessage(WM_USER + 1000, job_id, stat);	// one more message to wake up OnIdle and update UI
	}

	return ret;
}


void DecoderJob::Quit()	// break an execution and exit
{
	impl_->Quit();
}

void DecoderJob::Impl::Quit()
{
	break_ = true;	// signal break
	if (thread_ && pending_)
	{
		DWORD exit= ::WaitForSingleObject(thread_, 3000);
		if (exit == WAIT_OBJECT_0)
			return;	  // thread quit
		if (exit == WAIT_TIMEOUT)
		{
			ASSERT(false);
			::TerminateThread(thread_, -1);
		}
	}
	thread_ = 0;
}


// DecoderProgress method invoked by decoder (worker thread)
//
bool DecoderJob::Impl::LinesDecoded(int lines_ready, bool finished)
{
	if (break_)
		return false;	// do not continue

	if (!finished && lines_ready == 0)
	{
		LinesDecoded(START, -1, 0);
		return true;
	}

	const int STEP= LINES_PER_STEP;
	int delta_lines= lines_ready - last_line_ready_;

	if (delta_lines >= STEP || finished)
	{
		last_line_ready_ = lines_ready;
		int lines_from= lines_ready - delta_lines;
		if (finished)
		{
			if (lines_ready < STEP)
			{
				int rest= lines_ready % STEP;
				if (rest)
					lines_from = lines_ready - rest;
				else
					lines_from = lines_ready - STEP;
			}

		}
		if (lines_from < 0)
			lines_from = 0;
		LinesDecoded(PROGRESS, lines_from, lines_ready);
	}

	if (finished)
		LinesDecoded(FINISHED, -1, lines_ready);

	return true;
}

// this method is invoked from the worker thread
//
void DecoderJob::Impl::LinesDecoded(State state, int lines_from, int lines_to)
{
	UINT_PTR job_id= GetUniqueId();

	if (state == START)
	{
		// quick check of dimensions: if photo was altered (rotated) auto-rotation shouldn't be applied
		CSize size= decoder_->GetOriginalSize();

//		bool original_orientation_preserved = size.cx == exif_bmp_size_.cx && size.cy == exif_bmp_size_.cy && size.cx >= size.cy;
		bool original_orientation_preserved= false;
		if (size.cx > 0 && size.cy > 0 && exif_bmp_size_.cx > 0 && exif_bmp_size_.cy > 0)
		{
			double size_ratio= size.cx / double(size.cy);
			double exif_ratio= exif_bmp_size_.cx / double(exif_bmp_size_.cy);
			// big tolerance
			original_orientation_preserved = fabs(size_ratio - exif_ratio) < 0.1;
		}

		// move working copy to the bmp visible by a client now that it is created
		//{
		//	CSingleLock lock(&lock_, true);
		//	working_copy_.swap(bmp_);
		//}

		bool send_rotation_notification= false;

		if (auto_rotate_ && original_orientation_preserved || requested_orientation_ != 0)
		{
			int width= working_copy_->GetWidth();
			int height= working_copy_->GetHeight();
			int bpp= working_copy_->GetBitsPerPixel();
			if (requested_orientation_ != 0)
			{
				if (requested_orientation_ == 1)
					rotation_dir_ = 1;
				else if (requested_orientation_ == 3)
					rotation_dir_ = -1;
				else if (requested_orientation_ == 2)
					rotation_dir_ = 2;
			}
			else if (rotation_dir_ == 0)
			{
				if (orientation_ == PhotoInfo::ORIENT_90CW)
					rotation_dir_ = -1;
				else if (orientation_ == PhotoInfo::ORIENT_90CCW)
					rotation_dir_ = 1;
			}
			if (rotation_dir_ != 0 && bpp == 24)
			{
				{
					CSingleLock lock(&lock_, true);
					temp_ = working_copy_;
					if (rotation_dir_ == 2)
						working_copy_.reset(new Dib(width, height, bpp, 0));	// bitmap upside down
					else
						working_copy_.reset(new Dib(height, width, bpp, 0));	// rotated bitmap
				}

				// inform about auto rotation only
				send_rotation_notification = true;
			}
		}

		// move working copy to the bmp visible by a client now that it is created
		{
			CSingleLock lock(&lock_, true);
			working_copy_.swap(bmp_);
		}

		if (send_rotation_notification && requested_orientation_ == 0)
			if (::IsWindow(display_wnd_->GetSafeHwnd()))
				display_wnd_->PostMessage(WM_USER + 998, job_id, rotation_dir_);

		if (::IsWindow(display_wnd_->GetSafeHwnd()))
			display_wnd_->PostMessage(WM_USER + 999, job_id, 0);	// init display
	}
	else if (state == PROGRESS)
	{
		if (lines_to > 0 && lines_from != -1)
		{
//			if (gamma_ != 0.0)
//				::ApplyGammaInPlace(rotation_dir_ != 0 ? temp_.get() : bmp_.get(), gamma_, lines_from, lines_to);
			if (rotation_dir_ != 0 && temp_.get() != 0)
				if (rotation_dir_ == 2)
					::Rotate180(temp_.get(), bmp_.get(), lines_from, lines_to);
				else
					::Rotate(temp_.get(), bmp_.get(), rotation_dir_ > 0, lines_from, lines_to);
			lines_ready_ = lines_to;
		}
//::Sleep(150);
		if (::IsWindow(display_wnd_->GetSafeHwnd()))
			display_wnd_->PostMessage(WM_USER + 999, job_id, MAKELPARAM(lines_from, lines_to));	// display decoded part
	}
	else if (state == FINISHED)
	{
		temp_.reset();

		if (::IsWindow(display_wnd_->GetSafeHwnd()))
			display_wnd_->PostMessage(WM_USER + 999, job_id, MAKELPARAM(-1, lines_to));	// decoding finished
	}
	else
	{
		ASSERT(false);
	}
}


UINT_PTR DecoderJob::Impl::GetUniqueId() const
{
	return reinterpret_cast<UINT_PTR>(this);	// not used as a pointer, but as an unique id
}


UINT_PTR DecoderJob::GetUniqueId() const
{
	return impl_->GetUniqueId();
}


bool DecoderJob::Pending() const
{
	return impl_->pending_;
}


DibPtr DecoderJob::GetBitmap() const
{
	CSingleLock lock(&impl_->lock_, true);
	DibPtr copy= impl_->bmp_;
	return copy;
}


const String& DecoderJob::FileName() const				{ return impl_->file_; }

int DecoderJob::Rotated() const						{ return impl_->rotation_dir_; }

bool DecoderJob::AutoRotation() const					{ return impl_->auto_rotate_ != 0; }

int DecoderJob::LinesReady() const						{ return impl_->lines_ready_; }

bool DecoderJob::GammaEnabled() const					{ return impl_->ICC_transform_.IsValid(); }

// current ICM transformation
const ICMTransform& DecoderJob::GetICMTransform() const	{ return impl_->ICC_transform_; }

// how many times img was reduced
int DecoderJob::ReductionFactor() const				{ return impl_->decoder_->ReductionFactor(); }

// orientation
PhotoInfo::ImgOrientation DecoderJob::GetOrientation() const	{ return impl_->orientation_; }

CSize DecoderJob::GetExifImgSize() const				{ return impl_->exif_bmp_size_; }

CSize DecoderJob::GetOriginalSize() const				{ return impl_->decoder_->GetOriginalSize(); }

CSize DecoderJob::GetRequestedSize() const				{ return impl_->requested_size_; }

ConstPhotoInfoPtr DecoderJob::GetPhoto() const			{ return impl_->photo_; }

ImageStat DecoderJob::GetDecodingStatus() const		{ return impl_->decoding_status_; }


DibPtr GetDecoderBitmap(DecoderJob* decoder)
{
	DibPtr dib;
	if (decoder)
		dib = decoder->GetBitmap();
	return dib;
}


UINT_PTR GetDecoderUniqueId(DecoderJob* decoder)
{
	if (decoder == nullptr)
		return 0;
	return decoder->GetUniqueId();
}

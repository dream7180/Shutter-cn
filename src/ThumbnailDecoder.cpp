/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "ThumbnailDecoder.h"
#include "PhotoInfo.h"
#include "Dib.h"
#include "BmpAlpha.h"
#include "DecoderProgress.h"
#include "Config.h"

//#ifdef WIN64
//#define MAGICK_STATIC_LINK 1
//#define MAGICKCORE_LQR_DELEGATE 1
//#include "C:\data\ImageMagick-6.4.4\magick\MagickCore.h"
//#endif

struct ThumbnailDecoder::Impl : DecoderProgress
{
	DibPtr dib_;
	PhotoInfo& photo_;
	HANDLE thread_;
//	DibPtr dummy_;
	CWnd* receiver_;
	CSize size_;
	UINT_PTR unique_id_;
	bool continue_decoding_;
	std::auto_ptr<CWinThread> win_thread_;

	Impl(PhotoInfo& photo, CSize size, CWnd* receiver)
		: thread_(0), photo_(photo), receiver_(receiver), size_(size)
	{
		ASSERT(size.cx > 0 && size.cy > 0);
		static UINT_PTR id= 0;
		unique_id_ = ++id;
		continue_decoding_ = true;
		dib_.reset(new Dib());
	}

	void StartThread();
	static UINT AFX_CDECL WorkerProc(LPVOID param);
	int Decode();
	void Quit();

	// decoder progress
	virtual bool LinesDecoded(int linesReady, bool finished);
};


ThumbnailDecoder::ThumbnailDecoder(PhotoInfo& photo, CSize size, CWnd* receiver) : impl_(new Impl(photo, size, receiver))
{
	impl_->StartThread();
}


ThumbnailDecoder::~ThumbnailDecoder()
{
	impl_->Quit();
}


void ThumbnailDecoder::Impl::StartThread()
{
//	pending_ = true;
	//lines_ready_ = 0;
	//last_line_ready_ = 0;
	thread_ = 0;

	// idle priority is not to be used, or else any background activity will prevent decoding
	// from executing
	if (CWinThread* thread= ::AfxBeginThread(WorkerProc, this,
		THREAD_PRIORITY_LOWEST/*THREAD_PRIORITY_IDLE*//*THREAD_PRIORITY_NORMAL*/, 0, CREATE_SUSPENDED))
	{
		thread->m_bAutoDelete = false;
		win_thread_.reset(thread);
		thread_ = *thread;
		thread->ResumeThread();
	}
	else
		AfxThrowMemoryException();
}


UINT AFX_CDECL ThumbnailDecoder::Impl::WorkerProc(LPVOID param)
{
	return reinterpret_cast<ThumbnailDecoder::Impl*>(param)->Decode();
}


extern void OutputTimeStr(LARGE_INTEGER start, const TCHAR* str)
{
	LARGE_INTEGER time;
	::QueryPerformanceCounter(&time);
	time.QuadPart -= start.QuadPart;

	LARGE_INTEGER frq;
	::QueryPerformanceFrequency(&frq);

	time.QuadPart = time.QuadPart * 1000000 / frq.QuadPart;
	int ms= int(time.QuadPart / 1000);
	int fr= int(time.QuadPart % 1000);

	TCHAR buf[1000];
	wsprintf(buf, _T("%s: %d.%03d ms\n"), str, ms, fr);
	::OutputDebugString(buf);
}


int ThumbnailDecoder::Impl::Decode()
{
//TRACE(L"START>> == ThumbnailDecoder %x,%d (%s)\n", (int)thread_, unique_id_, photo_.GetName().c_str());
	int status= IS_DECODING_FAILED;

	try
	{
//LARGE_INTEGER start,time;
//::QueryPerformanceCounter(&time);
//start.QuadPart = time.QuadPart;

		status = photo_.CreateThumbnail(*dib_, this);

//OutputTimeStr(time, _T("decode"));

		if (status == IS_OK && dib_->IsValid())
		{
//::QueryPerformanceCounter(&time);

			dib_->ResizeToFit(size_, Dib::RESIZE_CUBIC);

			if (float amount= g_Settings.GetThumbnailSharpening())
			{
				Dib sharp;
				int threshold= 10;
				if (UnsharpMask(*dib_, sharp, threshold, amount))
					dib_->Swap(sharp);
//OutputTimeStr(time, _T("sharp"));
			}

/*
#ifdef WIN64

OutputTimeStr(time, _T("resize"));


::QueryPerformanceCounter(&time);

			ExceptionInfo exc;
			GetExceptionInfo(&exc);

			// image magick requires packed pixels, no way to skip padding, so repackage the bmp:
			vector<BYTE> bmp(dib_.GetWidth() * dib_.GetHeight() * dib_.GetColorComponents());
			const size_t line_data_len= dib_.GetWidth() * dib_.GetColorComponents();
			BYTE* p= &bmp[0];
			const BYTE* src= dib_.GetBuffer();
			for (int y= 0; y < dib_.GetHeight(); ++y)
			{
				memcpy(p, src, line_data_len);
				src += dib_.GetLineBytes();
				p += line_data_len;
			}

			Image* img= ConstituteImage(dib_.GetWidth(), dib_.GetHeight(), "RGB", CharPixel, &bmp[0], &exc);

OutputTimeStr(time, _T("constit"));

			if (img)
			{
::QueryPerformanceCounter(&time);

				double radius= 1.0;
				double sigma= 0.5;
				double amount= 1.0;
				double threshold= 0.05;

				Image* sharp= UnsharpMaskImage(img, radius, sigma, amount, threshold, &exc);

OutputTimeStr(time, _T("unsharp"));

				//int w= size.cy * 16 / 9;
				//Image* sharp= LiquidRescaleImage(img, w, size.cy, 1.0, 0.0, &exc);

				if (sharp)
				{
					//ImageInfo ii;
					//GetImageInfo(&ii);
::QueryPerformanceCounter(&time);

					CSize size(sharp->columns, sharp->rows);

					const PixelPacket* pix= AcquireImagePixels(sharp, 0, 0, size.cx, size.cy, &exc);

					Dib b(size.cx, size.cy, 24);
					for (int y= size.cy - 1; y >= 0; --y)
					{
						BYTE* line= b.LineBuffer(y);
						for (int x= 0; x < size.cx; ++x)
						{
							line[0] = pix->red;
							line[1] = pix->green;
							line[2] = pix->blue;

							line += 3;
							pix++;
						}
					}

					b.Swap(dib_);

OutputTimeStr(time, _T("reconstr"));

::QueryPerformanceCounter(&time);

					DestroyImage(sharp);
				}

				DestroyImage(img);
			}

OutputTimeStr(time, _T("destroy")); */
//OutputTimeStr(start, _T("total"));
//#endif

		}
	}
	catch (...)
	{
		ASSERT(false);
	}
//TRACE(L"DONE<< ThumbnailDecoder %x,%d (%s)\n", (int)thread_, unique_id_, photo_.GetName().c_str());

	if (::IsWindow(receiver_->m_hWnd))
		receiver_->PostMessage(WM_USER + 1000, unique_id_, status);

	return 0;
}


void ThumbnailDecoder::Impl::Quit()
{
	continue_decoding_ = false;
//	break_ = true;	// signal break
	if (thread_)// && pending_)
	{
		HANDLE thread= thread_;
		thread_ = 0;
		DWORD exit= ::WaitForSingleObject(thread, 1000);
		//if (exit == WAIT_OBJECT_0)
		//	return;	  // thread quit

		if (exit == WAIT_TIMEOUT)
		{
			// give it a boost, so it can finish
			::SetThreadPriority(thread, THREAD_PRIORITY_ABOVE_NORMAL);
			exit = ::WaitForSingleObject(thread, 5000);

			if (exit == WAIT_TIMEOUT)
			{
				ASSERT(false);
				::TerminateThread(thread, -1);
			}
		}

		//::CloseHandle(thread);
		win_thread_.reset(0);
	}

	thread_ = 0;
}


// interface implementation

void ThumbnailDecoder::Quit()
{
	impl_->Quit();
}


DibPtr ThumbnailDecoder::GetBitmap() const
{
	return impl_->dib_;
}


int ThumbnailDecoder::ReductionFactor() const
{
	return 1;
}


ConstPhotoInfoPtr ThumbnailDecoder::GetPhoto() const
{
	return &impl_->photo_;
}


CSize ThumbnailDecoder::GetOriginalSize() const
{
	return PhotoInfo::GetThumbnailSize();
}


CSize ThumbnailDecoder::GetRequestedSize() const
{
	return impl_->size_;
}


UINT_PTR ThumbnailDecoder::GetUniqueId() const
{
	return impl_->unique_id_;
}


bool ThumbnailDecoder::Impl::LinesDecoded(int linesReady, bool finished)
{
	return continue_decoding_;
}

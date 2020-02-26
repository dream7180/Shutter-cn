/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// TIFFDecoder.cpp: implementation of the TIFFDecoder class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "TIFFDecoder.h"
#include "ColorProfile.h"
#include "TIFFReader.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


struct TIFFDecoder::Impl
{
	Impl(const TCHAR* file_path) : file_path_(file_path)
	{}

	ImageStat DecodeImg(TIFFDecoder* self, Dib& bmp, CSize& img_size, bool resize, COLORREF rgb_back);

	String file_path_;
	TIFFReader tiff_;
};


TIFFDecoder::TIFFDecoder(const TCHAR* file_path) : impl_(new Impl(file_path))
{
}

TIFFDecoder::~TIFFDecoder()
{
}


ImageStat TIFFDecoder::DecodeImg(Dib& bmp, CSize& img_size, bool resize, COLORREF rgb_back/*= RGB(255,255,255)*/)
{
TRACE(L"tiff decoding: (%d,%d)\n", img_size.cx, img_size.cy);
	return impl_->DecodeImg(this, bmp, img_size, resize, rgb_back);
}


ImageStat TIFFDecoder::Impl::DecodeImg(TIFFDecoder* self, Dib& bmp, CSize& img_size, bool resize, COLORREF rgb_back)
{
	if (ImageStat stat= tiff_.Open(file_path_.c_str()))
		return stat;

	if (!tiff_.IsSupported())
		return IS_FMT_NOT_SUPPORTED;

	CSize size2= img_size;
	if (ImageStat stat= tiff_.PrepareReading(self->CalcReductionFactor(size2, false)))
		return stat;

	// image may be reduced in size
	CSize size= tiff_.GetResized();
	bmp.Create(size.cx, size.cy, tiff_.GetBitsPerPixel());

	self->SetTotalLines(tiff_.GetHeight());

	if (!self->LinesDecoded(0, false))
	{
		tiff_.Close();
		return IS_DECODING_CANCELLED;
	}

	while (tiff_.GetScanLine() < tiff_.GetHeight())
	{
		if (ImageStat stat= tiff_.ReadNextLine(bmp, 0))
		{
			tiff_.Close();
			return stat;
		}

		if (self->IsICCEnabled())
			self->ApplyICC(bmp.LineBuffer(tiff_.GetScanLine() - 1), bmp.GetWidth(), bmp.GetColorComponents());

		int scan_line= tiff_.GetScanLine();

		// report progress
		if (!self->LinesDecoded(scan_line, scan_line == tiff_.GetHeight()))
		{
			tiff_.Close();
			return IS_DECODING_CANCELLED;
		}
	}

	if (resize)
		bmp.ResizeToFit(img_size, Dib::RESIZE_CUBIC);

	return IS_OK;
}


CSize TIFFDecoder::GetOriginalSize() const
{
	return impl_->tiff_.GetSize();
}

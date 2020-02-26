/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// NEFDecoder.cpp: implementation of the CNEFDecoder class.

#include "stdafx.h"
#include "resource.h"
#include "NEFDecoder.h"
#include "ColorProfile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


CNEFDecoder::CNEFDecoder(const TCHAR* file_path) : file_path_(file_path)
{
}

CNEFDecoder::~CNEFDecoder()
{
}


ImageStat CNEFDecoder::DecodeImg(Dib& bmp, CSize& img_size, bool resize, COLORREF rgb_back/*= RGB(255,255,255)*/)
{
	if (ImageStat stat= reader_.Open(file_path_.c_str()))
		return stat;

	//TODO
//	if (!reader_.IsSupported())
//		return IS_FMT_NOT_SUPPORTED;

	CSize size2= img_size;
	if (ImageStat stat= reader_.PrepareReading(CalcReductionFactor(size2, false)))
		return stat;

	// image may be reduced in size
	CSize size= reader_.GetResized();
	bmp.Create(size.cx, size.cy, reader_.GetBitsPerPixel());

	SetTotalLines(reader_.GetHeight());

	if (!LinesDecoded(0, false))
	{
		reader_.Close();
		return IS_DECODING_CANCELLED;
	}

//	while (tiff_.GetScanLine() < tiff_.GetHeight())
	{
		if (ImageStat stat= reader_.ReadNextLine(bmp, 0))
		{
			reader_.Close();
			return stat;
		}

		if (IsICCEnabled())
			for (int y= 0; y < bmp.GetHeight(); ++y)
				ApplyICC(bmp.LineBuffer(y), bmp.GetWidth(), bmp.GetColorComponents());

//		int scan_line= reader_.GetScanLine();

		// report progress
		if (!LinesDecoded(reader_.GetHeight(), true))
//		if (!LinesDecoded(scan_line, scan_line == tiff_.GetHeight()))
		{
			reader_.Close();
			return IS_DECODING_CANCELLED;
		}
	}

	if (resize)
		bmp.ResizeToFit(img_size, Dib::RESIZE_CUBIC);

	return IS_OK;
}


CSize CNEFDecoder::GetOriginalSize() const
{
	return reader_.GetSize();
}


int CNEFDecoder::ReductionFactor() const
{
	return reader_.IsHalfSize() ? 2 : 1;
}

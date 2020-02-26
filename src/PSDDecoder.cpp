/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// PSDDecoder.cpp: implementation of the PSDDecoder class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "PSDDecoder.h"
#include "File.h"
#include "ColorProfile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

PSDDecoder::PSDDecoder(const TCHAR* file_path) : file_path_(file_path)
{
}

PSDDecoder::~PSDDecoder()
{
}


ImageStat PSDDecoder::DecodeImg(Dib& bmp, CSize& img_size, bool resize, COLORREF rgb_back/*= RGB(255,255,255)*/)
{
	FileStream ifs;
	if (!ifs.Open(file_path_.c_str()))
		return IS_OPEN_ERR;

	if (!psd_.OpenHeader(ifs))
		return IS_FMT_NOT_SUPPORTED;

	if (!psd_.IsSupported())
		return IS_FMT_NOT_SUPPORTED;

	if (!psd_.PrepareReading(ifs, CalcReductionFactor(img_size, false)))
		return IS_FMT_NOT_SUPPORTED;

	// image may be reduced in size
	CSize size= psd_.GetResized();
	bmp.Create(size.cx, size.cy, 24);

	SetTotalLines(psd_.GetHeight());

	if (!LinesDecoded(0, false))
		return IS_DECODING_CANCELLED;

	while (psd_.GetScanLine() < psd_.GetHeight())
	{
		psd_.ReadNextLine(ifs, bmp, 0);

		if (IsICCEnabled())
			ApplyICC(bmp.LineBuffer(psd_.GetScanLine() - 1), bmp.GetWidth(), bmp.GetColorComponents());

		int scan_line= psd_.GetScanLine();

		// report progress
		if (!LinesDecoded(scan_line, scan_line == psd_.GetHeight()))
			return IS_DECODING_CANCELLED;
	}

	if (resize)
		bmp.ResizeToFit(img_size, Dib::RESIZE_CUBIC);

	return IS_OK;
}


CSize PSDDecoder::GetOriginalSize() const
{
	return psd_.GetSize();
}

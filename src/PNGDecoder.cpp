/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// PNGDecoder.cpp: implementation of the PNGDecoder class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "PNGDecoder.h"
#include "ColorProfile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

PNGDecoder::PNGDecoder(const TCHAR* file_path) : file_path_(file_path)
{
}

PNGDecoder::~PNGDecoder()
{
}


ImageStat PNGDecoder::DecodeImg(Dib& bmp, CSize& img_size, bool resize, COLORREF rgb_back/*= RGB(255,255,255)*/)
{
	if (ImageStat stat= png_.Open(file_path_.c_str(), 0.0))
		return stat;

	if (!png_.IsSupported())
		return IS_FMT_NOT_SUPPORTED;

	CSize size2= img_size;
	if (ImageStat stat= png_.PrepareReading(CalcReductionFactor(size2, false)))
		return stat;

	// image may be reduced in size
	CSize size= png_.GetResized();
	int bits= png_.IsGrayscale() ? 8 : 3 * 8;
	bmp.Create(size.cx, size.cy, bits);

	SetTotalLines(png_.GetHeight());

	if (!LinesDecoded(0, false))
	{
		png_.Close();
		return IS_DECODING_CANCELLED;
	}

	while (png_.GetPass() < png_.GetNoOfPasses())
	{
		while (png_.GetScanLine() < png_.GetHeight())
		{
			png_.ReadNextLine(bmp);

			if (png_.GetPass() == png_.GetNoOfPasses() - 1)
			{
				int scan_line= png_.GetScanLine();

				// report progress
				if (!LinesDecoded(scan_line, scan_line == png_.GetHeight()))
				{
					png_.Close();
					return IS_DECODING_CANCELLED;
				}
			}
		}

		png_.NextPass();
	}

	if (resize)
		bmp.ResizeToFit(img_size, Dib::RESIZE_CUBIC);

	return IS_OK;
}


CSize PNGDecoder::GetOriginalSize() const
{
	return png_.GetSize();
}

/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// PNGDecoder.h: interface for the PNGDecoder class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PNGDECODER_H__FB772933_D607_4118_BF1C_58BD7E1C6074__INCLUDED_)
#define AFX_PNGDECODER_H__FB772933_D607_4118_BF1C_58BD7E1C6074__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ImageDecoder.h"
#include "PNGReader.h"


class PNGDecoder : public ImageDecoder
{
public:
	PNGDecoder(const TCHAR* file_path);
	virtual ~PNGDecoder();

	virtual ImageStat DecodeImg(Dib& bmp, CSize& img_size, bool resize, COLORREF rgb_back= RGB(255,255,255));
	virtual CSize GetOriginalSize() const;

private:
	String file_path_;
	PNGReader png_;
};

#endif // !defined(AFX_PNGDECODER_H__FB772933_D607_4118_BF1C_58BD7E1C6074__INCLUDED_)

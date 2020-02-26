/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// GIFDecoder.h: interface for the GIFDecoder class.
//

#pragma once
#include "ImageDecoder.h"


class GIFDecoder : public ImageDecoder
{
public:
	GIFDecoder(const TCHAR* filePath);
	virtual ~GIFDecoder();

	virtual ImageStat DecodeImg(Dib& bmp, CSize& img_size, bool resize, COLORREF rgb_back= RGB(255,255,255));

	virtual CSize GetOriginalSize() const;

private:
	String filePath_;
	CSize img_size_;
};

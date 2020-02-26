/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// BMPDecoder.h: interface for the CBMPDecoder class.
//

#pragma once
#include "ImageDecoder.h"


class BMPDecoder : public ImageDecoder
{
public:
	BMPDecoder(const TCHAR* filePath);
	virtual ~BMPDecoder();

	virtual ImageStat DecodeImg(Dib& bmp, CSize& img_size, bool resize, COLORREF rgb_back= RGB(255,255,255));

	virtual CSize GetOriginalSize() const;

private:
	String filePath_;
	CSize img_size_;
};

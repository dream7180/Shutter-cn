/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once

#include "ImageDecoder.h"
#include "NEFReader.h"


class CNEFDecoder : public ImageDecoder
{
public:
	CNEFDecoder(const TCHAR* file_path);
	virtual ~CNEFDecoder();

	virtual ImageStat DecodeImg(Dib& bmp, CSize& img_size, bool resize, COLORREF rgb_back= RGB(255,255,255));
	virtual CSize GetOriginalSize() const;
	virtual int ReductionFactor() const;

private:
	String file_path_;
	NEFReader reader_;
};

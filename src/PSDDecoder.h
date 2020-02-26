/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// PSDDecoder.h: interface for the PSDDecoder class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PSDDECODER_H__A6F30569_0B00_4D2C_A5A5_9D2CD2835B5E__INCLUDED_)
#define AFX_PSDDECODER_H__A6F30569_0B00_4D2C_A5A5_9D2CD2835B5E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ImageDecoder.h"
#include "PSDReader.h"


class PSDDecoder : public ImageDecoder
{
public:
	PSDDecoder(const TCHAR* file_path);
	virtual ~PSDDecoder();

	virtual ImageStat DecodeImg(Dib& bmp, CSize& img_size, bool resize, COLORREF rgb_back= RGB(255,255,255));
	virtual CSize GetOriginalSize() const;

private:
	String file_path_;
	PSDReader psd_;
};

#endif // !defined(AFX_PSDDECODER_H__A6F30569_0B00_4D2C_A5A5_9D2CD2835B5E__INCLUDED_)

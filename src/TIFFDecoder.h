/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// TIFFDecoder.h: interface for the TIFFDecoder class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TIFFDECODER_H__EDE5FADA_E0DC_413C_A61B_D2CDEF830C21__INCLUDED_)
#define AFX_TIFFDECODER_H__EDE5FADA_E0DC_413C_A61B_D2CDEF830C21__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ImageDecoder.h"


class TIFFDecoder : public ImageDecoder
{
public:
	TIFFDecoder(const TCHAR* file_path);
	virtual ~TIFFDecoder();

	virtual ImageStat DecodeImg(Dib& bmp, CSize& img_size, bool resize, COLORREF rgb_back= RGB(255,255,255));
	virtual CSize GetOriginalSize() const;

private:
	TIFFDecoder(const TIFFDecoder&);
	TIFFDecoder& operator = (const TIFFDecoder&);

	struct Impl;
	std::auto_ptr<Impl> impl_;
};

#endif // !defined(AFX_TIFFDECODER_H__EDE5FADA_E0DC_413C_A61B_D2CDEF830C21__INCLUDED_)

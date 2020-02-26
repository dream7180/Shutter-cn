/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// Jpeg.h: interface for the CJpeg class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_JPEG_H__70C111B8_B1CF_4FBB_B7B8_860534C65ED2__INCLUDED_)
#define AFX_JPEG_H__70C111B8_B1CF_4FBB_B7B8_860534C65ED2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Dib.h"
#include "ImageStat.h"


class CJpeg
{
public:
	CJpeg();
	~CJpeg();

	ImageStat GetDib(Dib& dib) const;

	bool IsEmpty() const;

	void Empty();

	void SwapBuffer(std::vector<uint8>& buffer);
	void SetBuffer(const std::vector<uint8>& buffer);

	const uint8* JpegData() const;
	size_t JpegDataSize() const;

private:
	std::vector<uint8> jpeg_data_;
//	mutable CCriticalSection access_ctrl_;
};

#endif // !defined(AFX_JPEG_H__70C111B8_B1CF_4FBB_B7B8_860534C65ED2__INCLUDED_)

/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// ResizingThread.h: interface for the ResizingThread class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Path.h"
#include "Dib.h"
#include "ImgProcessingThread.h"
#include "ImageDecoder.h"
#include "PhotoInfoPtr.h"
class CSlideShowGenerator;
class ImageDecoder;
class JPEGEncoder;


struct ResizeFormat
{
	ResizeFormat(CSize img_size, int quality, bool progressive, bool baseline, int format,
		CSize thumb_size, Dib::ResizeMethod method, bool preserve_exif, bool copyTags)
	: img_size_(img_size), quality_(quality), progressive_(progressive),
		baseline_(baseline), format_(format), thumb_size_(thumb_size), method_(method),
		preserve_exif_block_(preserve_exif), copyTags_(copyTags)
	{}

	CSize img_size_;
	int   quality_;
	bool  progressive_;
	bool  baseline_;
	int   format_;
	CSize thumb_size_;
	Dib::ResizeMethod method_;
	bool  preserve_exif_block_;
	bool  copyTags_;
};


struct ResizePhotoInfo
{
	Path  src_file_;

	CImageDecoderPtr decoder_;
	CSize  photo_size_;
	int    orientation_;
	Path  dest_file_;
	bool   fit_this_size_;
	CSize  thumbnail_size_;
	Path  dest_thumbnail_;
	String date_time_;
	std::wstring description_;
	PhotoInfoPtr photo_;

	ResizePhotoInfo()
	{
		photo_size_ = CSize(0, 0);
		orientation_ = 0;
		fit_this_size_ = true;
		thumbnail_size_ = CSize(0, 0);
		photo_ = 0;
	}

	~ResizePhotoInfo()
	{
	}

private:
//	ResizePhotoInfo& operator = (const ResizePhotoInfo& src)	{ return *this; }
};


class ResizingThread : public ImgProcessingThread
{
public:
	ResizingThread(std::vector<ResizePhotoInfo>& files, const ResizeFormat& fmt, CSlideShowGenerator* slide_show= 0);

	virtual ResizingThread* Clone();

private:
	// process one image
	virtual void Process(size_t index);

	virtual String GetSourceFileName(size_t index) const;
	virtual String GetDestFileName(size_t index) const;

	std::vector<ResizePhotoInfo>& files_;
//	CWnd* status_wnd_;
	const ResizeFormat& params_;
	CSlideShowGenerator* slide_show_;
//	CResizeImg proc_;	// decoding/encoding engine
	std::auto_ptr<JPEGEncoder> encoder_;
};

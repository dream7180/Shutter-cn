/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// ExtractingThread - worker thread for extracting JPEG images from raw photos
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
class PhotoInfo;
class JPEGEncoder;


struct ExtractFormat
{
//	ResizeFormat(CSize img_size, int quality, bool progressive, bool baseline, int format,
//		CSize thumb_size, Dib::ResizeMethod method, bool preserve_exif, bool copyTags)
//	: img_size_(img_size), quality_(quality), progressive_(progressive),
//		baseline_(baseline), format_(format), thumb_size_(thumb_size), method_(method),
//		preserve_exif_block_(preserve_exif), copyTags_(copyTags)
//	{}
//
//	CSize img_size_;
//	int   quality_;
//	bool  progressive_;
//	bool  baseline_;
//	int   format_;
//	CSize thumb_size_;
//	Dib::ResizeMethod method_;
	bool  preserve_exif_block_;
	bool  copy_tags_;
};


struct ExtractJpegInfo
{
	Path  src_file_path_;
//	CImageDecoderPtr decoder_;
//	CSize  photo_size_;
	int    orientation_;
	Path  dest_file_path_;
//	bool   fit_this_size_;
//	CSize  thumbnail_size_;
//	Path  dest_thumbnail_;
//	String date_time_;
	std::wstring description_;
	PhotoInfoPtr photo_;
	uint32 jpeg_data_offset_;
	uint32 jpeg_data_size_;

	ExtractJpegInfo()
	{
//		photo_size_ = CSize(0, 0);
		orientation_ = 0;
//		fit_this_size_ = true;
//		thumbnail_size_ = CSize(0, 0);
		photo_ = 0;
		jpeg_data_offset_ = 0;
		jpeg_data_size_ = 0;
	}

	~ExtractJpegInfo()
	{
	}

private:
//	ResizePhotoInfo& operator = (const ResizePhotoInfo& src)	{ return *this; }
};


class ExtractingThread : public ImgProcessingThread
{
public:
	ExtractingThread(std::vector<ExtractJpegInfo>& files, const ExtractFormat& fmt);

	virtual ImgProcessingThread* Clone();

private:
	// process one image
	virtual void Process(size_t index);

	virtual String GetSourceFileName(size_t index) const;
	virtual String GetDestFileName(size_t index) const;

//	vector<ResizePhotoInfo>& files_;
	std::vector<ExtractJpegInfo>& files_;
//	CWnd* status_wnd_;
	const ExtractFormat& params_;
};

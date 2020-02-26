/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// PhotoInfoDNG.h: interface for the PhotoInfoDNG class.
//
//////////////////////////////////////////////////////////////////////

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "PhotoInfo.h"


class PhotoInfoDNG : public PhotoInfo
{
public:
	PhotoInfoDNG();
	virtual ~PhotoInfoDNG();

	virtual bool Scan(const TCHAR* filename, ExifBlock& exifData, bool generateThumbnails, ImgLogger* logger) override;
	virtual bool CanEditIPTC(int& err_code) const;
	//virtual bool IsDescriptionEditable() const;
	virtual bool IsRotationFeasible() const;
	virtual CImageDecoderPtr GetDecoder() const;
	virtual ImageStat CreateThumbnail(Dib& bmp, DecoderProgress* progress) const;
	virtual bool ReadExifBlock(ExifBlock& exif) const;
	virtual void CompleteInfo(ImageDatabase& db, OutputStr& out) const;
	virtual bool GetEmbeddedJpegImage(uint32& jpeg_data_offset, uint32& jpeg_data_size) const;

private:
	OutputStr output_;
	uint32 jpeg_data_offset_;
	uint32 jpeg_data_size_;

	PhotoInfoDNG& operator = (const PhotoInfoDNG&);
	PhotoInfoDNG(const PhotoInfoDNG&);
};

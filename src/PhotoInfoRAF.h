/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// PhotoInfoRAF.h: Fuji raw
//
//////////////////////////////////////////////////////////////////////

#pragma once
#include "PhotoInfo_XMP.h"


class PhotoInfoRAF : public PhotoInfo_XMP
{
public:
	PhotoInfoRAF();
	virtual ~PhotoInfoRAF();

	virtual bool IsRotationFeasible() const;

	virtual bool Scan(const TCHAR* filename, ExifBlock& exifData, bool generateThumbnails, ImgLogger* logger) override;

	ImageStat CreateThumbnail(Dib& bmp, DecoderProgress* progress) const;

	// scan photo, store entire EXIF block in the exif_data
//	virtual bool Scan(const TCHAR* filename, ExifBlock& exifData);

	virtual bool ReadExifBlock(ExifBlock& exif) const;

	virtual void CompleteInfo(ImageDatabase& db, OutputStr& out) const;

private:
	PhotoInfoRAF& operator = (const PhotoInfoRAF&);
	PhotoInfoRAF(const PhotoInfoRAF&);

	OutputStr output_;
};

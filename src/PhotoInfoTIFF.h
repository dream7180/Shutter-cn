/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// PhotoInfoTIFF.h: interface for the PhotoInfoTIFF class.

#pragma once
#include "PhotoInfo.h"


class PhotoInfoTIFF : public PhotoInfo
{
public:
	PhotoInfoTIFF();
	virtual ~PhotoInfoTIFF();

	virtual bool Scan(const TCHAR* filename, ExifBlock& exifData, bool generateThumbnails, ImgLogger* logger) override;
	virtual bool CanEditIPTC(int& err_code) const;
	//virtual bool IsDescriptionEditable() const;
	virtual bool IsRotationFeasible() const;
	virtual CImageDecoderPtr GetDecoder() const;
	virtual ImageStat CreateThumbnail(Dib& bmp, DecoderProgress* progress) const;
	virtual bool ReadExifBlock(ExifBlock& exif) const;
	virtual void CompleteInfo(ImageDatabase& db, OutputStr& out) const;

private:
	OutputStr output_;

	PhotoInfoTIFF& operator = (const PhotoInfoTIFF&);
	PhotoInfoTIFF(const PhotoInfoTIFF&);
};

/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// PhotoInfoGIF.h: interface for the PhotoInfoGIF class.
//
//////////////////////////////////////////////////////////////////////

#pragma once
#include "PhotoInfo.h"


class PhotoInfoGIF : public PhotoInfo
{
public:
	PhotoInfoGIF();
	virtual ~PhotoInfoGIF();

	virtual bool Scan(const TCHAR* filename, ExifBlock& exifData, bool generateThumbnails, ImgLogger* logger) override;
	virtual bool CanEditIPTC(int& err_code) const;
	//virtual bool IsDescriptionEditable() const;
	virtual bool IsRotationFeasible() const;
	virtual CImageDecoderPtr GetDecoder() const;
	virtual ImageStat CreateThumbnail(Dib& bmp, DecoderProgress* progress) const;
	virtual bool ReadExifBlock(ExifBlock& exif) const;

private:
	PhotoInfoGIF& operator = (const PhotoInfoGIF&);
	PhotoInfoGIF(const PhotoInfoGIF&);
};

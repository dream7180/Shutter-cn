/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// PhotoInfoBMP.h: interface for the PhotoInfoBMP class.
//
//////////////////////////////////////////////////////////////////////

#pragma once
#include "PhotoInfo.h"


class PhotoInfoBMP : public PhotoInfo
{
public:
	PhotoInfoBMP();
	virtual ~PhotoInfoBMP();

	virtual bool Scan(const TCHAR* filename, ExifBlock& exifData, bool generateThumbnails, ImgLogger* logger) override;
	virtual bool CanEditIPTC(int& err_code) const;
	virtual bool IsRotationFeasible() const;
	virtual CImageDecoderPtr GetDecoder() const;
	virtual ImageStat CreateThumbnail(Dib& bmp, DecoderProgress* progress) const;
	virtual bool ReadExifBlock(ExifBlock& exif) const;

private:
	PhotoInfoBMP& operator = (const PhotoInfoBMP&);
	PhotoInfoBMP(const PhotoInfoBMP&);
};

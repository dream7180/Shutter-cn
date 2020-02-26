/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// PhotoInfoPNG.h: interface for the PhotoInfoPNG class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PHOTOINFOPNG_H__92E243AB_6298_42CA_8378_73EB8E7ECEB4__INCLUDED_)
#define AFX_PHOTOINFOPNG_H__92E243AB_6298_42CA_8378_73EB8E7ECEB4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "PhotoInfo.h"


class PhotoInfoPNG : public PhotoInfo
{
public:
	PhotoInfoPNG();
	virtual ~PhotoInfoPNG();

	virtual bool Scan(const TCHAR* filename, ExifBlock& exifData, bool generateThumbnails, ImgLogger* logger) override;
	virtual bool CanEditIPTC(int& err_code) const;
	//virtual bool IsDescriptionEditable() const;
	virtual bool IsRotationFeasible() const;
	virtual CImageDecoderPtr GetDecoder() const;
	virtual ImageStat CreateThumbnail(Dib& bmp, DecoderProgress* progress) const;
	virtual bool ReadExifBlock(ExifBlock& exif) const;

private:
	PhotoInfoPNG& operator = (const PhotoInfoPNG&);
	PhotoInfoPNG(const PhotoInfoPNG&);
};

#endif // !defined(AFX_PHOTOINFOPNG_H__92E243AB_6298_42CA_8378_73EB8E7ECEB4__INCLUDED_)

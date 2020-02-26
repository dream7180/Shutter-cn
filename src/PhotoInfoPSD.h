/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// PhotoInfoPSD.h: interface for the PhotoInfoPSD class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PHOTOINFOPSD_H__575724EB_AC4F_49AF_8B42_6F3CE61D0840__INCLUDED_)
#define AFX_PHOTOINFOPSD_H__575724EB_AC4F_49AF_8B42_6F3CE61D0840__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "PhotoInfo.h"


class PhotoInfoPSD : public PhotoInfo
{
public:
	PhotoInfoPSD();
	virtual ~PhotoInfoPSD();

	virtual bool Scan(const TCHAR* filename, ExifBlock& exifData, bool generateThumbnails, ImgLogger* logger) override;
	virtual bool CanEditIPTC(int& err_code) const;
	//virtual bool IsDescriptionEditable() const;
	virtual bool IsRotationFeasible() const;
	virtual CImageDecoderPtr GetDecoder() const;
	virtual ImageStat CreateThumbnail(Dib& bmp, DecoderProgress* progress) const;
	virtual bool ReadExifBlock(ExifBlock& exif) const;

private:
	PhotoInfoPSD& operator = (const PhotoInfoPSD&);
	PhotoInfoPSD(const PhotoInfoPSD&);
};

#endif // !defined(AFX_PHOTOINFOPSD_H__575724EB_AC4F_49AF_8B42_6F3CE61D0840__INCLUDED_)

/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// PhotoInfoCRW.h: interface for the PhotoInfoCRW class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PHOTOINFOCRW_H__4467BCE2_F083_47BE_B521_4E4F2CA51F35__INCLUDED_)
#define AFX_PHOTOINFOCRW_H__4467BCE2_F083_47BE_B521_4E4F2CA51F35__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "PhotoInfo_XMP.h"


class PhotoInfoCRW : public PhotoInfo_XMP
{
public:
	PhotoInfoCRW();
	virtual ~PhotoInfoCRW();

	virtual bool IsRotationFeasible() const;

	// scan photo, store entire EXIF block in the exif_data
	virtual bool Scan(const TCHAR* filename, ExifBlock& exifData, bool generateThumbnails, ImgLogger* logger) override;

	virtual bool ReadExifBlock(ExifBlock& exif) const;

	virtual Path GetAssociatedFilePath(size_t index) const;

private:
	PhotoInfoCRW& operator = (const PhotoInfoCRW&);
	PhotoInfoCRW(const PhotoInfoCRW&);

	OutputStr output_;

	bool GetEmbeddedJpegImage(uint32& jpeg_data_offset, uint32& jpeg_data_size) const;
};

#endif // !defined(AFX_PHOTOINFOCRW_H__4467BCE2_F083_47BE_B521_4E4F2CA51F35__INCLUDED_)

/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// PhotoInfoBMP.cpp: implementation of the PhotoInfoBMP class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "PhotoInfoBMP.h"
#include "BMPDecoder.h"
#include "BMPReader.h"
#include "PhotoFactory.h"
#include "FileTypeIndex.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


namespace {
	RegisterPhotoType<PhotoInfoBMP> img(_T("bmp"), FT_BMP);
}


PhotoInfoBMP::PhotoInfoBMP()
{
	file_type_index_ = FT_BMP;
}

PhotoInfoBMP::~PhotoInfoBMP()
{
}


//int PhotoInfoBMP::GetTypeMarkerIndex() const
//{
//	return 11;	// in indicators.png
//}


CImageDecoderPtr PhotoInfoBMP::GetDecoder() const
{
	return new BMPDecoder(path_.c_str());
}


bool PhotoInfoBMP::CanEditIPTC(int& err_code) const
{
	// TODO
	err_code = 1;
	return false;
}


bool PhotoInfoBMP::IsRotationFeasible() const
{
	return false;
}


bool PhotoInfoBMP::Scan(const TCHAR* filename, ExifBlock& exifData, bool generateThumbnails, ImgLogger* logger)
{
	// extract size info at the very least
	if (GetWidth() == 0 && GetHeight() == 0)
	{
		BMPReader bmp;
		ImageStat stat= bmp.Open(filename);

		if (stat == IS_OK)
			SetSize(bmp.GetSize().cx, bmp.GetSize().cy);
		else
			throw stat;
	}

	return false;	// no EXIF in PNG files
}


ImageStat PhotoInfoBMP::CreateThumbnail(Dib& bmp, DecoderProgress* progress) const
{
	try
	{
		BMPDecoder decoder(path_.c_str());
		return decoder.DecodeImg(bmp, GetThumbnailSize(), true);
	}
	catch (...)	// for now (TODO--failure codes)
	{
	}
	return IS_READ_ERROR;
}


bool PhotoInfoBMP::ReadExifBlock(ExifBlock& exif) const
{
	return false;
}

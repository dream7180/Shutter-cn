/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// PhotoInfoPNG.cpp: implementation of the PhotoInfoPNG class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "PhotoInfoPNG.h"
#include "PNGDecoder.h"
#include "PhotoFactory.h"
#include "FileTypeIndex.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

namespace {
	RegisterPhotoType<PhotoInfoPNG> img(_T("png"), FT_PNG);
}


PhotoInfoPNG::PhotoInfoPNG()
{
	file_type_index_ = FT_PNG;
}

PhotoInfoPNG::~PhotoInfoPNG()
{
}


//int PhotoInfoPNG::GetTypeMarkerIndex() const
//{
//	return 4;	// in indicators.png
//}


CImageDecoderPtr PhotoInfoPNG::GetDecoder() const
{
	return new PNGDecoder(path_.c_str());
}


bool PhotoInfoPNG::CanEditIPTC(int& err_code) const
{
	// TODO
	err_code = 1;
	return false;
}


//bool PhotoInfoPNG::IsDescriptionEditable() const
//{
//	// TODO?
//	AfxMessageBox(_T("PNG file: description is not supported."), MB_OK);
//
//	return false;
//}


bool PhotoInfoPNG::IsRotationFeasible() const
{
	return false;
}


bool PhotoInfoPNG::Scan(const TCHAR* filename, ExifBlock& exifData, bool generateThumbnails, ImgLogger* logger)
{
	// extract size info at the very least
	if (GetWidth() == 0 && GetHeight() == 0)
	{
		PNGReader png;
		if (png.Open(filename, 1.0) == IS_OK)
		{
			SetSize(png.GetSize().cx, png.GetSize().cy);
		}
	}

	return false;	// no EXIF in PNG files
}


ImageStat PhotoInfoPNG::CreateThumbnail(Dib& bmp, DecoderProgress* progress) const
{
	try
	{
		PNGDecoder decoder(path_.c_str());
		decoder.SetProgressClient(progress);
		return decoder.DecodeImg(bmp, GetThumbnailSize(), true);
	}
	catch (...)	// for now (TODO--failure codes)
	{
	}
	return IS_READ_ERROR;
}


bool PhotoInfoPNG::ReadExifBlock(ExifBlock& exif) const
{
	return false;
}

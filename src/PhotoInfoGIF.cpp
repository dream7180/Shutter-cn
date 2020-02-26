/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// PhotoInfoGIF.cpp: implementation of the PhotoInfoGIF class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "PhotoInfoGIF.h"
#include "GIFDecoder.h"
#include "GIFReader.h"
#include "PhotoFactory.h"
#include "FileTypeIndex.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


namespace {
	RegisterPhotoType<PhotoInfoGIF> img(_T("gif"), FT_GIF);
}


PhotoInfoGIF::PhotoInfoGIF()
{
	file_type_index_ = FT_GIF;
}

PhotoInfoGIF::~PhotoInfoGIF()
{
}


//int PhotoInfoGIF::GetTypeMarkerIndex() const
//{
//	return 10;	// in indicators.png
//}


CImageDecoderPtr PhotoInfoGIF::GetDecoder() const
{
	return new GIFDecoder(path_.c_str());
}


bool PhotoInfoGIF::CanEditIPTC(int& err_code) const
{
	// TODO
	err_code = 1;
	return false;
}


//bool PhotoInfoGIF::IsDescriptionEditable() const
//{
//	// TODO?
//	AfxMessageBox(_T("GIF file: description is not supported."), MB_OK);
//
//	return false;
//}


bool PhotoInfoGIF::IsRotationFeasible() const
{
	return false;
}


bool PhotoInfoGIF::Scan(const TCHAR* filename, ExifBlock& exifData, bool generateThumbnails, ImgLogger* logger)
{
	// extract size info at the very least
	if (GetWidth() == 0 && GetHeight() == 0)
	{
		GIFReader gif;
		if (gif.Open(filename) == IS_OK)
		{
			SetSize(gif.GetSize().cx, gif.GetSize().cy);
		}
	}

	return false;	// no EXIF in PNG files
}


ImageStat PhotoInfoGIF::CreateThumbnail(Dib& bmp, DecoderProgress* progress) const
{
	try
	{
		GIFDecoder decoder(path_.c_str());
		decoder.SetProgressClient(progress);
		return decoder.DecodeImg(bmp, GetThumbnailSize(), true);
	}
	catch (...)	// for now (TODO--failure codes)
	{
	}
	return IS_READ_ERROR;
}


bool PhotoInfoGIF::ReadExifBlock(ExifBlock& exif) const
{
	return false;
}

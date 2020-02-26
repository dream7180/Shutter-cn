/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// PhotoInfoPSD.cpp: implementation of the PhotoInfoPSD class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "PhotoInfoPSD.h"
#include "PSDDecoder.h"
#include "ExifBlock.h"
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
	RegisterPhotoType<PhotoInfoPSD> img(_T("psd"), FT_PSD);
}


PhotoInfoPSD::PhotoInfoPSD()
{
	file_type_index_ = FT_PSD;
}

PhotoInfoPSD::~PhotoInfoPSD()
{}


//int PhotoInfoPSD::GetTypeMarkerIndex() const
//{
//	return 5;	// in indicators.png
//}


CImageDecoderPtr PhotoInfoPSD::GetDecoder() const
{
	return new PSDDecoder(path_.c_str());
}


bool PhotoInfoPSD::CanEditIPTC(int& err_code) const
{
	// TODO
	err_code = 1;
	return false;
}


//bool PhotoInfoPSD::IsDescriptionEditable() const
//{
//	// TODO?
//	AfxMessageBox(_T("PSD file: description is not supported."), MB_OK);
//
//	return false;
//}


bool PhotoInfoPSD::IsRotationFeasible() const
{
	return false;
}


bool PhotoInfoPSD::Scan(const TCHAR* filename, ExifBlock& exifData, bool generateThumbnails, ImgLogger* logger)
{
	exifData.clear();

	// scan PSD (TODO)

	// extract size info at the very least
	if (GetWidth() == 0 && GetHeight() == 0)
	{
		try
		{
			PSDReader psd;
			FileStream ifs;
			if (ifs.Open(filename))
			{
				if (psd.OpenHeader(ifs))
					SetSize(psd.GetSize().cx, psd.GetSize().cy);
			}
		}
		catch(...)
		{}
	}

	return false;	// no EXIF (decoded)
}


ImageStat PhotoInfoPSD::CreateThumbnail(Dib& bmp, DecoderProgress* progress) const
{
	try
	{
		PSDDecoder decoder(path_.c_str());
		decoder.SetProgressClient(progress);
		return decoder.DecodeImg(bmp, GetThumbnailSize(), true);
	}
	catch (...)	// for now (TODO--failure codes)
	{
	}
	return IS_READ_ERROR;
}


bool PhotoInfoPSD::ReadExifBlock(ExifBlock& exif) const
{
	return false;
}

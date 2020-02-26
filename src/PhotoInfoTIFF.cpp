/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// PhotoInfoTIFF.cpp: implementation of the PhotoInfoTIFF class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "PhotoInfoTIFF.h"
#include "TIFFDecoder.h"
#include "TIFFReader.h"
#include "ExifBlock.h"
#include "File.h"
#include "PhotoFactory.h"
#include "FileTypeIndex.h"
#include "scan.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


namespace {
	RegisterPhotoType<PhotoInfoTIFF> tiff(_T("tif;tiff"), FT_TIFF);
}

//////////////////////////////////////////////////////////////////////


PhotoInfoTIFF::PhotoInfoTIFF()
{
	file_type_index_ = FT_TIFF;
}

PhotoInfoTIFF::~PhotoInfoTIFF()
{
}


//int PhotoInfoTIFF::GetTypeMarkerIndex() const
//{
//	return 2;	// in indicators.png
//}


CImageDecoderPtr PhotoInfoTIFF::GetDecoder() const
{
	return new TIFFDecoder(path_.c_str());
}


bool PhotoInfoTIFF::CanEditIPTC(int& err_code) const
{
	// TODO
	err_code = 1;
	return false;
}


//bool PhotoInfoTIFF::IsDescriptionEditable() const
//{
//	// TODO?
//	AfxMessageBox(_T("TIFF file: description is not supported."), MB_OK);
//
//	return false;
//}


bool PhotoInfoTIFF::IsRotationFeasible() const
{
	return false;
}


bool PhotoInfoTIFF::Scan(const TCHAR* filename, ExifBlock& exifData, bool generateThumbnails, ImgLogger* logger)
{
	exifData.clear();

	// scan TIFF

	bool hasExif= false;

	// extract size info at the very least
	if (GetWidth() == 0 && GetHeight() == 0)
	{
//		try
		{
			uint32 exif_offset= 0;
			bool intel_order= false;
			{
				TIFFReader tiff;
				if (tiff.Open(filename) == IS_OK)
				{
					SetSize(tiff.GetSize().cx, tiff.GetSize().cy);
					exif_offset = tiff.GetExifOffset();
					intel_order = tiff.IsIntelOrder();
				}
			}

			if (exif_offset > 0)
			{
				FileStream ifs;
				if (ifs.Open(filename))
				{
					ifs.SetByteOrder(!intel_order);
					ifs.RPosition(exif_offset);

					String make, model;

					try
					{
						hasExif = ::ScanExif(filename, ifs, 0, std::make_pair(exif_offset, ifs.GetLength32()), make, model, this, &exifData, &output_, false);
					}
					catch (MemPointer::MemPtrException&)
					{
						// ignore EXIF block in TIFF
						hasExif = false;
					}

					if (hasExif)
						SetExifInfo(exif_offset, ifs.GetLength32(), 0, ifs.GetByteOrder());
				}
			}
		}
		//catch(...)
		//{}
	}

	return hasExif;
}


ImageStat PhotoInfoTIFF::CreateThumbnail(Dib& bmp, DecoderProgress* progress) const
{
	try
	{
		TIFFDecoder decoder(path_.c_str());
		decoder.SetProgressClient(progress);
		return decoder.DecodeImg(bmp, GetThumbnailSize(), true);
	}
	catch (...)	// for now (TODO--failure codes)
	{
	}
	return IS_READ_ERROR;
}


bool PhotoInfoTIFF::ReadExifBlock(ExifBlock& exif) const
{
	return false;
}


void PhotoInfoTIFF::CompleteInfo(ImageDatabase& db, OutputStr& out) const
{
	out = output_;
}

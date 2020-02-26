/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// PhotoInfoRAF.cpp: implementation of the Fuji RAF reader.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PhotoInfoRAF.h"
#include "scan.h"
#include "ExifBlock.h"
#include "FileTypeIndex.h"
#include "PhotoFactory.h"
#include "File.h"
#include "JPEGDecoder.h"
#include "FileDataSource.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


namespace {
	RegisterPhotoType<PhotoInfoRAF> raf(_T("raf"), FT_RAF);
}

PhotoInfoRAF::PhotoInfoRAF()
{
	file_type_index_ = FT_RAF;
}

PhotoInfoRAF::~PhotoInfoRAF()
{}


//int PhotoInfoRAF::GetTypeMarkerIndex() const
//{
//	return 9;	// in indicators.png
//}


bool PhotoInfoRAF::Scan(const TCHAR* filename, ExifBlock& exifData, bool generateThumbnails, ImgLogger* logger)
{
	exifData.clear();
	// scan RAF

	bool hasExif= false;
	bool valid= false;

	// when database starts caching non-JPEG photos this condition will be meaningful...
	if (GetWidth() == 0 && GetHeight() == 0)
	{
		FileStream ifs;
		if (ifs.Open(filename) && ifs.GetLength32() > 0x1000)
		{
			ifs.SetByteOrder(true);	// big endian

			char buf[16];
			ifs.Read(buf);
			buf[15] = 0;
			if (strcmp(buf, "FUJIFILMCCD-RAW") == 0)
			{
				ifs.RPosFromBeg(0x54);

				// offset to the JPEG preview
				uint32 preview= ifs.GetUInt32();

				if (preview < ifs.GetLength32())
				{
					//ifs.RPosition(preview);

					jpeg_offset_ = preview;

					ifs.RPosFromBeg(preview);

					// if generateThumbnails is false use embedded thumbnail
					bool decodeEmbeddedThumbnail= !generateThumbnails;

					hasExif = ::Scan(filename, ifs, this, &output_, &exifData, decodeEmbeddedThumbnail, logger);

					if (generateThumbnails)
						jpeg_thumb_.Empty();

					valid = true;
				}
			}
		}

		if (!valid)
			throw std::exception("Unsupported RAF file");
	}

	return hasExif;
}


ImageStat PhotoInfoRAF::CreateThumbnail(Dib& bmp, DecoderProgress* progress) const
{
	try
	{
		if (!jpeg_thumb_.IsEmpty())
			return jpeg_thumb_.GetDib(bmp);

		AutoPtr<JPEGDataSource> file= new CFileDataSource(path_.c_str(), jpeg_offset_);
		JPEGDecoder decoder(file, JDEC_INTEGER_LOQ);
		decoder.SetFast(true, true);
		decoder.SetProgressClient(progress);
		return decoder.DecodeImg(bmp, GetThumbnailSize(), true);
	}
	catch (...)	// for now (TODO--failure codes)
	{
		ASSERT(false);
	}
	return IS_READ_ERROR;
}


bool PhotoInfoRAF::IsRotationFeasible() const
{
	return false;
}


bool PhotoInfoRAF::ReadExifBlock(ExifBlock& exif) const
{
	if (!exif_data_present_)
		return false;

	try
	{
		FileStream ifs;
		if (ifs.Open(path_.c_str()))
			return ::Scan(path_.c_str(), ifs, 0, 0, &exif, false, nullptr);
	}
	catch(...)
	{
		ASSERT(false);
	}

	return false;
}


void PhotoInfoRAF::CompleteInfo(ImageDatabase& db, OutputStr& out) const
{
	out = output_;
}

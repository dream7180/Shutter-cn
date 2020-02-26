/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "PhotoInfoX3F.h"
#include "file.h"
#include "ExifBlock.h"
#include "data.h"
#include "JPEGDecoder.h"
#include "FileDataSource.h"
#include "Config.h"
#include "ExifTags.h"
#include "scan.h"
#include "PhotoFactory.h"
#include "Exception.h"

extern bool StripBlackFrame(Dib& dib, bool yuv);

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

namespace {
	RegisterPhotoType<PhotoInfoX3F> x3f(_T("x3f"), FT_X3F);
}


PhotoInfoX3F::PhotoInfoX3F()
{
	file_type_index_ = FT_X3F;
	jpeg_thm_data_offset_ = jpeg_data_offset_ = jpeg_data_size_ = 0;
}

PhotoInfoX3F::~PhotoInfoX3F()
{}


//int PhotoInfoX3F::GetTypeMarkerIndex() const
//{
//	return FT_X3F + 1;	// in indicators.png
//}


//void PhotoInfoX3F::ParseMakerNote(FileStream& ifs)
//{
//	//char buf[10];
//	//ifs.Read(buf, 10);
//	uint32 base= 0;
//
////	if (memcmp(buf, "Nikon", 6) == 0)
////	{
////		base = ifs.RPosition();
////
////		uint16 order= ifs.GetUInt16();
////		if (order != 'MM' && order != 'II')
////			return;
////
//////		ifs.SetByteOrder(start[10] == 'M');
////
////		if (ifs.GetUInt16() != 0x2a)	// has to be 0x2a (this is IFD header)
////			return;
////
////		ifs.RPosition(ifs.GetUInt32() - 8);
////	}
////	else
////		ifs.RPosition(-10);
//
//	uint16 entries= ifs.GetUInt16();
//
//	if (entries > 0x200)
//	{
//		ASSERT(false);
//		return;
//	}
//
//	for (int i= 0; i < entries; ++i)
//	{
//		uint16 tag= ifs.GetUInt16();
//		Data data(ifs, base);
//		Offset temp= ifs.RPosition();
//
//TRACE(_T("mkn: %x (= %d)\n"), int(tag), int(data.GetData()));
//
//		switch (tag)
//		{
//		//case 0x11:	// thumbnail's ifd
//		//	{
//		//		ifs.RPosition(base + data.AsULong(), FileStream::beg);
//		//		uint16 ifd_entries= ifs.GetUInt16();
//
//		//		if (ifd_entries > 0x200)
//		//		{
//		//			ASSERT(false);
//		//			break;
//		//		}
//
//		//		for (int i= 0; i < ifd_entries; ++i)
//		//		{
//		//			uint16 tag= ifs.GetUInt16();
//		//			Data data(ifs, 0);
//
//		//			if (tag == 0x201)	// JPEGInterchangeFormat
//		//			{
//		//				jpeg_thm_data_offset_ = base + data.AsULong();
//		//				break;
//		//			}
//		//		}
//		//	}
//		//	break;
//
//		default:
//			break;
//		}
//
//		ifs.RPosition(temp, FileStream::beg);
//	}
//}


//void PhotoInfoX3F::ParseExif(FileStream& ifs, uint32 base)
//{
//	uint16 entries= ifs.GetUInt16();
//
//	if (entries > 0x200)
//	{
//		ASSERT(false);
//		return;
//	}
//
//	for (int i= 0; i < entries; ++i)
//	{
//		uint16 tag= ifs.GetUInt16();
//		Data data(ifs, 0);
//		Offset temp= ifs.RPosition();
//
////TRACE(_T("exf: %x (= %d)\n"), int(tag), int(data.GetData()));
//
//		switch (tag)
//		{
//		case 0x927c:
//			ifs.RPosition(data.GetData(), FileStream::beg);
//			ParseMakerNote(ifs);
//			break;
//
//		default:
//			break;
//		}
//
//		ifs.RPosition(temp, FileStream::beg);
//	}
//}

/*
uint32 PhotoInfoX3F::ParseIFD(FileStream& ifs, uint32 base, bool main_image)
{
	uint16 entries= ifs.GetUInt16();

	if (entries > 0x200)
	{
		ASSERT(false);
		return 0;
	}

	uint32 exif_offset= 0;

	for (int i= 0; i < entries; ++i)
	{
		Offset tag_start= ifs.RPosition();

		uint16 tag= ifs.GetUInt16();
		Data data(ifs, 0);

		Offset temp= ifs.RPosition();

		// TODO: read time and orientation
		//...

TRACE(_T("rw2: %x (= %d)\n"), int(tag), int(data.GetData()));

//		if (main_image)
//			output_.RecordInfo(tag, TagName(tag), data);

		switch (tag)
		{
//		case 0xfe:	// sub file?
//			main_image = (data.AsULong() & 1) == 0;	// main image?
//			break;

		case 0x2:		// img width
//			if (main_image)
				SetWidth(data.AsULong());
			break;

		case 0x3:		// img height
//			if (main_image)
				SetHeight(data.AsULong());
			break;

		case TIFF_ORIENTATION:		// orientation
//			if (main_image)
				SetOrientation(static_cast<uint16>(data.AsULong()));
			break;

		case 0x2e:		// embedded JPEG
			jpeg_data_offset_ = data.GetData();
			jpeg_data_size_ = data.Components();
			break;

		case 0x8769:	// EXIF tag
			exif_offset = data.AsULong();
//			ifs.RPosition(exif_offset, FileStream::beg);
//			ParseExif(ifs, exif_offset);
			break;

		case 0x8825:	// GPSInfo
			SetGpsData(ReadGPSInfo(ifs, 0, data, &output_));
			break;
		}
/ *
		if (main_image)
		{
			ifs.RPosition(tag_start, FileStream::beg);
//			ReadEntry(ifs, base, make_, model_, this, output_);
			//output_.RecordInfo(tag, TagName(tag), data);
		}
* /
		ifs.RPosition(temp, FileStream::beg);
	}

	return exif_offset;
}*/


bool PhotoInfoX3F::Scan(const TCHAR* file_name, ExifBlock& exifData, bool generateThumbnails, ImgLogger* logger)
{
	exifData.clear();

	bool exif_present= false;

	// scan Sigma raw

	FileStream ifs;

	if (!ifs.Open(file_name))
		return exif_present;

	if (ifs.GetLength32() < 200)	// too short?
		return exif_present;

	ifs.SetByteOrder(false);		// little endian

	if (ifs.GetBigEndianUInt32() != 'FOVb')	// x3f magic number (Foveon begin?)
		return exif_present;

	ifs.RPosFromBeg(0x24);			// orientation
	uint16 orientation_angle= ifs.GetUInt16();

	ifs.RPosition(4, FileStream::end);
	uint32 dir_offset= ifs.GetUInt32();	// directory offset
	ifs.RPosFromBeg(dir_offset);

	if (ifs.GetBigEndianUInt32() != 'SECd')	// directory section
		return exif_present;

	ifs.RPosition(4, FileStream::cur);
	uint32 entries= ifs.GetUInt32();

	for (int i= 0; i < std::min<int>(99, entries); ++i)		// limit no of entries
	{
		uint32 offset= ifs.GetUInt32();		// data offset
		uint32 length= ifs.GetUInt32();		// length of data
		uint32 tag= ifs.GetBigEndianUInt32();

		Offset pos= ifs.RPosition();

		if (!ifs.IsOffsetValid(offset))
			break;	// bogus entry

		ifs.RPosFromBeg(offset);
		uint32 section= ifs.GetBigEndianUInt32();
		if (section != ('SEC ' | (tag >> 24)))
			break;

		switch (tag)
		{
		case 'IMAG':
		case 'IMA2':
			{
				ifs.RPosition(12, FileStream::cur);
				uint32 width= ifs.GetUInt32();
				uint32 height= ifs.GetUInt32();
				ifs.RPosition(4, FileStream::cur);
				Offset preview= ifs.RPosition();
				if (ifs.GetUInt8() == 0xff && ifs.GetUInt8() == 0xd8)	// start of image?
				{
					uint32 size= length - (preview - offset);
					if (jpeg_data_size_ < size)
					{
						jpeg_data_offset_ = preview;
						jpeg_data_size_ = size;
						SetWidth(width);
						SetHeight(height);

						if (ifs.GetUInt8() == 0xff && ifs.GetUInt8() == 0xe1)	// marker 1?
						{
							ifs.RPosition(2, FileStream::cur);
							uint32 exif= ifs.GetBigEndianUInt32();
							if (exif == 'Exif')
							{
								ifs.RPosFromBeg(preview);
								exif_present = ::Scan(file_name, ifs, this, &output_, &exifData, true, logger, nullptr);
							}
						}

					}
					else
						jpeg_thm_data_offset_ = preview;
				}

				if (GetWidth() == 0 && GetHeight() == 0)
				{
					SetWidth(width);
					SetHeight(height);
				}
			}
			break;

		default:
			break;
		}

		ifs.RPosFromBeg(pos);
	}

	if (jpeg_data_offset_ == 0 && jpeg_thm_data_offset_ == 0)
		THROW_EXCEPTION(L"Unsupported X3F file format", L"This file is not supported. Cannot find embedded preview image.");

	switch ((orientation_angle + 3600) % 360)
	{
	case 0:		SetOrientation(EXIF_ORIENTATION_NORMAL); break;
	case 90:	SetOrientation(EXIF_ORIENTATION_90CCW); break;
	case 180:	SetOrientation(EXIF_ORIENTATION_UPSDN); break;
	case 270:	SetOrientation(EXIF_ORIENTATION_90CW); break;
	}

	return exif_present;
}


bool PhotoInfoX3F::IsRotationFeasible() const
{
	return false;
}


bool PhotoInfoX3F::ReadExifBlock(ExifBlock& exif) const
{
	return false;


//	if (!exif_data_present_)
		return false;
}


ImageStat PhotoInfoX3F::CreateThumbnail(Dib& bmp, DecoderProgress* progress) const
{
	try
	{
		uint32 offset= jpeg_thm_data_offset_ ? jpeg_thm_data_offset_ : jpeg_data_offset_;

		CFileDataSource fsrc(path_.c_str(), offset);
		JPEGDecoder dec(fsrc, JDEC_INTEGER_HIQ);
		dec.SetFast(true, true);
		dec.SetProgressClient(progress);
		ImageStat stat= dec.DecodeImg(bmp, GetThumbnailSize(), true);

		if (stat == IS_OK && bmp.IsValid() && jpeg_thm_data_offset_)
			StripBlackFrame(bmp, false);

		return stat;
	}
	catch (...)	// for now (TODO--failure codes)
	{
	}
	return IS_READ_ERROR;
}


CImageDecoderPtr PhotoInfoX3F::GetDecoder() const
{
	AutoPtr<JPEGDataSource> file= new CFileDataSource(path_.c_str(), jpeg_data_offset_);
	JpegDecoderMethod dct_method= g_Settings.dct_method_; // lousy
	return new JPEGDecoder(file, dct_method);
}


void PhotoInfoX3F::CompleteInfo(ImageDatabase& db, OutputStr& out) const
{
	out = output_;
}


bool PhotoInfoX3F::GetEmbeddedJpegImage(uint32& jpeg_data_offset, uint32& jpeg_data_size) const
{
	if (jpeg_data_offset_ && jpeg_data_size_)
	{
		jpeg_data_offset = jpeg_data_offset_;
		jpeg_data_size = jpeg_data_size_;
		return true;
	}
	else
		return false;
}

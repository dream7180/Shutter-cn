/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "PhotoInfoCR2.h"
#include "file.h"
#include "ExifBlock.h"
#include "data.h"
#include "JPEGDecoder.h"
#include "FileDataSource.h"
#include "Config.h"
#include "ExifTags.h"
#include "scan.h"
#include "PhotoFactory.h"

extern bool StripBlackFrame(Dib& dib, bool YUV);

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

namespace {
	RegisterPhotoType<PhotoInfoCR2> cr2(_T("cr2"), FT_CRW);
}


PhotoInfoCR2::PhotoInfoCR2()
{
	file_type_index_ = FT_CRW;
	jpeg_thm_data_offset_ = jpeg_data_offset_ = jpeg_data_size_ = 0;
}

PhotoInfoCR2::~PhotoInfoCR2()
{}


//int PhotoInfoCR2::GetTypeMarkerIndex() const
//{
//	return 3;	// in indicators.png
//}


//void PhotoInfoCR2::ParseMakerNote(FileStream& ifs)
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


//void PhotoInfoCR2::ParseExif(FileStream& ifs, uint32 base)
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


uint32 PhotoInfoCR2::ParseIFD(FileStream& ifs, uint32 base, bool main_image)
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

TRACE(_T("cr2: %x (= %d)\n"), int(tag), int(data.GetData()));

//		if (main_image)
//			output_.RecordInfo(tag, TagName(tag), data);

		switch (tag)
		{
//		case 0xfe:	// sub file?
//			main_image = (data.AsULong() & 1) == 0;	// main image?
//			break;

		case 0x100:		// img width
			if (main_image)
				SetWidth(data.AsULong());
			break;

		case 0x101:		// img height
			if (main_image)
				SetHeight(data.AsULong());
			break;

		case 0x112:		// orientation
			if (main_image)
				SetOrientation(static_cast<uint16>(data.AsULong()));
			break;

		case 0x117:		// size of embedded JPEG image
			if (main_image)
				jpeg_data_size_ = data.AsULong();
			break;

		case 0x10f:		// make
//			make_ = data.AsString();
			break;

		case 0x110:		// model
//			model_ = data.AsString();
			break;

		case 0x111:		// StripOffsets
			if (main_image)
				jpeg_data_offset_ = data.AsULong();
			break;

		//case 0x14a:		// SubIFD
		//	{
		//		uint32 len= data.Components();
		//		uint32 offset[8];
		//		data.ReadLongs(offset, min<uint32>(len, array_count(offset)));
		//		for (uint32 i= 0; i < len; ++i)
		//		{
		//			ifs.RPosition(offset[i], FileStream::beg);
		//			ParseIFD(ifs, ifs.RPosition());
		//		}
		//	}
		//	break;

		case 0x201:		// JPEGInterchangeFormat
			if (!main_image)
				jpeg_thm_data_offset_ = data.AsULong();
			break;

		case 0x202:		// JPEGInterchangeFormatLength
//			if (!main_image)
//				jpeg_data_size_ = data.AsULong();
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

		if (main_image)
		{
			ifs.RPosition(tag_start, FileStream::beg);
			ReadEntry(ifs, base, make_, model_, this, output_);
			//output_.RecordInfo(tag, TagName(tag), data);
		}

		ifs.RPosition(temp, FileStream::beg);
	}

	return exif_offset;
}


bool PhotoInfoCR2::Scan(const TCHAR* filename, ExifBlock& exifData, bool generateThumbnails, ImgLogger* logger)
{
	exifData.clear();

	bool exif_present= false;

	// scan CR2

	FileStream ifs;

	if (!ifs.Open(filename))
		return exif_present;

	if (ifs.GetLength32() < 200)	// too short?
		return exif_present;

	uint16 order= ifs.GetUInt16();
	if (order == 'MM')
		ifs.SetByteOrder(true);
	else if (order == 'II')
		ifs.SetByteOrder(false);
	else
		return exif_present;

	if (ifs.GetUInt16() != 42)		// TIFF magic number
		return exif_present;

	for (int i= 0; i < 9; ++i)		// limit no of IFDs
	{
		uint32 offset= ifs.GetUInt32();		// IFD offset
		if (offset == 0)
			break;

		ifs.RPosition(offset, FileStream::beg);	// go to the IFD

		if (uint32 exif_offset= ParseIFD(ifs, 0, i == 0))
		{
			Offset temp= ifs.RPosition();

			ifs.RPosition(exif_offset, FileStream::beg);	// go to EXIF

			// IFD offset seems to be 0
			uint32 ifd_offset= 0;
			uint32 exif_size= ifs.GetLength32() - exif_offset;	// approximation only
			std::pair<uint32, uint32> range(exif_offset, exif_size);
			exif_present = ScanExif(filename, ifs, ifd_offset, range, _T("Canon"), String(), this, &exifData, &output_, false);

			ifs.RPosition(temp, FileStream::beg);

			SetExifInfo(exif_offset, exif_size, ifd_offset, ifs.GetByteOrder());
		}
	}

	return exif_present;
}


bool PhotoInfoCR2::IsRotationFeasible() const
{
	return false;
}


bool PhotoInfoCR2::ReadExifBlock(ExifBlock& exif) const
{
	return false;


//	if (!exif_data_present_)
		return false;
}


ImageStat PhotoInfoCR2::CreateThumbnail(Dib& bmp, DecoderProgress* progress) const
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


CImageDecoderPtr PhotoInfoCR2::GetDecoder() const
{
	AutoPtr<JPEGDataSource> file= new CFileDataSource(path_.c_str(), jpeg_data_offset_);
	JpegDecoderMethod dct_method= g_Settings.dct_method_; // lousy
	return new JPEGDecoder(file, dct_method);
}


void PhotoInfoCR2::CompleteInfo(ImageDatabase& db, OutputStr& out) const
{
	out = output_;
}


bool PhotoInfoCR2::GetEmbeddedJpegImage(uint32& jpeg_data_offset, uint32& jpeg_data_size) const
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

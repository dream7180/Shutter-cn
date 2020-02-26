/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "PhotoInfoORF.h"
#include "TIFFDecoder.h"
#include "NEFDecoder.h"
#include "file.h"
#include "ExifBlock.h"
#include "data.h"
#include "JPEGDecoder.h"
#include "FileDataSource.h"
#include "Config.h"
#include "scan.h"
#include "PhotoFactory.h"
#include "FileTypeIndex.h"
#include "Exception.h"

extern bool StripBlackFrame(Dib& dib, bool YUV);

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

namespace {
	RegisterPhotoType<PhotoInfoORF> img(_T("orf"), FT_ORF);
}


PhotoInfoORF::PhotoInfoORF()
{
	file_type_index_ = FT_ORF;
	jpeg_thm_data_offset_ = jpeg_data_offset_ = jpeg_data_size_ = 0;
}

PhotoInfoORF::~PhotoInfoORF()
{}


//int PhotoInfoORF::GetTypeMarkerIndex() const
//{
//	return 7;	// in indicators.png
//}


void PhotoInfoORF::ParseMakerNote(FileStream& ifs)
{
	uint32 thumbnail_ifd_offset= ifs.RPosition();

	char buf[8];
	ifs.Read(buf, 8);
	uint32 base= 0;
	const uint16 thumbnail_ifd_tag= 0x2020;

	if (memcmp(buf, "OLYMP", 6) == 0)
	{
		base = ifs.RPosition();
		thumbnail_ifd_offset = 0;

//		uint16 order= ifs.GetUInt16();
//		if (order != 'MM' && order != 'II')
//			return;

//		ifs.SetByteOrder(start[10] == 'M');

//		if (ifs.GetUInt16() != 0x2a)	// has to be 0x2a (this is IFD header)
//			return;

//		ifs.RPosition(ifs.GetUInt32() - 8);
	}
	else if (memcmp(buf, "OLYMPUS", 8) == 0)
	{
		uint16 order= ifs.GetUInt16();
		ifs.SetByteOrder(order == 'MM');
		uint16 ver= ifs.GetUInt16();
		base = ifs.RPosition();
	}
	else
		return; //ifs.RPosition(-10);

	uint16 entries= ifs.GetUInt16();

	if (entries > 0x200)
	{
		ASSERT(false);
		return;
	}

	for (int i= 0; i < entries; ++i)
	{
		uint16 tag= ifs.GetUInt16();
		Data data(ifs, base);
		Offset temp= ifs.RPosition();

		if (tag == thumbnail_ifd_tag)	// thumbnail's ifd
		{
			ifs.RPosition(data.GetData() + thumbnail_ifd_offset, FileStream::beg);
			uint16 ifd_entries= ifs.GetUInt16();

			if (ifd_entries > 0x200)
			{
				ASSERT(false);
				break;
			}

			for (int i= 0; i < ifd_entries; ++i)
			{
				uint16 tag= ifs.GetUInt16();
				Data data(ifs, 0);

				if (tag == 0x101)	// offset to the preview JPEG
					jpeg_data_offset_ = thumbnail_ifd_offset + data.AsULong();
				else if (tag == 0x102)
					jpeg_data_size_ = data.AsULong();
			}
		}

		ifs.RPosition(temp, FileStream::beg);
	}
}


void PhotoInfoORF::ParseExif(FileStream& ifs, uint32 base)
{
	uint16 entries= ifs.GetUInt16();

	if (entries > 0x200)
	{
		ASSERT(false);
		return;
	}

	for (int i= 0; i < entries; ++i)
	{
		uint16 tag= ifs.GetUInt16();
		Data data(ifs, 0);
		Offset temp= ifs.RPosition();

//TRACE(_T("exf: %x (= %d)\n"), int(tag), int(data.GetData()));

		switch (tag)
		{
		case 0x927c:
			ifs.RPosition(data.GetData(), FileStream::beg);
			ParseMakerNote(ifs);
			break;

		default:
			break;
		}

		ifs.RPosition(temp, FileStream::beg);
	}
}


uint32 PhotoInfoORF::ParseIFD(FileStream& ifs, uint32 base, bool main_image)
{
	uint16 entries= ifs.GetUInt16();

	if (entries > 0x200)
	{
		ASSERT(false);
		return 0;
	}

	uint32 exif_offset= 0;
	jpeg_data_offset_ = 0;
	jpeg_data_size_ = 0;
	jpeg_thm_data_offset_ = 0;

	for (int i= 0; i < entries; ++i)
	{
		Offset tag_start= ifs.RPosition();

		uint16 tag= ifs.GetUInt16();
		Data data(ifs, 0);

		Offset temp= ifs.RPosition();

		switch (tag)
		{
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

		//case 0x10f:		// make
		//case 0x110:		// model
		//	if (main_image)
		//	{
		//		ifs.RPosition(tag_start, FileStream::beg);
		//		Offset offset= 0;
		//		ReadEntry(ifs, offset, make_, model_, this, output_);
		//	}
		//	break;

		case 0x201:		// JPEGInterchangeFormat
			if (main_image)
				jpeg_thm_data_offset_ = data.AsULong();
			break;

		//case 0x202:		// JPEGInterchangeFormatLength
		//	if (!main_image)
		//		jpeg_data_size_ = data.AsULong();
		//	break;

		case 0x8769:	// EXIF tag
			exif_offset = data.AsULong();
			ifs.RPosition(exif_offset, FileStream::beg);
			ParseExif(ifs, exif_offset);
			break;

		case 0x8825:	// GPSInfo
			SetGpsData(ReadGPSInfo(ifs, 0, data, &output_));
			break;
		}

		if (main_image)
		{
			ifs.RPosition(tag_start, FileStream::beg);
			Offset offset= 0;
			ReadEntry(ifs, offset, make_, model_, this, output_);
		}

		ifs.RPosition(temp, FileStream::beg);
	}

	//TODO: if no JPEG preview then report an error?

	return exif_offset;
}


bool PhotoInfoORF::Scan(const TCHAR* filename, ExifBlock& exifData, bool generateThumbnails, ImgLogger* logger)
{
	exifData.clear();

	bool exif_present= false;

	// scan ORF

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

	if (ifs.GetUInt8() != 'R')		// Olympus raw magic number
		return exif_present;
	if (ifs.GetUInt8() != 'O')		// Olympus raw magic number
		return exif_present;

	for (int i= 0; i < 9; ++i)	// sanity counter
	{
		uint32 offset= ifs.GetUInt32();		// IFD offset
		if (offset == 0)
			break;

		ifs.RPosition(offset, FileStream::beg);	// go to the IFD

		if (uint32 exif_offset= ParseIFD(ifs, 0, i == 0))
		{
			ifs.RPosition(exif_offset, FileStream::beg);	// go to EXIF

			// for ORF IFD offset seems to be 0
			uint32 ifd_offset= 0;
			uint32 exif_size= ifs.GetLength32() - exif_offset;	// approximation only
			std::pair<uint32, uint32> range(exif_offset, exif_size);
			exif_present = ScanExif(filename, ifs, ifd_offset, range, String(), String(), this, &exifData, &output_, false);

			if (exif_present)
				SetExifInfo(exif_offset, exif_size, ifd_offset, ifs.GetByteOrder());

			break;
		}
	}

	return exif_present;
}


bool PhotoInfoORF::IsRotationFeasible() const
{
	return false;
}


bool PhotoInfoORF::ReadExifBlock(ExifBlock& exif) const
{
	return false;


//	if (!exif_data_present_)
		return false;
}


ImageStat PhotoInfoORF::CreateThumbnail(Dib& bmp, DecoderProgress* progress) const
{
	try
	{
		// use thumbnail jpg if available
		uint32 offset= jpeg_thm_data_offset_ ? jpeg_thm_data_offset_ : jpeg_data_offset_;

		if (offset)
		{
			CFileDataSource fsrc(path_.c_str(), offset);
			JPEGDecoder dec(fsrc, JDEC_INTEGER_HIQ);
			dec.SetFast(true, true);
			dec.SetProgressClient(progress);
			return dec.DecodeImg(bmp, GetThumbnailSize(), true);
			// ???	StripBlackFrame(bmp, false);
		}
		else
			return IS_NO_IMAGE;
	}
	catch (...)	// for now (TODO--failure codes)
	{
	}
	return IS_READ_ERROR;
}


CImageDecoderPtr PhotoInfoORF::GetDecoder() const
{
	if (jpeg_data_offset_ && jpeg_data_size_)
	{
		AutoPtr<JPEGDataSource> file= new CFileDataSource(path_.c_str(), jpeg_data_offset_);
		JpegDecoderMethod dct_method= g_Settings.dct_method_; // lousy
		return new JPEGDecoder(file, dct_method);
	}
	else
		THROW_EXCEPTION(L"Unsupported ORF file format", L"This ORF file is not supported. Cannot find embedded preview image.");
}


void PhotoInfoORF::CompleteInfo(ImageDatabase& db, OutputStr& out) const
{
	out = output_;
}


bool PhotoInfoORF::GetEmbeddedJpegImage(uint32& jpeg_data_offset, uint32& jpeg_data_size) const
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

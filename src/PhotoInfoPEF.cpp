/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "PhotoInfoPEF.h"
#include "TIFFDecoder.h"
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
	RegisterPhotoType<PhotoInfoPEF> img(_T("pef"), FT_PEF);
}


PhotoInfoPEF::PhotoInfoPEF() : type_name_(_T("Pentax raw"))
{
	file_type_index_ = FT_PEF;
	jpeg_thm_data_offset_ = jpeg_data_offset_ = jpeg_data_size_ = 0;
}


PhotoInfoPEF::PhotoInfoPEF(const TCHAR* type_name) : type_name_(type_name)
{
	file_type_index_ = FT_PEF;
	jpeg_thm_data_offset_ = jpeg_data_offset_ = jpeg_data_size_ = 0;
}


PhotoInfoPEF::~PhotoInfoPEF()
{}


//int PhotoInfoPEF::GetTypeMarkerIndex() const
//{
//	return 12;	// in indicators.png
//}


void PhotoInfoPEF::ParseMakerNote(FileStream& ifs)
{
	char buf[4];
	ifs.Read(buf, 4);
	uint32 base= 0;

	if (memcmp(buf, "AOC\0", 4) == 0)
	{
		base = ifs.RPosition();

		uint16 order= ifs.GetUInt16();
		if (order != 'MM' && order != 'II')
			return;

//		ifs.SetByteOrder(start[10] == 'M');

//		if (ifs.GetUInt16() != 0x2a)	// has to be 0x2a (this is IFD header)
//			return;

//		ifs.RPosition(ifs.GetUInt32() - 8);
	}
	else
		return;
//		ifs.RPosition(-4);

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
/*
		switch (tag)
		{
		//case 0x11:	// thumbnail's ifd
		//	{
		//		ifs.RPosition(base + data.AsULong(), FileStream::beg);
		//		uint16 ifd_entries= ifs.GetUInt16();

		//		if (ifd_entries > 0x200)
		//		{
		//			ASSERT(false);
		//			break;
		//		}

		//		for (int i= 0; i < ifd_entries; ++i)
		//		{
		//			uint16 tag= ifs.GetUInt16();
		//			Data data(ifs, 0);

		//			if (tag == 0x201)	// JPEGInterchangeFormat
		//			{
		//				jpeg_thm_data_offset_ = base + data.AsULong();
		//				break;
		//			}
		//		}
		//	}
		//	break;

		default:
			break;
		}
*/
		ifs.RPosition(temp, FileStream::beg);
	}
}


void PhotoInfoPEF::ParseExif(FileStream& ifs, uint32 base)
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
			// parse maker note to find thumbnail image
			ParseMakerNote(ifs);
			break;

		//case 0x201:		// JPEGInterchangeFormat
		//	jpeg_thm_data_offset_ = data.AsULong();
		//	break;

		//case 0x202:		// JPEGInterchangeFormatLength
		//	jpeg_th_data_size_ = data.AsULong();
		//	break;

		default:
			break;
		}

		ifs.RPosition(temp, FileStream::beg);
	}
}


uint32 PhotoInfoPEF::ParseIFD(FileStream& ifs, uint32 base, bool main_image, bool thumbnail, bool big_jpeg)
{
	uint16 entries= ifs.GetUInt16();

	if (entries > 0x200)
	{
		ASSERT(false);
		return 0;
	}

//	bool main_image= true;	// first IDF in PEF describes main raw image
	uint32 exif_offset= 0;

	for (int i= 0; i < entries; ++i)
	{
		Offset tag_start= ifs.RPosition();

		uint16 tag= ifs.GetUInt16();
		Data data(ifs, 0);

		Offset temp= ifs.RPosition();

		// TODO: read time and orientation
		//...

		switch (tag)
		{
		//case 0xfe:		// sub file?
		//	main_image = (data.AsULong() & 1) == 0;	// main image?
		//	break;

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
			//else
			//	sub_img_orientation_ = static_cast<uint16>(data.AsULong());
			break;

		case 0x10f:		// make
		case 0x110:		// model
			if (!main_image)
			{
				ifs.RPosition(tag_start, FileStream::beg);
				Offset offset= 0;
				ReadEntry(ifs, offset, make_, model_, this, output_);
			}
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
			if (big_jpeg)
				jpeg_data_offset_ = data.AsULong();
			else if (thumbnail)
				jpeg_thm_data_offset_ = data.AsULong();
			break;

		case 0x202:		// JPEGInterchangeFormatLength
			if (big_jpeg)
				jpeg_data_size_ = data.AsULong();
			break;

		case 0x8769:	// EXIF tag
			exif_offset = data.AsULong();
			ifs.RPosition(exif_offset, FileStream::beg);
			// parse EXIF to find MakerNote
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
			//output_.RecordInfo(tag, TagName(tag), data);
		}

		ifs.RPosition(temp, FileStream::beg);
	}

	return exif_offset;
}

bool PhotoInfoPEF::IsMainImage(int ifd_index)
{
	// first IFD in the PEF file is devoted to the main image
	return ifd_index == 0;
}

bool PhotoInfoPEF::IsThumbnailImage(int ifd_index)
{
	// second IFD in the PEF file describes thumbnail
	return ifd_index == 1;
}

bool PhotoInfoPEF::IsBigImage(int ifd_index)
{
	// third IFD in the PEF file is for a big preview image
	return ifd_index == 2;
}


bool PhotoInfoPEF::Scan(const TCHAR* filename, ExifBlock& exifData, bool generateThumbnails, ImgLogger* logger)
{
	exifData.clear();

	bool exif_present= false;

	// scan PEF

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

	exifData.bigEndianByteOrder = ifs.GetByteOrder();

	if (ifs.GetUInt16() != 42)		// TIFF magic number
		return exif_present;

	for (int i= 0; i < 1000; ++i)	// safety counter only
	{
		uint32 offset= ifs.GetUInt32();		// IFD offset
		if (offset == 0)
			break;

		ifs.RPosition(offset, FileStream::beg);	// go to the IFD

		if (uint32 exif_offset= ParseIFD(ifs, 0, IsMainImage(i), IsThumbnailImage(i), IsBigImage(i)))
		{
			Offset temp= ifs.RPosition();

			ifs.RPosition(exif_offset, FileStream::beg);	// go to EXIF

			// for PEF IFD offset seems to be 0?
			uint32 ifd_offset= 0;
			std::pair<uint32, uint32> range(exif_offset, ifs.GetLength32() - exif_offset);	// approximation only
			exif_present = ScanExif(filename, ifs, ifd_offset, range, make_, model_, this, &exifData, &output_, false);

			//if (orientation_ == 0xffff)	// no orientation given for a main image?
			//	orientation_ = sub_img_orientation_;	// then use subimage orientation (this works for D200)

			ifs.RPosition(exif_offset, FileStream::beg);	// go to EXIF
			size_t exif_size= ::CalcExifSize(ifs);
			if (exif_size > 0) // && exif_size < 100 * 1024)	// some reasonable limits: 100 KB
			{
				exifData.exifBlockSize = exif_size;
				exifData.is_raw = true;
				exifData.ifd0Start = 0 - exif_offset;	// this is really going to be negative
				ifs.RPosition(exif_offset, FileStream::beg);	// go to EXIF
				exifData.exif_buffer.resize(exif_size, 0);
				ifs.Read(&exifData.exif_buffer[0], exif_size);

				SetExifInfo(exif_offset, exif_size, ifd_offset, ifs.GetByteOrder());
			}
			else
				exif_present = false;


			ifs.RPosition(temp, FileStream::beg);
		}
	}

	if (make_.empty())
		make_ = _T("Pentax");

	return exif_present;
}


bool PhotoInfoPEF::IsRotationFeasible() const
{
	return false;
}


bool PhotoInfoPEF::ReadExifBlock(ExifBlock& exif) const
{
	return false;


//	if (!exif_data_present_)
		return false;
}


ImageStat PhotoInfoPEF::CreateThumbnail(Dib& bmp, DecoderProgress* progress) const
{
	try
	{
		if (jpeg_thm_data_offset_ || jpeg_data_offset_)	// use thumbnail jpg if available
		{
			CFileDataSource fsrc(path_.c_str(), jpeg_thm_data_offset_ ? jpeg_thm_data_offset_ : jpeg_data_offset_);
			JPEGDecoder dec(fsrc, JDEC_INTEGER_HIQ);
			dec.SetFast(true, true);
			dec.SetProgressClient(progress);
			ImageStat stat= dec.DecodeImg(bmp, GetThumbnailSize(), true);
			if (stat == IS_OK && bmp.IsValid())
				StripBlackFrame(bmp, false);
			return stat;
		}
		else
		{
			// actually there is no TIFF preview in PEF...
			TIFFDecoder decoder(path_.c_str());
			ImageStat stat= decoder.DecodeImg(bmp, GetThumbnailSize(), true);
			if (stat == IS_OK && bmp.IsValid())
				StripBlackFrame(bmp, false);
			return stat;
		}
	}
	catch (...)	// for now (TODO--failure codes)
	{
	}
	return IS_READ_ERROR;
}


CImageDecoderPtr PhotoInfoPEF::GetDecoder() const
{
	if (jpeg_data_offset_ && jpeg_data_size_)
	{
		AutoPtr<JPEGDataSource> file= new CFileDataSource(path_.c_str(), jpeg_data_offset_);
		JpegDecoderMethod dct_method= g_Settings.dct_method_; // lousy
		return new JPEGDecoder(file, dct_method);
	}
	else
		THROW_EXCEPTION(SF(L"Unsupported " << type_name_ << L" file"), L"This file is not supported. Cannot find embedded preview image.");
}


void PhotoInfoPEF::CompleteInfo(ImageDatabase& db, OutputStr& out) const
{
	out = output_;
}


bool PhotoInfoPEF::GetEmbeddedJpegImage(uint32& jpeg_data_offset, uint32& jpeg_data_size) const
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

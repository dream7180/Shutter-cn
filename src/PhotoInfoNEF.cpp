/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "PhotoInfoNEF.h"
#include "TIFFDecoder.h"
#include "NEFDecoder.h"
#include "file.h"
#include "ExifBlock.h"
#include "ExifTags.h"
#include "data.h"
#include "JPEGDecoder.h"
#include "FileDataSource.h"
#include "Config.h"
#include "scan.h"
#include "PhotoFactory.h"
#include "FileTypeIndex.h"
#include "XMPAccess.h"

extern bool StripBlackFrame(Dib& dib, bool YUV);

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

namespace {
	RegisterPhotoType<PhotoInfoNEF> img1(_T("nef"), FT_NEF);
	RegisterPhotoType<PhotoInfoNEF> img2(_T("nrw"), FT_NEF);

	const int MAX_TAG_COUNT= 0x200;	// limit to protect decoder against reading bogus files
}

PhotoInfoNEF::PhotoInfoNEF()
{
	file_type_index_ = FT_NEF;
	jpeg_thm_data_offset_ = jpeg_data_offset_ = jpeg_data_size_ = 0;
	sub_img_orientation_ = ~0;
}

PhotoInfoNEF::~PhotoInfoNEF()
{}


//int PhotoInfoNEF::GetTypeMarkerIndex() const
//{
//	return 6;	// in indicators.png
//}


void PhotoInfoNEF::ParseMakerNote(FileStream& ifs)
{
	char buf[10];
	ifs.Read(buf, 10);
	uint32 base= 0;

	if (memcmp(buf, "Nikon", 6) == 0)
	{
		base = ifs.RPosition();

		uint16 order= ifs.GetUInt16();
		if (order != 'MM' && order != 'II')
			return;

//		ifs.SetByteOrder(start[10] == 'M');

		if (ifs.GetUInt16() != 0x2a)	// has to be 0x2a (this is IFD header)
			return;

		ifs.RPosition(ifs.GetUInt32() - 8);
	}
	else
		ifs.RPosition(-10);

	uint16 entries= ifs.GetUInt16();

	if (entries > MAX_TAG_COUNT)
	{
		ASSERT(false);
		return;
	}

	for (int i= 0; i < entries; ++i)
	{
		uint16 tag= ifs.GetUInt16();
		Data data(ifs, base);
		Offset temp= ifs.RPosition();

		switch (tag)
		{
		case 0x11:	// thumbnail's ifd
			{
				ifs.RPosition(base + data.AsULong(), FileStream::beg);
				uint16 ifd_entries= ifs.GetUInt16();

				if (ifd_entries > MAX_TAG_COUNT)
				{
					ASSERT(false);
					break;
				}

				for (int i= 0; i < ifd_entries; ++i)
				{
					uint16 tag= ifs.GetUInt16();
					Data data(ifs, 0);

					if (tag == TIFF_JPEG_INTERCHANGE_FORMAT)	// JPEGInterchangeFormat
					{
						jpeg_thm_data_offset_ = base + data.AsULong();
						break;
					}
				}
			}
			break;

		default:
			break;
		}

		ifs.RPosition(temp, FileStream::beg);
	}
}


void PhotoInfoNEF::ParseExif(FileStream& ifs, uint32 base)
{
	uint16 entries= ifs.GetUInt16();

	if (entries > MAX_TAG_COUNT)
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
		case EXIF_MAKER_NOTE:
			ifs.RPosition(data.GetData(), FileStream::beg);
			// parse maker note to find thumbnail image
			ParseMakerNote(ifs);
			break;

		default:
			break;
		}

		ifs.RPosition(temp, FileStream::beg);
	}
}


uint32 PhotoInfoNEF::ParseIFD(FileStream& ifs, uint32 base)
{
	uint16 entries= ifs.GetUInt16();

	TRACE(L"IFD: %d entries ----------------\n", int(entries));

	if (entries > MAX_TAG_COUNT)
	{
		ASSERT(false);
		return 0;
	}

	std::pair<uint32, uint32> jpeg(0, 0);
	bool main_image= false;
	uint32 exif_offset= 0;

	for (int i= 0; i < entries; ++i)
	{
		Offset tag_start= ifs.RPosition();

		uint16 tag= ifs.GetUInt16();
		Data data(ifs, 0);

		Offset temp= ifs.RPosition();

		// TODO: read time and orientation
		//...

		TRACE(L"tag: %x (%d)\n", tag, tag);

		switch (tag)
		{
		case TIFF_NEW_SUBFILE_TYPE:	// sub file?
			main_image = (data.AsULong() & 1) == 0;	// main image?
			TRACE(L"   subfile: %d\n", data.AsULong());
			break;

		case TIFF_IMAGE_WIDTH:		// img width
			if (main_image)
				SetWidth(data.AsULong());
			break;

		case TIFF_IMAGE_HEIGHT:		// img height
			if (main_image)
				SetHeight(data.AsULong());
			break;

		case TIFF_ORIENTATION:		// orientation
			if (main_image)
				SetOrientation(static_cast<uint16>(data.AsULong()));
			else
				sub_img_orientation_ = static_cast<uint16>(data.AsULong());
			break;

		case TIFF_MAKE:				// make
		case TIFF_MODEL:			// model
			if (!main_image)
			{
				ifs.RPosition(tag_start, FileStream::beg);
				Offset offset= 0;
				ReadEntry(ifs, offset, make_, model_, this, output_);
			}
			break;

		case TIFF_SUB_IFD:			// SubIFD
			{
				uint32 len= data.Components();
				uint32 offset[8];
				data.ReadLongs(offset, std::min<uint32>(len, array_count(offset)));
				for (uint32 i= 0; i < len; ++i)
				{
					ifs.RPosition(offset[i], FileStream::beg);
					ParseIFD(ifs, ifs.RPosition());
				}
			}
			break;

		case TIFF_JPEG_INTERCHANGE_FORMAT:			// JPEGInterchangeFormat
			if (!main_image)
				jpeg.first = data.AsULong();
			break;

		case TIFF_JPEG_INTERCHANGE_FORMAT_LENGTH:	// JPEGInterchangeFormatLength
			if (!main_image)
				jpeg.second = data.AsULong();
			break;

		case TIFF_XMP:		// embedded XMP metadata string
			// try to read embedded XMP metadata, even though ExifPro stores it in a separate file;
			// separate file (if present) will still be read leater and will take precedent over embedded xmp
			try
			{
				std::vector<char> str;
				if (data.AsRawData(str) && !str.empty())
				{
					XmpData xmp;
					Xmp::MetaToXmpData(str, xmp);
					SetMetadata(xmp);
				}
			}
			catch (String&)
			{
				ASSERT(false);	// ignore bogus metadata...
			}
			break;

		case TIFF_EXIF_IFD:		// EXIF tag
			exif_offset = data.AsULong();
			ifs.RPosition(exif_offset, FileStream::beg);
			// parse EXIF to find MakerNote
			ParseExif(ifs, exif_offset);
			break;

		case TIFF_GPS_IFD:		// GPSInfo
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

	TRACE(L"IFD: end ----------------------\n", int(entries));

	// choose bigger of the potentially multiple JPEG previews
	if (jpeg.first && jpeg.second && jpeg.second > jpeg_data_size_)
	{
		jpeg_data_offset_ = jpeg.first;
		jpeg_data_size_ = jpeg.second;
	}

	return exif_offset;
}


bool PhotoInfoNEF::Scan(const TCHAR* filename, ExifBlock& exifData, bool generateThumbnails, ImgLogger* logger)
{
	exifData.clear();

	bool exif_present= false;

	// scan NEF

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

		if (uint32 exif_offset= ParseIFD(ifs, 0))
		{
			Offset temp= ifs.RPosition();

			ifs.RPosition(exif_offset, FileStream::beg);	// go to EXIF

			// for NEF IFD offset seems to be 0
			uint32 ifd_offset= 0;
			std::pair<uint32, uint32> range(exif_offset, ifs.GetLength32() - exif_offset);	// approximation only
			exif_present = ScanExif(filename, ifs, ifd_offset, range, make_, model_, this, &exifData, &output_, false);

			if (OrientationField() == 0xffff)	// no orientation given for a main image?
				SetOrientation(sub_img_orientation_);	// then use subimage orientation (this works for D200)

			ifs.RPosition(exif_offset, FileStream::beg);	// go to EXIF
			size_t exif_size= ::CalcExifSize(ifs);
			// note: EXIF block as recorded in NEF exceeds 120 KB easily (MakerNote seems to be sparse)
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
		make_ = _T("Nikon");

	return exif_present;
}


bool PhotoInfoNEF::IsRotationFeasible() const
{
	return false;
}


bool PhotoInfoNEF::ReadExifBlock(ExifBlock& exif) const
{
	if (!exif_data_present_)
		return false;

	if (exif_info_.exif_offset == 0 || exif_info_.exif_block_size == 0)
		return false;

	const size_t size_limit= 0x10000 - 12;

	if (exif_info_.exif_block_size >= size_limit)
		return false;	// won't fit in JPEG's marker

	FileStream f;
	if (!f.Open(path_.c_str()))
		return false;

	f.RPosition(exif_info_.exif_offset);
	f.SetByteOrder(exif_info_.big_endian_byte_order);

	// length
	uint8 lo= static_cast<uint8>(exif_info_.exif_block_size & 0xff);
	uint8 hi= static_cast<uint8>((exif_info_.exif_block_size >> 8) & 0xff);

	uint8 order= exif_info_.big_endian_byte_order ? 'M' : 'I';

	// EXIF in the JPEG's app marker
	uint8 header[]= { 0xff, 0xe1, hi, lo, 'E', 'x', 'i', 'f', 0, 0, order, order };
	size_t len= array_count(header);

	exif.exif_buffer.resize(len + exif_info_.exif_block_size);
	uint8* p= &exif.exif_buffer[0];

	memcpy(p, header, len);

	f.Read(p + len, exif_info_.exif_block_size);

	return true;
}


ImageStat PhotoInfoNEF::CreateThumbnail(Dib& bmp, DecoderProgress* progress) const
{
	try
	{
		if (jpeg_thm_data_offset_)	// use thumbnail jpg if available
		{
			CFileDataSource fsrc(path_.c_str(), jpeg_thm_data_offset_);
			JPEGDecoder dec(fsrc, JDEC_INTEGER_HIQ);
			dec.SetFast(true, true);
			dec.SetProgressClient(progress);
			return dec.DecodeImg(bmp, GetThumbnailSize(), true);
		}
		else
		{
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


CImageDecoderPtr PhotoInfoNEF::GetDecoder() const
{
	if (jpeg_data_offset_ && jpeg_data_size_)
	{
		AutoPtr<JPEGDataSource> file= new CFileDataSource(path_.c_str(), jpeg_data_offset_);
		JpegDecoderMethod dct_method= g_Settings.dct_method_; // lousy
		return new JPEGDecoder(file, dct_method);
	}
	else
		return new CNEFDecoder(path_.c_str());
}


void PhotoInfoNEF::CompleteInfo(ImageDatabase& db, OutputStr& out) const
{
	out = output_;
}


bool PhotoInfoNEF::GetEmbeddedJpegImage(uint32& jpeg_data_offset, uint32& jpeg_data_size) const
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

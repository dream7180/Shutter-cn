/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// PhotoInfoDNG.cpp: implementation of the PhotoInfoDNG class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "PhotoInfoDNG.h"
#include "TIFFDecoder.h"
#include "TIFFReader.h"
#include "Config.h"
#include "ExifBlock.h"
#include "File.h"
#include "PhotoFactory.h"
#include "FileTypeIndex.h"
#include "scan.h"
#include "Markers.h"
#include "JPEGDecoder.h"
#include "FileDataSource.h"
#include "JPEGException.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//////////////////////////////////////////////////////////////////////

namespace {
	RegisterPhotoType<PhotoInfoDNG> img(_T("dng"), FT_DNG);
}


PhotoInfoDNG::PhotoInfoDNG()
{
	file_type_index_ = FT_DNG;
	jpeg_data_offset_= jpeg_data_size_ = 0;
	is_raw_ = true;
}

PhotoInfoDNG::~PhotoInfoDNG()
{
}

//int PhotoInfoDNG::GetTypeMarkerIndex() const
//{
//	return 8;	// in indicators.png
//}


bool PhotoInfoDNG::CanEditIPTC(int& err_code) const
{
	// TODO
	err_code = 1;
	return false;
}


//bool PhotoInfoDNG::IsDescriptionEditable() const
//{
//	// TODO?
//	AfxMessageBox(_T("DNG file: description is not supported."), MB_OK);
//
//	return false;
//}


bool PhotoInfoDNG::IsRotationFeasible() const
{
	return false;
}

struct Jpg
{
	Jpg()
	{
		width = heigth = offset = size = 0;
	}

	Jpg(uint32 width, uint32 heigth, uint32 offset, uint32 size)
		: width(width), heigth(heigth), offset(offset), size(size)
	{}

	bool IsValid() const
	{
		return width && heigth && offset && size;
	}

	uint32 width;
	uint32 heigth;
	uint32 offset;
	uint32 size;
};


Jpg ParseIFD(const TCHAR* filename, FileStream& ifs, bool main_image, PhotoInfoPtr photo)
{
	uint16 entries= ifs.GetUInt16();

	if (entries > 0x200)
	{
		ASSERT(false);
		return Jpg();
	}

	uint32 jpeg_offset= 0;
	uint32 jpeg_length= 0;
	uint32 width= 0, height= 0;

	for (int i= 0; i < entries; ++i)
	{
		Offset tag_start= ifs.RPosition();

		uint16 tag= ifs.GetUInt16();
		Data data(ifs, 0);

		Offset temp= ifs.RPosition();

		// TODO: read time
		//...

		switch (tag)
		{
		case TIFF_IMAGE_WIDTH:
			width = data.AsULong();
			break;

		case TIFF_IMAGE_HEIGHT:
			height = data.AsULong();
			break;

		case TIFF_SUB_IFD:
			{
				uint32 len= data.Components();
				uint32 offset[8];
				data.ReadLongs(offset, std::min<uint32>(len, array_count(offset)));
				Jpg cur_jpg;
				for (uint32 i= 0; i < len; ++i)
				{
					ifs.RPosition(offset[i], FileStream::beg);
					Jpg jpg= ParseIFD(filename, ifs, false, photo);

					// when iterating through IFDs, pick the biggest preview
					if (jpg.IsValid() && jpg.width > cur_jpg.width && jpg.heigth > cur_jpg.heigth)
						cur_jpg = jpg;
				}

				return cur_jpg;
			}
			break;

		case TIFF_STRIP_OFFSETS:
			if (!main_image)
				jpeg_offset = data.AsULong();
			break;

		case TIFF_ORIENTATION:
//			if (main_image)
				photo->SetOrientation(static_cast<uint16>(data.AsULong()));
			break;

		case TIFF_STRIP_BYTE_COUNTS:
			if (!main_image)
				jpeg_length = data.AsULong();
			break;
		}

		if (jpeg_offset)
		{
			ifs.RPosition(jpeg_offset, FileStream::beg);
			uint16 marker= (ifs.GetUInt8() << 8) | ifs.GetUInt8();
			if (marker != MARK_SOI)	// not a start of JPEG image?
				jpeg_offset = 0;
		}

		ifs.RPosition(temp, FileStream::beg);

		if (jpeg_offset && jpeg_length)
		{
			try
			{
				// make JPEG decoder read image header; it'll trigger exception if it has unsupported format
				std::auto_ptr<JPEGDataSource> img_src(new CFileDataSource(filename, jpeg_offset));
				JPEGDecoder::DecodeJpegImageDimensions(*img_src);
			}
			catch (JPEGException&)
			{
				// skip JPEG images that library cannot decode; not supported encoding, etc.
				jpeg_offset = 0;
				jpeg_length = 0;
				continue;
			}

			// found JPEG image is good to use
			break;
		}
	}

	return Jpg(width, height, jpeg_offset, jpeg_length);
}


bool PhotoInfoDNG::Scan(const TCHAR* filename, ExifBlock& exifData, bool generateThumbnails, ImgLogger* logger)
{
	exifData.clear();
	// scan DNG (other XML-EXIF tags to do)

	bool hasExif= false;

	// extract size info at the very least
	if (GetWidth() == 0 && GetHeight() == 0)
	{
		try
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

					tiff.Close();

					// scan a bit of a DNG to extract offset to the big preview image

					FileStream ifs;

					if (ifs.Open(filename))
					{
						ifs.SetByteOrder(!intel_order);

						ifs.GetUInt16();	// skip order marker
						if (ifs.GetUInt16() == 42)		// TIFF magic number
						{
							uint32 offset= ifs.GetUInt32();		// IFD offset

							ifs.RPosition(offset, FileStream::beg);	// go to the IFD

							Jpg jpeg= ParseIFD(filename, ifs, true, this);

							if (jpeg.IsValid())
							{
								jpeg_data_offset_ = jpeg.offset;
								jpeg_data_size_ = jpeg.size;
							}
						}
					}
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

					hasExif = ::ScanExif(filename, ifs, 0, std::make_pair(exif_offset, ifs.GetLength32()),
						make, model, this, &exifData, &output_, false);

					if (hasExif)
						SetExifInfo(exif_offset, ifs.GetLength32(), 0, ifs.GetByteOrder());
				}
			}
		}
		catch(...)
		{}
	}

	return hasExif;
}


ImageStat PhotoInfoDNG::CreateThumbnail(Dib& bmp, DecoderProgress* progress) const
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


bool PhotoInfoDNG::ReadExifBlock(ExifBlock& exif) const
{
	return false;
}


void PhotoInfoDNG::CompleteInfo(ImageDatabase& db, OutputStr& out) const
{
	out = output_;
}


CImageDecoderPtr PhotoInfoDNG::GetDecoder() const
{
	if (jpeg_data_offset_ && jpeg_data_size_)
	{
		AutoPtr<JPEGDataSource> file= new CFileDataSource(path_.c_str(), jpeg_data_offset_);
		JpegDecoderMethod dct_method= g_Settings.dct_method_; // lousy
		return new JPEGDecoder(file, dct_method);
	}
	else
		return new TIFFDecoder(path_.c_str());
}


bool PhotoInfoDNG::GetEmbeddedJpegImage(uint32& jpeg_data_offset, uint32& jpeg_data_size) const
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

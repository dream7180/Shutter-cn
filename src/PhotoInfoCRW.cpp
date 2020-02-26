/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// PhotoInfoCRW.cpp: implementation of the PhotoInfoCRW class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PhotoInfoCRW.h"
#include "PhotoInfoJPEG.h"
#include "scan.h"
#include "ExifBlock.h"
#include "FileTypeIndex.h"
#include "PhotoFactory.h"
#include "FileStatus.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

namespace {
	RegisterPhotoType<PhotoInfoCRW> img(_T("crw"), FT_CRW);
}

PhotoInfoCRW::PhotoInfoCRW()
{
	file_type_index_ = FT_CRW;
}

PhotoInfoCRW::~PhotoInfoCRW()
{}

//int PhotoInfoCRW::GetTypeMarkerIndex() const
//{
//	return 3;	// in indicators.png
//}


static Path GetThumbnailFilePath(const Path& file)
{
	Path xmp_file= file;
	xmp_file.ReplaceExtension(_T("THM"));
	return xmp_file;
}


bool PhotoInfoCRW::Scan(const TCHAR* filename, ExifBlock& exifData, bool generateThumbnails, ImgLogger* logger)
{
	// 'thm' file
	Path thm= GetThumbnailFilePath(filename);

	bool exif_present= false;

	// look for accompanying 'thm' file first
	FileStatus status;
	if (GetFileStatus(thm.c_str(), status))	// does it exist?
	{
		// scan 'thm' to extract EXIF data
		exif_present = ::Scan(thm.c_str(), this, &output_, &exifData, logger);

		// now scan CRW file to find offset to JPEG preview image inside
#ifdef PHOTO_INFO_SMART_PTR
		PhotoInfoJPEG* crw= new PhotoInfoJPEG();
		PhotoInfoPtr ptr(crw);
		::Scan(filename, ptr, &output_, nullptr, logger);
#else
		PhotoInfoJPEG crw;
		::Scan(filename, &crw, &output_, 0, logger);
#endif

		// combine: THM contains EXIF info and thumbnail
		// CRW has an offset to JPEG (preview)

		// grab this JPEG offset & size
		jpeg_offset_ = crw->jpeg_offset_;
		jpeg_size_ = crw->jpeg_size_;
	}
	else
	{
		// there's no 'thm' so scan CRW only
		exif_present = ::Scan(filename, this, &output_, &exifData, logger);
	}

	return exif_present;
}


bool PhotoInfoCRW::IsRotationFeasible() const
{
	return false;
}


bool PhotoInfoCRW::ReadExifBlock(ExifBlock& exif) const
{
	if (!exif_data_present_)
		return false;

	// 'thm' file
	Path thm= GetThumbnailFilePath(path_);

	// look for accompanying 'thm' file first
	FileStatus status;
	if (GetFileStatus(thm.c_str(), status))	// does it exist?
	{
		//PhotoInfoCRW dummy;
		// scan 'thm' to extract EXIF data
		return ::Scan(thm.c_str(), 0, 0, &exif, nullptr);
	}
	else
	{
		//PhotoInfoCRW dummy;
		// there's no 'thm' so scan CRW only
		return ::Scan(path_.c_str(), 0, 0, &exif, nullptr);
	}
}


Path PhotoInfoCRW::GetAssociatedFilePath(size_t index) const
{
	if (index == 0)
	{
		Path path= path_;
		path.ReplaceExtension(_T("XMP"));
		return path;
	}
	else if (index == 1)
		return GetThumbnailFilePath(path_);
	else
		return Path();
}


bool PhotoInfoCRW::GetEmbeddedJpegImage(uint32& jpeg_data_offset, uint32& jpeg_data_size) const
{
	if (jpeg_offset_ && jpeg_size_)
	{
		jpeg_data_offset = jpeg_offset_;
		jpeg_data_size = jpeg_size_;
		return true;
	}
	else
		return false;
}


// find JPEG preview image inside CRW; return file offset to this image, and its size in bytes
extern std::pair<uint32, uint32> ParseCRW(FileStream& ifs, uint32 offset, uint32 length, uint16& orientation)
{
	ifs.RPosition(offset + length - 4, FileStream::beg);
	ifs.RPosition(offset + ifs.GetUInt32(), FileStream::beg);
	const int records= ifs.GetUInt16();

	uint32 jpeg_offset= 0;
	uint32 jpeg_size= 0;

	for (int i = 0; i < records; ++i)
	{
		int type= ifs.GetUInt16();
		uint32 length= 0;
		uint32 rel_offset= 0;
		uint32 abs_offset= 0;

		if (type < 0x4000)
		{
			length = ifs.GetUInt32();
			rel_offset = ifs.GetUInt32();
			abs_offset = offset + rel_offset;
		}
		else
		{
			ifs.RPosition(8);
		}

		if (type == 0x2007)	// JPEG thumbnail
		{
			jpeg_offset = abs_offset;
			jpeg_size = length;
		}
		else if (type == 0x1810)	// image info
		{
			if (length >= 0x1c)
			{
				uint32 temp= ifs.RPosition();
				ifs.RPosition(abs_offset, FileStream::beg);

				uint32 info[7];
				ifs.Read(info, 7);
				if (info[3] == 270)
					orientation = 8;
				else if (info[3] == 90)
					orientation = 6;

				ifs.RPosition(temp, FileStream::beg);
			}
		}
		else if (type >> 8 == 0x28 || type >> 8 == 0x30)	// Get sub-tables
		{
			if (jpeg_offset == 0)
			{
				std::pair<uint32, uint32> offset_size= ParseCRW(ifs, abs_offset, length, orientation);

				if (jpeg_offset == 0)
				{
					jpeg_offset = offset_size.first;
					jpeg_size = offset_size.second;
				}
			}
		}
	}

	return std::make_pair(jpeg_offset, jpeg_size);
}

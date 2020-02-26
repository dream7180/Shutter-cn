/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "PhotoInfoCatalog.h"
#include "file.h"
#include "ExifBlock.h"
#include "data.h"
#include "JPEGDecoder.h"
#include "MemoryDataSource.h"
#include "FileTypeIndex.h"
#include "scan.h"
#include "CatalogImgRecord.h"
#include "Database/Database.h"
#include "Path.h"
#include "StringConversions.h"
#include "MemoryDataSource2.h"
#include "PhotoInfoPtr.h"
#include "BlockPhotoDelete.h"
#include "DateTimeUtils.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


void ScanExifBlock(const TCHAR* fileName, const ExifBlock& exif, String make, String model, PhotoInfoPtr photo, OutputStr* out)
{
	if (!exif.empty())
	{
		// scanning EXIF block will reassign IPTC record (if present), description and thumbnail orientation,
		// and time (among other things)

		if (exif.is_raw)
		{
			FileStream ifs;

			if (ifs.Open(exif.exif_buffer))
			{
				// when reading from memory instruct ifs to return zeros instead of throwing
				// exception on out of bound reads
				ifs.SetExceptions(false);

				ifs.SetByteOrder(exif.bigEndianByteOrder);

				::ScanExif(fileName, ifs, exif.ifd0Start, std::make_pair(0, uint32(exif.exif_buffer.size())), make, model, photo, 0, out, false);
			}
		}
		else
			ScanMem(fileName, exif.exif_buffer, photo, out, nullptr);
	}
}



PhotoInfoCatalog::PhotoInfoCatalog(const CatalogImgRecord& img, boost::shared_ptr<Database> db, uint64 record_offset) : db_(db), record_offset_(record_offset)
{
	BlockPhotoDelete hold_it(this);	// keep reference to *this, so ScanExifBlock won't delete us on exit

	can_delete_ = false;
	can_rename_ = false;

	// assume identity of original photo
	marker_index_ = img.marker_index_;
	file_type_index_ = img.type_; // FT_CATALOG;

	SetVisitedDirId(img.dir_visited_);
	SetFileSize(img.file_size_);
	SetPhotoName(Path(WStr2String(img.path_)).GetFileName());
	path_ = WStr2String(img.path_);
	exif_data_present_ = img.has_exif_;
	SetMake(WStr2String(img.make_));
	SetModel(WStr2String(img.model_));
	SetOrientation(img.orientation_);

	if (exif_data_present_ && !img.exif_.empty())
	{
		// scanning EXIF block will reassign IPTC record (if present), description and thumbnail orientation,
		// and time (among other things)

		::ScanExifBlock(path_.c_str(), img.exif_, GetMake(), GetModel(), this, 0);

		AssignColorProfile(0);

		// TODO: reset?
		//thumbnail_orientation_ = 0x80;
		//rotation_flag_ = RF_INTACT;
	}

	if (GetDateTime().is_not_a_date_time())
		SetDateTime(FileTimeToDateTime(img.creation_time_));

	if (img.iptc_.get())
	{
//		IPTC_ = new IPTCRecord;
		SetIPTCInfo(*img.iptc_);
//		tags_.AssignKeywords(IPTC_->keywords_);
	}

	SetSize(img.photo_width_, img.photo_height_);

	if (GetWidth() == 0 || GetHeight() == 0)
		SetSize(img.img_width_, img.img_height_);

	SetThumbnailOrientation(0);
	rotation_flag_ = RF_INTACT;

	//TODO:
	SetExifDescription(img.description_);

	bmp_ = 0;
	jpeg_thumb_.SetBuffer(img.thumbnail_);
	index_.ConstructFromBuffer(img.index_);
}


PhotoInfoCatalog::~PhotoInfoCatalog()
{}


void PhotoInfoCatalog::ParseExif(FileStream& ifs, uint32 base)
{
}


bool PhotoInfoCatalog::Scan(const TCHAR* filename, ExifBlock& exifData, bool generateThumbnails, ImgLogger* logger)
{
	exifData.clear();

	bool exif_present= false;

	return exif_present;
}


bool PhotoInfoCatalog::CanEditIPTC(int& err_code) const
{
	err_code = 1;
	return false;
}


//bool PhotoInfoCatalog::IsDescriptionEditable() const
//{
//	AfxMessageBox(_T("This is image stored in a catalog: description cannot be modified."), MB_OK);
//
//	return false;
//}


bool PhotoInfoCatalog::IsRotationFeasible() const
{
	return false;
}


bool PhotoInfoCatalog::ReadExifBlock(ExifBlock& exif) const
{
	return false;
}


ImageStat PhotoInfoCatalog::CreateThumbnail(Dib& bmp, DecoderProgress* progress) const
{
	if (!jpeg_thumb_.IsEmpty())
		return jpeg_thumb_.GetDib(bmp);

	return IS_READ_ERROR;
}


CImageDecoderPtr PhotoInfoCatalog::GetDecoder() const
{
//	AutoPtr<JPEGDataSource> file= new CMemoryDataSource(jpeg_thumb_.JpegData(), jpeg_thumb_.JpegDataSize());
//	return new JPEGDecoder(file, JDEC_INTEGER_HIQ);

	// read image from the catalog db
	CatalogImgRecord img;

	std::vector<uint8> record;
	db_->Read(record_offset_, record);

	img.Construct(record);

	if (img.preview_.empty())	// no preview? use thumbnail instead...
	{
		AutoPtr<JPEGDataSource> file= new CMemoryDataSource(jpeg_thumb_.JpegData(), jpeg_thumb_.JpegDataSize());
		return new JPEGDecoder(file, JDEC_INTEGER_HIQ);
	}
	else
	{
		AutoPtr<JPEGDataSource> file= new CMemoryDataSource2(&img.preview_.front(), img.preview_.size());
		return new JPEGDecoder(file, JDEC_INTEGER_HIQ);
	}
}


void PhotoInfoCatalog::CompleteInfo(ImageDatabase& db, OutputStr& out) const
{
	out.Clear();

	if (!exif_data_present_)
		return;

	// read exif block from the catalog db
	CatalogImgRecord img;

	std::vector<uint8> record;
	db_->Read(record_offset_, record);

	img.Construct(record);

	// re-scan EXIF block
	::ScanExifBlock(path_.c_str(), img.exif_, GetMake(), GetModel(), 0, &out);
}


const Path& PhotoInfoCatalog::GetPhysicalPath() const
{
	static const Path empty;
	return empty;
}


//int PhotoInfoCatalog::GetTypeMarkerIndex() const
//{
//	return marker_index_;
//}

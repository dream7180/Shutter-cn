/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// ImageDatabase.cpp: implementation of the ImageDatabase class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ImageDatabase.h"
#include "../MemPointer.h"
#include "../Path.h"
#include "../ItemIdList.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ImageDatabase::ImageDatabase()
{
}

ImageDatabase::~ImageDatabase()
{
}


bool ImageDatabase::OpenDb(const String& db_path, bool read_only, uint32 db_version)
{
	db_file_path_ = db_path;
	return Database::Open(db_path, db_version, read_only);
}


bool ImageDatabase::IsOpen() const
{
	return Database::IsOpen();
}


ImgDataRecord::ImgDataRecord()
{
	file_size_ = 0u;
	time_stamp_.dwHighDateTime = time_stamp_.dwLowDateTime = 0u;
	img_width_ = img_height_ = 0u;
	jpeg_offset_ = 0u;
	jpeg_img_size_ = 0;
	flags_ = 0u;
	exif_orientation_ = 0u;
}


uint32 ImgDataRecord::GetSize() const
{
	return static_cast<uint32>(
		// key has to be first
		4 + path_.size() * sizeof(TCHAR) +
		2 + // version
		sizeof(file_size_) +
		sizeof(time_stamp_) +
		4 + bufEXIF_.size() +
		4 + buf_thumbnail_.size() +
		4 + buf_index_.size() +
		sizeof(img_width_) +
		sizeof(img_height_) +
		sizeof(jpeg_offset_) +
		sizeof(jpeg_img_size_) +
		sizeof(flags_) +
		sizeof(exif_orientation_) +
		0); // more later
}


// save image data into the buffer
void ImgDataRecord::Serialize(std::vector<uint8>& buffer) const
{
	uint32 size= GetSize();
	buffer.resize(size);
	MemPointer data(&buffer.front(), buffer.size());
	data.SetByteOrder(false);

	// key must be first!
	data.WriteString(path_);

	data.PutUInt16(0x0002);	// version

	data.PutUInt32(static_cast<uint32>(file_size_ & 0xffffffff));
	data.PutUInt32(static_cast<uint32>(file_size_ >> 32));
	data.PutUInt32(time_stamp_.dwLowDateTime);
	data.PutUInt32(time_stamp_.dwHighDateTime);

	data.WriteBufWithSize(bufEXIF_);
	data.WriteBufWithSize(buf_thumbnail_);
	data.WriteBufWithSize(buf_index_);

	data.PutUInt32(img_width_);
	data.PutUInt32(img_height_);
	data.PutUInt32(jpeg_offset_);
	data.PutUInt32(jpeg_img_size_);
	data.PutUInt32(flags_);
	data.PutUInt32(exif_orientation_);
}

// reconstruct record from memory buffer
void ImgDataRecord::Construct(const std::vector<uint8>& record)
{
	MemPointer data(const_cast<uint8*>(&record.front()), record.size());
	data.SetByteOrder(false);

	data.ReadString(path_);

	uint16 version= data.GetUInt16();

	file_size_ = data.GetUInt32();
	file_size_ += uint64(data.GetUInt32()) << 32;
	time_stamp_.dwLowDateTime = data.GetUInt32();
	time_stamp_.dwHighDateTime = data.GetUInt32();

	data.ReadBufWithSize(bufEXIF_);
	data.ReadBufWithSize(buf_thumbnail_);
	data.ReadBufWithSize(buf_index_);

	img_width_ = data.GetUInt32();
	img_height_ = data.GetUInt32();

	jpeg_offset_ = data.GetUInt32();
	jpeg_img_size_ = data.GetUInt32();

	flags_ = data.GetUInt32();
	exif_orientation_ = data.GetUInt32();
}


void ImgDataRecord::Clear()
{
	path_.clear();
	file_size_ = 0u;
	time_stamp_.dwHighDateTime = 0u;
	time_stamp_.dwLowDateTime = 0u;
	bufEXIF_.clear();
	buf_thumbnail_.clear();
	buf_index_.clear();
	img_width_ = 0u;
	img_height_ = 0u;
	jpeg_offset_ = 0u;
	jpeg_img_size_ = 0;
	flags_ = 0u;
	exif_orientation_ = 0u;
}


// append new entry
void ImageDatabase::AddImage(ImgDataRecord& img)
{
	std::vector<uint8> buffer;
	img.Serialize(buffer);

	// add record
	Append(img.path_, buffer);
}


void ImageDatabase::Serialize(const TCHAR* file_path, __int64 file_size, DWORD time_stamp, uint64 offset, std::vector<uint8>& buffer)
{
	// dump arguments to the buffer

}


uint64 ImageDatabase::FindImage(const String& file_path, ImgDataRecord& img)
{
	DatabaseLocker lock(*this);

	try
	{
		std::vector<uint8> record;

		if (uint64 next= NextRecordOffset())
		{
			Read(next, record);
			img.Construct(record);
			if (file_path == img.path_)
				return next;
		}
//		if (ReadNext(record))
		//{
		//	img.Construct(record);
		//	if (file_path == img.path_)
		//		return true;
		//}

		String key_found;
		uint64 offset= FindRecord(file_path, key_found);
		if (offset == 0 || key_found != file_path)
			return 0;

		// read record
		Read(offset, record);

		img.Construct(record);

		return offset;
	}
	catch (int err)
	{
		// db error -> regenerate

#ifdef _DEBUG
		AfxMessageBox(_T("数据库遇到错误, 正在清空数据库文件."), MB_OK);
#endif

		err = err;

		Close();

		RemoveDbFilesDirect(db_file_path_);

		return 0;
	}

	return 0;
}


uint64 ImageDatabase::AddModify(const ImgDataRecord& img)
{
	DatabaseLocker lock(*this);

	// look for 'img' in db; if present try to update it in-place
	String key_found;
	uint64 offset= FindRecord(img.path_, key_found);

	std::vector<uint8> buffer;
	img.Serialize(buffer);

	if (offset != 0 && key_found == img.path_)	// record found?
	{
		// record exists and will be updated
		uint32 existing_rec_size= ReadRecordSize(offset);

		if (buffer.size() <= existing_rec_size) // enough space?
		{
			// in-place update: same name, so no index update necessary
			Update(offset, img.path_, buffer);

			// verification
#ifdef _DEBUG
			{
				ImgDataRecord rec;
				uint64 o= FindImage(img.path_, rec);
			}
#endif
			return offset;
		}

		// cannot update in place, so remove it
		DeleteRecord(offset);
	}

	// just append a new one
	uint64 new_offset= Append(img.path_, buffer);

	// verification
#ifdef _DEBUG
	{
		ImgDataRecord rec;
		uint64 o= FindImage(img.path_, rec);
	}
#endif

	return new_offset;
}

/*
String ImageDatabase::CreateDbFolderIfNeeded()
{
	Path dir= _T("c:\\");

	ITEMIDLIST* pidl= 0;
	if (::SHGetSpecialFolderLocation(0, CSIDL_COMMON_APPDATA, &pidl) == NOERROR && pidl != 0)
	{
		ItemIdList folder(pidl, false);
		dir.assign(folder.GetPath());
	}
	else if (::SHGetSpecialFolderLocation(0, CSIDL_APPDATA, &pidl) == NOERROR && pidl != 0)
	{
		ItemIdList folder(pidl, false);
		dir.assign(folder.GetPath());
	}

	dir.AppendDir(_T("MiK\\ExifPro"), false);
	if (!dir.CreateFolders())
		return _T("");

	return CreateDbFolderIfNeeded(dir);
} */


String ImageDatabase::CreateDbFolderIfNeeded(String path)
{
	if (path.empty())
	{
		ASSERT(false);
		return String();
	}

	Path dir(path);

	// those names are mirrored in the Uninstaller project
#ifdef _UNICODE
	#ifdef _WIN64
		dir.AppendDir(_T("CacheDb64.bin"), false);
	#else
		dir.AppendDir(_T("CacheDb.bin"), false);
	#endif
#else
	dir.AppendDir(_T("CacheDbA.bin"), false);
#endif

	return dir;
}


// delete db files
bool ImageDatabase::RemoveDbFiles(String dir)
{
//	Path path= CreateDbFolderIfNeeded(dir);
	return RemoveDbFilesDirect(dir);
}


bool ImageDatabase::RemoveDbFilesDirect(const Path& file_path)
{
	if (!file_path.FileExists())
		return true;	// already gone

	if (::DeleteFile(file_path.c_str()) == 0)
		return false;

	::DeleteFile(GetIndexName(file_path).c_str());

	return true;
}


bool ImageDatabase::HaveDbFiles(String dir)
{
	Path path= CreateDbFolderIfNeeded(dir);
	return path.FileExists();
}


// close db files
void ImageDatabase::Close()
{
	Database::Close();
}


void ImageDatabase::LimitFileLength(const String& file_path, uint64 db_file_length_limit_)
{
	uint64 size= static_cast<const Path&>(file_path).GetFileLength();

	if (size >= db_file_length_limit_)
	{
		::DeleteFile(file_path.c_str());
		::DeleteFile(GetIndexName(file_path).c_str());
	}
}


bool ImageDatabase::ReadImageAt(uint64 record_offset, ImgDataRecord& img)
{
	if (record_offset == 0)
	{
//		ASSERT(false);
		return false;	// no record?
	}

	std::vector<uint8> rec;
	Read(record_offset, rec);

	img.Construct(rec);

	return true;
}


void ImageDatabase::Flush()
{
	Database::Flush();
}


bool ImageDatabase::ReadNextRecord(std::vector<uint8>& record)
{
	return ReadNext(record);
}


uint64 ImageDatabase::AppendRecord(const std::vector<uint8>& record)
{
	return Append(String(), record);
}


void ImageDatabase::UpdateRecordAt(uint64 record_offset, const std::vector<uint8>& record)
{
	Update(record_offset, record);
}


void ImageDatabase::DeleteImage(uint64 record_offset)
{
	DatabaseLocker lock(*this);
	DeleteRecord(record_offset);
}


String ImageDatabase::GetDbFileAndPath() const
{
	return db_file_path_;
}

/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// ImageDatabase.h: interface for the ImageDatabase class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IMAGEDATABASE_H__079EF87B_5A65_4142_AF5B_D52AEA998A5E__INCLUDED_)
#define AFX_IMAGEDATABASE_H__079EF87B_5A65_4142_AF5B_D52AEA998A5E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "Database.h"
#include "StorageFile.h"
#include <vector>

// prevent multiple ExifPro instances from running (one db only)
class Path;


class ImgDataRecord
{
public:
	ImgDataRecord();

	void Construct(const std::vector<uint8>& record);

	void Serialize(std::vector<uint8>& buffer) const;

	void Clear();

	String path_;
	uint64 file_size_;
	FILETIME time_stamp_;		// last write time stamp
	FILETIME access_time_;		// last access time stamp
	std::vector<uint8> bufEXIF_;
	std::vector<uint8> buf_thumbnail_;
	std::vector<uint8> buf_index_;
	uint32 img_width_;
	uint32 img_height_;
	uint32 jpeg_offset_;
	uint32 jpeg_img_size_;
	enum { REGENERATED_THUMBNAIL= 0x01, BROKEN_PHOTO= 0x02 };
	uint32 flags_;

	// copy of EXIF orientation (for CRW files)
	uint32 exif_orientation_;

	// keywords are stored in JPEG app marker;
	// description as well
	// rotation flags as well

private:
	uint32 GetSize() const;
};



class ImageDatabase : Database
{
public:
	ImageDatabase();
	virtual ~ImageDatabase();

	// open image database; version number must match or else it will be recreated with a new version number;
	// changes to image data record require bumping up this version to force cache rebuild
	bool OpenDb(const String& db_path, bool read_only, uint32 db_version= 9);

	// find an entry for a given file
	uint64 FindImage(const String& file_path, ImgDataRecord& img);

	// remove given record
	void DeleteImage(uint64 record_offset);

	// append new entry
	void AddImage(ImgDataRecord& img);

	// modify (update) existing entry
	void ModifyImage(const TCHAR* file_path, __int64 file_size, DWORD time_stamp, ImgDataRecord& img);

	// update record; check if it already exists; return offset to it
	uint64 AddModify(const ImgDataRecord& img);

	// is db file open?
	bool IsOpen() const;

	// find placement for db folder; create it if it doesn't exist
	static String CreateDbFolderIfNeeded(String path);

	// close db files
	void Close();

	// flush files
	void Flush();

	// limit db length (when it exceeds the limit it gets *deleted* and will be rebuilt)
	void LimitFileLength(const String& file_path, uint64 db_file_length_limit_);

	// delete db files
	static bool RemoveDbFiles(String dir);

	// true if db files are present
	bool HaveDbFiles(String dir);

	// read img record
	bool ReadImageAt(uint64 record_offset, ImgDataRecord& img);

	// file name and path of a db that has been opened
	String GetDbFileAndPath() const;

	// low level record access ---------------------

	// try to read next record
	bool ReadNextRecord(std::vector<uint8>& record);
	// append record of any type
	uint64 AppendRecord(const std::vector<uint8>& record);
	// modify record at given offset (it cannot be longer than record at this offset)
	void UpdateRecordAt(uint64 record_offset, const std::vector<uint8>& record);

private:
	// images and EXIF data are kept in here
	StorageFile file_data_;
	String db_file_path_;

	// save image data into the buffer
	void Serialize(const TCHAR* file_path, __int64 file_size, DWORD time_stamp, uint64 offset, std::vector<uint8>& buffer);

//	static String CreateDbFolderIfNeeded();
	static bool RemoveDbFilesDirect(const Path& file_path);
};


#endif // !defined(AFX_IMAGEDATABASE_H__079EF87B_5A65_4142_AF5B_D52AEA998A5E__INCLUDED_)

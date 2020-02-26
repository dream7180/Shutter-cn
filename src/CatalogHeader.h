/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
#include "intrusive_ptr.h"
#include <deque>
//class MemPointer;


struct CatalogHeader
{
	CatalogHeader();

	//===================================================================================

	uint32 version_;
	uint32 img_count_;			// total amount of images in the catalog
	FILETIME creation_time_;	// catalog creation time (UTC)
	std::vector<bool> scan_types_;	// what types were scanned (FT_*)
	std::wstring title_;
	std::wstring description_;

	// type of a drive (that has been scanned) as returned by GetDriveType
	uint8 drive_type_;

	// scanned drive (1) or a folder (0)
	uint8 drive_or_folder_;

	std::wstring volume_name_;	// of scanned drive (may come in handy when retrieving images)
	uint32 volume_flags_;
	uint32 volume_serial_no_;

	struct Dir : public mik::counter_base
	{
		int32 id_;				// unique id given by directory scanner (1 for root, > 1 for subfolders)
		std::wstring name_;		// this folder name (not a path) unless it's root (then it's an abs. path)
		//TODO: date/time?
		uint32 attributes_;		// folder attributes
		std::deque<mik::intrusive_ptr<Dir> > subdirs_;	// all sub folders (but no files)

		size_t reserved_capacity_;	// this is only needed during Dir construction
		std::vector<uint64> records_;	// offsets to records containing images from this folder

		Dir(const wchar_t* dir, int id, uint32 attribs) : id_(id), name_(dir), attributes_(attribs)
		{
			reserved_capacity_ = 0;
		}
		bool IsRoot() const		{ return id_ == 1; }
	};

	typedef mik::intrusive_ptr<Dir> DirPtr;

	DirPtr directory_;			// directory structure

	uint64 statistics_record_offset_;

	//===================================================================================

	void Read(const std::vector<uint8>& header);
	void Write(std::vector<uint8>& header) const;
};

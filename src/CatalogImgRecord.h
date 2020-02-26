/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
#include "IPTCRecord.h"
#include "ExifBlock.h"


struct CatalogImgRecord
{
public:
	CatalogImgRecord();

	void Construct(const std::vector<uint8>& record);

	void Serialize(std::vector<uint8>& buffer, uint16 rec_version) const;

	void Clear();

	std::wstring path_;
	uint64 file_size_;
	FILETIME time_stamp_;		// last write time stamp
	FILETIME access_time_;		// last access time stamp
	FILETIME creation_time_;
	uint32 file_attribs_;

	std::wstring make_;
	std::wstring model_;

	bool has_exif_;
	//enum ExifType { UNKNOWN= 0, RAW_II_EXIF_BLOCK, RAW_MM_EXIF_BLOCK, JPEG_EXIF_BLOCK } exif_type_;
	//int32 exif_ifd_offset_;	// could be negative
	//std::vector<uint8> exif_;

	ExifBlock exif_;

	// EXIF orientation (preserved separately; important for some raw formats)
	uint16 orientation_;

	std::vector<uint8> thumbnail_;
	std::vector<uint8> preview_;

	std::vector<uint8> index_;

	uint32 photo_width_;	// size of original photo
	uint32 photo_height_;

	uint32 img_width_;		// size of image stored here
	uint32 img_height_;

	enum { REGENERATED_THUMBNAIL= 0x01, BROKEN_PHOTO= 0x02 };
	uint32 flags_;

	// copy of EXIF orientation (for CRW files)
	uint32 exif_orientation_;

	// IPTC
	std::auto_ptr<IPTCRecord> iptc_;

	// description as well
	std::wstring description_;

	// rotation flags as well
	uint16 rotation_;

	// photo info type (FT_*)
	uint16 type_;

	// index to the file marker displayed at the corner of thumbnail image
	uint16 marker_index_;

	// id matching dir id in the catalog directory
	uint32 dir_visited_;

private:
	size_t GetSize() const;
	void Init();
	static uint32 CalcRecordCRC(const std::vector<uint8>& buffer);

	CatalogImgRecord(const CatalogImgRecord&);
	CatalogImgRecord& operator = (const CatalogImgRecord&);
};

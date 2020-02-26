/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "CatalogImgRecord.h"
#include "MemPointer.h"
#include <boost/bind.hpp>
#include <boost/crc.hpp>      // for boost::crc_basic, boost::crc_optimal
#include "Exception.h"


CatalogImgRecord::CatalogImgRecord()
{
	// initialize all
	Init();
}


void CatalogImgRecord::Init()
{
	file_size_ = ~0;
	time_stamp_.dwHighDateTime = time_stamp_.dwLowDateTime = ~0;
	access_time_.dwHighDateTime = access_time_.dwLowDateTime = ~0;
	creation_time_.dwHighDateTime = creation_time_.dwLowDateTime = ~0;
	file_attribs_ = ~0;
	orientation_ = ~0;
	has_exif_ = false;
	img_width_ = img_height_ = ~0;
	flags_ = 0;
	exif_orientation_ = ~0;
	rotation_ = ~0;
	type_ = ~0;
	marker_index_ = ~0;
	dir_visited_ = ~0;
//	exif_type_ = UNKNOWN;
//	exif_ifd_offset_ = 0;
	exif_.clear();
}


size_t CatalogImgRecord::GetSize() const	// this is only estimation!
{
	size_t size= 0;

	//const uint32 keys= keywords_.size();
	//for (uint32 i= 0; i < keys; ++i)
	//	size += 4 + keywords_[i].size() * sizeof(uint16);

	return size +
		4 + // CRC32
		4 + path_.size() * sizeof(uint16) +
		2 + // version
		sizeof(file_size_) +
		sizeof(time_stamp_) +
		sizeof(access_time_) +
		sizeof(creation_time_) +
		sizeof(file_attribs_) +
		4 + make_.size() * sizeof(uint16) +
		4 + model_.size() * sizeof(uint16) +
		sizeof(orientation_) +
		1 +	// has_exif_
//		4 + // exif_type_
//		sizeof(exif_ifd_offset_) +
		24 + // exif block
		4 + exif_.exif_buffer.size() +
		4 + thumbnail_.size() +
		4 + preview_.size() +
		4 + index_.size() +
		(iptc_.get() ? 200 : 1) + // iptc_ guesstimation
		sizeof(img_width_) +
		sizeof(img_height_) +
		sizeof(flags_) +
		sizeof(exif_orientation_) +
		4 + description_.size() * sizeof(uint16) +
		sizeof(rotation_) +
		sizeof(type_) +
		sizeof(marker_index_) +
		sizeof(dir_visited_) +
		8 * 4; // more later
}


static void ResizeBuffer(std::vector<uint8>* buffer, size_t size, MemPointer* ptr)
{
	buffer->resize(size);
	ptr->Resize(&buffer->front(), buffer->size());
}


uint32 CalcCRC32(const uint8* data, size_t size)
{
    boost::crc_32_type crc;
    crc = std::for_each(data, data + size, crc);
	return crc();
}


uint32 CatalogImgRecord::CalcRecordCRC(const std::vector<uint8>& buffer)
{
	if (buffer.size() < 4)
		return ~0;

	const uint8* data= &buffer.front() + 4;
	size_t length= buffer.size() - 4;

	return CalcCRC32(data, length);
}


// save image data into the buffer
void CatalogImgRecord::Serialize(std::vector<uint8>& buffer, uint16 rec_version) const
{
	size_t size= GetSize();
	buffer.resize(size);	// estimation only
	MemPointer ptr(&buffer.front(), buffer.size());
	ptr.SetResizeCallback(boost::bind(&ResizeBuffer, &buffer, _1, &ptr));
	ptr.SetByteOrder(false);	// little endian

	// CRC
	ptr.PutUInt32(0);

	ptr.WriteWString(path_);
	ptr.WriteWString(make_);
	ptr.WriteWString(model_);

	ptr.PutUInt16(rec_version);	// version

	ptr.PutUInt32(static_cast<uint32>(file_size_ >> 32));
	ptr.PutUInt32(static_cast<uint32>(file_size_ & 0xffffffff));
	ptr.PutUInt32(time_stamp_.dwHighDateTime);
	ptr.PutUInt32(time_stamp_.dwLowDateTime);
	ptr.PutUInt32(access_time_.dwHighDateTime);
	ptr.PutUInt32(access_time_.dwLowDateTime);
	ptr.PutUInt32(creation_time_.dwHighDateTime);
	ptr.PutUInt32(creation_time_.dwLowDateTime);
	ptr.PutUInt32(file_attribs_);
	ptr.PutUInt16(orientation_);

	ptr.PutChar(has_exif_ ? 1 : 0);
	if (has_exif_)
	{
		ptr.PutChar(exif_.bigEndianByteOrder ? 1 : 0);
		ptr.PutUInt32(exif_.offsetExifMarkerBlock);
		ptr.PutUInt32(exif_.exifBlockSize);
		ptr.PutUInt32(exif_.ifd0Start);
		ptr.PutUInt16(exif_.ifd0Entries);
		ptr.PutUInt32(exif_.offsetIfd0Entries);
		ptr.PutUInt32(exif_.ifd1Start);
		ptr.WriteBufWithSize(exif_.exif_buffer);
		ptr.PutChar(exif_.is_raw ? 1 : 0);
//	bool bigEndianByteOrder;
//	uint32 offsetExifMarkerBlock;
//	uint32 exifBlockSize;			// size including app marker (2 bytes) and size field (2 bytes)
//	uint32 ifd0Start;
//	uint16 ifd0Entries;
//	uint32 offsetIfd0Entries;
//	uint32 ifd1Start;
//	std::vector<uint8> exif_buffer;
//	bool is_raw;
		//ptr.PutUInt32(exif_type_);
		//ptr.PutUInt32(exif_ifd_offset_);
		//ptr.WriteBufWithSize(exif_);
	}

	ptr.WriteBufWithSize(thumbnail_);
	ptr.WriteBufWithSize(preview_);
	ptr.WriteBufWithSize(index_);

	ptr.PutChar(iptc_.get() ? 1 : 0);	// IPTC present?
	if (iptc_.get())
		iptc_->Write(ptr);

	ptr.PutUInt32(photo_width_);
	ptr.PutUInt32(photo_height_);

	ptr.PutUInt32(img_width_);
	ptr.PutUInt32(img_height_);

	ptr.PutUInt32(flags_);
	ptr.PutUInt32(exif_orientation_);

	ptr.WriteWString(description_);

	ptr.PutUInt16(rotation_);
	ptr.PutUInt16(type_);
	ptr.PutUInt16(marker_index_);
	ptr.PutUInt32(dir_visited_);

	// reserved
	ptr.PutUInt32(0);
	ptr.PutUInt32(0);
	ptr.PutUInt32(0);
	ptr.PutUInt32(0);
	ptr.PutUInt32(0);
	ptr.PutUInt32(0);
	ptr.PutUInt32(0);
	ptr.PutUInt32(0);

	// resize buffer to the proper size (the one really used) and pad it to 4 byte boundary (padding's not required)
	buffer.resize((ptr.GetPos() + 3) & ~3);

	ptr.Reset(&buffer.front(), buffer.size(), 0);
	ptr.PutUInt32(CalcRecordCRC(buffer));
}


// reconstruct record from memory buffer
void CatalogImgRecord::Construct(const std::vector<uint8>& record)
{
	MemPointer ptr(const_cast<uint8*>(&record.front()), record.size());
	ptr.SetByteOrder(false);	// little endian

	uint32 crc= ptr.GetUInt32();

	if (record.size() <= 4 || CalcRecordCRC(record) != crc)
		throw UserException(L"Cannot read catalog file.", L"Checksum error while reading catalog record.");

	ptr.ReadWString(path_);
	ptr.ReadWString(make_);
	ptr.ReadWString(model_);

	uint16 version= ptr.GetUInt16();

	file_size_ = uint64(ptr.GetUInt32()) << 32;
	file_size_ |= ptr.GetUInt32();

	time_stamp_.dwHighDateTime = ptr.GetUInt32();
	time_stamp_.dwLowDateTime = ptr.GetUInt32();
	access_time_.dwHighDateTime = ptr.GetUInt32();
	access_time_.dwLowDateTime = ptr.GetUInt32();
	creation_time_.dwHighDateTime = ptr.GetUInt32();
	creation_time_.dwLowDateTime = ptr.GetUInt32();
	file_attribs_ = ptr.GetUInt32();
	orientation_ = ptr.GetUInt16();

	has_exif_ = false;
	if (ptr.GetUInt8())
	{
		exif_.bigEndianByteOrder = ptr.GetUInt8() != 0;
		exif_.offsetExifMarkerBlock = ptr.GetUInt32();
		exif_.exifBlockSize = ptr.GetUInt32();
		exif_.ifd0Start = ptr.GetUInt32();
		exif_.ifd0Entries = ptr.GetUInt16();
		exif_.offsetIfd0Entries = ptr.GetUInt32();
		exif_.ifd1Start = ptr.GetUInt32();
		ptr.ReadBufWithSize(exif_.exif_buffer);
		exif_.is_raw = ptr.GetUInt8() != 0;
/*
		exif_type_ = static_cast<ExifType>(ptr.GetUInt32());
		exif_ifd_offset_ = ptr.GetUInt32();
		ptr.ReadBufWithSize(exif_); */
		has_exif_ = true;
	}

	ptr.ReadBufWithSize(thumbnail_);
	ptr.ReadBufWithSize(preview_);
	ptr.ReadBufWithSize(index_);

	if (ptr.GetUInt8())
	{
		iptc_.reset(new IPTCRecord);
		iptc_->Read(ptr);
	}

	photo_width_ = ptr.GetUInt32();
	photo_height_ = ptr.GetUInt32();

	img_width_ = ptr.GetUInt32();
	img_height_ = ptr.GetUInt32();

	flags_ = ptr.GetUInt32();
	exif_orientation_ = ptr.GetUInt32();

	ptr.ReadWString(description_);

	rotation_ = ptr.GetUInt16();
	type_ = ptr.GetUInt16();
	marker_index_ = ptr.GetUInt16();
	dir_visited_ = ptr.GetUInt32();
}


void CatalogImgRecord::Clear()
{
	Init();
	path_.clear();
	exif_.clear();
	thumbnail_.clear();
	preview_.clear();
	index_.clear();
	iptc_.reset();
	description_.clear();
}

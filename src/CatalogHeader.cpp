/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "CatalogHeader.h"
#include "MemPointer.h"
#include <boost/crc.hpp>      // for boost::crc_basic, boost::crc_optimal
#include "Exception.h"


CatalogHeader::CatalogHeader()
{
	version_ = 0;
	img_count_ = 0;
	creation_time_.dwLowDateTime = 0;
	creation_time_.dwHighDateTime = 0;
	drive_type_ = 0;
	drive_or_folder_ = 0;
	volume_flags_ = 0;
	volume_serial_no_ = 0;
}


static size_t GetDirSize(const CatalogHeader::Dir* dir)
{
	size_t size= sizeof(dir->id_);
	size += sizeof(uint32);	// str len
	size += dir->name_.length() * sizeof(dir->name_[0]);
	size += sizeof(dir->attributes_);
	size += sizeof(uint32);	// records_ len
	size += dir->reserved_capacity_ * sizeof(dir->records_[0]);

	const size_t count= dir->subdirs_.size();
	size += sizeof(uint32); // count
	for (size_t i= 0; i < count; ++i)
		size += GetDirSize(dir->subdirs_[i].get());

	return size;
}


static void DumpDir(const CatalogHeader::Dir* dir, MemPointer& p)
{
	p.PutUInt32(dir->id_);
	p.WriteWString(dir->name_);
	p.PutUInt32(dir->attributes_);
	{
		// save offsets to image records; real number of records can be smaller than reserved
		// number; output always reserved number to make physical record large enough for
		// subsequent update operation(s) (so it can be updated in-place)
		p.PutUInt32(dir->reserved_capacity_);
		const size_t count= dir->records_.size();
		for (size_t i= 0; i < count; ++i)
			p.PutUInt64(dir->records_[i]);
		for (size_t i= count; i < dir->reserved_capacity_; ++i)
			p.PutUInt64(0);
	}
	const size_t count= dir->subdirs_.size();
	p.PutUInt32(count);
	for (size_t i= 0; i < count; ++i)
		DumpDir(dir->subdirs_[i].get(), p);
}


CatalogHeader::DirPtr ReadDir(MemPointer& p)
{
	uint32 id= p.GetUInt32();
	std::wstring name;
	p.ReadWString(name);
	uint32 attribs= p.GetUInt32();

	CatalogHeader::DirPtr dir= new CatalogHeader::Dir(name.c_str(), id, attribs);

	{
		const uint32 count= p.GetUInt32();
		dir->records_.resize(count, 0);
		for (size_t i= 0; i < count; ++i)
			dir->records_[i] = p.GetUInt64();
	}

	const uint32 count= p.GetUInt32();
	dir->subdirs_.resize(count);
	for (uint32 i= 0; i < count; ++i)
		dir->subdirs_[i] = ReadDir(p);

	return dir;
}


void CatalogHeader::Read(const std::vector<uint8>& header)
{
	MemPointer ptr(const_cast<uint8*>(&header.front()), header.size());
	ptr.SetByteOrder(false);	// little endian

	if (header.size() <= 4)
		throw UserException(L"Cannot read catalog file.", L"Catalog file header is corrupted.");

	boost::crc_32_type crc;
	const uint8* data= &header.front();
    crc = std::for_each(data + 4, data + header.size() - 4, crc);

	if (crc() != ptr.GetUInt32())
		throw UserException(L"Catalog file is corrupted.", L"Checksum error while reading catalog header.");

	version_ = ptr.GetUInt32();
	img_count_ = ptr.GetUInt32();
	creation_time_.dwHighDateTime = ptr.GetUInt32();
	creation_time_.dwLowDateTime = ptr.GetUInt32();

	const size_t size= ptr.GetUInt16();
	scan_types_.resize(size, false);
	for (size_t i= 0; i < size; ++i)
		scan_types_[i] = ptr.GetUInt8() != 0;

	ptr.ReadWString(title_);
	ptr.ReadWString(description_);

	// type of drive scanned
	drive_type_ = ptr.GetUInt8();

	drive_or_folder_ = ptr.GetUInt8();

	// drive volume name
	ptr.ReadWString(volume_name_);

	uint32 tag= ptr.GetUInt32();	// dir start marker
	ASSERT(tag == 'dir{');

	directory_ = ReadDir(ptr);

	tag = ptr.GetUInt32();			// dir end marker
	ASSERT(tag == '}dir');

	statistics_record_offset_ = ptr.GetUInt64();
}


void CatalogHeader::Write(std::vector<uint8>& header) const
{
	ASSERT(directory_.get());
	Dir* dir= directory_.get();

	// (over)estimated header size (if it's too small mem ptr will throw an exception)
	std::vector<uint8> buffer(200 + 2 * (volume_name_.length() + title_.length() + description_.length()) + GetDirSize(dir), 0);
	MemPointer p(&buffer[0], buffer.size());
	p.SetByteOrder(false);	// little endian

	p.PutUInt32(0);		// CRC32
	p.PutUInt32(1);		// version no
	p.PutUInt32(img_count_);	// no of images

	FILETIME ft;
	::GetSystemTimeAsFileTime(&ft);
	// creation time (UTC time)
	p.PutUInt32(ft.dwHighDateTime);
	p.PutUInt32(ft.dwLowDateTime);

	// scanned img types
	const size_t size= scan_types_.size();
	p.PutUInt16(static_cast<uint16>(size));
	for (size_t i= 0; i < size; ++i)
		p.PutChar(scan_types_[i] ? 1 : 0);

	// title of the catalog
	p.WriteWString(title_);

	// description
	p.WriteWString(description_);

	// type of drive scanned
	p.PutChar(drive_type_);

	p.PutChar(drive_or_folder_);

	// drive volume name
	p.WriteWString(volume_name_);

	// tag to mark dir start (not really needed)
	p.PutUInt32('dir{');

	// directory structure
	DumpDir(dir, p);

	// tag to mark dir end (not really needed)
	p.PutUInt32('}dir');

	// record with catalog statistics
	p.PutUInt64(statistics_record_offset_);

	p.PutUInt32(0);		// reserved
	p.PutUInt32(0);		// reserved
	p.PutUInt32(0);		// reserved
	p.PutUInt32(0);		// reserved
	p.PutUInt32(0);		// reserved
	p.PutUInt32(0);		// reserved
	p.PutUInt32(0);		// reserved
	p.PutUInt32(0);		// reserved

	header.resize(p.RPosition());
	memcpy(&header.front(), &buffer.front(), p.RPosition());

	p.Reset(&header.front(), header.size(), 0);

	// calc CRC
    boost::crc_32_type crc;
	uint8* data= &header.front();
    crc = std::for_each(data + 4, data + header.size() - 4, crc);

	p.PutUInt32(crc());
}

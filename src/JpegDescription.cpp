/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "FileGuard.h"
#include "PhotoInfo.h"
#include "PhotoAttr.h"
#include "PhotoAttrAccess.h"
#include "SEException.h"
#include "MemPointer.h"
#include "StringConversions.h"
#include "Markers.h"
#include "ReadMarker.h"

#if 0	// jpeg comment field is not used

bool ModifyJpegComment(const String& file_path, const wstring& description)
{
	MemMappedFile photo;
	if (!photo.CreateWriteView(file_path.c_str(), 0))
		return false;

	MemPointer mem(photo.GetBaseAddrChr(), photo.GetFileSize());

	if (ReadMarker(mem) != MARK_SOI)		// not a JPEG image?
		return false;

	uint16 marker= 0;
	int before_marker_pos= 0;

	for (;;)
	{
		before_marker_pos = mem.GetPos();

		marker = ReadMarker(mem);

		if (marker < MARK_APP0 || marker >= MARK_APP15)
			break;

		uint16 len= mem.GetUInt16();
		if (len < 2)
			return false;		// bogus data--wrong size

		mem += len - 2;		// skip marker data
	}

	int existing_com_size= 0;

	if (marker == MARK_COM)		// COM already present?
	{
		existing_com_size = mem.GetPos() - before_marker_pos;

		int len= mem.GetUInt16();
		if (len < 2)
			return false;		// bogus data--wrong size

		existing_com_size += len;
	}
	else if (marker == 0)
	{
		ASSERT(false);	// expected valid marker--bogus jpeg
		return false;
	}

	mem.SetPos(before_marker_pos);

	string str;
	if (!WideStringToMultiByte(description, str) && !description.empty())
		return false;

	int str_len= str.length();
	if (str_len > 0xfffc)
	{
		ASSERT(false);
		str_len = 0xfffc;
	}

	// 2 for COM + 2 for size
	int bytes_to_insert = 2 + 2 + str_len - existing_com_size;

	if (bytes_to_insert > 0)
	{
		// increase file size

		if (!photo.CreateWriteView(file_path.c_str(), bytes_to_insert))
			return false;

		mem.Reset(photo.GetBaseAddrChr(), photo.GetFileSize(), before_marker_pos);

		// move file contents to make space
		int length= photo.GetFileSize() - before_marker_pos;
		ASSERT(length > 0);
		memmove(mem.GetPtr() + bytes_to_insert, mem.GetPtr(), length);
	}
	else if (bytes_to_insert < 0)
	{
		// move file contents to remove some existing comment's bytes
		int length= photo.GetFileSize() - (before_marker_pos + existing_com_size);
		ASSERT(length > 0);
		memmove(mem.GetPtr() + existing_com_size + bytes_to_insert, mem.GetPtr() + existing_com_size, length);
	}

	// COM marker
	mem.PutUInt16(MARK_COM);
	mem.PutUInt16(2 + str_len);

	memcpy(mem.GetPtr(), str.data(), str_len);

	// truncate file?
	if (bytes_to_insert < 0)
	{
		int new_file_length= photo.GetFileSize() + bytes_to_insert;
		photo.SetFileSize(new_file_length);
	}

	photo.CloseFile();

	return true;
}

// This function inserts description as app6 marker or just updates its content
//
bool InsertUserDescHelper(PhotoInfo& inf, const wstring& description, bool silent, CWnd* parent)
{
	// open photo attrib
	PhotoAttrAccess access(inf.GetPhysicalPath(), parent);

	if (!access.Open(silent) || access.GetAttribPtr() == 0)
		return false;

	PhotoAttr* attr= access.GetAttribPtr();

	if (!attr->IsValid())
		return false;

	// store description
	attr->FillDescription(description);

	// modify access time
	access.Touch();

	access.Close();

	return true;
}


bool InsertUserDesc(PhotoInfo& inf, const wstring& description, bool silent, CWnd* parent)
{
	String path= inf.GetPhysicalPath();
	if (path.empty())
		return false;

	// create backup of original file
	FileGuard copy;

	if (!copy.BackUp(path.c_str(), parent))
		return false;

	try
	{
		bool result= InsertUserDescHelper(inf, description, silent, parent);

		if (result)
			ModifyJpegComment(path, description);

		copy.DeleteBackup();

		return result;
	}
	catch (SEException&)
	{}
	catch (...)
	{}

	// writing description failed: restore modified file
	if (copy.Restore(parent)) // restored?
	{
		// original file restored; warn about failed InsertUserDescHelper()
		CString msg;
		msg.Format(_T("An attempt to modify file %s failed."), path.c_str());
		parent->MessageBox(msg, 0, MB_OK | MB_ICONERROR);
	}

	return false;
}
#endif


///////////////////////////////////////////////////////////////////////////////


/*
// This function inserts UserDescription tag into EXIF data if there is no one yet.
// Then it stores description in the newly inserted area.
//
bool InsertUserDesc(PhotoInfo& inf, const wstring& description)
{
	if (!inf.exif_data_present_ || inf.user_desc_offset_ != 0)
		return false;

	const int ENTRY_SIZE= 2 + 2 + 4 + 4;		// one IFD entry size
	const int DESC_CHARS= 512;					// 512 unicodes (1024 bytes)
	const int DESC_SIZE= 8 + DESC_CHARS * 2;	// 8 bytes of description format + 1024 of data
	const int BYTES_TO_ADD= ENTRY_SIZE + DESC_SIZE;

	MemMappedFile photo;
	if (!photo.CreateView(inf.path_.c_str(), BYTES_TO_ADD))
	{
		string msg= "Cannot open photograph\n'" + inf.path_ + "'\nfor writing.";
		AfxMessageBox(msg.c_str(), MB_OK);
		return false;
	}

	uint8* puData= photo.GetBaseAddrChr();
	MemPointer data(puData);

	if (data.GetUInt16() != MARK_SOI)		// not a JPEG image?
		return false;

	uint16 marker= data.GetUInt16();
	if (marker == MARK_APP0)				// app0 marker?
	{
		data += data.GetUInt16() - 2;		// skip app0 data
		marker = data.GetUInt16();
	}

	if (marker != MARK_APP1)				// app1 marker
		return false;

	MemPointer app1_size= data;
	uint16 exif_size= data.GetUInt16();	// app1 data size
	if (exif_size > 0xffff - BYTES_TO_ADD)
		return false;						// not enough place in app1 (not very likely)

	char exif_id[6];
	data.Read(exif_id);					// Exif id
	if (exif_id[4] || exif_id[5] || strcmp(exif_id, "Exif"))
		return false;

	Offset ifd_start= data.RPosition();

	if (data.GetUInt16() == 'II')			// Intel format?
		data.SetByteOrder(false);
	else
		data.SetByteOrder(true);

	data.GetUInt16();						// skip size
	data += data.GetUInt32() - 8;			// offset to first IFD (Image File Directory)

	uint16 entries= data.GetUInt16();		// no of entries in IFD0

	data += (entries - 1) * ENTRY_SIZE;	// go to last entry

	// read last entry
	uint16 tag=		data.GetUInt16();
	uint16 type=		data.GetUInt16();
	uint32 components=	data.GetUInt32();
	uint32 value=		data.GetUInt32();

	if (tag != EXIF_SUB_IFD)				// it should be a link to sub IFD
		return false;

	data.SetPos(ifd_start + value);		// go to sub IFD

	MemPointer p1stIFDDirectory= data;

	entries = data.GetUInt16();			// no of entries in sub IFD

	// place where the user description data will be inserted:
	// right after all the entries
	uint8* insertion_point= puData + data.GetPos() + entries * ENTRY_SIZE;

	int skip= insertion_point - puData;

	// move file contents to make space
	memmove(insertion_point + BYTES_TO_ADD, insertion_point, photo.GetFileSize() - skip);

	// fix all broken entries in current IFD
	for (uint32 i= 0; i < entries; ++i)
	{
		uint16 tag=		data.GetUInt16();
		uint16 type=		data.GetUInt16();
		uint32 components=	data.GetUInt32();
		// store offset position
		MemPointer offset= data;
		uint32 value=		data.GetUInt32();

		// for every entry that has data pointed by the offset fix the offset
		if (Data::HasOffsetData(type, components))
		{
			offset.PutUInt32(value + BYTES_TO_ADD);
		}
	}

	inf.user_desc_offset_ = data.GetPos();

	// now data points to added empty space; add new entry
	data.PutUInt16(EXIF_USER_DESC);
	data.PutUInt16(Data::UndefDataType());	// type of this entry is by EXIF definition UNDEFINED
	data.PutUInt32(DESC_SIZE);			// no of bytes in UserDescription
	int offset= data.GetPos() + 4 - ifd_start; //p1stIFDDirectory.GetPos();
	data.PutUInt32(offset);				// offset to data (data follows this entry)

	// now is one more entry
	p1stIFDDirectory.PutUInt16(entries + 1);
	// increased size of APP1 marker
	app1_size.PutUInt16(exif_size + BYTES_TO_ADD);

	// fill UserDescription
	data.PutChar('U');
	data.PutChar('N');
	data.PutChar('I');
	data.PutChar('C');
	data.PutChar('O');
	data.PutChar('D');
	data.PutChar('E');
	data.PutChar(0);

	// store description
	int str_length= description.size();
	for (int j= 0; j < DESC_CHARS; ++j)
	{
		if (j < str_length)
			data.PutUInt16(description[j]);
		else
			data.PutUInt16(0);
	}

	return true;
}
*/

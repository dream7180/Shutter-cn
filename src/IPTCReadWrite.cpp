/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "IPTCRecord.h"
#include "File.h"
#include "StringConversions.h"
#include "IPTCReadWrite.h"
#include "Markers.h"
#include "ReadMarker.h"
#include "FileGuard.h"
#include "SEException.h"
#include "Exception.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


extern bool IsJpegMarkerToBeSkipped(uint16 marker);
extern const char* g_photoshop= "Photoshop 3.0\0";
static const uint32 g_PS_signature= '8BIM';


String GetIPTCString(FileStream& ifs)
{
	uint32 len= ifs.GetUInt16();

	if (len & 0x8000)
	{
		throw 1;	// not supported (string > 32767 bytes)
/*
		uin32 long_len= 0;
		len &= 0x8000;
		if (len == 2)
			long_len = ifs.GetUInt16();
		else if (len == 4)
			long_len = ifs.GetUInt32();
		else
			return -1; // not supported

		ifs.RPosition(long_len); */
	}

	std::string str;
	if (len > 0)
	{
		std::vector<char> buf(len);
		ifs.Read(&buf.front(), len);
		str.assign(&buf.front(), len);
	}
#if _UNICODE
	return MultiByteToWideString(str, CP_ACP);
#else
	return str;
#endif
}


int ReadIPTC(FileStream& ifs, IPTCRecord& IPTC)
{
	while (ifs.GetUInt32() == g_PS_signature)	// PS marker
	{
		// Photoshop record id
		uint16 PS_record= ifs.GetUInt16();

		// pascal string
		uint16 str_len= ifs.GetUInt8();
		if (str_len == 0)
			str_len = 2;
		else
			str_len = (str_len + 1) & ~1;	// even length
		// skip the string
		ifs.RPosition(str_len - 1);

		// PS record length
		uint32 length= ifs.GetUInt32();

		if (length > 0x00ffffff)			// little endian record (?)
			return 0;						// is an error, bail out

		if (length & 1)
			length++;	// records are padded to even size

		if (PS_record == 0x0404)	// IPTC record?
		{
			while (ifs.GetUInt8() == 0x1c)	// tag marker (always 0x1c)
			{
				uint16 record= ifs.GetUInt16(); // record and data set bytes

				switch (record)			// record/dataset marker
				{
				case 0x219:		// keyword
					IPTC.keywords_.push_back(GetIPTCString(ifs));
					break;

				case 0x250:		// byline
					IPTC.byline_ = GetIPTCString(ifs);
					break;
				case 0x255:		// byline title
					IPTC.byline_title_ = GetIPTCString(ifs);
					break;
				case 0x26e:
					IPTC.credits_ = GetIPTCString(ifs);
					break;
				case 0x273:
					IPTC.source_ = GetIPTCString(ifs);
					break;

				case 0x205:
					IPTC.object_name_ = GetIPTCString(ifs);
					break;
				case 0x237:
					IPTC.date_created_ = GetIPTCString(ifs);
					break;
				case 0x25a:
					IPTC.city_ = GetIPTCString(ifs);
					break;
				case 0x25f:
					IPTC.state_ = GetIPTCString(ifs);
					break;
				case 0x265:
					IPTC.country_ = GetIPTCString(ifs);
					break;
				case 0x267:
					IPTC.original_transmission_reference_ = GetIPTCString(ifs);
					break;

				case 0x274:
					IPTC.copyright_notice_ = GetIPTCString(ifs);
					break;
				case 0x276:
					IPTC.contact_ = GetIPTCString(ifs);
					break;

				case 0x278:
					IPTC.caption_ = GetIPTCString(ifs);
					break;
				case 0x27a:
					IPTC.caption_writer_ = GetIPTCString(ifs);
					break;
				case 0x269:
					IPTC.headline_ = GetIPTCString(ifs);
					break;
				case 0x228:
					IPTC.special_instructions_ = GetIPTCString(ifs);
					break;

				default:
					uint16 len= ifs.GetUInt16();
					if (len & 0x8000)
					{
						uint32 long_len= 0;
						len &= 0x8000;
						if (len == 2)
							long_len = ifs.GetUInt16();
						else if (len == 4)
							long_len = ifs.GetUInt32();
						else
							return -1; // not supported

						ifs.RPosition(long_len);
					}
					else
						ifs.RPosition(len);
					break;
				}
			}

			return 1;
		}
		else
		{
			// skip whole Photoshop record
			ifs.RPosition(length);
		}
	}

	return 0;
}


extern int CanEditIPTCRecord(const TCHAR* file)
{
	DWORD attrib= ::GetFileAttributes(file);

	if (attrib == INVALID_FILE_ATTRIBUTES)
	{
		DWORD err= ::GetLastError();
		if (err = 0)
			err = -1;
		return err;
	}

	if (attrib & FILE_ATTRIBUTE_READONLY)
		return -2;

//	DWORD ::GetFileAttributes(file)
/*
	MemMappedFile photo;

	if (!photo.CreateWriteView(file, 0, true))	// try to open in write mode
	{
//TRACE(L"open file %s in can edit failed\n", file);
		// if it's only access violation lets hope it'll be unlocked later
		if (::GetLastError() == ERROR_SHARING_VIOLATION)
			return ERROR_SHARING_VIOLATION;

		return -1;
	}

	// JPEG img start marker expected
	if (photo.GetBaseAddrChr()[0] != 0xff || photo.GetBaseAddrChr()[1] != 0xd8)
		return -2;
*/
	return 0;
}


extern bool WriteIPTC(MemMappedFile& photo, const TCHAR* file_name, const IPTCRecord& iptc)
{
	// note: it is expected that 'photo' is already open, but view may need to be established
	if (!photo.CreateWriteView(file_name, 0))	// open view
		THROW_EXCEPTION(L"Cannot write IPTC file info", SF(L"Cannot open file: " << file_name))

	MemPointer data(photo.GetBaseAddrChr(), photo.GetFileSize());

	if (ReadMarker(data) != MARK_SOI)		// not a JPEG image?
		return false;

	// check if APP13 is already present
	bool found_marker_13= false;
	for (int i= 0; i < 1000; ++i)	// safety counter
	{
		uint32 pos= data.RPosition();

		uint16 marker= ReadMarker(data);

		if (marker == MARK_APP13)
		{
			// marker found, read ptr is at the size field
			found_marker_13 = true;
			break;
		}

		// looking for app markers only (and make an exception for comment marker)
		if (!IsJpegMarkerToBeSkipped(marker))
		{
			data.SetPos(pos);	// position before marker
			break;
		}

		// data length
		uint16 dataSize= data.GetUInt16();

		if (dataSize < 2)
			return false;	// not a valid length; corrupted JPEG file?

		data += dataSize - 2;	// skip app marker data
	}

	int bytes_to_shorten_the_file= 0;
	bool write_marker_13_data= true;
	int app_marker_size= 0;

	if (found_marker_13)				// existing app13 marker?
	{
		MemPointer marker= data;

		ptrdiff_t offset_to_jpeg_marker_size= data.GetPos();
		app_marker_size = data.GetUInt16();	// JPEG app marker data size (16 bits)
		if (app_marker_size < 2)
			return false;		// invalid length field

		char PS_marker[g_photoshop_len];
		data.Read(PS_marker);

		if (memcmp(PS_marker, g_photoshop, g_photoshop_len) == 0)
		{
			while (data.GetUInt32() == g_PS_signature)	// PS marker
			{
				// Photoshop record id
				uint16 PS_record= data.GetUInt16();

				// pascal string
				uint16 str_len= data.GetUInt8();
				if (str_len == 0)
					str_len = 2;
				else
					str_len = (str_len + 1) & ~1;	// even length
				// skip the string
				data += str_len - 1;

				// PS record length
				uint32 length= data.GetUInt32();

				if (length > 0x00ffffff)			// little endian record (?)
					break;							// is an error, bail out

				if (length & 1)
					length++;	// records are padded to even size

				if (PS_record == 0x0404)	// IPTC record?
				{
					// current offset
					ptrdiff_t offset= data.GetPos();

					// IPTC data size
					size_t IPTC_phys_size= iptc.CalcRecordSize();
					size_t IPTC_size= IPTC_phys_size;
					bool odd_size= !!(IPTC_size & 1);
					if (odd_size)
						IPTC_size++;	// PS requires even size

					int bytes_to_insert= IPTC_size - static_cast<int>(length);

					if (!photo.CreateWriteView(file_name, MAX(bytes_to_insert, 0)))
						THROW_EXCEPTION(L"Cannot write IPTC file info", SF(L"Cannot insert IPTC record in file: " << file_name))

					data.Reset(photo.GetBaseAddrChr(), photo.GetFileSize(), offset);

					if (bytes_to_insert != 0)
					{
						// move file contents to make space or contract it to remove extra space
						int length_to_move= photo.GetFileSize() - offset - length;
						memmove(data.GetPtr() + length + bytes_to_insert, data.GetPtr() + length, length_to_move);
					}

					// change PS record size
					data.UncheckedOffset(-4);
					data.PutUInt32(IPTC_phys_size);

					// copy IPTC structure
					iptc.CopyTo(data.GetPtr());
					data += IPTC_phys_size;

					if (odd_size)
						data.PutChar(0);		// pad

					// change JPEG app marker data size
					data.Reset(photo.GetBaseAddrChr(), photo.GetFileSize(), offset_to_jpeg_marker_size);
					data.PutUInt16(app_marker_size + bytes_to_insert);

					// cache may miss changes without updating date/time stamp
					// update cache outside of this routine

			//		photo.Touch();	// cache may miss changes without updating date/time stamp
			//		photo.CloseFile();

					if (bytes_to_insert < 0)
						bytes_to_shorten_the_file = -bytes_to_insert;

					write_marker_13_data = false;

					break;
				}
			}
		}

		data = marker;
	}

	if (write_marker_13_data)
	{
		if (found_marker_13)
			data -= 2;		// move before APP13

		ptrdiff_t offset= data.GetPos();

		// here new APP13 marker will be added or existing one will be overwritten

		// marker (2 bytes), size (2 bytes), Photoshop header (14 bytes)
		int overhead= 2 + 2 + g_photoshop_len;
		// 8BIM signature (4 bytes), PS record type (2 bytes), empty string name (2 bytes), record size (4 bytes)
		int PS_record_size= 4 + 2 + 2 + 4;
		// IPTC data size
		int IPTS_phys_size= iptc.CalcRecordSize();
		int IPTS_size= IPTS_phys_size;
		bool odd_size= !!(IPTS_size & 1);
		if (odd_size)
			IPTS_size++;	// PS requires even size

		int new_app_marker_size= overhead + PS_record_size + IPTS_size;
		if (app_marker_size > 0)		// does APP13 marker already exist?
			app_marker_size += 2;	// size of marker area including 'size field' itself

		if (!photo.CreateWriteView(file_name, MAX(new_app_marker_size - app_marker_size, 0)))
			THROW_EXCEPTION(L"Cannot write IPTC file info", SF(L"Cannot resize file: " << file_name))

		data.Reset(photo.GetBaseAddrChr(), photo.GetFileSize(), offset);

		// move file contents to make/contract space
		int length= photo.GetFileSize() - offset;
		if (new_app_marker_size > app_marker_size)
		{
			memmove(data.GetPtr() + (new_app_marker_size - app_marker_size), data.GetPtr(), length);
		}
		else if (new_app_marker_size < app_marker_size)
		{
			bytes_to_shorten_the_file = app_marker_size - new_app_marker_size;
			memmove(data.GetPtr() + new_app_marker_size, data.GetPtr() + app_marker_size, length - app_marker_size);
		}

		// add app13 marker
		data.PutUInt16(MARK_APP13);
		data.PutUInt16(new_app_marker_size - 2);

		// copy Photoshot header structure
		memcpy(data.GetPtr(), g_photoshop, g_photoshop_len);
		data += g_photoshop_len;

		data.PutUInt32(g_PS_signature);
		data.PutUInt16(0x0404);	// IPTC record
		data.PutUInt16(0);			// empty string name
		data.PutUInt32(IPTS_phys_size);	// record size (this number may be odd)

		// copy IPTC structure
		iptc.CopyTo(data.GetPtr());
		data += IPTS_phys_size;

		if (odd_size)
			data.PutChar(0);		// pad
	}

	//if (photo.IsOpen())
	//{
	//	photo.Touch();	// cache may miss changes without updating date/time stamp
	//	photo.CloseFile();
	//}

	// shorten photo?
	if (bytes_to_shorten_the_file != 0)
	{
		ASSERT(bytes_to_shorten_the_file > 0);
		//CFile file(file, CFile::modeReadWrite | CFile::modeNoTruncate);
		//file.SetLength(file.GetLength() - bytes_to_shorten_the_file);
		photo.SetFileSize(photo.GetFileSize() - bytes_to_shorten_the_file);
	}

	return true;
}


bool WriteIPTC(const TCHAR* file, const IPTCRecord& iptc)
{
	MemMappedFile photo;
	if (!photo.CreateWriteView(file, 0)) //, BYTES_TO_ADD))
		THROW_EXCEPTION(L"Cannot write IPTC file info", SF(L"Cannot open file: " << file))

	return WriteIPTC(photo, file, iptc);
}

/*
bool WriteIPTCWithBackup(const TCHAR* file, const IPTCRecord& iptc)
{
	// create backup of original file
	FileGuard copy;

	CWnd* parent= 0;
	if (!copy.BackUp(file, parent))
		return false;

	try
	{
		bool result= WriteIPTCHelper(file, iptc);

		copy.DeleteBackup();

		return result;
	}
	catch (SEException&)
	{
	}
	catch (...)
	{
		//parent->MessageBox(_T("Fatal error encountered."), 0, MB_OK | MB_ICONERROR);
	}

	// writing IPTC failed: restore modified file
	if (copy.Restore(parent)) // restored?
	{
		// original file restored; warn about failed WriteIPTC()
		CString msg;
		msg.Format(_T("An attempt to modify file %s failed."), file);
		if (parent)
			parent->MessageBox(msg, 0, MB_OK | MB_ICONERROR);
		else
			throw String(msg);
	}

	return false;
}
*/

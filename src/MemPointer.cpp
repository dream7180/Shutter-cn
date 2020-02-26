/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// MemPointer.cpp: implementation of the MemPointer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MemPointer.h"
#include "StringConversions.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////


void MemPointer::GetString(std::string& buf, uint32 size)
{
	for (uint32 i= 0; i < size; ++i)
		if (char c= GetUInt8())
			buf += c;
		else
			break;
}


void MemPointer::Read(void* data, size_t size)
{
	if (data_ + size > end_)
	{
		Error();
		memset(data, 0, size);
	}
	else
	{
		memcpy(data, data_, size);
		data_ += size;
	}
}


void MemPointer::Write(const void* data, size_t size)
{
	if (data_ + size > end_)
		throw MemPtrException();
	memcpy(data_, data, size);
	data_ += size;
}


void MemPointer::WriteString(const String& str)
{
	PutUInt32(static_cast<uint32>(str.size()));
	for (uint32 i= 0; i < str.size(); ++i)
#ifndef _UNICODE
		PutChar(str[i]);
#else
		PutUInt16(str[i]);
#endif
}


void MemPointer::WriteWString(const std::wstring& str)
{
	PutUInt32(static_cast<uint32>(str.size()));
	for (uint32 i= 0; i < str.size(); ++i)
		PutUInt16(str[i]);
}


void MemPointer::WriteWString(const std::string& str)
{
	PutUInt32(static_cast<uint32>(str.size()));
	for (uint32 i= 0; i < str.size(); ++i)
		PutUInt16(str[i]);
}


void MemPointer::ReadString(String& str)
{
	const size_t count= GetUInt32();
	str.resize(count, _T('\0'));
	//vector<TCHAR> buf(count);
//	if (!buf.empty())
//	{
		for (size_t i= 0; i < count; ++i)
#ifdef _UNICODE
			str[i] = GetUInt16();
#else
			str[i] = GetUInt8();
#endif
//		Read(&buf.front(), buf.size() * sizeof(buf[0]));
//		str.assign(&buf.front(), buf.size());
//	}
//	else
//		str.erase();
}


void MemPointer::ReadWString(std::wstring& str)
{
	const size_t count= GetUInt32();
	str.resize(count, _T('\0'));

	for (size_t i= 0; i < count; ++i)
		str[i] = GetUInt16();

//	ASSERT(sizeof(wchar_t) == 2);
//	vector<wchar_t> buf(GetUInt32(), _T('\0'));
//	if (!buf.empty())
	//{
	//	Read(&buf.front(), buf.size() * sizeof(buf[0]));
	//	str.assign(&buf.front(), buf.size());
	//}
	//else
	//	str.erase();
}


void MemPointer::ReadWString(std::string& str)
{
	std::wstring wstr;
	ReadWString(wstr);
	::WideStringToMultiByte(wstr, str);
}


void MemPointer::WriteBufWithSize(const std::vector<uint8>& data)	// write buffer storing its size too
{
	ASSERT(data.size() <= std::numeric_limits<uint32>::max());

	if (!data.empty())
	{
		PutUInt32(static_cast<uint32>(data.size()));
		MakeSpace(data.size());
		memcpy(GetPtr(), &data.front(), data.size());
		*this += data.size();
	}
	else
		PutUInt32(0);
}


void MemPointer::ReadBufWithSize(std::vector<uint8>& data)		// read back buffer from memory
{
	data.resize(GetUInt32());
	if (!data.empty())
		Read(&data.front(), data.size());
}


uint8 MemPointer::Error() const
{
//	ASSERT(false);

	if (ret_zero_at_err_)
		return 0;
	else
		throw MemPtrException();
}


void MemPointer::MakeSpace(size_t size)
{
	if (data_ + size <= end_)
		return;

	if (resize_)
	{
		size_t new_size= data_ + size - base_;
//		new_size += (new_size + 1) / 2;
		resize_(new_size);
	}

	if (data_ + size > end_)
		throw MemPtrException();
}


void MemPointer::Resize(uint8* data, size_t size)
{
	ptrdiff_t pos= GetPos();
	base_ = data;
	end_ = data + size;
	data_ = base_ + pos;	// restore position
}


void MemPointer::PutUInt64(uint64 val)
{
	MakeSpace(sizeof val);
	uint8 buf[sizeof val]=
	{ uint8(val >> 0x38), uint8(val >> 0x30), uint8(val >> 0x28), uint8(val >> 0x20),
	uint8(val >> 0x18), uint8(val >> 0x10), uint8(val >> 0x08), uint8(val) };
	if (big_endian_)
	{
		data_[0] = buf[0], data_[1] = buf[1], data_[2] = buf[2], data_[3] = buf[3];
		data_[4] = buf[4], data_[5] = buf[5], data_[6] = buf[6], data_[7] = buf[7];
	}
	else
	{
		data_[0] = buf[7], data_[1] = buf[6], data_[2] = buf[5], data_[3] = buf[4];
		data_[4] = buf[3], data_[5] = buf[2], data_[6] = buf[1], data_[7] = buf[0];
	}
	data_ += sizeof val;
}


void MemPointer::PutUInt32(uint32 val)
{
	MakeSpace(sizeof val);
	uint8 buf[sizeof val]= { static_cast<uint8>(val >> 24), static_cast<uint8>(val >> 16), static_cast<uint8>(val >> 8), static_cast<uint8>(val) };
	if (big_endian_)
		data_[0] = buf[0], data_[1] = buf[1], data_[2] = buf[2], data_[3] = buf[3];
	else
		data_[0] = buf[3], data_[1] = buf[2], data_[2] = buf[1], data_[3] = buf[0];
	data_ += sizeof val;
}


void MemPointer::PutUInt16(uint16 val)
{
	MakeSpace(sizeof val);
	uint8 buf[sizeof val]= { static_cast<uint8>(val >> 8), static_cast<uint8>(val) };
	if (big_endian_)
		data_[0] = buf[0], data_[1] = buf[1];
	else
		data_[0] = buf[1], data_[1] = buf[0];
	data_ += sizeof val;
}


uint32 MemPointer::GetUInt32(uint8* external_buff)
{
	if (data_ + 4 > end_)
		return Error();
	external_buff[0] = data_[0];
	external_buff[1] = data_[1];
	external_buff[2] = data_[2];
	external_buff[3] = data_[3];
	uint32 val= big_endian_ ?
		data_[3] | (uint32(data_[2]) << 8) | (uint32(data_[1]) << 16) | (uint32(data_[0]) << 24) :
		data_[0] | (uint32(data_[1]) << 8) | (uint32(data_[2]) << 16) | (uint32(data_[3]) << 24);
	data_ += sizeof val;
	return val;
}


uint64 MemPointer::GetUInt64()
{
	uint64 val;
	if (data_ + sizeof(val) > end_)
		return Error();
	if (big_endian_)
	{
		val =
			(uint64(data_[7]) << 0x00) | (uint64(data_[6]) << 0x08) |
			(uint64(data_[5]) << 0x10) | (uint64(data_[4]) << 0x18) |
			(uint64(data_[3]) << 0x20) | (uint64(data_[2]) << 0x28) |
			(uint64(data_[1]) << 0x30) | (uint64(data_[0]) << 0x38);
	}
	else
	{
		val = 
			(uint64(data_[0]) << 0x00) | (uint64(data_[1]) << 0x08) |
			(uint64(data_[2]) << 0x10) | (uint64(data_[3]) << 0x18) |
			(uint64(data_[4]) << 0x20) | (uint64(data_[5]) << 0x28) |
			(uint64(data_[6]) << 0x30) | (uint64(data_[7]) << 0x38);
	}
	data_ += sizeof val;
	return val;
}


uint32 MemPointer::GetBigEndianUInt32()
{
	if (data_ + 4 > end_)
		return Error();
	uint32 val= data_[3] | (uint32(data_[2]) << 8) | (uint32(data_[1]) << 16) | (uint32(data_[0]) << 24);
	data_ += sizeof val;
	return val;
}


uint32 MemPointer::GetUInt32()
{
	if (data_ + 4 > end_)
		return Error();
	uint32 val= big_endian_ ?
		data_[3] | (uint32(data_[2]) << 8) | (uint32(data_[1]) << 16) | (uint32(data_[0]) << 24) :
		data_[0] | (uint32(data_[1]) << 8) | (uint32(data_[2]) << 16) | (uint32(data_[3]) << 24);
	data_ += sizeof val;
	return val;
}


uint16 MemPointer::GetUInt16()
{
	if (data_ + 2 > end_)
		return Error();
	uint16 val= big_endian_ ? data_[1] | (uint16(data_[0]) << 8) : data_[0] | (uint16(data_[1]) << 8);
	data_ += sizeof val;
	return val;
}


const char* MemPointer::MemPtrException::what() const
{
	return "Out of bounds access to the buffer detected";
}

int32 MemPointer::GetInt32()
{
	return static_cast<int32>(GetUInt32());
}

int16 MemPointer::GetInt16()
{
	return static_cast<int16>(GetUInt16());
}

int8 MemPointer::GetInt8()
{
	return static_cast<int8>(GetUInt8());
}

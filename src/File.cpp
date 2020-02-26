/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "File.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


FileStream::~FileStream()
{
	try
	{
		Close();
	}
	catch (...)
	{
		ASSERT(false);
	}
}


bool FileStream::Open(const TCHAR* file_name, bool read_only/*= true*/)
{
	if (read_only)
	{
		if (file_map_.CreateReadOnlyView(file_name))
			ptr_.Reset(file_map_.GetBaseAddrChr(), file_map_.GetFileSize(), 0);
		else
			ptr_.Reset(0, 0, 0);
	}
	else
	{
		// open for in-place write

		if (file_map_.CreateReadWriteView(file_name))
			ptr_.Reset(file_map_.GetBaseAddrChr(), file_map_.GetFileSize(), 0);
		else
			ptr_.Reset(0, 0, 0);
//			throw String("Cannot open file and write to it: ") + file_name;
	}

	SetByteOrder(true);

	return ptr_.GetPtr() != 0;
}


bool FileStream::Open(const std::vector<uint8>& buffer)
{
	if (buffer.empty())
		return false;
	ptr_.Reset(const_cast<uint8*>(&buffer.front()), buffer.size(), 0);
	SetByteOrder(true);
	return true;
}


#ifdef USE_MEM_MAP_FILE // ==============================================================


uint32 FileStream::GetUInt32(uint8* external_buff)
{
	return ptr_.GetUInt32(external_buff);
}


void FileStream::RPosition(int32 offset, seekdir dir/*= cur*/)
{
	switch (dir)
	{
	case cur:
		ptr_ += offset;
		break;

	case beg:
		ptr_.SetPos(offset);
		break;

	case end:
		ptr_.SetPos(file_map_.GetFileSize() - offset);
		break;
	}
}


#else


//FileStream::FileStream(const TCHAR* file_name)
// : file_(file_name, CFile::modeRead | CFile::shareDenyWrite)

// : fs_(file_name, ios_base::in /*| ios_base::out*/ | ios_base::binary),
//		big_endian_(true)
//{
//	fs_.exceptions(ios_base::badbit | ios_base::failbit | ios_base::eofbit);
//}


void FileStream::GetString(string& buf, uint32 size)
{
	for (uint32 i= 0; i < size; ++i)
		if (char c= GetUInt8())
			buf += c;
//	getline(fs_, buf, '\0');
}


void FileStream::RPosition(int32 offset, seekdir dir/*= ios_base::cur*/)
{
	UINT from= CFile::begin;
	if (dir == cur)
		from = CFile::current;
	else if (dir == end)
		from = CFile::end;

	file_.Seek(offset, from);
}


void FileStream::RPosition(int32 offset_base, int32 offset, bool)
{
	file_.Seek(offset_base + offset, CFile::begin); //   ios_base::beg);
}


Offset FileStream::RPosition()
{
	Offset pos= file_.Seek(0, CFile::current);
	//if (pos == -1) throw 1; return pos;
	return pos;
}


//bool FileStream::Open(const TCHAR* file_name)
//{
//		fs_.open(file_name, ios_base::in /*| ios_base::out*/ | ios_base::binary);
//		if (!fs_)
//			return false;
//		fs_.exceptions(ios_base::badbit | ios_base::failbit | ios_base::eofbit);
//		return true;
//}

#endif


void FileStream::Read(uint32* vect, size_t size)
{
	for (size_t i= 0; i < size; ++i)
		vect[i] = GetUInt32();
}

/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// MemMappedFile.cpp: implementation of the MemMappedFile class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MemMappedFile.h"
#include "Exception.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
static int file_counter_= 0;
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

MemMappedFile::MemMappedFile()
{
	file_ = 0;
	mem_map_file_ = 0;
	view_base_addr_ = 0;
	size_ = 0;
#ifdef _DEBUG
	++file_counter_;
	//CURRENT_CALL_STACK_DUMP
	//CString s(callstack.c_str());
	//TRACE(L"new mem file: %d\n%s\n\n", file_counter_, s);
#endif;
}


MemMappedFile::~MemMappedFile()
{
	if (view_base_addr_)
		::UnmapViewOfFile(view_base_addr_);
	if (mem_map_file_)
		::CloseHandle(mem_map_file_);
	if (IsOpen())
		::CloseHandle(file_);
#ifdef _DEBUG
	--file_counter_;
	TRACE(L"mem files left: %d\n", file_counter_);
#endif;
}


bool MemMappedFile::CreateReadOnlyView(const char* file_name)
{
	return CreateView(file_name, true);
}

bool MemMappedFile::CreateReadOnlyView(const wchar_t* file_name)
{
	return CreateView(file_name, true);
}


bool MemMappedFile::CreateReadWriteView(const char* file_name)
{
	return CreateView(file_name, false);
}

bool MemMappedFile::CreateReadWriteView(const wchar_t* file_name)
{
	return CreateView(file_name, false);
}


bool MemMappedFile::CreateView(const char* file_name, bool read_only/*= true*/)
{
	ASSERT(file_ == 0);
	file_ = CreateFileA(file_name, GENERIC_READ | (read_only ? 0 : GENERIC_WRITE),
		read_only ? FILE_SHARE_READ : 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

	if (file_ == INVALID_HANDLE_VALUE || file_ == 0)
	{
		file_ = 0;
		return false;
	}

	DWORD temp;
	size_ = ::GetFileSize(file_, &temp);

	mem_map_file_ = ::CreateFileMappingA(file_, NULL, read_only ? PAGE_READONLY : PAGE_READWRITE, 0, 0, NULL);

	if (mem_map_file_ == 0)
		return false;

	view_base_addr_ = ::MapViewOfFile(mem_map_file_, read_only ? FILE_MAP_READ : FILE_MAP_WRITE, 0, 0, 0);

	return view_base_addr_ != 0;
}


bool MemMappedFile::CreateView(const wchar_t* file_name, bool read_only/*= true*/)
{
	ASSERT(file_ == 0);
	file_ = CreateFileW(file_name, GENERIC_READ | (read_only ? 0 : GENERIC_WRITE),
		read_only ? FILE_SHARE_READ : 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

	if (file_ == INVALID_HANDLE_VALUE || file_ == 0)
	{
		file_ = 0;
		return false;
	}

	DWORD temp;
	size_ = ::GetFileSize(file_, &temp);

	mem_map_file_ = ::CreateFileMappingW(file_, NULL, read_only ? PAGE_READONLY : PAGE_READWRITE, 0, 0, NULL);

	if (mem_map_file_ == 0)
		return false;

	view_base_addr_ = ::MapViewOfFile(mem_map_file_, read_only ? FILE_MAP_READ : FILE_MAP_WRITE, 0, 0, 0);

	return view_base_addr_ != 0;
}


bool MemMappedFile::CreateWriteView(const TCHAR* file_name, int bytes_to_add, bool shareRead)
{
	if (file_ == 0)
	{
		file_ = CreateFile(file_name, GENERIC_READ | GENERIC_WRITE,
			shareRead ? FILE_SHARE_READ : 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	}
	else
		CloseView();

	if (file_ == INVALID_HANDLE_VALUE || file_ == 0)
	{
		file_ = 0;
		return false;
	}

	DWORD temp;
	size_ = ::GetFileSize(file_, &temp);

	mem_map_file_ = ::CreateFileMapping(file_, NULL, PAGE_READWRITE, 0, size_ + bytes_to_add, NULL);

	if (mem_map_file_ == 0)
	{
		ASSERT(false);
		return false;
	}

	view_base_addr_ = ::MapViewOfFile(mem_map_file_, FILE_MAP_WRITE, 0, 0, 0);
	ASSERT(view_base_addr_ != 0);

//	TRACE(L"mem file: %x - %s\n", file_, file_name);

	return view_base_addr_ != 0;
}


void MemMappedFile::CloseFile()
{
	if (view_base_addr_)
	{
		::UnmapViewOfFile(view_base_addr_);
		view_base_addr_ = 0;
	}
	if (mem_map_file_)
	{
		::CloseHandle(mem_map_file_);
		mem_map_file_ = 0;
	}
	if (IsOpen())
	{
		if (::CloseHandle(file_) == 0)
		{
			//TODO: improve?
			throw ::GetLastError();
		}
		file_ = 0;
	}
}


void MemMappedFile::CloseView()
{
	if (view_base_addr_)
	{
		::UnmapViewOfFile(view_base_addr_);
		view_base_addr_ = 0;
	}
	if (mem_map_file_)
	{
		::CloseHandle(mem_map_file_);
		mem_map_file_ = 0;
	}
}


void MemMappedFile::Touch()
{
	if (IsOpen())
	{
		FILETIME modify;
		::GetSystemTimeAsFileTime(&modify);
		VERIFY(::SetFileTime(file_, 0, 0, &modify));
	}
	else
	{ ASSERT(false); }
}


void MemMappedFile::SetFileSize(LONGLONG new_size)
{
	CloseView();

	if (!IsOpen())
		throw String(L"MemMappedFile::SetFileSize - cannot change file size, file is closed");

	LARGE_INTEGER size;
	size.QuadPart = new_size;
	if (::SetFilePointerEx(file_, size, nullptr, FILE_BEGIN) == 0)
	{
		_com_error err(::GetLastError());
		oStringstream ost;
		ost << L"MemMappedFile::SetFileSize - seek failed: " << err.ErrorMessage();
		throw ost.str();
	}

	if (::SetEndOfFile(file_) == 0)
	{
		_com_error err(::GetLastError());
		oStringstream ost;
		ost << L"MemMappedFile::SetFileSize - error setting file size: " << err.ErrorMessage();
		throw ost.str();
	}
}


bool MemMappedFile::IsOpen() const
{
	return file_ && file_ != INVALID_HANDLE_VALUE;
}


void MemMappedFile::SetFileTime(const FileTimes& time)
{
	if (!IsOpen())
		throw String(L"MemMappedFile::SetFileTime - cannot change file time, file is closed");

	if (::SetFileTime(file_, &time.creation_time_, &time.last_access_time_, &time.last_write_time_) == 0)
	{
		_com_error err(::GetLastError());
		oStringstream ost;
		ost << L"MemMappedFile::SetFileTime - error setting file time: " << err.ErrorMessage();
		throw ost.str();
	}
}


FileTimes MemMappedFile::GetFileTime() const
{
	if (!IsOpen())
		throw String(L"MemMappedFile::GetFileTime - cannot read file size, file is closed");

	FileTimes time;
	if (::GetFileTime(file_, &time.creation_time_, &time.last_access_time_, &time.last_write_time_) == 0)
	{
		_com_error err(::GetLastError());
		oStringstream ost;
		ost << L"MemMappedFile::GetFileTime - error reading file time: " << err.ErrorMessage();
		throw ost.str();
	}

	return time;
}


void MemMappedFile::AttachWriteFile(HANDLE file)
{
	CloseFile();

	file_ = file;
}

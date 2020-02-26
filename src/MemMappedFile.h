/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// MemMappedFile.h: interface for the MemMappedFile class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MEMMAPPEDFILE_H__01E46295_1954_11D3_B426_000000000000__INCLUDED_)
#define AFX_MEMMAPPEDFILE_H__01E46295_1954_11D3_B426_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <boost/noncopyable.hpp>

struct FileTimes
{
	FileTimes()
	{
		creation_time_.dwLowDateTime = creation_time_.dwHighDateTime = 0;
		last_access_time_.dwLowDateTime = last_access_time_.dwHighDateTime = 0;
		last_write_time_.dwLowDateTime = last_write_time_.dwHighDateTime = 0;
	}

	FILETIME creation_time_;
	FILETIME last_access_time_;
	FILETIME last_write_time_;
};

class MemMappedFile : boost::noncopyable
{
public:
	MemMappedFile();
	virtual ~MemMappedFile();

	bool  CreateReadOnlyView(const char* file_name);
	bool  CreateReadOnlyView(const wchar_t* file_name);

	bool  CreateReadWriteView(const char* file_name);
	bool  CreateReadWriteView(const wchar_t* file_name);

	bool  CreateWriteView(const TCHAR* file_name, int bytes_to_add, bool shareRead= false);

	void AttachWriteFile(HANDLE file);

	void* GetBaseAddr() const				{ return view_base_addr_; }
	uint8* GetBaseAddrChr() const			{ return reinterpret_cast<uint8*>(view_base_addr_); }
	DWORD GetFileSize() const				{ return size_; }
	void  CloseFile();

	bool operator ! () const				{ return view_base_addr_ != 0; }

	void Touch();

	void SetFileSize(LONGLONG new_size);

	bool IsOpen() const;

	void SetFileTime(const FileTimes& time);
	FileTimes GetFileTime() const;

private:
	HANDLE file_;
	void*  view_base_addr_;
	HANDLE mem_map_file_;
	DWORD  size_;
	void CloseView();

	bool CreateView(const char* file_name, bool read_only);
	bool CreateView(const wchar_t* file_name, bool read_only);
};

#endif // !defined(AFX_MEMMAPPEDFILE_H__01E46295_1954_11D3_B426_000000000000__INCLUDED_)

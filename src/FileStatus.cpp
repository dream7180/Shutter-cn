/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "StdAfx.h"
#include "FileStatus.h"
#include "DateTimeUtils.h"

FileStatus::FileStatus()
{
	FILETIME zero= { 0, 0 };
	creation_ = last_access_ = last_write_ = zero;
	attributes_ = 0;
}


// This function reads file status and (unlike CFile::GetStatus) doesn't throw when bogus time is encountered
// Note that time is left in UTC format
//
bool GetFileStatus(const TCHAR* file_name, FileStatus& status)
{
	WIN32_FIND_DATA find_file_data;
	HANDLE find= ::FindFirstFile(file_name, &find_file_data);
	if (find == INVALID_HANDLE_VALUE)
		return false;
	VERIFY(::FindClose(find));

	// file attributes (FILE_ATTRIBUTE_*)
	status.attributes_ = find_file_data.dwFileAttributes;

	// find_file_data.nFileSizeHigh
	//status.m_size = find_file_data.nFileSizeLow;

	status.creation_ = find_file_data.ftCreationTime;
	status.last_access_ = find_file_data.ftLastAccessTime;
	status.last_write_ = find_file_data.ftLastWriteTime;

	return true;
}


void SetFileStatus(const TCHAR* file_name, const FileStatus& status)
{
	HANDLE file= ::CreateFile(file_name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

	if (file == INVALID_HANDLE_VALUE)
		CFileException::ThrowOsError((LONG)::GetLastError(), file_name);

	if (!SetFileTime(file, &status.creation_, &status.last_access_, &status.last_write_))
	{
		auto err= ::GetLastError();
		::CloseHandle(file);
		CFileException::ThrowOsError(err, file_name);
	}

	if (!::CloseHandle(file))
		CFileException::ThrowOsError((LONG)::GetLastError(), file_name);
}


DateTime GetFileModifiedTime(const TCHAR* file_name)
{
	WIN32_FIND_DATA findFileData;
	HANDLE find = ::FindFirstFile(file_name, &findFileData);
	if (find == INVALID_HANDLE_VALUE)
		return DateTime();
	VERIFY(::FindClose(find));

	FILETIME local;
	if (!::FileTimeToLocalFileTime(&findFileData.ftLastWriteTime, &local))
		return DateTime();

	return FileTimeToDateTime(local);
}

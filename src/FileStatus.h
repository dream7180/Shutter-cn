/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once

// helper function to get/set file status

struct FileStatus
{
	FileStatus();

	// original UTC file time (not local)
	FILETIME creation_;
	FILETIME last_access_;
	FILETIME last_write_;

	DWORD attributes_;
};


// read file status
bool GetFileStatus(const TCHAR* file_name, FileStatus& status);

// restore file times only; throw on failure
void SetFileStatus(const TCHAR* file_name, const FileStatus& status);

// read file modification time
DateTime GetFileModifiedTime(const TCHAR* file_name);

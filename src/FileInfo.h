/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once


struct FileInfo
{
	FileInfo() : attributes_(0), size_(0)
	{
		created_.dwLowDateTime = 0;
		created_.dwHighDateTime = 0;
		modified_.dwLowDateTime = 0;
		modified_.dwHighDateTime = 0;
	}

	FileInfo(const TCHAR* path, DWORD attr, uint64 size, FILETIME created, FILETIME modified)
		: path_(path), attributes_(attr), size_(size), created_(created), modified_(modified)
	{}

	String path_;
	DWORD attributes_;
	uint64 size_;
	FILETIME created_;
	FILETIME modified_;
	FILETIME last_access_;
};

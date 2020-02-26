/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
#include "Path.h"
#include "PhotoInfoPtr.h"


class CopyPhoto
{
public:
	CopyPhoto();
	~CopyPhoto();

	void Copy(const Path& file_path, const TCHAR* dest_folder, const TCHAR* rename_pattern);

private:
	void Progress(uint64 pos, uint64 length);

	Path CreateDestPath(const Path& file_path, uint64 length, PhotoInfoPtr photo);
};

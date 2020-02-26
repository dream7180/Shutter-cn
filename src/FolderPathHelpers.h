/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
#include "FolderPath.h"
class ItemIdList;


extern FolderPathPtr CreateFolderPath(const TCHAR* path);
extern FolderPathPtr CreateFolderPath(const String& path);
extern FolderPathPtr CreateFolderPath(const ItemIdList& path);

//extern bool StoreFolderPath(const TCHAR* regSection, const TCHAR* regEntry);
extern FolderPathPtr RestoreFolderPath(const TCHAR* regSection, const TCHAR* regEntry);

/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
#include <boost/function.hpp>
//typedef bool FileFilterCallback(const TCHAR* path, WIN32_FIND_DATA& fileInfo);

typedef boost::function<bool (const TCHAR* path)> FileFilterCallback;

/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "ImgLogger.h"


ImgLogger::ImgLogger()
{
}

ImgLogger::~ImgLogger()
{
}


void ImgLogger::AddEntry(const TCHAR* msg, const TCHAR* path)
{
	CSingleLock lock(&sentry_, true);

	logs_.push_back(std::make_pair(msg, path));
}


size_t ImgLogger::GetCount() const
{
	CSingleLock lock(&sentry_, true);

	return logs_.size();
}


ImgLogger::LogEntry ImgLogger::GetItem(size_t index) const
{
	CSingleLock lock(&sentry_, true);

	return logs_.at(index);
}

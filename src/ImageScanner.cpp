/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
#include "stdafx.h"
#include "ImageScanner.h"
#include "Path.h"


ImageScanner::ImageScanner() : images_db_(0)
{}

ImageScanner::~ImageScanner()
{}


void ImageScanner::SetNotificationCallback(const NotifyFn& fn)
{
	notify_fn_ = fn;
}


ImageScanner::NotifyFn ImageScanner::GetNotificationCallback()
{
	return notify_fn_;
}


void ImageScanner::Notify()
{
	if (notify_fn_)
		notify_fn_();
}


void ImageScanner::ReadPhotosWithExifOnly(bool exif_only)
{
	exif_only_ = exif_only;
}


bool ImageScanner::ReadExifImagesOnly() const
{
	return exif_only_;
}


void ImageScanner::SetImageDatabase(ImageDatabase& dbImages)
{
	images_db_ = &dbImages;
}


ImageDatabase* ImageScanner::Database() const
{
	return images_db_;
}


void ImageScanner::LogError(const TCHAR* msg, const Path& path)
{
	log_entries_.AddEntry(msg, path.c_str());
}


void ImageScanner::LogError(const String& msg, const Path& path)
{
	log_entries_.AddEntry(msg.c_str(), path.c_str());
}


const ImgLogger& ImageScanner::GetLogger() const
{
	return log_entries_;
}


ImgLogger* ImageScanner::Logger()
{
	return &log_entries_;
}

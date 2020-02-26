/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
#include "boost/function.hpp"
#include "ImgLogger.h"
class PhotoInfoStorage;
class ImageDatabase;
class Path;


class ImageScanner
{
public:
	ImageScanner();
	virtual ~ImageScanner();

	// traverse source of images and store them in the 'store'
	virtual bool Scan(bool visitSubDirs, uint32& dirVisited, PhotoInfoStorage& store) = 0;

	// signal break
	virtual void CancelScan() = 0;

	typedef boost::function<void (void)> NotifyFn;

	// set progress callback
	void SetNotificationCallback(const NotifyFn& fn);
	NotifyFn GetNotificationCallback();

	// read all images or those with EXIF info only?
	void ReadPhotosWithExifOnly(bool exif_only);

	// cache for photos read from a directory
	void SetImageDatabase(ImageDatabase& dbImages);

	// returns logger with info about images that failed to load/scan
	const ImgLogger& GetLogger() const;

private:
	NotifyFn notify_fn_;
	bool exif_only_;
	ImageDatabase* images_db_;
	ImgLogger log_entries_;

protected:
	ImgLogger* Logger();
	void Notify();
	bool ReadExifImagesOnly() const;
	ImageDatabase* Database() const;

	void LogError(const TCHAR* msg, const Path& path);
	void LogError(const String& msg, const Path& path);
};

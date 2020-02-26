/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
#include "ImgProcessingThread.h"
class PhotoInfo;
#include "VectPhotoInfo.h"
class FileStream;
class Data;


class DateTimeAdjThread : public ImgProcessingThread
{
public:
	DateTimeAdjThread(const VectPhotoInfo& files, DateTime new_date_time);				// absolute form
	DateTimeAdjThread(const VectPhotoInfo& files, int days, int hours, int minutes, int seconds);	// relative form

	virtual ImgProcessingThread* Clone();

private:
	// process one file
	virtual void Process(size_t index);

	virtual String GetSourceFileName(size_t index) const;
	virtual String GetDestFileName(size_t index) const;

	void ModifyDateTimeField(FileStream& ifs, uint16 exif_field, const Data& val, PhotoInfoPtr photo, DateTime dt, bool* changed);

	bool ModifyDate(FileStream& ifs, const Data& val, PhotoInfoPtr photo, DateTime dt);

	const VectPhotoInfo& files_;
	bool relative_change_;
	DateTime date_time_;
	int days_;
	int hours_;
	int minutes_;
	int seconds_;
};

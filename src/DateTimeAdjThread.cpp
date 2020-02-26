/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "DateTimeAdjThread.h"
#include "PhotoInfo.h"
#include "scan.h"
#include <boost/bind.hpp>
#include "ExifFields.h"
#include "DateTimeUtils.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


DateTimeAdjThread::DateTimeAdjThread(const VectPhotoInfo& files, DateTime new_date_time)
  : ImgProcessingThread(files.size()), files_(files)
{
	relative_change_ = false;
	date_time_ = new_date_time;
	days_ = hours_ = minutes_ = seconds_ = 0;
}

DateTimeAdjThread::DateTimeAdjThread(const VectPhotoInfo& files, int days, int hours, int minutes, int seconds)
  : ImgProcessingThread(files.size()), files_(files)
{
	relative_change_ = true;
	days_ = days;
	hours_ = hours;
	minutes_ = minutes;
	seconds_ = seconds;
}


String DateTimeAdjThread::GetSourceFileName(size_t index) const
{
	ASSERT(index < files_.size());
	return files_[index]->GetPhysicalPath();
}


String DateTimeAdjThread::GetDestFileName(size_t index) const
{
	ASSERT(index < files_.size());
	return files_[index]->GetPhysicalPath();
}


void DateTimeAdjThread::Process(size_t index)
{
	ASSERT(index < files_.size());

	PhotoInfoPtr photo= files_[index];
	Path path= photo->GetPhysicalPath();

	// prepare modified date/time
	auto date= photo->GetDateTime();

	if (relative_change_)
	{
		auto span= CreateTimeDuration(days_, hours_, minutes_, seconds_);
		date = AdjustDateTime(date, span);
	}
	else
		date = date_time_;

	if (files_[index]->IsExifDataPresent())
	{
		FileStream ifs;

		if (!ifs.Open(path.c_str(), false))
		{
			// handle error: cannot open file for writing

			throw String(_T("Cannot open file for writing operation: ")) + path;
		}
		else
		{
			bool date_time_changed= false;

			FieldCallback fn= boost::bind(&DateTimeAdjThread::ModifyDateTimeField, this, _1, _2, _3, photo, date, &date_time_changed);

			ifs.SetByteOrder(photo->exif_info_.big_endian_byte_order);
			std::pair<uint32, uint32> rangeExif(ifs.RPosition(), ifs.RPosition() + photo->exif_info_.exif_block_size);
			ifs.RPosFromBeg(photo->exif_info_.exif_offset);

			::ScanExif(path.c_str(), ifs, photo->exif_info_.offset_to_Ifd_start, rangeExif, photo->GetMake(), photo->GetModel(), 0, 0, 0, false, &fn);

			if (!date_time_changed)
				throw String(_T("Cannot find valid date/time EXIF field in the file: ")) + path;
		}
	}

	// modify file time stamp too (cache update needs that)

	SetFileCreationModificationTime(path.c_str(), date);
/*
	CFileStatus status;
	if (!CFile::GetStatus(path.c_str(), status))
		throw String(_T("Cannot read file status: ")) + path;

	status.m_ctime = date;
	status.m_mtime = date;
	CFile::SetStatus(path.c_str(), status);
*/
	photo->SetDateTime(date);
}


bool DateTimeAdjThread::ModifyDate(FileStream& ifs, const Data& val, PhotoInfoPtr photo, DateTime dt)
{
	try
	{
		// modify EXIF block
		const int LEN= 19;

		if (val.Components() >= LEN && val.FormatSingleByte() && val.IsLongData())
		{
			Offset temp= ifs.RPosition();

			ifs.RPosition(val.IfdOffset(), val.GetData(), false);

			std::string exif_date= DateTimeToExifDate(dt);
/*
			char date_time[200];
			wsprintfA(date_time, "%04d:%02d:%02d %02d:%02d:%02d", dt->GetYear(), dt->GetMonth(), dt->GetDay(), dt->GetHour(), dt->GetMinute(), dt->GetSecond()); */
			if (exif_date.length() > LEN)
				throw String(_T("Incorrect date encountered: ")) + photo->GetPhysicalPath();

			ifs.Write(exif_date.c_str(), val.Components() > LEN ? LEN + 1 : LEN);

			ifs.RPosition(temp, FileStream::beg);

			return true;
		}
	}
	catch (...)	//TODO: report failure? error caught locally to prevent it from aborting processing of remaining photos; this should be done outside of this fn
	{
		ASSERT(false);
	}

	return false;
}


void DateTimeAdjThread::ModifyDateTimeField(FileStream& ifs, uint16 exif_field, const Data& val, PhotoInfoPtr photo, DateTime dt, bool* changed)
{
	switch (exif_field)
	{
	case EXIF_DateTime:
	case EXIF_DateTimeOriginal:
	case EXIF_DateTimeDigitized:
		if (ModifyDate(ifs, val, photo, dt))
			*changed = true;
		break;

	default:
		break;
	}
}


ImgProcessingThread* DateTimeAdjThread::Clone()
{
	return relative_change_ ? new DateTimeAdjThread(files_, days_, hours_, minutes_, seconds_) : new DateTimeAdjThread(files_, date_time_);
}

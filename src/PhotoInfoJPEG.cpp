/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "PhotoInfoJPEG.h"
#include "PhotoFactory.h"
#include "FileTypeIndex.h"
#include "XMPAccess.h"
#include "ExifTags.h"
#include "StringConversions.h"
#include "DbOperations.h"
#include "Exception.h"
#include "FileGuard.h"
#include "IPTCReadWrite.h"

extern String GetAppIdentifier(bool);


namespace {
	RegisterPhotoType<PhotoInfoJPEG> jpeg(_T("jpg;jpe;jpeg"), FT_JPEG);
}


PhotoInfoJPEG::PhotoInfoJPEG()
{
	file_type_index_ = FT_JPEG;
}

PhotoInfoJPEG::~PhotoInfoJPEG()
{
}


extern void IptcToXmp(const IPTCRecord& iptc, XmpData& xmp)
{
	xmp.Author = iptc.byline_;
	xmp.CreatorsJob = iptc.byline_title_;
	xmp.Provider = iptc.credits_;	// verify
	xmp.Source = iptc.source_;

	xmp.DescriptionWriter = iptc.caption_writer_;
	xmp.Description = iptc.caption_;
	xmp.Headline = iptc.headline_;
	xmp.Instructions = iptc.special_instructions_;

//	xmp.Title = iptc.object_name_;
	xmp.CreationDate = iptc.date_created_;
	xmp.City = iptc.city_;
	xmp.State = iptc.state_;
	xmp.Country = iptc.country_;
	xmp.JobIdentifier = iptc.original_transmission_reference_;

	const size_t size= iptc.keywords_.size();
	for (size_t i= 0; i < size; ++i)
	{
		xmp.Keywords += iptc.keywords_[i];

		if (i < size - 1)
			xmp.Keywords += _T(", ");
	}

	xmp.CopyrightNotice = iptc.copyright_notice_;
	xmp.Location = iptc.contact_;	// not really...
}


// XMP to IPTC; some fields in IPTC don't have corresponding XMP fields...
extern void XmpToIptc(const XmpData& xmp, IPTCRecord& iptc)
{
	// copy XMP data to IPTC record

	iptc.byline_ = xmp.Author;
	iptc.byline_title_ = xmp.CreatorsJob;
	iptc.credits_ = xmp.Provider;	// verify
	iptc.source_ = xmp.Source;

	// Caption
	iptc.caption_writer_ = xmp.DescriptionWriter;
	iptc.caption_ = xmp.Description;
	iptc.headline_ = xmp.Headline;
	iptc.special_instructions_ = xmp.Instructions;

	// Origin
//	iptc.object_name_ = xmp.Title;
	iptc.date_created_ = xmp.CreationDate;
	iptc.city_ = xmp.City;
	iptc.state_ = xmp.State;
	iptc.country_ = xmp.Country;
	iptc.original_transmission_reference_ = xmp.JobIdentifier;

	// Keywords
	Xmp::ParseItems(xmp.Keywords, iptc.keywords_);
	iptc.LimitKeywordsLength();

	// Copyright
	iptc.copyright_notice_ = xmp.CopyrightNotice;
	iptc.contact_ = xmp.Location;	// not really...
}


const wchar_t* g_err_title= L"Error saving file info metadata";


void SaveFileInfo(const XmpData& xmp, const wchar_t* path, const PhotoInfo::WriteAccessFn& get_write_access)
{
	// create backup of original file
	FileGuard backup(path, false);

	{
		// try to open it for up to 1 seconds (if get_write_access is empty)
		int retry= 10;
		const int delay= 100;	// 100 ms
		int wait= 0;

		MemMappedFile photo;

		if (!get_write_access.empty())
		{
			HANDLE file= get_write_access(path);
			if (file == 0)
				return;
			photo.AttachWriteFile(file);
			retry = 1;
		}

		for (int t= 0; t < retry; ++t)
		{
			if (photo.CreateWriteView(path, 0, true))
				break;

			::Sleep(delay);

			wait += delay;
			TRACE(L"Waiting %d ms for %s\n", wait, path);
		}

		if (!photo.IsOpen())
			THROW_EXCEPTION(g_err_title, SF(L"Cannot open file: " << path))

		// keep original file times
		FileTimes time= photo.GetFileTime();

		// restore original file form backup if saving metadata fails
		backup.SetAutoRestore();

		// embed XMP (updates existing XMP)
		Xmp::SaveXmpIntoJpeg(xmp, photo, path);

		IPTCRecord iptc;
		XmpToIptc(xmp, iptc);
		// attempt to save IPTC record
		WriteIPTC(photo, path, iptc);

		// restore original data/time
		photo.SetFileTime(time);

		// metadata saved successfully
		backup.AnnulAutoRestore();

		// invalidate DB record
		MarkRecordAsStale(path);
	}

	backup.DeleteBackup();
}


void PhotoInfoJPEG::SaveMetadata(const XmpData& xmp, const WriteAccessFn& get_write_access) const
{
	int errCode= 0;
	if (CanEditIPTC(errCode))
	{
		SaveFileInfo(xmp, path_.c_str(), get_write_access);
	}
	else
	{
		switch (errCode)
		{
		case -2:
			THROW_EXCEPTION(g_err_title, SF(L"Cannot edit JPEG image, file is read-only.\nFile: " << path_))
		case -1:
			THROW_EXCEPTION(g_err_title, SF(L"Cannot edit JPEG image, cannot read file attributes.\nFile: " << path_))
		case ERROR_SHARING_VIOLATION:
			THROW_EXCEPTION(g_err_title, SF(L"Cannot edit JPEG image, file cannot be open for writing (sharing violation).\nFile: " << path_))
		default:
			THROW_EXCEPTION(g_err_title, SF(L"Cannot edit JPEG image\nFile: " << path_))
		}
	}

}


bool PhotoInfoJPEG::CanEditIPTC(int& err_code) const
{
	extern int CanEditIPTCRecord(const TCHAR* file);
	err_code = CanEditIPTCRecord(path_.c_str());
	return err_code == 0 || err_code == ERROR_SHARING_VIOLATION;	// optimistic 'ok'
}


// save tags inside photo
void PhotoInfoJPEG::SaveTags(const WriteAccessFn& get_write_access)
{
	if (xmp_.get() == 0)
	{
		xmp_.reset(new XmpData());
#ifdef _UNICODE
		xmp_->Description = photo_desc_;
#else
		::WideStringToMultiByte(photo_desc_, xmp_->Description);
#endif
	}

	xmp_->Keywords = tags_.CommaSeparated();

	xmp_->CreatorTool = GetAppIdentifier(false);

	TCHAR ratingStr[32];
	_itot(image_rate_, ratingStr, 10);
	xmp_->ImageRating = ratingStr;

	SaveMetadata(*xmp_, get_write_access);
}


void PhotoInfoJPEG::SaveRating(int rating, const WriteAccessFn& get_write_access)
{
	image_rate_ = rating;
	SaveTags(get_write_access);
}

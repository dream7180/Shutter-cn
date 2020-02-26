/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "ApplyMetadataThread.h"
#include "PhotoInfo.h"
#include "Config.h"
#include "Messages.h"
#include "Exception.h"
#include "EditFileInfo.h"
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
using namespace boost::algorithm;

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


ApplyMetadataThread::ApplyMetadataThread(const VectPhotoInfo& files, const XmpData& xmp, CWnd* view)
  : ImgProcessingThread(files.size()), files_(files), xmp_(xmp), wnd_to_report_changes_(view)
{
	write_metadata_ = true;
	apply_tag_ = false;
	persist_tags_ = false;
	stars_ = 0;
	apply_rating_ = false;
}


ApplyMetadataThread::ApplyMetadataThread(const VectPhotoInfo& files, const String& tag, bool apply, CWnd* view)
  : ImgProcessingThread(files.size()), files_(files), tag_(tag), wnd_to_report_changes_(view)
{
	write_metadata_ = false;
	apply_tag_ = apply;
	persist_tags_ = g_Settings.save_tags_to_photo_;
	stars_ = 0;
	apply_rating_ = false;
}


ApplyMetadataThread::ApplyMetadataThread(const VectPhotoInfo& files, int stars, CWnd* view)
  : ImgProcessingThread(files.size()), files_(files), wnd_to_report_changes_(view)
{
	write_metadata_ = false;
	persist_tags_ = false;
	apply_tag_ = false;
	stars_ = stars;
	apply_rating_ = true;
}


String ApplyMetadataThread::GetSourceFileName(size_t index) const
{
	ASSERT(index < files_.size());
	return files_[index]->GetPhysicalPath();
}


String ApplyMetadataThread::GetDestFileName(size_t index) const
{
	ASSERT(index < files_.size());
	return files_[index]->GetPhysicalPath();
}


extern void ApplyTagToPhoto(PhotoInfoPtr photo, const String& tag, bool apply, bool persist, const PhotoInfo::WriteAccessFn& get_write_access)
{
	if (photo == 0)
		return;

	std::vector<String> tags;
	split(tags, tag, is_any_of(L"\n"));

	const size_t count= tags.size();
	for (size_t i= 0; i < count; ++i)
	{
		if (tags[i].empty())
			continue;

		if (apply)
			photo->GetTags().ApplyTag(tags[i]);
		else
			photo->GetTags().RemoveTag(tags[i]);
	}

	// save changes
	if (persist)
	{
		//TODO: improve: collect all errors and show them ONCE?
		photo->SaveTags(get_write_access);
	}
}


HANDLE GetFileWriteAccess(ImgProcessingThread* thread, const TCHAR* path)
{
	// try for up to 10 seconds
	int retry= 100;
	const int delay= 100;	// 100 ms

	int wait= 0;

	// try to obtain read-write access to a file, retry if it's in use

	for (int t= 0; t < retry; ++t)
	{
		HANDLE file= CreateFile(path, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
		if (file == INVALID_HANDLE_VALUE || file == 0)
		{
			DWORD err= ::GetLastError();
			if (err != ERROR_SHARING_VIOLATION)
			{
				_com_error e(err);
				THROW_EXCEPTION(L"Cannot open file for writing.", SF(L"File: " << path << L"\nError: " << e.ErrorMessage()))
			}
		}
		else
			return file;	// got it

		::Sleep(delay);

		wait += delay;
		TRACE(L"Waiting %d ms for %s\n", wait, path);

		if (thread->StopProcessing())
			return nullptr;
	}

	_com_error e(::GetLastError());
	THROW_EXCEPTION(L"Cannot open file for writing.", SF(L"File: " << path << L"\nError: " << e.ErrorMessage()))
}


void ApplyMetadataThread::Process(size_t index)
{
	ASSERT(index < files_.size());

	PhotoInfoPtr photo= files_[index];
	PhotoInfo::WriteAccessFn get_write_access(boost::bind(GetFileWriteAccess, this, _1));
//TRACE(L"ApplyMetadataThread::Process - start %d\n", index);
	if (write_metadata_)
	{
		int errCode= 0;
		if (photo->CanEditIPTC(errCode))
		{
			try
			{
				SaveMetadata(*photo, xmp_, false, get_write_access);

				if (wnd_to_report_changes_)
					wnd_to_report_changes_->SendMessage(METADATA_APPLIED_MESSAGE, 0, reinterpret_cast<LPARAM>(::GetRawPointer(photo)));
			}
			catch (CException* ex)
			{
				TCHAR err[MAX_PATH];
				ex->GetErrorMessage(err, MAX_PATH);
				String msg= L"Error writing file info metadata for '" + photo->GetOriginalPath() + L"' image.\n\nError: " + err;

				throw msg;
			}
			catch (String& err)
			{
				throw String(L"Error writing file info metadata for '" + photo->GetOriginalPath() + L"' image.\n\nError: " + err);
			}
		}
	}
	else if (apply_rating_)
	{
		photo->SaveRating(stars_, get_write_access);

		if (wnd_to_report_changes_)
			wnd_to_report_changes_->SendMessage(RATING_APPLIED_MESSAGE, 0, reinterpret_cast<LPARAM>(GetRawPointer(photo)));
	}
	else	// apply/remove tag
	{
		ApplyTagToPhoto(photo, tag_, apply_tag_, persist_tags_, get_write_access);
//TRACE(L"ApplyMetadataThread::Process - applied %d\n", index);

		//TODO:
		//exif_view_wnd_.TagApplied(*it);
		if (wnd_to_report_changes_)
			wnd_to_report_changes_->SendMessage(TAG_APPLIED_MESSAGE, 0, reinterpret_cast<LPARAM>(GetRawPointer(photo)));
	}
//TRACE(L"ApplyMetadataThread::Process - done %d\n", index);
}


ImgProcessingThread* ApplyMetadataThread::Clone()
{
	if (write_metadata_)
		return new ApplyMetadataThread(files_, xmp_, wnd_to_report_changes_);
	else if (apply_rating_)
		return new ApplyMetadataThread(files_, stars_, wnd_to_report_changes_);
	else
		return new ApplyMetadataThread(files_, tag_, apply_tag_, wnd_to_report_changes_);
}

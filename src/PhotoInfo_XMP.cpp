/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "PhotoInfo_XMP.h"
#include "XMPAccess.h"
#include "CatchAll.h"
#include "StringConversions.h"
extern String GetAppIdentifier(bool);


PhotoInfo_XMP::PhotoInfo_XMP()
{
	is_raw_ = true;
}

PhotoInfo_XMP::~PhotoInfo_XMP()
{}


void PhotoInfo_XMP::SaveTags(const WriteAccessFn& get_write_access)
{
//	try
	{
		std::vector<String> tags;
		tags_.CopyTagsToArray(tags);

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

//		Xmp::SavePhotoTags(path_, &tags, image_rate_, GetAppIdentifier());
		SaveMetadata(*xmp_, get_write_access);
	}
//	CATCH_ALL_W(parent)
}


void PhotoInfo_XMP::SaveRating(int rating, const WriteAccessFn& get_write_access)
{
	image_rate_ = rating;
	SaveTags(get_write_access);
//	Xmp::SavePhotoTags(path_, 0, rating, GetAppIdentifier());
}


static Path GetMetadataFilePath(const Path& file)
{
	Path xmp_file= file;
	xmp_file.ReplaceExtension(_T("xmp"));
	return xmp_file;
}

void PhotoInfo_XMP::SaveMetadata(const XmpData& xmp, const WriteAccessFn& get_write_access) const
{
	Path xmp_file= GetMetadataFilePath(path_);

	Xmp::UpdateXmpFile(xmp, xmp_file.c_str());
}


bool PhotoInfo_XMP::LoadMetadata(XmpData& xmp) const
{
	Path xmp_file= GetMetadataFilePath(path_);

	if (!xmp_file.FileExists())
		return false;

	Xmp::LoadFromXmpFile(xmp, xmp_file.c_str());
	return true;
}


bool PhotoInfo_XMP::CanEditIPTC(int& errCode) const
{
	return Xmp::CanEditXmpFile(path_, errCode);
}


Path PhotoInfo_XMP::GetAssociatedFilePath(size_t index) const
{
	// there may be one associated file; XMP metadata
	if (index == 0)
		return GetMetadataFilePath(path_);
	else
		return Path();
}

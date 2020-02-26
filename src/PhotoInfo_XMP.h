/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
#include "PhotoInfo.h"


// base class for all photos that want to supoprt XMP metadata in the stand-alone file
// (not an embedded one); typically raw formats (except DNG)


class PhotoInfo_XMP : public PhotoInfo
{
public:
	PhotoInfo_XMP();
	virtual ~PhotoInfo_XMP();

private:
	virtual void SaveTags(const WriteAccessFn& get_write_access);
	virtual void SaveRating(int rating, const WriteAccessFn& get_write_access);

	virtual void SaveMetadata(const XmpData& xmp, const WriteAccessFn& get_write_access) const;
	virtual bool LoadMetadata(XmpData& xmp) const;

	virtual bool CanEditIPTC(int& err_code) const;

	virtual Path GetAssociatedFilePath(size_t index) const;
};

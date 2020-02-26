/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
#include "PhotoInfo.h"

class PhotoInfoJPEG : public PhotoInfo
{
public:
	PhotoInfoJPEG();
	virtual ~PhotoInfoJPEG();

private:
	virtual void SaveMetadata(const XmpData& xmp, const WriteAccessFn& get_write_access) const;
	virtual void SaveTags(const WriteAccessFn& get_write_access);

	virtual bool CanEditIPTC(int& err_code) const;
	virtual void SaveRating(int rating, const WriteAccessFn& get_write_access);
};

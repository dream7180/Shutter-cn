/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once

#include "PhotoInfoPEF.h"


class PhotoInfoARW : public PhotoInfoPEF
{
public:
	PhotoInfoARW();
	virtual ~PhotoInfoARW();

private:
	PhotoInfoARW& operator = (const PhotoInfoARW&);
	PhotoInfoARW(const PhotoInfoARW&);

	// meaning of the IFD directories present in the raw file
	virtual bool IsMainImage(int ifd_index);
	virtual bool IsThumbnailImage(int ifd_index);
	virtual bool IsBigImage(int ifd_index);

	virtual void ParseMakerNote(FileStream& ifs);
};

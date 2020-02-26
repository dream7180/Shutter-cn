/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once

#include "PhotoInfoPtr.h"


#ifdef PHOTO_INFO_SMART_PTR

typedef std::vector<PhotoInfoPtr> VectPhotoInfo;

#else

typedef std::vector<PhotoInfo*> VectPhotoInfo;

#endif

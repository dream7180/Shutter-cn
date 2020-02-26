/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
#include <memory>

#ifdef PHOTO_INFO_SMART_PTR
	typedef mik::intrusive_ptr<PhotoInfo> SmartPtr;
#else
	typedef std::auto_ptr<PhotoInfo> SmartPtr;
#endif

SmartPtr CreateSamplePhotoInfo();

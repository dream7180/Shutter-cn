/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma pack(push)
#pragma pack(8)

extern "C" {
#ifdef WIN64
	#include "libjpeg-turbo/jpeglib.h"
	#include "libjpeg-turbo/jerror.h"
#else
	#include "jpeg-mmx/jpeglib.h"
	#include "jpeg-mmx/jerror.h"
#endif
}

#pragma pack(pop)

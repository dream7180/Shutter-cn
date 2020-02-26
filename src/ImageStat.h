/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once

enum ImageStat
{
	IS_OK= 0,
	IS_OPEN_ERR,
	IS_OUT_OF_MEM,
	IS_READ_ERROR,
	IS_FMT_NOT_SUPPORTED,
	IS_DECODING_CANCELLED,
	IS_OPERATION_NOT_SUPPORTED,
	IS_DECODING_FAILED,
	IS_NO_IMAGE
};


const TCHAR* ImageStatMsg(ImageStat status);

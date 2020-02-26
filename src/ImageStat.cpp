/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "ImageStat.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


const TCHAR* ImageStatMsg(ImageStat status)
{
	switch (status)
	{
	case IS_OK:					return _T("Ok");
	case IS_OPEN_ERR:			return _T("Open error");
	case IS_OUT_OF_MEM:			return _T("Out of memory");
	case IS_READ_ERROR:			return _T("Read error");
	case IS_FMT_NOT_SUPPORTED:	return _T("Format not supported");
	case IS_DECODING_CANCELLED:	return _T("Decoding cancelled");
	case IS_OPERATION_NOT_SUPPORTED:	return _T("Operation not supported");
	case IS_DECODING_FAILED:	return _T("Decoding failed");
	case IS_NO_IMAGE:			return _T("No image");
	}

	ASSERT(false);
	return 0;
}

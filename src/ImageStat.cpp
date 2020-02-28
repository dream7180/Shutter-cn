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
	case IS_OPEN_ERR:			return _T("打开错误");
	case IS_OUT_OF_MEM:			return _T("内存溢出");
	case IS_READ_ERROR:			return _T("读取错误");
	case IS_FMT_NOT_SUPPORTED:	return _T("格式不支持");
	case IS_DECODING_CANCELLED:	return _T("解码取消");
	case IS_OPERATION_NOT_SUPPORTED:	return _T("操作不支持");
	case IS_DECODING_FAILED:	return _T("解码失败");
	case IS_NO_IMAGE:			return _T("无图像");
	}

	ASSERT(false);
	return 0;
}

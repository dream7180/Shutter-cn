/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// CmdLineInfo.cpp: implementation of the CmdLineInfo class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "CmdLineInfo.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


CmdLineInfo::CmdLineInfo()
{
	start_scanning_ = true;
	scan_sub_folders_ = DEFAULT_SCAN;
	load_onlyEXIF_ = DEFAULT_EXIF;
	log_ = false;
	startTransferTool_ = false;
}


CmdLineInfo::~CmdLineInfo()
{}


void CmdLineInfo::ParseParam(const TCHAR* param, BOOL flag, BOOL last)
{
	if (flag)
	{
		USES_CONVERSION;
		ParseParamFlag2(T2CA(param));
	}
	else
		ParseParamNotFlag(param);

	ParseLast(last);
}

#ifdef UNICODE
void CmdLineInfo::ParseParam(const char* param, BOOL flag, BOOL last)
{
	if (flag)
		ParseParamFlag2(param);
	else
		ParseParamNotFlag(param);

	ParseLast(last);
}
#endif // UNICODE


void CmdLineInfo::ParseParamFlag2(const char* param)
{
	if (strcmp(param, "noscan") == 0)
		start_scanning_ = false;
	else if (strcmp(param, "subfolders") == 0)
		scan_sub_folders_ = SCAN;
	else if (strcmp(param, "nosubfolders") == 0)
		scan_sub_folders_ = DONT_SCAN;
	else if (strcmp(param, "exifonly") == 0)
		load_onlyEXIF_ = ONLY_EXIF;
	else if (strcmp(param, "allphotos") == 0)
		load_onlyEXIF_ = ALL_PHOTOS;
	else if (strcmp(param, "log") == 0)
		log_ = true;
	else if (strcmp(param, "transfer") == 0)
		startTransferTool_ = true;
}

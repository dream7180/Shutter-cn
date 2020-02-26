/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// CmdLineInfo.h: interface for the CmdLineInfo class.

#pragma once


class CmdLineInfo : public CCommandLineInfo
{
public:
	CmdLineInfo();
	virtual ~CmdLineInfo();

	virtual void ParseParam(const TCHAR* param, BOOL flag, BOOL last);
#ifdef _UNICODE
	virtual void ParseParam(const char* param, BOOL flag, BOOL last);
#endif

	void ParseParamFlag2(const char* param);


	// if true ExifPro will start reloading photos from current directory after starting up
	bool start_scanning_;
	enum { DEFAULT_SCAN, SCAN, DONT_SCAN } scan_sub_folders_;
	enum { DEFAULT_EXIF, ONLY_EXIF, ALL_PHOTOS } load_onlyEXIF_;
	bool log_;
	bool startTransferTool_;
};

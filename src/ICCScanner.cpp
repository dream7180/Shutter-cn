/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "ICCScanner.h"
#include "ColorProfile.h"
#include "CatchAll.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


ICCScanner::ICCScanner(std::vector<ICCProfileInfo>& icc_files) : icc_files_(icc_files)
{
	icc_files_.reserve(20);
}

ICCScanner::~ICCScanner()
{}


bool ICCScanner::FileScan(Path path, int64 file_length, const CFileFind& find, int dir_visited, bool scanSubdirs)
{
	if (path.MatchExtension(_T("icc")) || path.MatchExtension(_T("icm")))
	{
		try
		{
			ColorProfilePtr profile= new ColorProfile();

			// open profile file
			if (profile->Open(path.c_str()))
				icc_files_.push_back(ICCProfileInfo(profile, path));
		}
		catch (ColorException& ex)
		{
			CString msg= _T("读取配置文件错误.\n");
//			msg += path.c_str();

			const TCHAR* message= ex.GetMessage();
			if (message && *message)
			{
				msg += _T("\n");
				msg += message;
			}
			else
				msg += path.c_str();

			if (ex.GetErrorCode() != 0)
			{
				msg += _T("\n错误代码: ");
				TCHAR code[64];
				msg += _itot(ex.GetErrorCode(), code, 10);
			}

			AfxMessageBox(msg, MB_ICONERROR | MB_OK);
		}
		//catch (pair<int, const char*> ex)
		//{
		//	CString msg= _T("Error reading profile file.\nFile: ");
		//	msg += path.c_str();
		//	if (ex.second != 0)
		//	{
		//		msg += _T("\nError: ");
		//		msg += ex.second;
		//	}

		//	AfxMessageBox(msg, MB_ICONERROR | MB_OK);
		//}
		CATCH_ALL_W(AfxGetMainWnd())
	}

	return true;
}

/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "WhistlerLook.h"
#include "Path.h"

DWORD GetDllVersion(LPCTSTR dll_name);

static DWORD CommonControlLibVersion= GetDllVersion(_T("comctl32.dll"));


namespace WhistlerLook
{
	bool IsAvailable()
	{
		static bool available= CommonControlLibVersion >= PACKVERSION(6, 0);

		return available;
	}
}


bool IsDLLAvailable(const TCHAR* dll_name)
{
	Path path;
	TCHAR dir[MAX_PATH];
	if (::GetSystemDirectory(dir, MAX_PATH))
		path = dir;

	path.AppendDir(dll_name, false);

	HINSTANCE hinst_dll= ::LoadLibrary(path.c_str());
	bool ok= hinst_dll != nullptr;
	::FreeLibrary(hinst_dll);

	return ok;
}


namespace Direct2D
{
	bool IsAvailable()
	{
		static bool available= IsDLLAvailable(L"d2d1.dll");

		return available;
	}
}

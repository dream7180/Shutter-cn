/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "AutoComplete.h"

typedef HRESULT (__stdcall *fnSHAutoComplete)(HWND edit, DWORD flags);

static fnSHAutoComplete FindSHAutoCompleteFn()
{
	if (HINSTANCE hinstDll= ::LoadLibrary(_T("Shlwapi.dll")))
	{
		// look for SHAutoComplete function

		fnSHAutoComplete pfnAutoComplete= reinterpret_cast<fnSHAutoComplete>(::GetProcAddress(hinstDll, "SHAutoComplete"));

		if (pfnAutoComplete != 0)
			return pfnAutoComplete;	// return fn address, do not free loaded library

		::FreeLibrary(hinstDll);
	}

	return 0;
}

static fnSHAutoComplete pfnAutoCompleteFn= FindSHAutoCompleteFn();


namespace AutoComplete
{
	bool TurnOn(HWND edit_ctrl, DWORD flags)
	{
		if (pfnAutoCompleteFn)
			return (*pfnAutoCompleteFn)(edit_ctrl, flags) == S_OK;
		else
			return false;
	}

	bool TurnOn(HWND edit_ctrl)
	{
		return TurnOn(edit_ctrl, SHACF_FILESYSTEM);
	}
}

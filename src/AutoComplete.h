/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
#include <shlwapi.h>


namespace AutoComplete
{
	bool TurnOn(HWND edit_ctrl);

	bool TurnOn(HWND edit_ctrl, DWORD flags);
}

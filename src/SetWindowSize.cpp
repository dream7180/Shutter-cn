/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "SetWindowSize.h"


static CSize SetWindowSize(CWnd& wnd, int x, int y, int width, int height, UINT flags)
{
	CSize size(std::max(0, width), std::max(0, height));
	wnd.SetWindowPos(0, x, y, size.cx, size.cy, flags);
	return size;
}


CSize SetWindowSize(CWnd& wnd, int x, int y, int width, int height)
{
	return SetWindowSize(wnd, x, y, width, height, SWP_NOZORDER | SWP_NOACTIVATE);
}


CSize SetWindowSize(CWnd& wnd, int width, int height)
{
	return SetWindowSize(wnd, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
}

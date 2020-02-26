/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


extern void EnableCtrl(CWnd* wnd, bool enable, bool hide_disabled/*= true*/)
{
	if (wnd == 0 || wnd->m_hWnd == 0)
		return;

	if (enable)
	{
		wnd->EnableWindow(enable);
		wnd->ShowWindow(SW_SHOWNA);
	}
	else
	{
		if (hide_disabled)
			wnd->ShowWindow(SW_HIDE);
		wnd->EnableWindow(enable);
		if (!hide_disabled)
			wnd->ShowWindow(SW_SHOWNA);
	}
}

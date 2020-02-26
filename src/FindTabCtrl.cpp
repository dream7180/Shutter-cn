/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"

#if 1
namespace {

// is ctrl window and its parent windows enabled?
//
bool IsCtrlWindowEnabled(HWND wnd)
{
	if (!::IsWindowEnabled(wnd) && ::GetParent(wnd) != 0)
		return false;

	wnd = ::GetParent(wnd);
	if (wnd == 0)
		return true;

	if (::GetWindowLong(wnd, GWL_STYLE) & WS_POPUP)
		return true;

	return IsCtrlWindowEnabled(wnd);
}


BOOL EnumChildProc(HWND wnd, CWnd** ctrl)
{
	if (IsCtrlWindowEnabled(wnd) && ::IsWindowVisible(wnd))
		if ((::GetWindowLong(wnd, GWL_STYLE) & WS_TABSTOP) &&
			(::GetWindowLong(wnd, GWL_EXSTYLE) & WS_EX_CONTROLPARENT) == 0)
		{
			*ctrl = CWnd::FromHandle(wnd);
			return false;
		}
	return true;
}


BOOL CALLBACK EnumChildProcStatic(HWND wnd, LPARAM lParam)
{
	return EnumChildProc(wnd, reinterpret_cast<CWnd**>(lParam));
}

} // namespace end
#endif


extern CWnd* FindFirstTabCtrl(CWnd* parent)
{
	ASSERT(parent);
	if (parent == 0 || parent->m_hWnd == 0)
		return 0;

//	return parent->GetNextDlgTabItem(0);

	CWnd* ctrl= 0;
	// Note: this is not perfect as EnumChildWindows traverses all children windows rather than
	// those whose parents have WS_EX_CONTROLPARENT style
	::EnumChildWindows(parent->m_hWnd, EnumChildProcStatic, reinterpret_cast<LPARAM>(&ctrl));

	return ctrl;
}

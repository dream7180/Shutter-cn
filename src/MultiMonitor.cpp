/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "StdAfx.h"
#include "MultiMonitor.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


namespace {

	typedef HMONITOR (WINAPI *fnMonitorFromRect)(IN LPCRECT lprc, IN DWORD flags);
	typedef BOOL (WINAPI *fnGetMonitorInfo)(IN HMONITOR monitor, OUT LPMONITORINFO lpmi);

	bool initialized_= false;
	fnMonitorFromRect pfnMonitorFromRect= 0;
	fnGetMonitorInfo pfnGetMonitorInfo= 0;

}


static void InitFnPointers()
{
	if (!initialized_)
	{
		initialized_ = true;

		if (HMODULE lib= ::LoadLibrary(_T("User32.dll")))
		{
			pfnMonitorFromRect = reinterpret_cast<fnMonitorFromRect>(::GetProcAddress(lib, "MonitorFromRect"));

			pfnGetMonitorInfo = reinterpret_cast<fnGetMonitorInfo>(::GetProcAddress(lib,
#ifdef UNICODE
				"GetMonitorInfoW"));
#else
				"GetMonitorInfoA"));
#endif // !UNICODE

		}
	}
}


extern CRect GetWorkArea(const CRect& window_rect, bool useMonitorArea/*= false*/)
{
	InitFnPointers();

	CRect work_rect(0, 0, 0, 0);

	if (pfnMonitorFromRect && pfnGetMonitorInfo)
	{
		HMONITOR mon= (*pfnMonitorFromRect)(window_rect, MONITOR_DEFAULTTONEAREST);

		MONITORINFO mi;
		memset(&mi, 0, sizeof mi);
		mi.cbSize = sizeof mi;

		if ((*pfnGetMonitorInfo)(mon, &mi))
			work_rect = useMonitorArea ? mi.rcMonitor : mi.rcWork;
	}

	if (work_rect.IsRectEmpty())
		::SystemParametersInfo(SPI_GETWORKAREA, 0, work_rect, 0);

	return work_rect;
}


extern void RectToWorkArea(CRect& window, bool resize, bool use_monitor_area/*= false*/)
{
	InitFnPointers();

	CRect work(0, 0, 0, 0);

	if (pfnMonitorFromRect && pfnGetMonitorInfo)
	{
		HMONITOR mon= (*pfnMonitorFromRect)(window, MONITOR_DEFAULTTONEAREST);

		MONITORINFO mi;
		memset(&mi, 0, sizeof mi);
		mi.cbSize = sizeof mi;

		if ((*pfnGetMonitorInfo)(mon, &mi))
			work = use_monitor_area ? mi.rcMonitor : mi.rcWork;
	}

	if (work.IsRectEmpty())
	{
		::SystemParametersInfo(SPI_GETWORKAREA, 0, work, 0);
	}

	// prevent from appearing outside of the work area

	CRect rect= window;

	// amount of pixels that window may be off screen to the left & top
	const long extra= 8;

	CRect clip= work;
	clip.OffsetRect(-extra, -extra);

	// make sure window is visible in the 'work' area and user can still grab it by a title bar

	if (rect.left < clip.left)
		rect.left = clip.left;
	else if (rect.left > clip.right)
		rect.left = clip.right;

	if (rect.top < clip.top)
		rect.top = clip.top;
	else if (rect.top > clip.bottom)
		rect.top = clip.bottom;

//	rect.left	= max(work_rect.left - extra, min<long>(work_rect.right - window_rect.Width(),  rect.left));
//	rect.top	= max(work_rect.top - extra,  min<long>(work_rect.bottom - window_rect.Height(), rect.top));
	rect.right	= rect.left + window.Width();
	rect.bottom	= rect.top  + window.Height();

	// note: no size change; only shif if needed

	window = rect;
}


// find monitor at the center of window_rect and return it's max rect
//
extern CRect GetFullScreenRect(const CRect& window_rect)
{
	InitFnPointers();

	CRect rect(0, 0, 0, 0);

	if (pfnMonitorFromRect && pfnGetMonitorInfo)
	{
		HMONITOR mon= (*pfnMonitorFromRect)(window_rect, MONITOR_DEFAULTTONEAREST);

		MONITORINFO mi;
		memset(&mi, 0, sizeof mi);
		mi.cbSize = sizeof mi;

		if ((*pfnGetMonitorInfo)(mon, &mi))
			rect = mi.rcMonitor;
	}

	if (rect.IsRectEmpty())
	{
		CDC info_dc;
		info_dc.CreateIC(_T("DISPLAY"), NULL, NULL, NULL);
		rect.right = info_dc.GetDeviceCaps(HORZRES);
		rect.bottom = info_dc.GetDeviceCaps(VERTRES);
	}

	return rect;
}


WindowPosition::WindowPosition(const TCHAR* registry_entry, int index, const CRect* default_rect/*= 0*/)
{
	Init(registry_entry, index, default_rect);
}


WindowPosition::WindowPosition(const TCHAR* registry_entry, const CRect* default_rect/*= 0*/)
{
	Init(registry_entry, -1, default_rect);
}


void WindowPosition::Init(const TCHAR* registry_entry, int index, const CRect* default_rect)
{
	CRect rect;

	if (default_rect)
		rect = *default_rect;
	else
	{
		::SystemParametersInfo(SPI_GETWORKAREA, 0, rect, 0);
		rect.DeflateRect(40, 25);
	}

	if (index > 0)
		registry_.Format(_T("%s_%04x"), registry_entry, index);
	else
		registry_ = registry_entry;

	profile_left_.Register(registry_, _T("PosX"), rect.left);
	profile_top_.Register(registry_, _T("PosY"), rect.top);
	profile_width_.Register(registry_, _T("PosW"), rect.Width());
	profile_height_.Register(registry_, _T("PosH"), rect.Height());
	profile_maximized_.Register(registry_, _T("Maximized"), false);
}


bool WindowPosition::IsRegEntryPresent()
{
	return profile_left_.IsRegEntryPresent();
}


CRect WindowPosition::GetLocation(bool enforce_safe_limits)
{
	CRect rect(CPoint(profile_left_, profile_top_), CSize(profile_width_, profile_height_));
	// do not automatically limit restored area here, let SetWindowPlacement deal with coordinates
	if (enforce_safe_limits)
		RectToWorkArea(rect, true);
	return rect;
}


CPoint WindowPosition::GetPosition()
{
	CPoint pos(profile_left_, profile_top_);
	CRect rect(pos, pos);
	RectToWorkArea(rect, true);
	return rect.TopLeft();
}


void WindowPosition::StoreState(CWnd& wnd, const CRect& window_rect)
{
	profile_left_	= window_rect.left;
	profile_top_	= window_rect.top;
	profile_width_	= window_rect.Width();
	profile_height_	= window_rect.Height();

	WINDOWPLACEMENT wp;
	if (wnd.GetWindowPlacement(&wp))
		profile_maximized_ = wp.showCmd == SW_SHOWMAXIMIZED;
}


void WindowPosition::StoreState(CWnd& wnd)
{
	WINDOWPLACEMENT wp;
	if (wnd.GetWindowPlacement(&wp))
		StoreState(wp);
}


void WindowPosition::StoreState(const WINDOWPLACEMENT& wp)
{
	CRect rect(wp.rcNormalPosition);

	profile_left_	= rect.left;
	profile_top_	= rect.top;
	profile_width_	= rect.Width();
	profile_height_	= rect.Height();

	profile_maximized_ = wp.showCmd == SW_SHOWMAXIMIZED;
}


void WindowPosition::StorePosition(const WINDOWPLACEMENT& wp)
{
	profile_left_	= wp.rcNormalPosition.left;
	profile_top_	= wp.rcNormalPosition.top;
}

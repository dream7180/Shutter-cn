/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once

extern void RectToWorkArea(CRect& rect, bool resize, bool use_monitor_area= false);

extern CRect GetWorkArea(const CRect& window_rect, bool use_monitor_area= false);

extern CRect GetFullScreenRect(const CRect& window_rect);

#include "Profile.h"


class WindowPosition
{
public:
	WindowPosition(const TCHAR* registry_entry, const CRect* default_rect= 0);
	WindowPosition(const TCHAR* registry_entry, int index, const CRect* default_rect= 0);

	CRect GetLocation(bool enforce_safe_limits);
	CPoint GetPosition();

	bool IsMaximized()			{ return profile_maximized_; }

	bool IsRegEntryPresent();

	// store current location
	void StoreState(CWnd& wnd);

	// store given location
	void StoreState(CWnd& wnd, const CRect& window_rect);

	// as above
	void StoreState(const WINDOWPLACEMENT& wp);

	// store current location
	void StorePosition(const WINDOWPLACEMENT& wp);

private:
	Profile<int> profile_left_;
	Profile<int> profile_top_;
	Profile<int> profile_width_;
	Profile<int> profile_height_;
	Profile<bool> profile_maximized_;

private:
	void WindowPosition::Init(const TCHAR* registry_entry, int index, const CRect* default_rect);
	CString registry_;
};

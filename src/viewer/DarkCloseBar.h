/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once

#include "FancyToolBar.h"
#include "../Dib.h"

/////////////////////////////////////////////////////////////////////////////
// DarkCloseBar window

class DarkCloseBar : public CWnd
{
public:
	DarkCloseBar() {}

	bool Create(CWnd* parent);

	static const int CLOSE_BAR_WIDTH= 106;

protected:
	DECLARE_MESSAGE_MAP()

private:
	class Toolbar : public FancyToolBar
	{
		virtual CString GetToolTip(int cmdId);
	};

	Toolbar toolbar_;
	Dib backgnd_;

	BOOL OnEraseBkgnd(CDC* dc);
	LRESULT OnPrintClient(WPARAM HDC, LPARAM flags);
	void OnSize(UINT type, int cx, int cy);
};

/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// SmoothScroll.cpp: implementation of the CSmoothScroll class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SmoothScroll.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSmoothScroll::CSmoothScroll(CWnd* wnd) : wnd_(wnd)
{
	ASSERT(wnd);
}


void CSmoothScroll::Hide(CRect rect, COLORREF rgb_back)
{
	CClientDC dc(wnd_);

	const int STEPS= 25;
	int delta= rect.Height() / STEPS;

	for (int i= STEPS - 1; i >= 0; --i)
	{
		int height= rect.Height() * i / STEPS;

		dc.BitBlt(rect.left, rect.top, rect.Width(), height, &dc, rect.left, rect.top + delta, SRCCOPY);
		dc.FillSolidRect(rect.left, rect.top + height, rect.Width(), rect.Height() - height, rgb_back);
		::GdiFlush();
		::Sleep(10);
	}
}

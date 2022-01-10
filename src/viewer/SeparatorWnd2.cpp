/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// ViewerSeparatorWnd.cpp : implementation file
//

#include "stdafx.h"
#include "../resource.h"
#include "SeparatorWnd2.h"
#include "../MemoryDC.h"
#include "../Color.h"
#include "../ResizeWnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// ViewerSeparatorWnd

ViewerSeparatorWnd::ViewerSeparatorWnd() : start_(0, 0)
{
	resizing_ = false;
	pane_height_ = 0;
	resize_wnd_ = 0;
	horizontal_ = true;
}

ViewerSeparatorWnd::~ViewerSeparatorWnd()
{}


BEGIN_MESSAGE_MAP(ViewerSeparatorWnd, CWnd)
	//{{AFX_MSG_MAP(ViewerSeparatorWnd)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_SETCURSOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// ViewerSeparatorWnd message handlers


bool ViewerSeparatorWnd::Create(CWnd* parent, ResizeWnd* resize_wnd)
{
	resize_wnd_ = resize_wnd;

	const TCHAR* class_name= AfxRegisterWndClass(CS_VREDRAW | CS_HREDRAW,
//		AfxGetApp()->LoadCursor(IDC_VERT_RESIZE));
		AfxGetApp()->LoadStandardCursor(IDC_SIZENS));

	if (!CWnd::Create(class_name, _T(""), WS_CHILD | WS_VISIBLE, CRect(0,0,0,0), parent, -1))
		return false;

	return true;
}


BOOL ViewerSeparatorWnd::OnEraseBkgnd(CDC* dc)
{
	bool whistler= true;
	COLORREF rgb_darker_color= CalcNewColor(::GetSysColor(COLOR_3DSHADOW), 10.0);

	COLORREF rgb_gray= ::GetSysColor(COLOR_3DFACE);
	COLORREF rgb_dark= whistler ? rgb_darker_color : rgb_gray;
	COLORREF rgb_light= ::GetSysColor(COLOR_3DHILIGHT);

	CRect rect;
	GetClientRect(rect);

//	dc->FillSolidRect(rect, 0);
//	DrawSeparatorBar(*dc, rect, true);

	int incx= 0, incy= 0;
	int w= 1, h= 1;

	if (horizontal_)
	{
		incy = 1;
		w = rect.Width();
	}
	else
	{
		incx = 1;
		h = rect.Height();
	}

	int y= rect.top;
	int x= rect.left;
	dc->FillSolidRect(x, y, w, h, RGB(80,80,80));
	x += incx; y += incy;
	dc->FillSolidRect(x, y, w, h, RGB(140,140,140));
	x += incx; y += incy;
	dc->FillSolidRect(x, y, w, h, RGB(140,140,140));
	x += incx; y += incy;
	dc->FillSolidRect(x, y, w, h, RGB(140,140,140));
	x += incx; y += incy;
	dc->FillSolidRect(x, y, w, h, RGB(140,140,140));
	x += incx; y += incy;
	dc->FillSolidRect(x, y, w, h, RGB(140,140,140));
	x += incx; y += incy;
	dc->FillSolidRect(x, y, w, h, RGB(80,80,80));
/*
	dc->FillSolidRect(0, y++, rect.Width(), 1, rgb_gray);
	dc->FillSolidRect(0, y++, rect.Width(), 1, whistler ? rgb_light : rgb_gray);
	dc->FillSolidRect(0, y, rect.Width(), GetHeight() - 4, rgb_gray);

	// draw rivets
	int x= rect.CenterPoint().x - 10;
	for (int n= 0; n < 6; ++n, x += 5)
	{
		dc->FillSolidRect(x, y, 2, 2, rgb_light);
		dc->FillSolidRect(x + 1, y + 1, 2, 2, rgb_darker_color);
	}

	y += GetHeight() - 4;
	dc->FillSolidRect(0, y++, rect.Width(), 1, rgb_light);
	dc->FillSolidRect(0, y++, rect.Width(), 1, rgb_dark);
*/
	return true;
/*
	MemoryDC mem_dc(*dc, this, ::GetSysColor(COLOR_3DFACE));

	CRect client_rect;
	GetClientRect(client_rect);

	CRect rect= client_rect;
	rect.bottom = rect.top + 1;

	mem_dc.FillSolidRect(rect, ::GetSysColor(COLOR_3DFACE));
	rect.OffsetRect(0, 1);
	mem_dc.FillSolidRect(rect, ::GetSysColor(COLOR_3DHILIGHT));

	rect = client_rect;
	rect.top = rect.bottom - 1;

//	mem_dc.FillSolidRect(rect, ::GetSysColor(COLOR_3DSHADOW));
	mem_dc.FillSolidRect(rect, CalcNewColor(::GetSysColor(COLOR_3DSHADOW), 10.0));

	mem_dc.BitBlt();

	return CWnd::OnEraseBkgnd(dc);
*/
}
/*
#if 0
extern COLORREF CalcShade(COLORREF rgb_color, float shade)
{
	shade /= 100.0f;

	int red= GetRValue(rgb_color);
	int green= GetGValue(rgb_color);
	int blue= GetBValue(rgb_color);

	if (shade > 0.0f)	// lighter
	{
		return RGB(red + shade * (0xff - red), green + shade * (0xff - green), blue + shade * (0xff - blue));
	}
	else if (shade < 0.0f)	// darker
	{
		shade = 1.0f + shade;

		return RGB(red * shade, green * shade, blue * shade);
	}

	return rgb_color;
}


void ViewerSeparatorWnd::DrawSeparatorBar(CDC& dc, CRect rect, bool horizontal)
{
	COLORREF rgb_gray= ::GetSysColor(COLOR_3DFACE);

	static const float shade[]= { 0.0f, 40.0f, 98.0f, 40.0f, 0.0f, -5.0f, -18.0f };
	CPoint pos= rect.TopLeft();

	for (int i= 0; i < array_count(shade); ++i)
	{
		COLORREF rgb_color= CalcShade(rgb_gray, shade[i]);

		if (horizontal)
			dc.FillSolidRect(pos.x, pos.y++, rect.Width(), 1, rgb_color);
		else
			dc.FillSolidRect(pos.x++, pos.y, 1, rect.Height(), rgb_color);
	}


	COLORREF rgb_darker_color= CalcNewColor(::GetSysColor(COLOR_3DSHADOW), 10.0);
	COLORREF rgb_light= ::GetSysColor(COLOR_3DHILIGHT);

	// draw rivets
	if (horizontal)
	{
		int x= rect.CenterPoint().x - 10;
		int y= rect.top;
		for (int n= 0; n < 6; ++n, x += 5)
		{
			dc.FillSolidRect(x, y + 2, 2, 2, rgb_darker_color);
			//dc.FillSolidRect(x + 1, y + 3, 2, 2, rgb_light);
		}
	}
	else
	{
		int x= rect.left;
		int y= rect.CenterPoint().y - 10;
		for (int n= 0; n < 6; ++n, y += 5)
		{
			dc.FillSolidRect(x + 2, y, 2, 2, rgb_darker_color);
			//dc.FillSolidRect(x + 3, y + 1, 2, 2, rgb_light);
		}
	}
}

#endif
*/
void ViewerSeparatorWnd::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// Do not call CWnd::OnPaint() for painting messages
}


void ViewerSeparatorWnd::OnLButtonDown(UINT flags, CPoint pos)
{
	if (resize_wnd_)
	{
		SetCapture();
		::GetCursorPos(&start_);
		pane_height_ = resize_wnd_->GetPaneHeight();
		resizing_ = true;
		resize_wnd_->Resizing(resizing_);
	}
}


void ViewerSeparatorWnd::OnLButtonUp(UINT flags, CPoint pos)
{
	if (resize_wnd_)
	{
		ReleaseCapture();
		resizing_ = false;
		resize_wnd_->Resizing(resizing_);
	}
}


void ViewerSeparatorWnd::OnMouseMove(UINT flags, CPoint pos)
{
	SetCursor();
	if ((flags & MK_LBUTTON) && resizing_)
	{
		ASSERT(resize_wnd_);
		::GetCursorPos(&pos);
		CSize delta_size= pos - start_;
		resize_wnd_->ResizePane(horizontal_ ? pane_height_ - delta_size.cy : pane_height_ + delta_size.cx);
	}
}


void ViewerSeparatorWnd::Show(bool show)
{
	if (m_hWnd == 0)
		return;

	if (!!(GetStyle() & WS_VISIBLE) == show)
		return;

	if (show)
	{
		ModifyStyle(WS_DISABLED, 0);
		ShowWindow(SW_SHOWNA);
	}
	else
		ModifyStyle(WS_VISIBLE, WS_DISABLED);
}


void ViewerSeparatorWnd::SetOrientation(bool horizontal)
{
	horizontal_ = horizontal;
	Invalidate();
}


BOOL ViewerSeparatorWnd::OnSetCursor(CWnd* wnd, UINT hitTest, UINT message)
{
	SetCursor();
	return true;
}


void ViewerSeparatorWnd::SetCursor()
{
	::SetCursor(AfxGetApp()->LoadStandardCursor(horizontal_ ? IDC_SIZENS : IDC_SIZEWE));
}

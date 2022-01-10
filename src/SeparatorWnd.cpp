/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// SeparatorWnd.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "SeparatorWnd.h"
#include "MemoryDC.h"
#include "Color.h"
#include "ResizeWnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// SeparatorWnd

static const TCHAR* const WND_CLASS= _T("SeparatorCtrlMiK");

static void RegisterWndClass(const TCHAR* class_name)
{
	HINSTANCE instance= AfxGetInstanceHandle();

	// see if the class already exists
	WNDCLASS wndcls;
	if (!::GetClassInfo(instance, class_name, &wndcls))
	{
		// otherwise we need to register a new class
		wndcls.style = CS_VREDRAW | CS_HREDRAW;
		wndcls.lpfnWndProc = ::DefWindowProc;
		wndcls.cbClsExtra = wndcls.cbWndExtra = 0;
		wndcls.hInstance = instance;
		wndcls.hIcon = 0;
		wndcls.hCursor = 0;//AfxGetApp()->LoadCursor(IDC_VERT_RESIZE);
		wndcls.hbrBackground = 0;
		wndcls.lpszMenuName = NULL;
		wndcls.lpszClassName = class_name;

		AfxRegisterClass(&wndcls);
	}
}


SeparatorWnd::SeparatorWnd(bool horizontal) : start_(0, 0)
{
	resizing_ = false;
	pane_height_ = 0;
	resize_wnd_ = 0;
	horizontal_ = horizontal;
	draw_bar_ = true;
	grow_when_moving_left_ = true;

	RegisterWndClass(WND_CLASS);

	cursor_ = AfxGetApp()->LoadCursor(horizontal ? IDC_VERT_RESIZE : IDC_HORZ_RESIZE);
}

SeparatorWnd::~SeparatorWnd()
{}


BEGIN_MESSAGE_MAP(SeparatorWnd, CWnd)
	//{{AFX_MSG_MAP(SeparatorWnd)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_SETCURSOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// SeparatorWnd message handlers

void SeparatorWnd::DrawBar(bool draw)
{
	draw_bar_ = draw;
	if (m_hWnd)
		Invalidate();
}


bool SeparatorWnd::Create(CWnd* parent, ResizeWnd* resize_wnd)
{
	resize_wnd_ = resize_wnd;

//	const TCHAR* class_name= AfxRegisterWndClass(CS_VREDRAW | CS_HREDRAW,
//		AfxGetApp()->LoadCursor(IDC_VERT_RESIZE));

	if (!CWnd::Create(WND_CLASS, _T(""), WS_CHILD | WS_VISIBLE, CRect(0,0,0,0), parent, -1))
		return false;

	return true;
}


extern void DrawSeparatorRivets(CDC& dc, CRect rect, bool horizontal)
{
	COLORREF rgb_darker_color= ::GetSysColor(COLOR_3DSHADOW); //CalcNewColor(::GetSysColor(COLOR_3DSHADOW), 0.0);
	//COLORREF rgb_light= ::GetSysColor(COLOR_3DHILIGHT);

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


BOOL SeparatorWnd::OnEraseBkgnd(CDC* dc)
{
	bool whistler= true; //GetApp()->IsWhistlerLookAvailable();
	COLORREF rgb_darker_color= CalcNewColor(::GetSysColor(COLOR_3DSHADOW), 10.0);

	COLORREF gray= ::GetSysColor(COLOR_3DFACE);
	COLORREF rgb_dark= whistler ? rgb_darker_color : gray;
	//COLORREF rgb_light= ::GetSysColor(COLOR_3DHILIGHT);

	CRect rect(0,0,0,0);
	GetClientRect(rect);

	if (draw_bar_)
		DrawSeparatorBar(*dc, rect, horizontal_);
	else
	{
		COLORREF gray= ::GetSysColor(COLOR_3DFACE);
		dc->FillSolidRect(rect, gray);
		DrawSeparatorRivets(*dc, rect, horizontal_);
	}
/*
	int y= rect.top;

	dc->FillSolidRect(0, y++, rect.Width(), 1, gray);
	dc->FillSolidRect(0, y++, rect.Width(), 1, whistler ? rgb_light : gray);
	dc->FillSolidRect(0, y, rect.Width(), GetHeight() - 4, gray);

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


void SeparatorWnd::DrawSeparatorBar(CDC& dc, CRect rect, bool horizontal)
{
	COLORREF gray= ::GetSysColor(COLOR_3DFACE);

	/*static const float shades[]= { 0.0f, 40.0f, 98.0f, 40.0f, 0.0f, -5.0f, -18.0f };
	CPoint pos= rect.TopLeft();

	for (int i= 0; i < array_count(shades); ++i)
	{
		COLORREF rgb_color= gray;//CalcShade(gray, shades[i]);

		if (horizontal)
			dc.FillSolidRect(pos.x, pos.y++, rect.Width(), 1, rgb_color);
		else
			dc.FillSolidRect(pos.x++, pos.y, 1, rect.Height(), rgb_color);
	}

	if (horizontal && rect.Height() > array_count(shades))
		dc.FillSolidRect(pos.x, pos.y, rect.Width(), rect.Height() - array_count(shades), gray);
	else if (!horizontal && rect.Width() > array_count(shades))
		dc.FillSolidRect(pos.x, pos.y, rect.Width() - array_count(shades), rect.Height(), gray);
*/
	dc.FillSolidRect(rect.left, rect.top, rect.Width(), rect.Height(), gray);
	dc.FillSolidRect(rect.left, rect.top, rect.Width(), 1, RGB(160, 160, 160));
	dc.FillSolidRect(rect.left, rect.top, 1, rect.Height(), RGB(160, 160, 160));
	//rect.DeflateRect(0,2,0,0);
	rect.DeflateRect(1,1,0,0);
	DrawSeparatorRivets(dc, rect, horizontal);
}


void SeparatorWnd::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// Do not call CWnd::OnPaint() for painting messages
}


void SeparatorWnd::OnLButtonDown(UINT flags, CPoint pos)
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


void SeparatorWnd::OnLButtonUp(UINT flags, CPoint pos)
{
	if (resize_wnd_)
	{
		ReleaseCapture();
		resizing_ = false;
		resize_wnd_->Resizing(resizing_);
	}
}


void SeparatorWnd::OnMouseMove(UINT flags, CPoint pos)
{
	if ((flags & MK_LBUTTON) && resizing_)
	{
		ASSERT(resize_wnd_);
		::GetCursorPos(&pos);
		CSize delta_size= pos - start_;
		int delta= horizontal_ ? delta_size.cy : delta_size.cx;
		resize_wnd_->ResizePane(pane_height_ + (grow_when_moving_left_ ? delta : -delta));
	}
}


void SeparatorWnd::Show(bool show)
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


void SeparatorWnd::SetCallbacks(ResizeWnd* client_interface)
{
	resize_wnd_ = client_interface;
}


BOOL SeparatorWnd::OnSetCursor(CWnd* wnd, UINT hit_test, UINT message)
{
	SetCursor(cursor_);
	return true;
}


void SeparatorWnd::GrowWhenMovingLeft(bool set)
{
	grow_when_moving_left_ = set;
}
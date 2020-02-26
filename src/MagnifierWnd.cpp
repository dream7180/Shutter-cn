/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// MagnifierWnd.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "MagnifierWnd.h"
#include "MemoryDC.h"
#include "RMenu.h"
#include "Exception.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

MagnifierWnd* MagnifierWnd::magnifier_wnd_= 0;
/////////////////////////////////////////////////////////////////////////////
// MagnifierWnd

void MagnifierWnd::Close()
{
	if (magnifier_wnd_)
		magnifier_wnd_->DestroyWindow();
	if (magnifier_wnd_)
		delete magnifier_wnd_;
	magnifier_wnd_ = 0;
}


MagnifierWnd::MagnifierWnd(DibPtr dibImage, const CRect& display_rect, CWnd* display_wnd)
 : bitmap_rect_(display_rect), dib_image_(dibImage), display_(display_wnd)
{
	// make sure there's only one window at a time
	if (magnifier_wnd_ != 0)
		delete magnifier_wnd_;

	if (dib_image_.get() == 0)
		THROW_EXCEPTION(L"Magnifier window error", L"MagnifierWnd ctor - expected image");

	magnification_factor_ = 1;
	magnifier_wnd_ = this;

	const TCHAR* REG_MAGNIFIER= _T("MagnifierGlass");
	profile_window_size_.Register(REG_MAGNIFIER, _T("WndSize"), MEDIUM);
	profile_magnification_factor_.Register(REG_MAGNIFIER, _T("MagnifFactor"), magnification_factor_);

	magnification_factor_ = profile_magnification_factor_;
	CSize window_size;
	window_size.cx = window_size.cy = profile_window_size_;
	if (window_size.cx < SMALL || window_size.cx > BIG)
		window_size.cx = window_size.cy = MEDIUM;

	offset_ = CPoint(0, 0);

	if (!CreateEx(WS_EX_WINDOWEDGE | WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
		AfxRegisterWndClass(0 /*CS_SAVEBITS*/, ::LoadCursor(NULL, IDC_CROSS)), 0,
		WS_POPUP | WS_CLIPCHILDREN | MFS_SYNCACTIVE /*| MFS_THICKFRAME*/, CRect(CPoint(0, 0), window_size), display_wnd/*AfxGetMainWnd()*/))
		return;

	// trace mouse outside of the window too
	timer_ = SetTimer(1, 10, 0);

	SetWindowPos(0, 0, 0, window_size.cx, window_size.cy, SWP_NOMOVE | SWP_NOZORDER);

	m_bAutoMenuEnable = false;
	popup_menu_ = false;

	// very narrow and tall or very wide and small bitmaps may be reduced to 0 size when zoomed out
	if (bitmap_rect_.Width() < 1)
		bitmap_rect_.right = bitmap_rect_.left + 1;
	if (bitmap_rect_.Height() < 1)
		bitmap_rect_.bottom = bitmap_rect_.top + 1;

	if (display_)
	{
		display_->GetWindowRect(limit_rect_);
		limit_rect_ &= bitmap_rect_;
	}
	else
		limit_rect_ = bitmap_rect_;

	// show window under mouse cursor
	last_mouse_pos_ = CPoint(99999,99999);
	MouseMoved();
}


MagnifierWnd::~MagnifierWnd()
{}


BEGIN_MESSAGE_MAP(MagnifierWnd, CMiniFrameWnd)
	//{{AFX_MSG_MAP(MagnifierWnd)
	ON_WM_ERASEBKGND()
	ON_WM_MOUSEMOVE()
	ON_WM_ACTIVATE()
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_WM_LBUTTONDOWN()
	ON_WM_KEYDOWN()
	ON_WM_MOUSEWHEEL()
	ON_WM_CONTEXTMENU()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_USER, OnFinish)
	ON_WM_MBUTTONDOWN()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// MagnifierWnd message handlers


BOOL MagnifierWnd::OnEraseBkgnd(CDC* dc)
{
	try
	{
		return EraseBkgnd(dc);
	}
	catch (...)
	{}
	return true;
}


BOOL MagnifierWnd::EraseBkgnd(CDC* pdc)
{
	MemoryDC dc(*pdc, this, RGB(200,200,200));

	CRect rect;
	GetClientRect(rect);

	COLORREF rgb_outline= ::GetSysColor(COLOR_3DSHADOW);
	dc.Draw3dRect(rect, rgb_outline, rgb_outline);

	rect.DeflateRect(1, 1);

	// draw bitmap

	const int MAG= magnification_factor_;
	CPoint src_center( offset_.x * dib_image_->GetWidth() / std::max(1, bitmap_rect_.Width()),
						offset_.y * dib_image_->GetHeight() / std::max(1, bitmap_rect_.Height()) );

	CSize src_size(rect.Width() / MAG, rect.Height() / MAG);

	int x= src_center.x;
	int y= src_center.y;

	src_center.x -= src_size.cx / 2;
	src_center.y -= src_size.cy / 2;

	CRect src_rect(src_center, src_size);
	dib_image_->Draw(&dc, rect, &src_rect, false);

	dc.SelectStockObject(SYSTEM_FONT);
	dc.SetBkMode(TRANSPARENT);
	CString str;
	str.Format(_T("x%d"), MAG);
	dc.SetTextColor(RGB(255,255,255));
	dc.TextOut(5, 2, str);
	dc.TextOut(3, 2, str);
	dc.TextOut(5, 0, str);
	dc.TextOut(3, 0, str);
	dc.SetTextColor(RGB(0,0,0));
	dc.TextOut(4, 1, str);

	// print RGB values of central pixel
	if (x < 0)
		x = 0;
	else if (x >= dib_image_->GetWidth())
		x = dib_image_->GetWidth() - 1;
	if (y < 0)
		y = 0;
	else if (y >= dib_image_->GetHeight())
		y = dib_image_->GetHeight() - 1;

	RGBQUAD q= dib_image_->GetPixel(x, y);

	str.Format(_T("R: %d G: %d B: %d"), int(q.rgbRed), int(q.rgbGreen), int(q.rgbBlue));
	{
		CSize s= pdc->GetTextExtent(str);
		int x= rect.right - s.cx - 4;
		int y= rect.bottom - s.cy - 2;

		dc.SetTextColor(RGB(255,255,255));
		dc.TextOut(x+1, y+1, str);
		dc.TextOut(x-1, y+1, str);
		dc.TextOut(x+1, y-1, str);
		dc.TextOut(x-1, y-1, str);
		dc.SetTextColor(RGB(0,0,0));
		dc.TextOut(x, y, str);
	}

	dc.BitBlt();

	return true;
}


void MagnifierWnd::PostNcDestroy()
{
	delete this;
	magnifier_wnd_ = 0;
}


void MagnifierWnd::OnMouseMove(UINT flags, CPoint point)
{
	MouseMoved();
}


void MagnifierWnd::MouseMoved()
{
	if (popup_menu_)
		return;

	CPoint pos(0, 0);
	GetCursorPos(&pos);

	if (pos != last_mouse_pos_)
	{
		last_mouse_pos_ = pos;
		MoveTo(pos);
	}
}


void MagnifierWnd::OnActivate(UINT state, CWnd* wnd_other, BOOL minimized)
{
	CMiniFrameWnd::OnActivate(state, wnd_other, minimized);

	if (state == WA_INACTIVE)
		PostMessage(WM_USER);
}


void MagnifierWnd::OnDestroy()
{
	KillTimer(timer_);

	profile_magnification_factor_ = magnification_factor_;

	CRect rect;
	GetWindowRect(rect);
	profile_window_size_ = rect.Width();

	CMiniFrameWnd::OnDestroy();
}


void MagnifierWnd::OnTimer(UINT_PTR id_event)
{
	MouseMoved();
}


LRESULT MagnifierWnd::OnFinish(WPARAM, LPARAM)
{
	DestroyWindow();
	return 0;
}


void MagnifierWnd::OnLButtonDown(UINT flags, CPoint point)
{
	PostMessage(WM_USER);
}


void MagnifierWnd::OnKeyDown(UINT chr, UINT rep_cnt, UINT flags)
{
	if (chr == ' ' || chr == VK_RETURN || chr == VK_ESCAPE)
		PostMessage(WM_USER);
	else if (chr == VK_OEM_PLUS)
		ChangeMagnification(true);
	else if (chr == VK_OEM_MINUS)
		ChangeMagnification(false);
	else
		CMiniFrameWnd::OnKeyDown(chr, rep_cnt, flags);
}


BOOL MagnifierWnd::OnMouseWheel(UINT flags, short delta, CPoint pt)
{
	if (delta < 0)
		ChangeMagnification(true);
	else if (delta > 0)
		ChangeMagnification(false);

	return false;
}


void MagnifierWnd::ChangeMagnification(bool increase)
{
	static int zoom[]= { 1, 2, 3, 4, 6, 9, 12, 16 };
	int* end= zoom + array_count(zoom);

	//TODO: revise (is really upper_bound needed or lower_bound would suffice?)
	if (increase)
	{
		int* next= std::upper_bound(zoom, end, magnification_factor_);
		if (next < end)
			magnification_factor_ = *next;
	}
	else
	{
		int* prev= std::lower_bound(zoom, end, magnification_factor_);

		if (prev > zoom)
			magnification_factor_ = prev[-1];
	}

	Invalidate();
}


void MagnifierWnd::OnContextMenu(CWnd* wnd, CPoint pos)
{
	RMenu menu;
	if (!menu.LoadMenu(IDR_MAGNIFIER_CONTEXT))
		return;
	CMenu* popup= menu.GetSubMenu(0);
	if (popup == 0)
		return;

	if (pos == CPoint(-1, -1))	// kbd invoked?
	{
		CRect rect;
		GetWindowRect(rect);
		pos = rect.CenterPoint();
	}

	CRect rect;
	GetWindowRect(rect);
	int check= ID_MAGNIFY_MEDIUM;
	if (rect.Width() == SMALL)
		check = ID_MAGNIFY_SMALL;
	else if (rect.Width() == BIG)
		check = ID_MAGNIFY_BIG;
	popup->CheckMenuRadioItem(ID_MAGNIFY_SMALL, ID_MAGNIFY_BIG, check, MF_BYCOMMAND);

	popup_menu_ = true;
	int cmd= popup->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON | TPM_RETURNCMD, pos.x, pos.y, this);
	popup_menu_ = false;

	switch (cmd)
	{
	case ID_MAGNIFY_MORE:
		ChangeMagnification(true);
		break;

	case ID_MAGNIFY_LESS:
		ChangeMagnification(false);
		break;

	case ID_MAGNIFY_SMALL:
		ResizeWindow(CSize(SMALL, SMALL));
		break;

	case ID_MAGNIFY_MEDIUM:
		ResizeWindow(CSize(MEDIUM, MEDIUM));
		break;

	case ID_MAGNIFY_BIG:
		ResizeWindow(CSize(BIG, BIG));
		break;

	case ID_MAGNIFY_CLOSE:
		PostMessage(WM_USER);
		break;
	}
}


void MagnifierWnd::ResizeWindow(CSize wnd_size)
{
	CRect rect;
	GetWindowRect(rect);

	if (wnd_size != rect.Size())
	{
		CPoint pos;
		GetCursorPos(&pos);
		MoveTo(pos, wnd_size);
	}
}


void MagnifierWnd::MoveTo(CPoint pos, CSize wnd_size/*= CSize(0, 0)*/)
{
	if (pos.x < limit_rect_.left)
		pos.x = limit_rect_.left;
	else if (pos.x > limit_rect_.right)
		pos.x = limit_rect_.right;

	if (pos.y < limit_rect_.top)
		pos.y = limit_rect_.top;
	else if (pos.y > limit_rect_.bottom)
		pos.y = limit_rect_.bottom;

	CRect rect;
	GetWindowRect(rect);

	if (rect.CenterPoint() != pos || wnd_size.cx || wnd_size.cy)
	{
		offset_ = pos - bitmap_rect_.TopLeft();
		int x= pos.x;
		int y= pos.y;

		UINT flags= SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW;

		if (wnd_size.cx && wnd_size.cy)
		{
			flags &= ~SWP_NOSIZE;
			x -= wnd_size.cx / 2;
			y -= wnd_size.cy / 2;
		}
		else
		{
			x -= rect.Width() / 2;
			y -= rect.Height() / 2;
		}

		Invalidate();	// invalidate prevents moving current display
		SetWindowPos(0, x, y, wnd_size.cx, wnd_size.cy, flags);

		//Invalidate();
		
		ValidateRect(0);
		RedrawWindow();

		if (display_)
			display_->UpdateWindow();
		if (CWnd* wnd= AfxGetMainWnd())
			wnd->UpdateWindow();
	}
}


void MagnifierWnd::OnMButtonDown(UINT flags, CPoint point)
{
	PostMessage(WM_USER);
}

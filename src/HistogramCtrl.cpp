/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// HistogramCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "HistogramCtrl.h"
#include "MemoryDC.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// HistogramCtrl

HistogramCtrl::HistogramCtrl()
{
	const TCHAR* cls_name= _T("HistogramCtrl");

	HINSTANCE inst= AfxGetResourceHandle();
	// force same dll as resources
//	HINSTANCE inst= AfxFindResourceHandle(MAKEINTRESOURCE(IDD_FORM_DOUBLE), RT_DIALOG);

	// see if the class already exists
	WNDCLASS wndcls;
	if (!::GetClassInfo(inst, cls_name, &wndcls))
	{
		// otherwise we need to register a new class
		wndcls.style = 0;
		wndcls.lpfnWndProc = ::DefWindowProc;
		wndcls.cbClsExtra = wndcls.cbWndExtra = 0;
		wndcls.hInstance = inst;
		wndcls.hIcon = 0;
		wndcls.hCursor = ::LoadCursor(NULL, IDC_ARROW);
		wndcls.hbrBackground = 0;
		wndcls.lpszMenuName = NULL;
		wndcls.lpszClassName = cls_name;

		AfxRegisterClass(&wndcls);
	}

	channel_ = Histogram::RGBLines;
	host_ = 0;
	last_histX_pos_ = -1;
	tracing_cursor_ = false;
	drawing_flags_ = Histogram::DRAW_GRADIENT | Histogram::DRAW_LABELS;
}


HistogramCtrl::~HistogramCtrl()
{
}


BEGIN_MESSAGE_MAP(HistogramCtrl, CWnd)
	//{{AFX_MSG_MAP(HistogramCtrl)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_SETCURSOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// HistogramCtrl message handlers

//bool HistogramCtrl::Create(CWnd* parent, CRect rect)
//{
////	pattern_bmp_.LoadBitmap(IDB_GRID_PATTERN);
//
//	if (!CWnd::Create(AfxRegisterWndClass(CS_VREDRAW | CS_HREDRAW, ::LoadCursor(NULL, IDC_ARROW)),
//		0, WS_CHILD | WS_VISIBLE, rect, parent, -1))
//		return false;
//
//	return true;
//}


void HistogramCtrl::OnPaint()
{
	CPaintDC paint_dc(this); // device context for painting

	COLORREF rgb_back= ::GetSysColor(COLOR_WINDOW);

	MemoryDC dc(paint_dc, this, rgb_back);

	CRect rect;
	GetClientRect(rect);

	if (CWnd* parent= GetParent())
	{
		// use background brush provided by parent window
		if (LPARAM brush= parent->SendMessage(WM_CTLCOLORSTATIC, WPARAM(dc.m_hDC), LPARAM(m_hWnd)))
		{
			HBRUSH br= HBRUSH(brush);
			::SelectObject(dc.m_hDC, br);
			dc.PatBlt(rect.left, rect.top, rect.Width(), rect.Height(), PATCOPY);
		}
	}

	if (rect.Height() <= 0)
		return;

	histogram_.rgb_back_ = rgb_back;
	histogram_.rgb_fore_ = ::GetSysColor(COLOR_WINDOW);

	rect.DeflateRect(2, 2);
	histogram_.Draw(&dc, rect, channel_, drawing_flags_ |
		Histogram::NO_ERASE_BACKGND | Histogram::NO_TOP_SPACE | Histogram::NO_BOTTOM_SPACE | Histogram::STRETCH_EACH_BAR);

	dc.BitBlt();
}


BOOL HistogramCtrl::OnEraseBkgnd(CDC* dc)
{
	return true;
}


void HistogramCtrl::Clear()
{
	// clear histogram data
	histogram_.BuildHistogram(0);

	if (m_hWnd)
		Invalidate();
}


void HistogramCtrl::Build(const Dib& dib)
{
	histogram_.BuildHistogram(&dib);

	if (m_hWnd)
		Invalidate();
}


void HistogramCtrl::Build(const Dib& dib, const CRect& selection_rect)
{
	histogram_.BuildHistogram(&dib, &selection_rect);

	if (m_hWnd)
		Invalidate();
}


void HistogramCtrl::SelectChannel(Histogram::ChannelSel channel)
{
	channel_ = channel;

	if (m_hWnd)
		Invalidate();
}


void HistogramCtrl::SetEdgeLines(int min, int max)
{
	histogram_.min_edge_ = min;
	histogram_.max_edge_ = max;

	if (m_hWnd)
		Invalidate();
}


void HistogramCtrl::SendMouseNotification(CPoint point)
{
	if (host_ == 0)
		return;

	CRect rect= histogram_.GetHistogramRect();

	// location on the histogram
	int x= 0;
	if (point.x < rect.left)
		x = 0;
	else if (point.x >= rect.right)
		x = 255;
	else
		x = (point.x - rect.left) * 256 / rect.Width();

	if (x != last_histX_pos_)
	{
		bool left= x - histogram_.min_edge_ < histogram_.max_edge_ - x;

		host_->MouseClicked(x, left);

		last_histX_pos_ = x;
	}
}


void HistogramCtrl::OnLButtonDown(UINT flags, CPoint point)
{
	SetCapture();
	tracing_cursor_ = true;
	SetCursor();
	SendMouseNotification(point);
}


void HistogramCtrl::OnLButtonUp(UINT flags, CPoint point)
{
	ReleaseCapture();
	tracing_cursor_ = false;
	SetCursor();
}


void HistogramCtrl::OnMouseMove(UINT flags, CPoint point)
{
	CWnd::OnMouseMove(flags, point);

	if ((flags & MK_LBUTTON) == 0)
	{
		tracing_cursor_ = false;
		return;
	}

	SendMouseNotification(point);
}


void HistogramCtrl::SetCursor()
{
	CPoint point;
	GetCursorPos(&point);
	ScreenToClient(&point);

	if (tracing_cursor_)
		::SetCursor(AfxGetApp()->LoadCursor(IDC_POINTING));
	else if (histogram_.GetHistogramRect().PtInRect(point))
		::SetCursor(AfxGetApp()->LoadCursor(IDC_HORZ_LINE_RESIZE));
	else
		::SetCursor(::LoadCursor(0, IDC_ARROW));
}


BOOL HistogramCtrl::OnSetCursor(CWnd* wnd, UINT hit_test, UINT message)
{
	if (hit_test == HTCLIENT)
	{
		SetCursor();
		return true;
	}
	else
		return CWnd::OnSetCursor(wnd, hit_test, message);
}


void HistogramCtrl::SetDrawFlags(UINT flags)
{
	drawing_flags_ = flags;

	if (m_hWnd)
		Invalidate(false);
}


void HistogramCtrl::SetLogarithmic(bool log)
{
	histogram_.logarithmic_ = log;

	if (m_hWnd)
		Invalidate(false);
}


void HistogramCtrl::SetRGBOverlaidOnly()
{
	histogram_.using_luminosity_and_rgb_ = true;
}

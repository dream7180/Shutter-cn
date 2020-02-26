/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// DockedPane.cpp : implementation file
//

#include "stdafx.h"
#include "DockedPane.h"
#include "Color.h"
#include "resource.h"
#include "SeparatorWnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// DockedPane

DockedPane::DockedPane(PanelAlignment alignment, int width)
 : alignment_(alignment), width_(width)
{
	resizing_ = false;
}

DockedPane::~DockedPane()
{}

CString DockedPane::wnd_class_;


BEGIN_MESSAGE_MAP(DockedPane, CWnd)
	//{{AFX_MSG_MAP(DockedPane)
	ON_WM_WINDOWPOSCHANGING()
	ON_WM_NCCALCSIZE()
	ON_WM_NCHITTEST()
	ON_WM_NCPAINT()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_SIZEPARENT, OnSizeParent)
	ON_MESSAGE(WM_ENTERSIZEMOVE, OnEnterSizeMove)
	ON_MESSAGE(WM_EXITSIZEMOVE, OnExitSizeMove)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////


bool DockedPane::Create(CWnd* parent, UINT id, int width)
{
	if (wnd_class_.IsEmpty())
		wnd_class_ = AfxRegisterWndClass(CS_VREDRAW | CS_HREDRAW, ::LoadCursor(NULL, IDC_ARROW));

	width_ = std::max(width, 8);
	CRect desk;
	::SystemParametersInfo(SPI_GETWORKAREA, 0, desk, 0);
	if (width_ > desk.Width())
		width_ = desk.Width();

	if (!CWnd::Create(wnd_class_, _T(""), WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN,
		CRect(0,0,0,0), parent, id))
		return false;

	return true;
}


void DockedPane::OnWindowPosChanging(WINDOWPOS FAR* wnd_pos)
{
	if (resizing_)
	{
		if ((wnd_pos->flags & (SWP_NOSIZE | SWP_NOMOVE)) != (SWP_NOSIZE | SWP_NOMOVE))
		{
			CWnd* parent= GetParent();
			if (CFrameWnd* frame= dynamic_cast<CFrameWnd*>(parent))
			{
				width_ = wnd_pos->cx;
				resizing_ = false;
				frame->RecalcLayout();
				resizing_ = true;
			}
		}
		wnd_pos->flags |= SWP_NOZORDER | SWP_NOSIZE | SWP_NOMOVE;
	}

	CWnd::OnWindowPosChanging(wnd_pos);
}


// Entering user initiated resizing
//
LRESULT DockedPane::OnEnterSizeMove(WPARAM wParam, LPARAM lParam)
{
	resizing_ = true;
	return Default();
}

LRESULT DockedPane::OnExitSizeMove(WPARAM wParam, LPARAM lParam)
{
	resizing_ = false;
	return Default();
}


LRESULT DockedPane::OnSizeParent(WPARAM, LPARAM lParam)
{
	AFX_SIZEPARENTPARAMS* layout = (AFX_SIZEPARENTPARAMS*)lParam;

	if (GetStyle() & WS_VISIBLE)
	{
		CRect rect= layout->rect;

		CSize avail_size = rect.Size();  // maximum size available

		CSize size(std::min(width_, static_cast<int>(avail_size.cx)), std::min(99999, static_cast<int>(avail_size.cy)));

//		if (dwStyle & CBRS_ORIENT_HORZ)
//		{
//			layout->sizeTotal.cy += size.cy;
//			layout->sizeTotal.cx = max(layout->sizeTotal.cx, size.cx);
//			if (dwStyle & CBRS_ALIGN_TOP)
//				layout->rect.top += size.cy;
//			else if (dwStyle & CBRS_ALIGN_BOTTOM)
//			{
//				rect.top = rect.bottom - size.cy;
//				layout->rect.bottom -= size.cy;
//			}
//		}
//		else if (dwStyle & CBRS_ORIENT_VERT)
		{
			layout->sizeTotal.cx += size.cx;
			layout->sizeTotal.cy = std::max(layout->sizeTotal.cy, size.cy);
			if (alignment_ == PANEL_ALIGN_LEFT)
				layout->rect.left += size.cx;
			else if (alignment_ == PANEL_ALIGN_RIGHT)
			{
				rect.left = rect.right - size.cx;
				layout->rect.right -= size.cx;
			}
		}

		rect.right = rect.left + size.cx;
		rect.bottom = rect.top + size.cy;

		// only resize the window if doing layout and not just rect query
		if (layout->hDWP != NULL)
			AfxRepositionWindow(layout, m_hWnd, &rect);
	}
	return 0;
}


namespace {
	const int MARGIN= 7;
}


void DockedPane::OnNcCalcSize(BOOL calc_valid_rects, NCCALCSIZE_PARAMS FAR* ncsp)
{
	CWnd::OnNcCalcSize(calc_valid_rects, ncsp);

	if (ncsp)
		if (alignment_ == PANEL_ALIGN_LEFT)
			ncsp->rgrc[0].right -= MARGIN;
		else if (alignment_ == PANEL_ALIGN_RIGHT)
			ncsp->rgrc[0].left += MARGIN;
}


LRESULT DockedPane::OnNcHitTest(CPoint point)
{
	CRect rect;
	GetWindowRect(rect);

	if (alignment_ == PANEL_ALIGN_LEFT)
	{
		rect.left = rect.right - MARGIN;
		if (rect.PtInRect(point))
			return HTRIGHT;
	}
	else if (alignment_ == PANEL_ALIGN_RIGHT)
	{
		rect.right = rect.left + MARGIN;
		if (rect.PtInRect(point))
			return HTLEFT;
	}

	return CWnd::OnNcHitTest(point);
}


void DockedPane::OnNcPaint()
{
	CRect rect;
	GetWindowRect(rect);
	rect.OffsetRect(-rect.TopLeft());

	CWindowDC dc(this);
	int x= 0;

	if (alignment_ == PANEL_ALIGN_LEFT)
	{
		int right= rect.Width() - MARGIN;
		x = right;
	}
	else if (alignment_ == PANEL_ALIGN_RIGHT)
	{
		x = 0;
	}

	CRect r(CPoint(x, 0), CSize(MARGIN, rect.Height()));

	if (paint_fn_)
		paint_fn_(dc, r);
	else
		SeparatorWnd::DrawSeparatorBar(dc, r, false);
}


bool DockedPane::IsVisible() const
{
	return m_hWnd && (GetStyle() & WS_VISIBLE) != 0;
}


void DockedPane::Show(bool show)
{
	if (m_hWnd == 0)
		return;

	if (IsVisible() == show)
		return;

	if (show)
		ModifyStyle(WS_DISABLED, WS_VISIBLE);
	else
		ModifyStyle(WS_VISIBLE, WS_DISABLED);
}


void DockedPane::ToggleVisibility()
{
	if (m_hWnd == 0)
		return;

	Show(!IsVisible());
}

/*
BOOL DockedPane::OnCmdMsg(UINT id, int code, void* extra, AFX_CMDHANDLERINFO* handler_info)
{
	// first pump through pane
	if (CWnd::OnCmdMsg(id, code, extra, handler_info))
		return TRUE;

	// then pump through parent wnd
	if (CWnd* parent= GetParent())
	{
		// special state for saving view before routing to document
//		CPushRoutingView push(this);
		return parent->OnCmdMsg(id, code, extra, handler_info);
	}

	return FALSE;
}*/


BOOL DockedPane::OnEraseBkgnd(CDC* dc)
{
	return true;
}

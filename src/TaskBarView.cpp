/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// TaskBarView.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "TaskBarView.h"
#include "Color.h"
#include "MemoryDC.h"
#include "LoadImageList.h"
#include "UIElements.h"
#include "Color.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// TaskBarView

TaskBarView::TaskBarView()
{
}

TaskBarView::~TaskBarView()
{
}


BEGIN_MESSAGE_MAP(TaskBarView, PaneWnd)
	//{{AFX_MSG_MAP(TaskBarView)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TaskBarView message handlers

static const CSize g_margin_size(6, 6);	// round-rect frame thickness
static const int g_GRADIENT= 4;

// draw task bar background
//
BOOL TaskBarView::OnEraseBkgnd(CDC* dc)
{
	MemoryDC mem_dc(*dc, this);

	Gdiplus::Graphics g(mem_dc);

	CRect rect;
	GetClientRect(rect);
	COLORREF rgb_frame= ::GetSysColor(COLOR_BTNFACE);
	COLORREF rgb_backgnd= ::GetSysColor(COLOR_WINDOW);
	mem_dc.FillSolidRect(rect, rgb_frame);

	CRect menu_rect= rect;
	menu_rect.DeflateRect(g_margin_size);

	if (menu_rect.Width() > 0 && menu_rect.Height() > 0)
	{
		Gdiplus::RectF area= CRectToRectF(menu_rect);
		float radius= Pixels(mem_dc, 5);
		Gdiplus::GraphicsPath fill;
		RoundRect(fill, area, radius);

		Gdiplus::SolidBrush brush(c2c(rgb_backgnd));
		g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
		g.TranslateTransform(-0.5f, -0.5f);
		g.FillPath(&brush, &fill);

/*
		COLORREF rgb_white_frame= RGB(255,255,255);
		CPen penFrame(PS_SOLID, 1, rgb_white_frame);
		CPen* old_pen= mem_dc.SelectObject(&penFrame);

		CBrush brFill(rgb_backgnd);
		CBrush* old_brush= mem_dc.SelectObject(&brFill);
		mem_dc.RoundRect(menu_rect, CPoint(7, 7));

		mem_dc.SelectObject(old_pen);
		mem_dc.SelectObject(old_brush);
*/
	}

	mem_dc.BitBlt();

	return true;
}


void TaskBarView::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	// Do not call CWnd::OnPaint() for painting messages
}


bool TaskBarView::Create(CWnd* parent)
{
	const TCHAR* class_name= AfxRegisterWndClass(CS_VREDRAW | CS_HREDRAW, ::LoadCursor(NULL, IDC_ARROW));

	if (!CWnd::Create(class_name, _T(""), WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN, CRect(0,0,0,0), parent, -1))
		return false;

	if (!LoadImageList(image_list_, IDB_TASK_BAR, 20, ::GetSysColor(COLOR_WINDOW)))
		return false;

	//HINSTANCE inst= AfxFindResourceHandle(MAKEINTRESOURCE(IDB_TASK_BAR), RT_BITMAP);
	//image_list_.Attach(ImageList_LoadImage(inst, MAKEINTRESOURCE(IDB_TASK_BAR),
	//	20, 0, RGB(255,0,255), IMAGE_BITMAP, LR_CREATEDIBSECTION));

	list_wnd_.Create(this, CRect(0,0,0,0), 0-1, 0);

	list_wnd_.SetOwner(parent->GetParent()->GetParent());

	list_wnd_.SetImageList(&image_list_);

	list_wnd_.SetMenuLikeSelection(true);

	list_wnd_.SetOnIdleUpdateState(true);

	static const int commands[]=
	{
		ID_VIEW,
		// order should be the same as in the Tools menu
		ID_TASK_TRANSFER, ID_TASK_COPY, ID_TASK_MOVE, ID_TASK_COPY_TAGGED,
		ID_TASK_DELETE,
		ID_TASK_ROTATE, ID_TASK_RESIZE, /*ID_TASK_EDIT_DESC,*/ ID_TASK_EDIT_IPTC,
		ID_TASK_GEN_SLIDE_SHOW, ID_TASK_GEN_HTML_ALBUM, ID_BUILD_CATALOG, ID_TASK_EXPORT,
		ID_TASK_HISTOGRAM,
		ID_TASK_PRINT, ID_TASK_PRINT_THUMBNAILS, ID_TASK_TOUCH_UP, ID_SEND_EMAIL, ID_DATE_TIME_ADJ,
		ID_TASK_EXTRACT_JPEG,
		0
	};

	list_wnd_.ReserveItems(array_count(commands) - 1);

	for (int i= 0; commands[i]; ++i)
	{
		CString cmd;
		cmd.LoadString(commands[i]);
		int sep= cmd.Find(_T('\n'));
		if (sep >= 0)
		{
			cmd = cmd.Mid(sep + 1);

			int sep= cmd.Find(_T('('));	// shortcut in parentheses
			if (sep >= 0)
				cmd.SetAt(sep, 0);
		}

		list_wnd_.AddItem(cmd, i, commands[i], 0);
	}

	return true;
}


void TaskBarView::Resize()
{
	if (list_wnd_.m_hWnd)
	{
		CRect rect;
		GetClientRect(rect);
		rect.DeflateRect(g_margin_size.cx, 2 * g_margin_size.cy, g_margin_size.cx, 2 * g_margin_size.cy - 1);

		list_wnd_.MoveWindow(rect);
	}
}


void TaskBarView::OnSize(UINT type, int cx, int cy)
{
	CWnd::OnSize(type, cx, cy);
	Resize();
}

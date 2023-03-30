/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "../resource.h"
#include "DarkCloseBar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


BEGIN_MESSAGE_MAP(DarkCloseBar, CWnd)
	ON_WM_ERASEBKGND()
	ON_MESSAGE(WM_PRINTCLIENT, OnPrintClient)
	ON_WM_SIZE()
END_MESSAGE_MAP()


bool DarkCloseBar::Create(CWnd* parent)
{
	if (!CWnd::Create(AfxRegisterWndClass(CS_VREDRAW | CS_HREDRAW, AfxGetApp()->LoadStandardCursor(IDC_ARROW), 0, 0),
		0, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, CRect(0,0,0,0), parent, -1))
		return false;

	//backgnd_.Load(IDR_CLOSE_BAR);

	toolbar_.SetOnIdleUpdateState(false);

	const int commands[]= { SC_MINIMIZE, SC_RESTORE, SC_CLOSE };

	//FancyToolBar::Params p;
	//p.shade = -0.28f;
	//if (!toolbar_.Create(this, "ppp", commands, IDR_CLOSEBAR_PNG, &p))
	if (!toolbar_.Create("ppp", commands, IDR_CLOSEBAR_PNG, 0, this, AFX_IDW_TOOLBAR))
		return false;

	toolbar_.SetPadding(4,4);
//	SetOption(FancyToolBar::BEVEL_LOOK, false);
	//toolbar_.SetOption(FancyToolBar::HOT_OVERLAY, false);
//	SetOption(FancyToolBar::SHIFT_BTN, false);

//	SetHotImageList(IDB_CLOSE_BAR_HOT);

	toolbar_.AutoResize();
	toolbar_.SetOwnerDraw(parent);

	toolbar_.SetOwner(parent);

	return true;
}


CString DarkCloseBar::Toolbar::GetToolTip(int cmdId)
{
	/*CString tip;

	if (cmdId == SC_RESTORE)
		tip.LoadString(IDS_RESTOR_WND);
	else if (cmdId == SC_CLOSE)
		tip.LoadString(IDS_CLOSE_WND);
	else if (cmdId == SC_MINIMIZE)
		tip.LoadString(IDS_MINIMIZE_WND);
	else
		return ToolBarWnd::GetToolTip(cmdId);

	return tip;*/
	return _T("");
}


BOOL DarkCloseBar::OnEraseBkgnd(CDC* dc)
{
	//if (backgnd_.IsValid())
	//{
		CRect rect(0,0,0,0);
		GetClientRect(rect);
		dc->FillSolidRect(rect, RGB(207,207,207));
		//backgnd_.Draw(dc, rect);
	//}
	return true;
}


LRESULT DarkCloseBar::OnPrintClient(WPARAM wdc, LPARAM flags)
{
	if (CDC* dc= CDC::FromHandle(HDC(wdc)))
		OnEraseBkgnd(dc);

	return 0;
}


void DarkCloseBar::OnSize(UINT type, int cx, int cy)
{
	if (toolbar_.m_hWnd)
	{
		CSize s= toolbar_.tb_size;
		tbsize = s;
		int x= cx - s.cx;
		int y= (cy - s.cy - 1) / 2;
		toolbar_.SetWindowPos(0, x, y, s.cx, s.cy, SWP_NOZORDER | SWP_NOACTIVATE);
	}
}

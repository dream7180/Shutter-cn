/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// ToolBarPopup.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "ToolBarPopup.h"
#include "UIElements.h"
#include "WhistlerLook.h"
#include "MultiMonitor.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CToolBarPopup* CToolBarPopup::tool_bar_popup_= 0;

/////////////////////////////////////////////////////////////////////////////
// CToolBarPopup


CToolBarPopup::CToolBarPopup(CWnd& tool_bar_wnd, CSize size, CWnd* owner)
 : tool_bar_(&tool_bar_wnd), owner_wnd_(owner)
{
	if (tool_bar_popup_)
		tool_bar_popup_->DestroyWindow();

	tool_bar_popup_ = this;

	CRect rect;
	tool_bar_->GetWindowRect(rect);
	rect.right = rect.left + size.cx;
	rect.bottom = rect.top + size.cy;
	rect.InflateRect(1, 1);

	RectToWorkArea(rect, false);

	tb_parent_ = tool_bar_->GetParent();

	WINDOWPLACEMENT wp;
	tool_bar_->GetWindowPlacement(&wp);
	original_pos_rect_ = wp.rcNormalPosition;

	UINT class_style= CS_SAVEBITS | (WhistlerLook::IsAvailable() ? CS_DROPSHADOW : 0);

	if (CreateEx(WS_EX_WINDOWEDGE | WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
		AfxRegisterWndClass(class_style, ::LoadCursor(NULL, IDC_ARROW)), 0,
		WS_POPUP | WS_VISIBLE | WS_CLIPCHILDREN | MFS_SYNCACTIVE,
		rect, AfxGetMainWnd()))
	{
		tool_bar_->SetOwner(owner_wnd_);
		tool_bar_->ShowWindow(SW_HIDE);
		tool_bar_->SetParent(this);
		GetClientRect(rect);
		rect.DeflateRect(1, 1);
		tool_bar_->MoveWindow(rect);
		tool_bar_->ShowWindow(SW_SHOW);
		SetFocus();
	}
}

CToolBarPopup::~CToolBarPopup()
{}


BEGIN_MESSAGE_MAP(CToolBarPopup, CMiniFrameWnd)
	//{{AFX_MSG_MAP(CToolBarPopup)
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_KILLFOCUS()
	ON_WM_PARENTNOTIFY()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_IDLEUPDATECMDUI, OnIdleUpdateCmdUI)
	ON_MESSAGE(WM_USER + 100, OnTbClicked)
	ON_MESSAGE(WM_USER, OnCloseWnd)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CToolBarPopup message handlers


void CToolBarPopup::OnDestroy()
{
	tool_bar_->ShowWindow(SW_HIDE);
	tool_bar_->SetParent(tb_parent_);
	tool_bar_->MoveWindow(original_pos_rect_);
	tool_bar_->ShowWindow(SW_SHOWNA);
	CMiniFrameWnd::OnDestroy();
}


BOOL CToolBarPopup::OnEraseBkgnd(CDC* dc)
{
	CRect rect;
	GetClientRect(rect);
	COLORREF rgb_frame= RGB(192,192,192); //TODO
	dc->Draw3dRect(rect, rgb_frame, rgb_frame);
	rect.DeflateRect(1, 1);
	::DrawPanelBackground(*dc, rect);
	return true;
}


void CToolBarPopup::PostNcDestroy()
{
	delete this;
	tool_bar_popup_ = 0;
}


void CToolBarPopup::OnKillFocus(CWnd* new_wnd)
{
	DestroyWindow();
}


LRESULT CToolBarPopup::OnIdleUpdateCmdUI(WPARAM wParam, LPARAM lParam)
{
	return tool_bar_ ? tool_bar_->SendMessage(WM_IDLEUPDATECMDUI, wParam, lParam) : 0L;
}


void CToolBarPopup::OnParentNotify(UINT message, LPARAM lParam)
{
	CMiniFrameWnd::OnParentNotify(message, lParam);

	TRACE(_T("msg: %x\n"), message);
}


LRESULT CToolBarPopup::OnTbClicked(WPARAM hwnd, LPARAM code)
{
	PostMessage(WM_USER);	// close itself
	return 0;
}

LRESULT CToolBarPopup::OnCloseWnd(WPARAM, LPARAM)
{
	DestroyWindow();
	return 0;
}

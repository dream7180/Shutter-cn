/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// TagsBar.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "TagsBar.h"
#include "BrowserFrame.h"
#include "TagsManageDlg.h"
#include "BalloonMsg.h"
#include "MultiMonitor.h"
#include "Config.h"
#include "Color.h"
#include "SnapFrame/CaptionWindow.h"
#include "CatchAll.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


static const TCHAR* REGISTRY_SECTION_TAGS_WND= _T("TagsBar");


static CRect GetInitialPos()
{
	// use desk for initial position only, later on rely on registry
	CRect desk;
	::SystemParametersInfo(SPI_GETWORKAREA, 0, desk, 0);

	CRect rect;
	rect.left   = desk.right - 150;
	rect.top    = desk.bottom - 250;
	int w       = 120;
	int h       = 200;
	rect.right  = w + rect.left;
	rect.bottom = h + rect.top;

	return rect;
}


CTagsBar::CTagsBar(PhotoTagsCollection& tagCollection, ConnectionPointer* link)
 : ConnectionPointer(link), bar_wnd_(tagCollection, L"TagsBar"), wnd_pos_(REGISTRY_SECTION_TAGS_WND, &GetInitialPos())
{
}

CTagsBar::~CTagsBar()
{
}


BEGIN_MESSAGE_MAP(CTagsBar, CMiniFrameWnd)
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
	ON_WM_DESTROY()
	ON_UPDATE_COMMAND_UI_RANGE(ID_TAG_SELECTED, ID_TAG_SELECTED + 9999, OnUpdateTags)
	ON_COMMAND_RANGE(ID_TAG_SELECTED, ID_TAG_SELECTED + 9999, OnTagSelected)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CTagsBar message handlers


bool CTagsBar::Create(CWnd* parent, BrowserFrame* frame)
{
	CRect rect= wnd_pos_.GetLocation(true);

	if (!CMiniFrameWnd::Create(
		AfxRegisterWndClass(0, ::LoadCursor(NULL, IDC_ARROW), reinterpret_cast<HBRUSH>(COLOR_3DFACE + 1)),
		_T("Tags"),
		WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_VISIBLE | WS_CLIPCHILDREN | MFS_THICKFRAME | MFS_SYNCACTIVE,
		rect, parent))
		return false;

	if (!bar_wnd_.Create(this, false))
		return false;

	bar_wnd_.SetTagCallback(&::ApplyTagToPhotos);
	bar_wnd_.SetRatingCallback(&::ApplyRatingToPhotos);

	Resize();

	return true;
}


void CTagsBar::Resize()
{
	CRect rect(0,0,0,0);
	GetClientRect(rect);
	if (bar_wnd_.m_hWnd)
		bar_wnd_.MoveWindow(rect);
}


void CTagsBar::OnSize(UINT type, int cx, int cy)
{
	CMiniFrameWnd::OnSize(type, cx, cy);

	Resize();
}


void CTagsBar::OnGetMinMaxInfo(MINMAXINFO FAR* MMI)
{
	CMiniFrameWnd::OnGetMinMaxInfo(MMI);
	MMI->ptMinTrackSize.x = 70;
	MMI->ptMinTrackSize.y = 100;
}


void CTagsBar::OnUpdateTags(CCmdUI* cmd_ui)
{
	cmd_ui->Enable();
}


void CTagsBar::OnTagSelected(UINT cmd)
{
	int index= cmd - ID_TAG_SELECTED;

	if (index < 0)
		return;

	bar_wnd_.AssignTag(index);
}


void CTagsBar::OnDestroy()
{
	WINDOWPLACEMENT wp;
	if (GetWindowPlacement(&wp))
		wnd_pos_.StoreState(wp);

	CMiniFrameWnd::OnDestroy();
}

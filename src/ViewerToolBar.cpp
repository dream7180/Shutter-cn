/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// ViewerToolBar.cpp : implementation file
//
#include "stdafx.h"

#include "resource.h"
#include "ViewerToolBar.h"
#include "RString.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// ViewerToolBar

ViewerToolBar::ViewerToolBar()
{
	small_icons_ = false;
	rebar_band_id_ = 0;
}

ViewerToolBar::~ViewerToolBar()
{
}


BEGIN_MESSAGE_MAP(ViewerToolBar, Derived)
	ON_WM_DESTROY()
	//ON_COMMAND(ID_TOOLBAR_CUSTOMIZE, OnToolbarCustomize)
	//ON_COMMAND(ID_SMALL_ICONS, OnSmallIcons)
	//ON_COMMAND(ID_LARGE_ICONS, OnLargeIcons)
	//ON_NOTIFY_REFLECT(NM_RCLICK, OnRightClick)
	ON_NOTIFY_REFLECT(TBN_RESET, OnResetToolbar)
	ON_WM_INITMENUPOPUP()
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONDOWN()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// ViewerToolBar message handlers

void CreateDisabledImageList(...)	{}


namespace {
	const TCHAR* REGISTRY_SECTION_TOOLBAR	= _T("ViewerToolBar");
	//const TCHAR* REG_STATE					= _T("TbState");
	const TCHAR* REG_ICONS					= _T("Icons");

	const int commands[]=
	{
		ID_PHOTO_FIRST, ID_PHOTO_PREV, ID_PHOTO_LIST, ID_PHOTO_NEXT, ID_PHOTO_LAST,
		ID_START_SLIDE_SHOW, //ID_STOP_SLIDE_SHOW,
		ID_ZOOM_OUT, ID_ZOOM_IN, ID_ZOOM_100, ID_ZOOM_FIT, ID_MAGNIFIER_LENS,
		ID_VIEWER_BAR, ID_TOGGLE_LIGHT_TABLE, ID_COMPARE_MULTIPLE,
		ID_JPEG_ROTATE_90_CCW, ID_JPEG_ROTATE_90_CW,
		ID_VIEWER_OPTIONS, ID_TAGS_BAR,
		ID_ROTATE_90_CCW, ID_ROTATE_90_CW
	};

	const char* tb_buttons= "ppppp|m|ppxx|p|xxv|pp|vx|pp";

	float saturation= -0.5f;
	float lightness= +0.25f;
}


bool ViewerToolBar::Create(CWnd* parent, UINT id)
{
//	rebar_band_id_ = rebar_band_id;

	small_icons_ = AfxGetApp()->GetProfileInt(REGISTRY_SECTION_TOOLBAR, REG_ICONS, 1) == 0;

	int bmp_id= small_icons_ ? IDB_VIEWER_TOOLBAR : IDB_VIEWER_TOOLBAR_BIG;

	Derived::Params p;

	//if (small_icons_)
	//	p.arrow_down_img_id = IDR_ARROW_DOWN_IMG;
	//else
		p.arrow_down_img_id = IDR_ARROW_DOWN_IMG;

	if (!Derived::Create(parent, tb_buttons, commands, bmp_id, &p))
		return false;

	SetOnIdleUpdateState(true);

	CSize s= Derived::Size();

	SetWindowPos(0, 0, 0, s.cx, s.cy, SWP_NOZORDER | SWP_NOACTIVATE);

	SetDlgCtrlID(id);

	//SetPadding(4, 4);
	//if (IsWhistlerLookAvailable())
	//	SetPadding(0, -2);	// btn with down arrow section inflates total buttons height

	//if (!Derived::Create(tb_buttons, commands, bmp_id, 0, parent, id))
	//	return false;

	//// img list for disabled images
	//CreateDisabledImageList(bmp_id, saturation, lightness);

	//ModifyStyle(0, CCS_ADJUSTABLE);


	//RestoreState(REGISTRY_SECTION_TOOLBAR, REG_STATE);

	CRect rect;
	GetWindowRect(rect);
	SetWindowPos(0, 0, 0, rect.Width() + 8, rect.Height(), SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE);

	return true;
}


void ViewerToolBar::OnDestroy()
{
	//SaveState(REGISTRY_SECTION_TOOLBAR, REG_STATE);

	AfxGetApp()->WriteProfileInt(REGISTRY_SECTION_TOOLBAR, REG_ICONS, small_icons_ ? 0 : 1);

	Derived::OnDestroy();
}

/*
void ViewerToolBar::OnRButtonDown(UINT flags, CPoint pos)
{
	CPoint pt(0, 0);
	GetCursorPos(&pt);
	OnContextMenu(this, pt);
}
*/

void ViewerToolBar::OnContextMenu(CWnd* wnd, CPoint pos)
{
	CMenu menu;
	if (!menu.LoadMenu(IDR_TOOLBAR_CONTEXT))
		return;

	if (CMenu* popup= menu.GetSubMenu(0))
	{
		if (pos.x == -1 && pos.y == -1)
		{
			CRect rect;
			GetClientRect(rect);
			ClientToScreen(rect);
			pos = rect.CenterPoint();
		}
		popup->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, pos.x, pos.y, GetOwner());
	}
}

/*
void ViewerToolBar::OnToolbarCustomize()
{
	Customize();
}
*/

void ViewerToolBar::OnResetToolbar(NMHDR* notify_struct, LRESULT* result)
{
	*result = 0;
	ResetToolBar(false);
	HideButton(ID_STOP_SLIDE_SHOW);
}


bool ViewerToolBar::SmallIcons()
{
	if (small_icons_)
		return false;

	if (ReplaceImageList(IDB_VIEWER_TOOLBAR))
	{
	//	CreateDisabledImageList(IDB_VIEWER_TOOLBAR, saturation, lightness);
	//	AdjustReBar();
		small_icons_ = true;
	}

	return true;
}


bool ViewerToolBar::LargeIcons()
{
	if (!small_icons_)
		return false;

	if (ReplaceImageList(IDB_VIEWER_TOOLBAR_BIG))
	{
	//	CreateDisabledImageList(IDB_VIEWER_TOOLBAR_BIG, saturation, lightness);
	//	AdjustReBar();
		small_icons_ = false;
	}

	return true;
}


//void ViewerToolBar::OnSmallIcons()
//{
//	if (!small_icons_)
//		if (ReplaceImageList(IDB_VIEWER_TOOLBAR))
//		{
//			CreateDisabledImageList(IDB_VIEWER_TOOLBAR, saturation, lightness);
//			AdjustReBar();
//			small_icons_ = true;
//		}
//}
//
//
//void ViewerToolBar::OnLargeIcons()
//{
//	if (small_icons_)
//		if (ReplaceImageList(IDB_VIEWER_TOOLBAR_BIG))
//		{
//			CreateDisabledImageList(IDB_VIEWER_TOOLBAR_BIG, saturation, lightness);
//			AdjustReBar();
//			small_icons_ = false;
//		}
//}


void ViewerToolBar::OnInitMenuPopup(CMenu* popup_menu, UINT index, BOOL sys_menu)
{
	popup_menu->CheckMenuRadioItem(ID_SMALL_ICONS, ID_LARGE_ICONS, small_icons_ ? ID_SMALL_ICONS : ID_LARGE_ICONS, MF_BYCOMMAND);
}


//void ViewerToolBar::AdjustReBar()
//{
//	CReBarCtrl* rebar= dynamic_cast<CReBarCtrl*>(GetParent());
//
//	if (rebar == 0)
//		return;
//
//	int band= rebar->IDToIndex(rebar_band_id_);
//	if (band == -1)
//	{
//		ASSERT(false);
//		return;
//	}
//
//	// resize rebar band
//
//	REBARBANDINFO bi;
//	memset(&bi, 0, sizeof bi);
//	bi.cbSize = sizeof bi;
//
//	CRect rect;
//	GetWindowRect(rect);
//	bi.cyMinChild = bi.cxMinChild = rect.Height();
//	bi.cxIdeal = rect.Width();
//	bi.fMask = RBBIM_CHILDSIZE | RBBIM_IDEALSIZE;
//
//	rebar->SetBandInfo(band, &bi);
//}

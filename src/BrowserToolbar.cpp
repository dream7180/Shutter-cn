/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// BrowserToolbar.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "RString.h"
#include "BrowserToolbar.h"
#include <afxpriv.h>
#include "Config.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// BrowserToolbar

static const TCHAR* const WND_CLASS= _T("MainToolBarMiK");

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
		wndcls.hCursor = ::LoadCursor(NULL, IDC_ARROW);
		wndcls.hbrBackground = HBRUSH(COLOR_3DFACE + 1);
		wndcls.lpszMenuName = NULL;
		wndcls.lpszClassName = class_name;

		AfxRegisterClass(&wndcls);
	}
}


BrowserToolbar::BrowserToolbar()
{
	background_ = ::GetSysColor(COLOR_3DFACE);
	//small_icons_ = true;
	rebar_band_id_ = 0;
	RegisterWndClass(WND_CLASS);
}

BrowserToolbar::~BrowserToolbar()
{}


BEGIN_MESSAGE_MAP(BrowserToolbar, CWnd)
	//ON_COMMAND(ID_TOOLBAR_CUSTOMIZE, OnToolbarCustomize)	//todo
	//ON_COMMAND(ID_SMALL_ICONS, OnSmallIcons)
	//ON_COMMAND(ID_LARGE_ICONS, OnLargeIcons)
	ON_WM_DESTROY()
	ON_WM_INITMENUPOPUP()
	ON_WM_CONTEXTMENU()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// BrowserToolbar message handlers

namespace {
	const TCHAR* REGISTRY_SECTION_TOOLBAR	= _T("MainToolBar");
	const TCHAR* REG_STATE					= _T("TbState");
	const TCHAR* REG_ICONS					= _T("Icons");
	float saturation= -0.5f;
	float lightness= +0.15f;
}

const static wchar_t LABEL[]= L"面板";


bool BrowserToolbar::Create(CWnd* parent, UINT id1, UINT id2, UINT rebar_band_id)
{
	rebar_band_id_ = rebar_band_id;

	if (!CWnd::Create(WND_CLASS, 0, WS_CHILD | WS_VISIBLE, CRect(0,0,0,0), parent, 1))
		return false;

	static const int commands[]=
	{
		/*ID_TASK_TRANSFER, ID_TASK_PRINT, ID_TAGS_BAR, ID_FULL_SCREEN, ID_FOLDER_LIST,
		ID_RECURSIVE, ID_READ_ONLY_EXIF, ID_FILE_MASK,
		ID_PANE_MENU, ID_FOLDERS, ID_PREVIEW, ID_INFO_BAR*/
		ID_TASK_PRINT, ID_FOLDER_LIST, ID_RECURSIVE, ID_FILE_MASK, ID_PANE_MENU, ID_PREVIEW, ID_INFO_BAR
	};

	main_bar_.SetOwnerDraw(true);
	main_bar_.SetPadding(8, 8);

//	small_icons_ = AfxGetApp()->GetProfileInt(REGISTRY_SECTION_TOOLBAR, REG_ICONS, 0) == 0;

	int bmp_id= IDB_BROWSER_TBAR;

	//if (!main_bar_.Create("PpPp|vxxv....", commands, bmp_id, IDS_BROWSER_TOOLBAR, this, id1))
	if (!main_bar_.Create("pvxv...", commands, bmp_id, IDS_BROWSER_TOOLBAR, this, id1))
		return false;

	main_bar_.CreateDisabledImageList(bmp_id, saturation, lightness);

	//main_bar_.DeleteButton(ID_TAGS_BAR);
	//main_bar_.DeleteButton(ID_FULL_SCREEN);
	//main_bar_.DeleteButton(ID_READ_ONLY_EXIF);

	main_bar_.RestoreState(REGISTRY_SECTION_TOOLBAR, REG_STATE);

	main_bar_.AutoResize();
	main_bar_.SetOwner(parent);

	panels_.SetOwnerDraw(true);
	panels_.SetPadding(8, 8);

	//if (!panels_.Create(":.:.....vxxX", commands, bmp_id, IDS_BROWSER_TOOLBAR, this, id2))
	if (!panels_.Create("....vxX", commands, bmp_id, IDS_BROWSER_TOOLBAR, this, id2))
		return false;

	//panels_.DeleteButton(ID_FOLDERS);

	panels_.AutoResize();
	panels_.SetOwner(parent);

	Resize();

	return true;
}


void BrowserToolbar::ResetToolBar(bool resize_to_fit)
{
	main_bar_.ResetToolBar(resize_to_fit);

	//main_bar_.DeleteButton(ID_TAGS_BAR);
	//main_bar_.DeleteButton(ID_FULL_SCREEN);
	//main_bar_.DeleteButton(ID_READ_ONLY_EXIF);
}

//todo:
//void BrowserToolbar::OnRightClick(NMHDR* notify_struct, LRESULT* result)
//{
//	*result = 0;
//	CPoint pos(0, 0);
//	GetCursorPos(&pos);
//	OnContextMenu(this, pos);
//}

/*
void BrowserToolbar::OnContextMenu(CWnd* wnd, CPoint pos)
{
#if 0	// todo: big img
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
		popup->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, pos.x, pos.y, this);
	}
#endif
}


void BrowserToolbar::OnToolbarCustomize()
{
	//todo: big icons, and proper customization
//	main_bar_.Customize();
}
*/

void BrowserToolbar::OnDestroy()
{
	main_bar_.SaveState(REGISTRY_SECTION_TOOLBAR, REG_STATE);

	//AfxGetApp()->WriteProfileInt(REGISTRY_SECTION_TOOLBAR, REG_ICONS, small_icons_ ? 0 : 1);

	CWnd::OnDestroy();
}

/*
void BrowserToolbar::OnSmallIcons()
{
	//if (!small_icons_)
	//	if (main_bar_.ReplaceImageList(IDB_BROWSER_TBAR))
	//	{
	//		main_bar_.CreateDisabledImageList(IDB_BROWSER_TBAR, saturation, lightness);
	//		AdjustReBar();
	//		small_icons_ = true;
	//	}
}


void BrowserToolbar::OnLargeIcons()
{
	//if (small_icons_)
	//	if (main_bar_.ReplaceImageList(IDB_BROWSER_TBAR_BIG))
	//	{
	//		main_bar_.CreateDisabledImageList(IDB_BROWSER_TBAR_BIG, saturation, lightness);
	//		AdjustReBar();
	//		small_icons_ = false;
	//	}
}

void BrowserToolbar::OnInitMenuPopup(CMenu* popup_menu, UINT index, BOOL sys_menu)
{
	popup_menu->CheckMenuRadioItem(ID_SMALL_ICONS, ID_LARGE_ICONS, small_icons_ ? ID_SMALL_ICONS : ID_LARGE_ICONS, MF_BYCOMMAND);
}
*/

void BrowserToolbar::AdjustReBar()
{
	CReBarCtrl* rebar= dynamic_cast<CReBarCtrl*>(GetParent());

	if (rebar == 0)
		return;

	int band= rebar->IDToIndex(rebar_band_id_);
	if (band == -1)
	{
		ASSERT(false);
		return;
	}

	// resize rebar band

	REBARBANDINFO bi;
	memset(&bi, 0, sizeof bi);
	bi.cbSize = sizeof bi;

	CRect rect;
	GetWindowRect(rect);
	bi.cyMinChild = bi.cxMinChild = rect.Height();
	bi.cxIdeal = rect.Width();
	bi.fMask = RBBIM_CHILDSIZE | RBBIM_IDEALSIZE;

	rebar->SetBandInfo(band, &bi);
}


BOOL BrowserToolbar::OnEraseBkgnd(CDC* dc)
{
	CRect rect(0,0,0,0);
	GetClientRect(rect);
	dc->FillSolidRect(rect, background_);

	if (main_bar_.m_hWnd == 0)
		return true;

	CRect tb(0,0,0,0);
	main_bar_.GetWindowRect(tb);

	// draw label
	g_Settings.SelectDefaultFont(*dc);
	dc->SetTextColor(g_Settings.AppColors()[AppColors::DimText]);
	dc->SetBkMode(TRANSPARENT);

	CRect text_rect= rect;
	text_rect.left += tb.Width() + 10;

	dc->DrawText(LABEL, static_cast<int>(_tcslen(LABEL)), text_rect, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

	return true;
}


void BrowserToolbar::PressButton(int cmd, bool down)
{
	UINT index= main_bar_.CommandToIndex(cmd);
	if (index == 0-1)
		panels_.PressButton(cmd, down);
	else
		main_bar_.PressButton(cmd, down);
}


CRect BrowserToolbar::GetButtonRect(int cmd)
{
	CRect rect(0,0,0,0);

	UINT index= main_bar_.CommandToIndex(cmd);
	if (index == 0-1)
	{
		panels_.GetRect(cmd, rect);
		panels_.ClientToScreen(&rect);
	}
	else
	{
		main_bar_.GetRect(cmd, rect);
		main_bar_.ClientToScreen(&rect);
	}

	return rect;
}


void BrowserToolbar::OnSize(UINT type, int cx, int cy)
{
	CWnd::OnSize(type, cx, cy);
}


void BrowserToolbar::Resize()
{
	CRect rect(0,0,0,0);

	int height= 0;
	int width= 0;

	CClientDC dc(this);
	g_Settings.SelectDefaultFont(dc);
	int label_width = dc.GetTextExtent(LABEL, int(wcslen(LABEL))).cx + 13;

	if (main_bar_.m_hWnd && panels_.m_hWnd)
	{
		main_bar_.GetWindowRect(rect);
		width = rect.Width();
		height = rect.Height();

		main_bar_.SetWindowPos(nullptr, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

		panels_.GetWindowRect(rect);
		panels_.SetWindowPos(nullptr, width + label_width, 0, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

		width += rect.Width();
	}

	SetWindowPos(nullptr, 0, 0, width + label_width, height, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
}


void BrowserToolbar::SetBackgroundColor(COLORREF color)
{
	background_ = color;
	if (m_hWnd)
		Invalidate();
}

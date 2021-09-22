/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "TaskToolbar.h"
#include "resource.h"
#include "GetDefaultGuiFont.h"

TaskToolbar::TaskToolbar()
{
	//small_icons_ = true;
	rebar_band_id_ = 0;
	horizontal_ = false;
	SetOwnerDraw(true);
}

TaskToolbar::~TaskToolbar()
{}


BEGIN_MESSAGE_MAP(TaskToolbar, ToolBarWnd)
	ON_COMMAND(ID_TOOLBAR_CUSTOMIZE, OnToolbarCustomize)
//	ON_COMMAND(ID_TOOLBAR_VERTICAL, OnToolbarVertical)
//	ON_UPDATE_COMMAND_UI(ID_TOOLBAR_VERTICAL, OnUpdateVerticalLayout)
//	ON_COMMAND(ID_TOOLBAR_HORIZONTAL, OnToolbarHorizontal)
//	ON_UPDATE_COMMAND_UI(ID_TOOLBAR_HORIZONTAL, OnUpdateHorizontalLayout)

	//ON_COMMAND(ID_SMALL_ICONS, OnSmallIcons)
//	ON_UPDATE_COMMAND_UI(ID_SMALL_ICONS, OnUpdateSmallIcons)
	//ON_COMMAND(ID_LARGE_ICONS, OnLargeIcons)
//	ON_UPDATE_COMMAND_UI(ID_LARGE_ICONS, OnUpdateLargeIcons)
	ON_WM_DESTROY()
//	ON_NOTIFY_REFLECT(NM_RCLICK, OnRightClick)
	ON_WM_INITMENUPOPUP()
	ON_WM_CONTEXTMENU()
	ON_NOTIFY_REFLECT(TBN_GETDISPINFO, OnGetDispInfo)
	ON_NOTIFY_REFLECT(TBN_RESTORE, OnButtonRestore)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TaskToolbar message handlers

namespace {
	const TCHAR* REGISTRY_SECTION_TOOLBAR	= _T("TaskToolBar");
	const TCHAR* REG_STATE					= _T("TbState");
	const TCHAR* REG_STATE_VERT				= _T("TbVertState");
	const TCHAR* REG_ICONS					= _T("Icons");
	const float saturation= -0.5f;
	const float lightness= 0.0f;
	const float alpha= 0.55f;
	//const int TOOLBAR_BITMAP_SMALL= IDB_BROWSER_TOOLS;
	//const int TOOLBAR_BITMAP_BIG= IDB_BROWSER_TOOLS;
}


void TaskToolbar::OnButtonRestore(NMHDR* notify_struct, LRESULT* result)
{
	NMTBRESTORE* restore= reinterpret_cast<NMTBRESTORE*>(notify_struct);
	*result = 0;

	// for vertical toolbar set separator's state to wrap
	if (!horizontal_ && restore->iItem >= 0 && restore->tbButton.fsStyle == TBSTYLE_SEP)
		restore->tbButton.fsState |= TBSTATE_WRAP;
}


void TaskToolbar::OnGetDispInfo(NMHDR* notify_struct, LRESULT* result)
{
	NMTBDISPINFO* nm_tb= reinterpret_cast<NMTBDISPINFO*>(notify_struct);
	*result = 0;
}


bool TaskToolbar::Create(CWnd* parent, UINT id, UINT rebar_band_id, bool vertical)
{
	rebar_band_id_ = rebar_band_id;

	bool ok= false;
	if (Create(parent, id, vertical))
	{
		horizontal_ = !vertical;

		RestoreState(REGISTRY_SECTION_TOOLBAR, vertical ? REG_STATE_VERT : REG_STATE);

		ok = true;
	}

	return ok;
}


bool TaskToolbar::Create(CWnd* parent, UINT id, bool vertical)
{
	static const int commands[]=
	{
		/*ID_VIEW,
		ID_TASK_TRANSFER, ID_TASK_COPY, ID_TASK_MOVE, ID_TASK_RENAME, ID_TASK_COPY_TAGGED,
		ID_TASK_RESIZE, ID_TASK_ROTATE, ID_TASK_EDIT_IPTC,
		ID_TASK_GEN_SLIDE_SHOW, ID_TASK_GEN_HTML_ALBUM, ID_BUILD_CATALOG,
		ID_TASK_PRINT, ID_TASK_EXPORT, ID_TASK_HISTOGRAM,
		ID_TASK_TOUCH_UP, ID_SEND_EMAIL, ID_DATE_TIME_ADJ,
		ID_TASK_EXTRACT_JPEG, ID_TASK_DELETE
		ID_VIEW,*/
		ID_TASK_TRANSFER, ID_TASK_COPY, ID_TASK_MOVE, ID_TASK_RENAME, ID_TASK_COPY_TAGGED,
		ID_TASK_RESIZE, ID_TASK_ROTATE, ID_TASK_TOUCH_UP, ID_TASK_HISTOGRAM, ID_TASK_EXTRACT_JPEG, ID_TASK_EDIT_IPTC,
		ID_TASK_GEN_SLIDE_SHOW, ID_TASK_GEN_HTML_ALBUM, ID_BUILD_CATALOG,
		ID_TASK_PRINT, ID_TASK_EXPORT,
		ID_SEND_EMAIL, ID_DATE_TIME_ADJ,
		ID_TASK_DELETE
	};

	if (vertical)
		SetPadding(0, 0);
	else
		SetPadding(0, 4);

	//small_icons_ = AfxGetApp()->GetProfileInt(REGISTRY_SECTION_TOOLBAR, REG_ICONS, 0) == 0;

	int bmp_id= IDB_BROWSER_TOOLS;//small_icons_ ? TOOLBAR_BITMAP_SMALL : TOOLBAR_BITMAP_BIG;

	DWORD tb_style= WS_CHILD | WS_VISIBLE | TBSTYLE_TOOLTIPS | TBSTYLE_FLAT | TBSTYLE_TRANSPARENT |
		/*CCS_TOP |*/ CCS_NORESIZE | CCS_NOPARENTALIGN | CCS_NODIVIDER | TBSTYLE_ALTDRAG | CCS_ADJUSTABLE;

	if (vertical)
		tb_style |= CCS_RIGHT;
	else
		tb_style |= CCS_TOP;

	if (!CToolBarCtrl::Create(tb_style, CRect(0,0,0,0), parent, id))
		return false;

	::SetWindowTheme(m_hWnd, L"", L"");

	int w= 0;
	{
		CClientDC dc(parent);
		dc.SelectObject(&GetDefaultGuiFont());//&_font);
		//dc.SelectStockObject(DEFAULT_GUI_FONT);
		w = dc.GetTextExtent(_T("NNNNNNNa"), 8).cx;
	}
	// limit button width to make labels wrap
	if (vertical)
		w = w * 13 / 10;
	SendMessage(TB_SETBUTTONWIDTH, 0, MAKELONG(w, w));
	SendMessage(TB_SETDRAWTEXTFLAGS, DT_WORDBREAK, DT_WORDBREAK);
	SendMessage(TB_SETDRAWTEXTFLAGS, DT_SINGLELINE, 0);
	SendMessage(TB_SETMAXTEXTROWS, vertical ? 1 : 2, 0);

	//AddButtons("P|PPPPP|PPPPPPPPPPPPPP", commands, bmp_id, /*IDS_BROWSER_TOOLS*/false, vertical);
	AddButtons("PPPPP|PPPPPPPPPPPPPP", commands, bmp_id, /*IDS_BROWSER_TOOLS*/false, vertical);
	CreateDisabledImageList(bmp_id, saturation, lightness, alpha);

	DeleteButtons();

	return true;
}


CSize TaskToolbar::GetSize()
{
	CRect rect(0,0,0,0);
	GetWindowRect(rect);
	return rect.Size();
}


void TaskToolbar::ResetToolBar(bool resize_to_fit)
{
	ToolBarWnd::ResetToolBar(resize_to_fit);
	DeleteButtons();
}


void TaskToolbar::DeleteButtons()
{
	DeleteButton(ID_TASK_COPY_TAGGED);
	DeleteButton(ID_TASK_GEN_SLIDE_SHOW);
	DeleteButton(ID_TASK_EXPORT);
	DeleteButton(ID_SEND_EMAIL);
	DeleteButton(ID_DATE_TIME_ADJ);
	DeleteButton(ID_TASK_EXTRACT_JPEG);
	//DeleteButton(ID_TASK_DELETE);
}

//void TaskToolbar::OnRightClick(NMHDR* notify_struct, LRESULT* result)
//{
//	*result = 0;
//	CPoint pos(0, 0);
//	GetCursorPos(&pos);
//	OnContextMenu(this, pos);
//}


void TaskToolbar::OnContextMenu(CWnd* wnd, CPoint pos)
{
	CMenu menu;
	if (!menu.LoadMenu(IDR_TASK_TOOLBAR_CONTEXT))
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
}


void TaskToolbar::OnToolbarCustomize()
{
	Customize();
}


void TaskToolbar::OnDestroy()
{
	SaveState(REGISTRY_SECTION_TOOLBAR, horizontal_ ? REG_STATE : REG_STATE_VERT);

	//AfxGetApp()->WriteProfileInt(REGISTRY_SECTION_TOOLBAR, REG_ICONS, small_icons_ ? 0 : 1);

	ToolBarWnd::OnDestroy();
}

/*
void TaskToolbar::OnSmallIcons()
{
	if (!small_icons_)
		if (ReplaceImageList(TOOLBAR_BITMAP_SMALL))
		{
			CreateDisabledImageList(TOOLBAR_BITMAP_SMALL, saturation, lightness, alpha);
			AdjustReBar();
			small_icons_ = true;
		}
}


void TaskToolbar::OnLargeIcons()
{
	if (small_icons_)
		if (ReplaceImageList(TOOLBAR_BITMAP_BIG))
		{
			CreateDisabledImageList(TOOLBAR_BITMAP_BIG, saturation, lightness, alpha);
			AdjustReBar();
			small_icons_ = false;
		}
}
*/

void TaskToolbar::OnInitMenuPopup(CMenu* popup_menu, UINT index, BOOL sys_menu)
{
//	popup_menu->CheckMenuRadioItem(ID_SMALL_ICONS, ID_LARGE_ICONS, small_icons_ ? ID_SMALL_ICONS : ID_LARGE_ICONS, MF_BYCOMMAND);
	popup_menu->CheckMenuRadioItem(ID_TOOLBAR_HORIZONTAL, ID_TOOLBAR_VERTICAL, horizontal_ ? ID_TOOLBAR_HORIZONTAL : ID_TOOLBAR_VERTICAL, MF_BYCOMMAND);
}


void TaskToolbar::AdjustReBar()
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


bool TaskToolbar::IsHorizontal() const
{
	return horizontal_;
}

void TaskToolbar::SetOrientation(bool horizontal)
{
	if (horizontal_ == horizontal)
		return;

	CWnd* parent= GetParent();
	int id= GetDlgCtrlID();
	DestroyWindow();
	Create(parent, id, !horizontal);

	horizontal_ = horizontal;

	RestoreState(REGISTRY_SECTION_TOOLBAR, horizontal ? REG_STATE : REG_STATE_VERT);
}


//void TaskToolbar::OnUpdateVerticalLayout(CCmdUI* cmd_ui)
//{
//	cmd_ui->Enable();
//	cmd_ui->SetRadio(!IsHorizontal());
//}
//
//void TaskToolbar::OnUpdateHorizontalLayout(CCmdUI* cmd_ui)
//{
//	cmd_ui->Enable();
//	cmd_ui->SetRadio(IsHorizontal());
//}

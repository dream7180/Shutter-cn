/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// OptionsColumns.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "OptionsColumns.h"
#include "Columns.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// OptionsColumns dialog


OptionsColumns::OptionsColumns(Columns& columns) : RPropertyPage(OptionsColumns::IDD), ColumnTree(columns)
{
//	psp_.flags |= PSP_HASHELP;
	//{{AFX_DATA_INIT(OptionsColumns)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void OptionsColumns::DoDataExchange(CDataExchange* DX)
{
	CPropertyPage::DoDataExchange(DX);
	//{{AFX_DATA_MAP(OptionsColumns)
	//}}AFX_DATA_MAP

	TreeDoDataExchange(DX, IDC_TREE);
}


BEGIN_MESSAGE_MAP(OptionsColumns, RPropertyPage)
	//{{AFX_MSG_MAP(OptionsColumns)
	ON_BN_CLICKED(IDC_RESET, OnReset)
	ON_BN_CLICKED(IDC_RESET_CANON, OnResetCanon)
	ON_BN_CLICKED(IDC_RESET_NIKON, OnResetNikon)
	ON_BN_CLICKED(IDC_RESET_FUJI, OnResetFuji)
	ON_BN_CLICKED(IDC_RESET_OLYMPUS, OnResetOlympus)
	ON_BN_CLICKED(IDC_CUSTOM_COLUMNS, OnDefineCustomColumns)
	//}}AFX_MSG_MAP
	ON_NOTIFY(TVN_ITEMCHANGED, IDC_TREE, &OptionsColumns::OnItemChanged)	// this notification works only in Vista and above
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// OptionsColumns message handlers

BOOL OptionsColumns::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	InitTree();

	if (define_columns_.empty())
		if (CWnd* btn= GetDlgItem(IDC_CUSTOM_COLUMNS))
			btn->EnableWindow(false);

	ResizeMgr().BuildMap(this);
	ResizeMgr().SetWndResizing(IDC_TREE, DlgAutoResize::RESIZE);
	ResizeMgr().SetWndResizing(IDC_LABEL_1, DlgAutoResize::MOVE_H);
	ResizeMgr().SetWndResizing(IDC_RESET, DlgAutoResize::MOVE_H);
	ResizeMgr().SetWndResizing(IDC_RESET_CANON, DlgAutoResize::MOVE_H);
	ResizeMgr().SetWndResizing(IDC_RESET_NIKON, DlgAutoResize::MOVE_H);
	ResizeMgr().SetWndResizing(IDC_RESET_FUJI, DlgAutoResize::MOVE_H);
	ResizeMgr().SetWndResizing(IDC_RESET_OLYMPUS, DlgAutoResize::MOVE_H);

	ResizeMgr().SetWndResizing(IDC_LABEL_2, DlgAutoResize::MOVE_H);
	ResizeMgr().SetWndResizing(IDC_CUSTOM_COLUMNS, DlgAutoResize::MOVE_H);

	ResizeMgr().SetWndResizing(IDC_LABEL_3, DlgAutoResize::MOVE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void OptionsColumns::OnReset()
{
	Reset(0);
}


void OptionsColumns::OnResetCanon()
{
	Reset(item_canon_);

	SetCheck(COL_EXP_PROG, false);		// exp. program
	SetCheck(COL_LIGHT_SRC, false);		// light src.

	SetCheck(COL_CAN_LENS, true);		// lens attached
	SetCheck(COL_CAN_TIMER, true);		// self timer
	SetCheck(COL_CAN_DRIVE, true);		// drive mode
	SetCheck(COL_CAN_FOCUS, true);		// focus mode
	SetCheck(COL_CAN_PROGRAM, true);	// program
	SetCheck(COL_CAN_MET_MODE, true);	// metering
}


void OptionsColumns::OnResetNikon()
{
	Reset(item_nikon_);

	SetCheck(COL_NIK_COLOR_MODE, true);
	SetCheck(COL_NIK_QUALITY, true);
	SetCheck(COL_NIK_IMG_SHARP, true);
	SetCheck(COL_NIK_FOCUS_MODE, true);
//	SetCheck(COL_NIK_MF_DIST, true);
	SetCheck(COL_NIK_DIGI_ZOOM, true);
//	SetCheck(COL_NIK_AF_POS, true);
}


void OptionsColumns::OnResetFuji()
{
	Reset(item_fuji_);

	SetCheck(COL_FUJI_QUALITY, true);
	SetCheck(COL_FUJI_WHITE_BALANCE, true);
	SetCheck(COL_FUJI_FLASH_MODE, true);
	SetCheck(COL_FUJI_MACRO, true);
	SetCheck(COL_FUJI_FOCUS_MODE, true);
	SetCheck(COL_FUJI_SLOWSYNC, true);
}


void OptionsColumns::OnResetOlympus()
{
	Reset(item_olympus_);

	SetCheck(COL_OLY_SPECIAL_MODE, true);
	SetCheck(COL_OLY_JPEG_QUALITY, true);
	SetCheck(COL_OLY_MACRO, true);
	SetCheck(COL_OLY_DIGITAL_ZOOM, true);
}


void OptionsColumns::OnDefineCustomColumns()
{
	if (define_columns_)
		if (CWnd* p= GetParent())
			define_columns_(p);
}

void OptionsColumns::OnItemChanged(NMHDR* nmhdr, LRESULT* result)
{
	NMTVITEMCHANGE* change= reinterpret_cast<NMTVITEMCHANGE*>(nmhdr);

	if ((change->uStateOld & TVIS_STATEIMAGEMASK) != (change->uStateNew & TVIS_STATEIMAGEMASK))
	{
		// check box state modified

		// remove checkbox from the node, only leaves are to have checkboxes
		if (tree_wnd_.m_hWnd && tree_wnd_.GetParentItem(change->hItem) == 0)
			tree_wnd_.SetItemState(change->hItem, INDEXTOSTATEIMAGEMASK(0), TVIS_STATEIMAGEMASK);
	}

	*result = 0;
}

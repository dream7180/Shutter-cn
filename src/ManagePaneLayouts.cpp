/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// ManagePaneLayouts.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "ManagePaneLayouts.h"
#include "BalloonMsg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// ManagePaneLayouts dialog

//IMPLEMENT_DYNAMIC(ManagePaneLayouts, CDialog)

ManagePaneLayouts::ManagePaneLayouts(CWnd* parent, bool add_new)
	: CDialog(ManagePaneLayouts::IDD, parent), adding_new_(add_new)
{
	//{{AFX_DATA_INIT(ManagePaneLayouts)
	name_ = _T("");
	//}}AFX_DATA_INIT
}


ManagePaneLayouts::~ManagePaneLayouts()
{}


void ManagePaneLayouts::DoDataExchange(CDataExchange* DX)
{
	CDialog::DoDataExchange(DX);
	//{{AFX_DATA_MAP(ManagePaneLayouts)
	DDX_Control(DX, IDOK, btn_ok_);
	DDX_Control(DX, IDC_NAME, edit_name_);
	DDX_Control(DX, IDC_LIST, list_wnd_);
	DDX_Control(DX, IDC_DELETE, btn_delete_);
	DDX_Text(DX, IDC_NAME, name_);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(ManagePaneLayouts, CDialog)
	//{{AFX_MSG_MAP(ManagePaneLayouts)
	ON_BN_CLICKED(IDC_DELETE, OnDelete)
	ON_LBN_SELCHANGE(IDC_LIST, OnSelChangeList)
	ON_EN_CHANGE(IDC_NAME, OnChangeName)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


// ManagePaneLayouts message handlers

void ManagePaneLayouts::OnDelete()
{
	int selected= list_wnd_.GetCurSel();
	ASSERT(names_.size() > selected);
	if (selected >= 0)
	{
		names_.erase(names_.begin() + selected);
		list_wnd_.DeleteString(selected);

		if (list_wnd_.GetCount() > selected)
			list_wnd_.SetCurSel(selected);
		else if (list_wnd_.GetCount() > selected - 1 && selected - 1 >= 0)
			list_wnd_.SetCurSel(selected - 1);

		CheckDelButton();
	}
}


BOOL ManagePaneLayouts::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetWindowText(adding_new_ ? _T("Add New Pane Layout") : _T("Manage Pane Layouts"));

	edit_name_.SetReadOnly(!adding_new_);

	const size_t count= names_.size();
	list_wnd_.InitStorage(static_cast<int>(count), 20);
	for (size_t i= 0; i < count; ++i)
		list_wnd_.AddString(names_[i].first.c_str());

	list_wnd_.SetCurSel(0);

	CheckDelButton();

	edit_name_.LimitText(200);

	OnChangeName();

	return TRUE;
}


void ManagePaneLayouts::OnSelChangeList()
{
	CheckDelButton();

	if (list_wnd_.m_hWnd != 0 && edit_name_.m_hWnd != 0)
	{
		int sel= list_wnd_.GetCurSel();
		CString name;
		if (sel >= 0)
			list_wnd_.GetText(sel, name);
		edit_name_.SetWindowText(name);
	}
}


void ManagePaneLayouts::CheckDelButton()
{
	if (btn_delete_.m_hWnd && list_wnd_.m_hWnd)
		btn_delete_.EnableWindow(list_wnd_.GetCurSel() >= 0);
}


void ManagePaneLayouts::OnChangeName()
{
//	if (edit_name_.m_hWnd == 0 || btn_ok_.m_hWnd == 0)
//		return;
//
//	btn_ok_.EnableWindow(edit_name_.GetWindowTextLength() > 0);
}


bool ManagePaneLayouts::FindLayout(const TCHAR* name)
{
	// this is not case sensitive search
	int index= list_wnd_.FindStringExact(0, name);

	if (index == LB_ERR)
		return false;

	list_wnd_.SetCurSel(index);

	return true;
}


void ManagePaneLayouts::OnOK()
{
	if (!UpdateData())
		return;

	if (adding_new_ && name_.IsEmpty())
	{
		new BalloonMsg(&edit_name_, _T("Missing Layout Name"),
			_T("Please name pane layout. This name will show up in a pane popup menu."), BalloonMsg::IERROR);
		return;
	}

	if (adding_new_ && FindLayout(name_))
	{
		new BalloonMsg(&edit_name_, _T("Duplicated Name"),
			_T("Please rename pane layout or delete conflicting one."), BalloonMsg::IERROR);
		return;
	}

	CDialog::OnOK();
}

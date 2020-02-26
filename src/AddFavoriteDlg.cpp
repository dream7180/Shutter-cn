/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// AddFavoriteDlg.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "AddFavoriteDlg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


// AddFavoriteDlg dialog

AddFavoriteDlg::AddFavoriteDlg(CWnd* parent /*=NULL*/)
	: CDialog(AddFavoriteDlg::IDD, parent)
{
}

AddFavoriteDlg::~AddFavoriteDlg()
{
}

void AddFavoriteDlg::DoDataExchange(CDataExchange* DX)
{
	CDialog::DoDataExchange(DX);
	DDX_Text(DX, IDC_PATH, path_);
	DDX_Control(DX, IDC_NAME, edit_name_);
	DDX_Text(DX, IDC_NAME, name_);
	DDX_Control(DX, IDC_PATH, path_wnd_);
	DDX_Control(DX, IDOK, btn_ok_);
}


BEGIN_MESSAGE_MAP(AddFavoriteDlg, CDialog)
	ON_EN_CHANGE(IDC_NAME, OnChangeName)
END_MESSAGE_MAP()


// AddFavoriteDlg message handlers

BOOL AddFavoriteDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	edit_name_.LimitText(200);
	path_wnd_.ModifyStyle(0, SS_PATHELLIPSIS);

	UpdateOkBtn();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void AddFavoriteDlg::UpdateOkBtn()
{
	if (btn_ok_.m_hWnd && edit_name_.m_hWnd)
		btn_ok_.EnableWindow(edit_name_.GetWindowTextLength() > 0);
}


void AddFavoriteDlg::OnChangeName()
{
	UpdateOkBtn();
}

/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// TagsManageDlg.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "TagsManageDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTagsManageDlg dialog


CTagsManageDlg::CTagsManageDlg(CWnd* parent /*=NULL*/)
	: CDialog(CTagsManageDlg::IDD, parent)
{
	//{{AFX_DATA_INIT(CTagsManageDlg)
	tags_ = _T("");
	//}}AFX_DATA_INIT
}

static const int LIMIT_TEXT= 0x100000 - 10;

void CTagsManageDlg::DoDataExchange(CDataExchange* DX)
{
	CDialog::DoDataExchange(DX);
	//{{AFX_DATA_MAP(CTagsManageDlg)
	DDX_Control(DX, IDC_TAGS, edit_tags_);
	DDX_Text(DX, IDC_TAGS, tags_);
//	DDV_MaxChars(DX, tags_, LIMIT_TEXT);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTagsManageDlg, CDialog)
	//{{AFX_MSG_MAP(CTagsManageDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTagsManageDlg message handlers

BOOL CTagsManageDlg::OnInitDialog()
{
	tags_.Replace(_T("\n"), _T("\r\n"));

	CDialog::OnInitDialog();

	edit_tags_.LimitText(LIMIT_TEXT);
	GotoDlgCtrl(&edit_tags_);
	edit_tags_.SetSel(-1, 0, true);

	CenterWindow(m_pParentWnd);

	return false;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void CTagsManageDlg::OnOK()
{
	UpdateData();

	tags_.Replace(_T("\r\n"), _T("\n"));

	CDialog::OnOK();
}

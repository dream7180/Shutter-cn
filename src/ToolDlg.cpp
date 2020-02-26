/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// ToolDlg.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "ToolDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CToolDlg dialog


CToolDlg::CToolDlg(const TCHAR* name, CWnd* parent /*=NULL*/)
	: caption_(name), RDialog(CToolDlg::IDD, parent)
{
	Create(CToolDlg::IDD, parent);
	//{{AFX_DATA_INIT(CToolDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CToolDlg::DoDataExchange(CDataExchange* DX)
{
	CDialog::DoDataExchange(DX);
	//{{AFX_DATA_MAP(CToolDlg)
	DDX_Control(DX, IDC_FILE, file_wnd_);
	DDX_Control(DX, IDC_PROGRESS, progress_wnd_);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CToolDlg, CDialog)
	//{{AFX_MSG_MAP(CToolDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CToolDlg message handlers

BOOL CToolDlg::OnInitDialog()
{
	RDialog::OnInitDialog();

	SetWindowText(caption_.c_str());

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

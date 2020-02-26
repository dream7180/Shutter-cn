/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// SlideShowOptDlg.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "SlideShowOptDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSlideShowOptDlg dialog


CSlideShowOptDlg::CSlideShowOptDlg(CWnd* parent /*=NULL*/)
	: CDialog(CSlideShowOptDlg::IDD, parent)
{
	//{{AFX_DATA_INIT(CSlideShowOptDlg)
	delay_ = 0;
	hide_toolbar_ = FALSE;
	repeat_ = FALSE;
	//}}AFX_DATA_INIT
}


void CSlideShowOptDlg::DoDataExchange(CDataExchange* DX)
{
	CDialog::DoDataExchange(DX);
	//{{AFX_DATA_MAP(CSlideShowOptDlg)
	DDX_Control(DX, IDC_SPIN, spin_wnd_);
	DDX_Text(DX, IDC_DELAY, delay_);
	DDV_MinMaxInt(DX, delay_, 0, 9999);
	DDX_Check(DX, IDC_HIDE_TB, hide_toolbar_);
	DDX_Check(DX, IDC_REPEAT, repeat_);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSlideShowOptDlg, CDialog)
	//{{AFX_MSG_MAP(CSlideShowOptDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSlideShowOptDlg message handlers

BOOL CSlideShowOptDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	spin_wnd_.SetRange(0, 9999);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

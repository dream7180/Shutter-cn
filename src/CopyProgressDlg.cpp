/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// CopyProgressDlg.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "CopyProgressDlg.h"


// CopyProgressDlg dialog

CopyProgressDlg::CopyProgressDlg(CWnd* parent /*=NULL*/)
	: CDialog(CopyProgressDlg::IDD, parent)
{
}

CopyProgressDlg::~CopyProgressDlg()
{
}

void CopyProgressDlg::DoDataExchange(CDataExchange* DX)
{
	CDialog::DoDataExchange(DX);
	DDX_Control(DX, IDC_ANIMATION, animation_);
}


BEGIN_MESSAGE_MAP(CopyProgressDlg, CDialog)
END_MESSAGE_MAP()


// CopyProgressDlg message handlers


BOOL CopyProgressDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	animation_.Open(IDR_COPY_ANIM);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

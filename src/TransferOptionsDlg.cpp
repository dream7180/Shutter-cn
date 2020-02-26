/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// TransferOptionsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "TransferOptionsDlg.h"
#include "EnableCtrl.h"

enum { IDD = IDD_TRANSFER_OPTIONS };

// CTransferOptionsDlg dialog

CTransferOptionsDlg::CTransferOptionsDlg(CWnd* parent /*=NULL*/)
	: CDialog(IDD, parent)
{
	makeReadOnly_ = false;
	clearArchiveAttr_ = false;
}

CTransferOptionsDlg::~CTransferOptionsDlg()
{
}

void CTransferOptionsDlg::DoDataExchange(CDataExchange* DX)
{
	CDialog::DoDataExchange(DX);
	DDX_Check(DX, IDC_READ_ONLY, makeReadOnly_);
	DDX_Check(DX, IDC_CLEAR_ARCHIVE_ATTR, clearArchiveAttr_);
}


BEGIN_MESSAGE_MAP(CTransferOptionsDlg, CDialog)
END_MESSAGE_MAP()


// CTransferOptionsDlg message handlers


bool CTransferOptionsDlg::Create(CWnd* parent)
{
	if (!CDialog::Create(IDD, parent))
		return false;

	return true;
}


BOOL CTransferOptionsDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	return false;
}

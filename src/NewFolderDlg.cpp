/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// NewFolderDlg.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "NewFolderDlg.h"
#include "Path.h"
#include "BalloonMsg.h"


// NewFolderDlg dialog

NewFolderDlg::NewFolderDlg(CWnd* parent /*=NULL*/)
	: CDialog(NewFolderDlg::IDD, parent)
{
}

NewFolderDlg::~NewFolderDlg()
{
}

void NewFolderDlg::DoDataExchange(CDataExchange* DX)
{
	CDialog::DoDataExchange(DX);
	DDX_Text(DX, IDC_NAME, folderName_);
}


BEGIN_MESSAGE_MAP(NewFolderDlg, CDialog)
	ON_EN_CHANGE(IDC_NAME, OnNameChanged)
END_MESSAGE_MAP()


// NewFolderDlg message handlers

BOOL NewFolderDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	VERIFY(btnOk_.SubclassDlgItem(IDOK, this));

	name_.InitAutoComplete(false);
	name_.FileNameEditing(true);
	VERIFY(name_.SubclassDlgItem(IDC_NAME, this));

	CenterWindow(m_pParentWnd);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void NewFolderDlg::OnNameChanged()
{
	if (btnOk_.m_hWnd && name_.m_hWnd)
		btnOk_.EnableWindow(name_.GetWindowTextLength() > 0);
}


void NewFolderDlg::SetCreateFolderCallback(const boost::function<bool (CWnd* edit, const CString& name)>& createFolder)
{
	createFolder_ = createFolder;
}


void NewFolderDlg::OnOK()
{
	if (!UpdateData())
		return;

	if (createFolder_ && createFolder_(&name_, folderName_))
		EndDialog(IDOK);
}


extern bool CreateFolderHelperFn(CWnd* edit, const CString& name, String* path)
{
	Path folder= *path;
	folder.AppendDir(name, false);
	if (folder.IsFolder())
	{
		new BalloonMsg(edit, _T("文件夹已存在"), _T("此文件夹已存在. 请提供另外的名字."), BalloonMsg::IWARNING);
		return false;
	}

	if (!folder.CreateIfDoesntExist(edit))
		return false;

	*path = folder;

	return true;
}

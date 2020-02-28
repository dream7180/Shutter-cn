/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// FileOperDlg.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "FileOperDlg.h"
#include "FolderSelect.h"
#include "RString.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// FileOperDlg dialog


FileOperDlg::FileOperDlg(bool copy, const TCHAR* dest_path, CWnd* parent /*=NULL*/)
	: DialogChild(FileOperDlg::IDD, parent)
{
	copy_operation_ = copy;
	label_ = copy ? _T("复制") : _T("移动");
	label_ += _T(" 选定的图像到此文件夹:");
//	label_ = RString(copy ? IDS_COPY_FILES : IDS_MOVE_FILES);
	path_ = dest_path;
	//{{AFX_DATA_INIT(FileOperDlg)
	//}}AFX_DATA_INIT
}


void FileOperDlg::DoDataExchange(CDataExchange* DX)
{
	DialogChild::DoDataExchange(DX);
	//{{AFX_DATA_MAP(FileOperDlg)
	DDX_Control(DX, IDOK, btn_ok_);
	DDX_Control(DX, IDC_PATH, edit_path_);
	DDX_Text(DX, IDC_LABEL, label_);
	DDX_Text(DX, IDC_PATH, path_);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(FileOperDlg, DialogChild)
	//{{AFX_MSG_MAP(FileOperDlg)
	ON_COMMAND(IDC_BROWSE, OnBrowse)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// FileOperDlg message handlers

BOOL FileOperDlg::OnInitDialog()
{
	DialogChild::OnInitDialog();

	SubclassHelpBtn(copy_operation_ ? _T("ToolCopy.htm") : _T("ToolMove.htm"));

	SetWindowText(copy_operation_ ? _T("复制照片") : _T("移动照片"));
	btn_ok_.SetWindowText(copy_operation_ ? _T("复制") : _T("移动"));

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void FileOperDlg::OnBrowse()
{
	UpdateData();

	CFolderSelect fs(AfxGetMainWnd());
	CString path= fs.DoSelectPath(_T("选择目标文件夹")/*RString(IDS_SELECT_DEST)*/, path_);

	if (path.IsEmpty())
		return;

	path_ = path;

	UpdateData(false);
}

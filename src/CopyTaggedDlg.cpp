/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// CopyTaggedDlg.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "CopyTaggedDlg.h"
#include "FolderSelect.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCopyTaggedDlg dialog


CCopyTaggedDlg::CCopyTaggedDlg(CWnd* parent /*=NULL*/)
	: DialogChild(CCopyTaggedDlg::IDD, parent)
{
	//{{AFX_DATA_INIT(CCopyTaggedDlg)
	path_ = _T("");
	separate_folders_ = -1;
	//}}AFX_DATA_INIT

	const TCHAR* REGISTRY_ENTRY_COPY_TAGGED= _T("CopyTaggedDlg");
	profile_separate_folders_.Register(REGISTRY_ENTRY_COPY_TAGGED, _T("SeparateFolders"), true);
	profile_dest_path_.Register(REGISTRY_ENTRY_COPY_TAGGED, _T("DestPath"), _T("d:\\Tagged Photos"));

	separate_folders_ = profile_separate_folders_ ? 1 : 0;
	path_ = profile_dest_path_;
}


void CCopyTaggedDlg::DoDataExchange(CDataExchange* DX)
{
	DialogChild::DoDataExchange(DX);
	//{{AFX_DATA_MAP(CCopyTaggedDlg)
	DDX_Control(DX, IDC_PATH, edit_path_);
	DDX_Text(DX, IDC_PATH, path_);
	DDX_Radio(DX, IDC_ONE_FOLDER, separate_folders_);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCopyTaggedDlg, DialogChild)
	//{{AFX_MSG_MAP(CCopyTaggedDlg)
	ON_BN_CLICKED(IDC_BROWSE, OnBrowse)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCopyTaggedDlg message handlers

BOOL CCopyTaggedDlg::OnInitDialog()
{
	DialogChild::OnInitDialog();

	SubclassHelpBtn(_T("ToolCopyTagged.htm"));

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void CCopyTaggedDlg::OnOK()
{
	profile_separate_folders_ = separate_folders_ == 1;
	profile_dest_path_ = path_;

	DialogChild::OnOK();
}


void CCopyTaggedDlg::OnBrowse()
{
	UpdateData();

	CFolderSelect fs(AfxGetMainWnd());
	CString path= fs.DoSelectPath(_T("选择目标文件夹")/*RString(IDS_SELECT_DEST)*/, path_);

	if (path.IsEmpty())
		return;

	path_ = path;

	UpdateData(false);
}

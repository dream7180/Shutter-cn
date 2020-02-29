/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// ImgBatchModeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "ImgBatchModeDlg.h"
#include "BalloonMsg.h"
#include "Path.h"
#include "FolderSelect.h"
#include "RString.h"
extern String ReplaceIllegalChars(const String& text);

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


// ImgBatchModeDlg dialog

ImgBatchModeDlg::ImgBatchModeDlg(CWnd* parent /*=NULL*/)
	: CDialog(ImgBatchModeDlg::IDD, parent)
{
	dest_folder_ = 0;
	suffix_ = _T("_modified");
}


ImgBatchModeDlg::~ImgBatchModeDlg()
{}


void ImgBatchModeDlg::DoDataExchange(CDataExchange* DX)
{
	CDialog::DoDataExchange(DX);
	DDX_Radio(DX, IDC_SAME_DIR, dest_folder_);
	DDX_Text(DX, IDC_DEST_PATH, dest_folder_str_);
	DDX_Control(DX, IDC_SUFFIX, edit_suffix_);
	DDX_Text(DX, IDC_SUFFIX, suffix_);
	DDX_Control(DX, IDC_EXAMPLE, example_wnd_);
	DDX_Control(DX, IDC_DEST_PATH, edit_dest_path_);
}


BEGIN_MESSAGE_MAP(ImgBatchModeDlg, CDialog)
	ON_WM_SIZE()
	ON_EN_CHANGE(IDC_SUFFIX, OnChangeSuffix)
	ON_BN_CLICKED(IDC_SAME_DIR, OnSameDir)
	ON_BN_CLICKED(IDC_SELECT, OnSelectDir)
	ON_BN_CLICKED(IDC_BROWSE, OnBrowse)
END_MESSAGE_MAP()



bool ImgBatchModeDlg::Finish()
{
	if (!UpdateData())
		return false;

	extern const TCHAR* PathIllegalChars();

	if (!suffix_.IsEmpty())
		if (suffix_.FindOneOf(PathIllegalChars()) >= 0)
		{
			String msg= _T("后缀文本不能包含以下字符: ");
			msg += PathIllegalChars();
			new BalloonMsg(&edit_suffix_, _T("非法字符"), msg.c_str(), BalloonMsg::IERROR);
			return false;
		}

	if (dest_folder_ == 0)
	{
		if (suffix_.IsEmpty())
		{
			new BalloonMsg(&edit_suffix_, _T("缺少后缀"),
				_T("请输入后缀文本, 使目标文件名和原文件名不相同."), BalloonMsg::IERROR);
			return false;
		}
	}
	else
	{
		Path path(dest_folder_str_);
		if (path.empty())
		{
			new BalloonMsg(&edit_dest_path_, _T("缺少目标文件夹"),
				_T("请指定被移动图像的储存文件夹."), BalloonMsg::IERROR);
			return false;
		}
		else
		{
			if (!path.CreateIfDoesntExist(&edit_dest_path_))
				return false;
		}
	}

	return true;
}


// ImgBatchModeDlg message handlers

bool ImgBatchModeDlg::Create(CWnd* parent)
{
	return !!CDialog::Create(IDD, parent);
}


void ImgBatchModeDlg::OnSize(UINT type, int cx, int cy)
{
	CDialog::OnSize(type, cx, cy);

	dlg_resize_map_.Resize();
}


BOOL ImgBatchModeDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	edit_suffix_.LimitText(100);

	dlg_resize_map_.BuildMap(this);

	dlg_resize_map_.SetWndResizing(IDC_FRAME, DlgAutoResize::RESIZE_H);
	dlg_resize_map_.SetWndResizing(IDC_DEST_PATH, DlgAutoResize::RESIZE_H);
	dlg_resize_map_.SetWndResizing(IDC_BROWSE, DlgAutoResize::MOVE_H);
	dlg_resize_map_.SetWndResizing(IDC_LABEL_1, DlgAutoResize::MOVE_H);
	dlg_resize_map_.SetWndResizing(IDC_SUFFIX, DlgAutoResize::MOVE_H);
	dlg_resize_map_.SetWndResizing(IDC_LABEL_2, DlgAutoResize::MOVE_H);
	dlg_resize_map_.SetWndResizing(IDC_EXAMPLE, DlgAutoResize::MOVE_H);

	UpdateExample();
	UpdateDirs();

	return true;
}


void ImgBatchModeDlg::UpdateExample()
{
	if (edit_suffix_.m_hWnd != 0)
	{
		CString suffix;
		edit_suffix_.GetWindowText(suffix);
		suffix = ReplaceIllegalChars(static_cast<const TCHAR*>(suffix)).c_str();
		example_wnd_.SetWindowText(_T("DSC01234") + suffix + _T(".jpg"));
	}
}


void ImgBatchModeDlg::OnChangeSuffix()
{
	UpdateExample();
}


void ImgBatchModeDlg::OnSameDir()
{
	UpdateDirs();
}

void ImgBatchModeDlg::OnSelectDir()
{
	UpdateDirs();
}

void ImgBatchModeDlg::UpdateDirs()
{
	if (edit_dest_path_.m_hWnd != 0)
	{
		bool same_dir= !!IsDlgButtonChecked(IDC_SAME_DIR);
		edit_dest_path_.EnableWindow(!same_dir);
		GetDlgItem(IDC_BROWSE)->EnableWindow(!same_dir);
	}
}


void ImgBatchModeDlg::OnBrowse()
{
	CString dest_path;
	edit_dest_path_.GetWindowText(dest_path);

	if (dest_path.IsEmpty())
		dest_path = _T("c:\\");

	CFolderSelect fs(this);
	CString path= fs.DoSelectPath(RString(IDS_SELECT_OUTPUT_FOLDER), dest_path);

	if (path.IsEmpty())
		return;

	edit_dest_path_.SetWindowText(path);
}

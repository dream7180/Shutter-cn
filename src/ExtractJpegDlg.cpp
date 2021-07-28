/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// ExtractJpegDlg.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "ExtractJpegDlg.h"
#include "Path.h"
#include "BalloonMsg.h"
#include "ItemIdList.h"
#include "FolderSelect.h"
#include "CatchAll.h"
#include "RString.h"
#include "PhotoInfo.h"

extern String ReplaceIllegalChars(const String& text);

namespace {
	const TCHAR REGISTRY_ENTRY_DLG[]	= _T("ExtractJpegDlg");
	const TCHAR REG_SUFFIX[]			= _T("Suffix");
	const TCHAR REG_OUTPUT_DIR[]		= _T("OutputDir");
	const TCHAR REG_SEL_DIR[]			= _T("SelectedDir");
	const TCHAR REG_RECENT_DEST_PATH[]	= _T("\\RecentDestPaths");

	const int MAX_RECENT_PATHS= 100;
}

// ExtractJpegDlg dialog

ExtractJpegDlg::ExtractJpegDlg(CWnd* parent /*=NULL*/)
	: DialogChild(ExtractJpegDlg::IDD, parent)
{
	same_dir_ = 0;
	registry_key_ = REGISTRY_ENTRY_DLG;
}


ExtractJpegDlg::~ExtractJpegDlg()
{
}


void ExtractJpegDlg::DoDataExchange(CDataExchange* DX)
{
	DialogChild::DoDataExchange(DX);
	DDX_Control(DX, IDC_SUFFIX, suffix_edit_);
	DDX_Text(DX, IDC_SUFFIX, suffix_);
	DDX_Radio(DX, IDC_SAME_DIR, same_dir_);
	DDX_Text(DX, IDC_DEST_PATH, dest_path_);
	DDX_Text(DX, IDC_SUFFIX, suffix_);
	DDX_Control(DX, IDC_EXAMPLE, example_wnd_);
}


BEGIN_MESSAGE_MAP(ExtractJpegDlg, DialogChild)
	ON_BN_CLICKED(IDC_SAME_DIR, OnSameDir)
	ON_BN_CLICKED(IDC_SELECT, OnSelectDir)
	ON_EN_CHANGE(IDC_SUFFIX, OnChangeSuffix)
	ON_BN_CLICKED(IDC_BROWSE, OnBrowse)
END_MESSAGE_MAP()


// ExtractJpegDlg message handlers

BOOL ExtractJpegDlg::OnInitDialog()
{
	try
	{
		CWinApp* app= AfxGetApp();
		suffix_		= app->GetProfileString(registry_key_, REG_SUFFIX, _T("_extr"));
		dest_path_	= app->GetProfileString(registry_key_, REG_OUTPUT_DIR, _T("c:\\"));
		same_dir_	= app->GetProfileInt(registry_key_, REG_SEL_DIR, same_dir_);

		dest_path_combo_.SubclassDlgItem(IDC_DEST_PATH, this);
		COMBOBOXINFO ci;
		memset(&ci, 0, sizeof ci);
		ci.cbSize = sizeof ci;
		if (dest_path_combo_.GetComboBoxInfo(&ci))
			dest_path_editbox_.SubclassWindow(ci.hwndItem);
		else
			dest_path_editbox_.SubclassWindow(::GetWindow(dest_path_combo_.m_hWnd, GW_CHILD));

		DialogChild::OnInitDialog();

		suffix_edit_.LimitText(100);

		//SubclassHelpBtn(_T("ToolExtractJPEG.htm"));

		// read recent paths
		::RecentPaths(recent_paths_, false, registry_key_ + REG_RECENT_DEST_PATH, MAX_RECENT_PATHS);

		const size_t count= recent_paths_.size();
		for (size_t i= 0; i < count; ++i)
		{
			CString path= recent_paths_[i]->GetPath();
			if (!path.IsEmpty())
				dest_path_combo_.AddString(path);
		}

		UpdateDirs();
		UpdateExample();

		return true;
	}
	CATCH_ALL

	EndDialog(IDCANCEL);
	return true;
}


Path ExtractJpegDlg::GetDestPath() const
{
	return Path(dest_path_);
}


ExtractFormat ExtractJpegDlg::GetParams() const
{
	ExtractFormat p;

	p.preserve_exif_block_ = false;
	p.copy_tags_ = true;

	return p;
}


void ExtractJpegDlg::OnChangeSuffix()
{
	UpdateExample();
}


void ExtractJpegDlg::UpdateExample()
{
	if (suffix_edit_.m_hWnd != 0)
	{
		CString suffix;
		suffix_edit_.GetWindowText(suffix);
		suffix = ReplaceIllegalChars(static_cast<const TCHAR*>(suffix)).c_str();
		example_wnd_.SetWindowText(_T("DSC01234") + suffix + _T(".jpg"));
	}
}


void ExtractJpegDlg::OnSameDir()
{
	UpdateDirs();
}


void ExtractJpegDlg::OnSelectDir()
{
	UpdateDirs();
}


void ExtractJpegDlg::UpdateDirs()
{
	if (dest_path_combo_.m_hWnd != 0)
	{
		bool same_dir= !!IsDlgButtonChecked(IDC_SAME_DIR);
		dest_path_combo_.EnableWindow(!same_dir);
		if (CWnd* btn= GetDlgItem(IDC_BROWSE))
			btn->EnableWindow(!same_dir);
	}
}


void ExtractJpegDlg::OnOK()
{
	try
	{
		if (!Finish())
			return;

		EndDialog(IDOK);
	}
	CATCH_ALL
}


bool ExtractJpegDlg::Finish()
{
	if (!UpdateData()) // || !dlg_options_.UpdateData())
		return false;

	Path path(dest_path_);
	if (!path.empty())
		if (!path.CreateIfDoesntExist(&dest_path_combo_))
			return false;

	extern const TCHAR* PathIllegalChars();

	if (!suffix_.IsEmpty())
		if (suffix_.FindOneOf(PathIllegalChars()) >= 0)
		{
			String msg= _T("后缀文本不能包含以下字符: ");
			msg += PathIllegalChars();
			new BalloonMsg(&suffix_edit_, _T("非法字符"), msg.c_str(), BalloonMsg::IERROR);
			return false;
		}

	//if (same_dir_ == 0 && suffix_.IsEmpty())
	//{
	//	new BalloonMsg(&suffix_edit_, _T("Empty Suffix"),
	//		_T("Please enter suffix text, so the destination file names differ from the source file names."), BalloonMsg::IERROR);
	//	return false;
	//}
	//else
	{
		if (path.empty())
		{
			new BalloonMsg(&dest_path_combo_, _T("目标文件夹缺失"),
				_T("请指定调整尺寸后的图像的储存文件夹."), BalloonMsg::IERROR);
			return false;
		}
	}

	CWinApp* app= AfxGetApp();

	app->WriteProfileInt(registry_key_, REG_SEL_DIR, same_dir_);
	app->WriteProfileString(registry_key_, REG_SUFFIX, suffix_);
	app->WriteProfileString(registry_key_, REG_OUTPUT_DIR, dest_path_);

	if (same_dir_ != 0)
	{
		// if destination path was given, store it in the recently used ones

		// add path to the recent paths
		InsertUniquePath(recent_paths_, ItemIdList(path.c_str()), true);

		// write paths
		RecentPaths(recent_paths_, true, registry_key_ + REG_RECENT_DEST_PATH, MAX_RECENT_PATHS);
	}

	return true;
}


void ExtractJpegDlg::OnBrowse()
{
	CString dest_path;
	dest_path_combo_.GetWindowText(dest_path);

	if (dest_path.IsEmpty())
		dest_path = _T("c:\\");

	CFolderSelect fs(this);
	CString path= fs.DoSelectPath(RString(IDS_SELECT_OUTPUT_FOLDER), dest_path);

	if (path.IsEmpty())
		return;

	dest_path_combo_.SetWindowText(path);
}

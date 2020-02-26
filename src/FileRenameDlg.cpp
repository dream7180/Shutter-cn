/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// FileRenameDlg.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "FileRenameDlg.h"
#include "Dib.h"
#include "CatchAll.h"
#include "ImageDraw.h"
#include "Path.h"


// FileRenameDlg dialog

FileRenameDlg::FileRenameDlg(const String& name, const String& path, const Dib* img, const Callback& fn, CWnd* parent)
	: CDialog(FileRenameDlg::IDD, parent), fileName_(name), filePath_(path), image_(img), action_(fn)
{
	rectImage_.SetRectEmpty();
	name_.InitAutoComplete(true);
	name_.FileNameEditing(true);
}

FileRenameDlg::~FileRenameDlg()
{
}

void FileRenameDlg::DoDataExchange(CDataExchange* DX)
{
	CDialog::DoDataExchange(DX);
	DDX_Control(DX, IDC_NAME, name_);
	DDX_Control(DX, IDC_PATH, path_);
	DDX_Control(DX, IDOK, ok_);
	DDX_Control(DX, IDC_WARNING, warning_icon_);
}


BEGIN_MESSAGE_MAP(FileRenameDlg, CDialog)
	ON_EN_CHANGE(IDC_NAME, OnNameChange)
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()


// FileRenameDlg message handlers

void FileRenameDlg::OnNameChange()
{
	if (ok_.m_hWnd && name_.m_hWnd && warning_icon_.m_hWnd)
	{
		ok_.EnableWindow(name_.GetWindowTextLength() > 0);

		try
		{
			CString str;
			name_.GetWindowText(str);
			bool file_exists= false;
			if (!str.IsEmpty() && str != fileName_.c_str())
			{
				Path path= filePath_;
				path.RenameFileName(str);
				file_exists = path.FileExists();
			}

			warning_icon_.ShowWindow(file_exists ? SW_SHOWNA : SW_HIDE);
		}
		catch (...)
		{}
	}
}


BOOL FileRenameDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	name_.SetWindowText(fileName_.c_str());

	int overhead= static_cast<int>(filePath_.size() - fileName_.size());
	int limit= MAX_PATH - overhead;
	if (limit <= 0 || limit > MAX_PATH)
		limit = MAX_PATH;	//TODO: we are exceeding max path limits...
	name_.LimitText(limit);

	path_.ModifyStyle(0, SS_PATHELLIPSIS);
	path_.SetWindowText(filePath_.c_str());

	if (CWnd* img= GetDlgItem(IDC_IMAGE))
	{
		WINDOWPLACEMENT wp;
		if (img->GetWindowPlacement(&wp))
			rectImage_ = wp.rcNormalPosition;
		img->DestroyWindow();
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void FileRenameDlg::OnOK()
{
	try
	{
		CString str;
		name_.GetWindowText(str);

		String name(str);

		if (name.empty())
			return;

		// rename action; can throw
		action_(name);

		CDialog::OnOK();
	}
	CATCH_ALL
}


BOOL FileRenameDlg::OnEraseBkgnd(CDC* dc)
{
	CRect rect(0,0,0,0);
	GetClientRect(rect);

	COLORREF bkColor= ::GetSysColor(COLOR_3DFACE);

	dc->FillSolidRect(rect, bkColor);

	if (image_ && !rectImage_.IsRectEmpty())
	{
		CRect rect= rectImage_;

		ImageDraw::Draw(image_, dc, rect, bkColor, 0, 0, 0, 0,
			ImageDraw::DRAW_BACKGND | ImageDraw::DRAW_HALFTONE | ImageDraw::DRAW_SHADOW, 0);
	}

	return true;
}

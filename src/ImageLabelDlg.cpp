/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// ImageLabelDlg.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "ImageLabelDlg.h"
#include "LoadImageList.h"


// ImageLabelDlg configuration dialog for the main view

ImageLabelDlg::ImageLabelDlg(const std::vector<uint16>& thumbs, const std::vector<uint16>& previews, CWnd* parent, Columns& columns)
	: CDialog(ImageLabelDlg::IDD, parent), thumbs_(thumbs), previews_(previews), tree_1_(columns), tree_2_(columns)
{
}

ImageLabelDlg::~ImageLabelDlg()
{
}

void ImageLabelDlg::DoDataExchange(CDataExchange* DX)
{
	CDialog::DoDataExchange(DX);
	DDX_Control(DX, IDC_TAB, tab_ctrl_);
//	DDX_Control(DX, IDC_LIST, list_);
	tree_1_.TreeDoDataExchange(DX, IDC_FIELDS_1);
	tree_2_.TreeDoDataExchange(DX, IDC_FIELDS_2);
}


BEGIN_MESSAGE_MAP(ImageLabelDlg, CDialog)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB, OnTabChanged)
END_MESSAGE_MAP()


// ImageLabelDlg message handlers

BOOL ImageLabelDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	tree_1_.columns_ = &thumbs_;
	tree_2_.columns_ = &previews_;

	tree_1_.InitTree();
	tree_2_.InitTree();

	VERIFY(::LoadImageList(imageList_, IDB_PHOTOS_TB, 18, ::GetSysColor(COLOR_3DFACE)));
	tab_ctrl_.SetImageList(&imageList_);

	tab_ctrl_.InsertItem(0, _T("缩略图模式"), 4);
	tab_ctrl_.InsertItem(1, _T("预览模式"), 7);

	if (m_pParentWnd)
		CenterWindow(m_pParentWnd);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void ImageLabelDlg::OnTabChanged(NMHDR* nmhdr, LRESULT* result)
{
	if (tree_1_.tree_wnd_.m_hWnd && tree_2_.tree_wnd_.m_hWnd && tab_ctrl_.m_hWnd)
	{
		tree_1_.tree_wnd_.ShowWindow(SW_HIDE);
		tree_2_.tree_wnd_.ShowWindow(SW_HIDE);

		if (tab_ctrl_.GetCurSel() == 0)
			tree_1_.tree_wnd_.ShowWindow(SW_SHOWNA);
		else
			tree_2_.tree_wnd_.ShowWindow(SW_SHOWNA);
	}

	// TODO: Add your control notification handler code here
	*result = 0;
}


void ImageLabelDlg::GetSelection(int index, std::vector<uint16>& fields)
{
	if (index == 0)
		fields = tree_1_.selected_;
	else if (index == 1)
		fields = tree_2_.selected_;
	else
	{ ASSERT(false); }
}

/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// ResizeOptionsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "ResizeOptionsDlg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


// CResizeOptionsDlg dialog

CResizeOptionsDlg::CResizeOptionsDlg(CWnd* parent /*=NULL*/)
	: CDialog(CResizeOptionsDlg::IDD, parent)
{
	resizing_method_ = 2;
	preserve_exif_block_ = true;
	baseline_jpeg_ = false;
	progressive_jpeg_ = false;
	copyTags_ = false;
}


CResizeOptionsDlg::~CResizeOptionsDlg()
{
}


void CResizeOptionsDlg::DoDataExchange(CDataExchange* DX)
{
	CDialog::DoDataExchange(DX);
	DDX_Control(DX, IDC_METHOD, resizing_method_wnd_);
	DDX_CBIndex(DX, IDC_METHOD, resizing_method_);
	DDX_Check(DX, IDC_BASELINE, baseline_jpeg_);
	DDX_Check(DX, IDC_PROGRESSIVE, progressive_jpeg_);
	DDX_Check(DX, IDC_EXIF, preserve_exif_block_);
	DDX_Check(DX, IDC_IPTC, copyTags_);
}


BEGIN_MESSAGE_MAP(CResizeOptionsDlg, CDialog)
END_MESSAGE_MAP()


// CResizeOptionsDlg message handlers

bool CResizeOptionsDlg::Create(CWnd* parent, int dlg_id)
{
	return !!CDialog::Create(dlg_id, parent);
}

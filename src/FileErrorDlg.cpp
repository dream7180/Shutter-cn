/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// FileErrorDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FileErrorDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// FileErrorDlg dialog


FileErrorDlg::FileErrorDlg(CWnd* parent /*=NULL*/)
	: CDialog(FileErrorDlg::IDD, parent)
{
	//{{AFX_DATA_INIT(FileErrorDlg)
	backup_ = _T("");
	original_ = _T("");
	//}}AFX_DATA_INIT
}


void FileErrorDlg::DoDataExchange(CDataExchange* DX)
{
	CDialog::DoDataExchange(DX);
	//{{AFX_DATA_MAP(FileErrorDlg)
	DDX_Text(DX, IDC_BACKUP, backup_);
	DDX_Text(DX, IDC_ORIGINAL, original_);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(FileErrorDlg, CDialog)
	//{{AFX_MSG_MAP(FileErrorDlg)
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// FileErrorDlg message handlers

BOOL FileErrorDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	title_fnt_.CreateFont(-18, 0, 0, 0, FW_BOLD, false, false, false, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH, _T("Tahoma"));
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


BOOL FileErrorDlg::OnEraseBkgnd(CDC* dc)
{
//	return CDialog::OnEraseBkgnd(dc);

	CRect rect;
	GetClientRect(rect);

	const int HEADER= 52;
	const COLORREF rgbWHITE= RGB(255,255,255);
	const COLORREF rgbRED= RGB(255,0,0);
	const TCHAR* error= _T("      Error Modifying File");

	CRect header_rect= rect;
	header_rect.bottom = rect.top + HEADER;

	dc->FillSolidRect(header_rect, rgbRED);

	CFont* old= dc->SelectObject(&title_fnt_);

	dc->SetTextColor(rgbWHITE);
	dc->SetBkColor(rgbRED);
	dc->DrawText(error, static_cast<int>(_tcslen(error)), header_rect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

	dc->SelectObject(old);

	dc->FillSolidRect(rect.left, rect.top + HEADER, rect.Width(), 1, ::GetSysColor(COLOR_3DDKSHADOW));
	dc->FillSolidRect(rect.left, rect.top + HEADER + 1, rect.Width(), rect.Height() - HEADER - 1, ::GetSysColor(COLOR_3DFACE));

	return true;
}

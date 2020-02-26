/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// OptionsDescription.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "OptionsDescription.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptionsDescription property page

//IMPLEMENT_DYNCREATE(COptionsDescription, CPropertyPage)

COptionsDescription::COptionsDescription() : RPropertyPage(COptionsDescription::IDD)
{
	//{{AFX_DATA_INIT(COptionsDescription)
	//}}AFX_DATA_INIT
	rgb_text_ = RGB(0,0,0);
}

COptionsDescription::~COptionsDescription()
{}

void COptionsDescription::DoDataExchange(CDataExchange* DX)
{
	CPropertyPage::DoDataExchange(DX);
	//{{AFX_DATA_MAP(COptionsDescription)
	DDX_Control(DX, IDC_EXAMPLE, example_wnd_);
	DDX_Control(DX, IDC_FONT, font_name_wnd_);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COptionsDescription, RPropertyPage)
	//{{AFX_MSG_MAP(COptionsDescription)
	ON_BN_CLICKED(IDC_SELECT, OnSelectFont)
	ON_WM_CTLCOLOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptionsDescription message handlers

void COptionsDescription::OnSelectFont()
{
	CFontDialog dlg;

	dlg.m_cf.lpLogFont = &lf_desc_;
	dlg.m_cf.rgbColors = rgb_text_;
	dlg.m_cf.Flags |= CF_EFFECTS | CF_INITTOLOGFONTSTRUCT;

	if (::GetVersion() < DWORD(0x80000000))
		dlg.m_cf.Flags |= CF_NOSCRIPTSEL;	// WinNT doesn't need this

	if (dlg.DoModal() != IDOK)
		return;

	rgb_text_ = dlg.m_cf.rgbColors;

	fnd_desc_.DeleteObject();
	fnd_desc_.CreateFontIndirect(&lf_desc_);
	example_wnd_.Invalidate();
	font_name_wnd_.SetWindowText(lf_desc_.lfFaceName);
}


HBRUSH COptionsDescription::OnCtlColor(CDC* dc, CWnd* wnd, UINT ctl_color)
{
	HBRUSH hbr= CPropertyPage::OnCtlColor(dc, wnd, ctl_color);

	if (wnd && wnd->GetDlgCtrlID() == IDC_EXAMPLE)
	{
		COLORREF rgb_back= RGB(0,0,0);
		br_back_.DeleteObject();
		br_back_.CreateSolidBrush(rgb_back);
		hbr = br_back_;
		dc->SetBkColor(rgb_back);
		dc->SetTextColor(rgb_text_);
		dc->SelectObject(&fnd_desc_);
	}

	return hbr;
}

BOOL COptionsDescription::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

//	lf_desc_ = g_Settings.lf_description_;
	fnd_desc_.CreateFontIndirect(&lf_desc_);
	font_name_wnd_.SetWindowText(lf_desc_.lfFaceName);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

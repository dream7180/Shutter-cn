/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// OptionsViewer.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "OptionsViewer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptionsViewer property page

//IMPLEMENT_DYNCREATE(COptionsViewer, CPropertyPage)

COptionsViewer::COptionsViewer() : RPropertyPage(COptionsViewer::IDD)
{
//	psp_.flags |= PSP_HASHELP;
	//{{AFX_DATA_INIT(COptionsViewer)
	gamma_ = 0.0;
	keep_sel_centered_ = FALSE;
	preload_ = FALSE;
	//}}AFX_DATA_INIT
	rgb_selection_ = 0;
}

COptionsViewer::~COptionsViewer()
{
}

void COptionsViewer::DoDataExchange(CDataExchange* DX)
{
	CPropertyPage::DoDataExchange(DX);
	//{{AFX_DATA_MAP(COptionsViewer)
	DDX_Control(DX, IDC_GAMMA, edit_gamma_);
	DDX_Control(DX, IDC_STATUS_BAR, status_wnd_);
//	DDX_Control(DX, IDC_SEL_COLOR, btn_color_);
	DDX_Text(DX, IDC_GAMMA, gamma_);
	DDV_MinMaxDouble(DX, gamma_, 0.1, 10.);
//	DDX_Check(DX, IDC_KEEP_SEL_CENTERED, keep_sel_centered_);
	DDX_Check(DX, IDC_PRELOAD, preload_);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COptionsViewer, CPropertyPage)
	//{{AFX_MSG_MAP(COptionsViewer)
	ON_NOTIFY(UDN_DELTAPOS, IDC_GAMMA_SPIN, OnDeltaPosGammaSpin)
	ON_BN_CLICKED(IDC_SEL_COLOR, OnSelColor)
	ON_EN_CHANGE(IDC_GAMMA, OnChangeGamma)
	//}}AFX_MSG_MAP
	ON_NOTIFY(NM_CLICK, IDC_STATUS_BAR, OnStausClick)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptionsViewer message handlers

void COptionsViewer::OnStausClick(NMHDR* nmhdr, LRESULT* result)
{
	NMMOUSE* mouse= reinterpret_cast<NMMOUSE*>(nmhdr);
	*result = 0;
}


void COptionsViewer::OnDeltaPosGammaSpin(NMHDR* nmhdr, LRESULT* result) 
{
	NM_UPDOWN* NM_up_down = (NM_UPDOWN*)nmhdr;

	CString temp;
	GetDlgItemText(IDC_GAMMA, temp);
	double val= _tcstod(temp, 0);

	if (NM_up_down->iDelta > 0)
		val -= 0.1;
	else
		val += 0.1;

	if (val > 10.0)
		val = 10.0;
	else if (val < 0.1)
		val = 0.1;

	temp.Format(_T("%.1f"), val);
	SetDlgItemText(IDC_GAMMA, temp);

	*result = 0;
}


BOOL COptionsViewer::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

//	btn_color_.SetColor(rgb_selection_);

	if (CWnd* wnd= GetDlgItem(IDC_GRAYSCALE))
	{
		WINDOWPLACEMENT wp;
		wnd->GetWindowPlacement(&wp);
		wnd->DestroyWindow();

		grayscale_wnd_.Create(this, wp.rcNormalPosition);
		grayscale_wnd_.SetGamma(gamma_);
	}


	int parts[]= { 50, 100, 150, 200, 250, -1 };
	status_wnd_.SetParts(array_count(parts), parts);
//	HICON down= AfxGetApp()->LoadIcon(IDI_DOWN);
//	for (int i= 0; i < array_count(parts); ++i)
//		status_wnd_.SetIcon(i, down);

	status_wnd_.SetText(_T("Blah blah"), 0, 0);
	status_wnd_.SetText(_T("Blah blah"), 1, 0);


	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void COptionsViewer::OnSelColor()
{
/*
	CColorDialog dlg(rgb_selection_, CC_FULLOPEN, this);
	if (dlg.DoModal() == IDOK && rgb_selection_ != dlg.GetColor())
	{
		rgb_selection_ = dlg.GetColor();
		btn_color_.SetColor(rgb_selection_);
		btn_color_.Invalidate(false);
	}
*/
}


void COptionsViewer::OnChangeGamma()
{
	if (edit_gamma_.m_hWnd == 0 || grayscale_wnd_.m_hWnd == 0)
		return;

	CString gamma;
	edit_gamma_.GetWindowText(gamma);
	TCHAR* end= 0;
	double gamma= _tcstod(gamma, &end);

	if (gamma >= 0.1 && gamma <= 10.0)
		grayscale_wnd_.SetGamma(gamma);
	else
		grayscale_wnd_.Clear();
}

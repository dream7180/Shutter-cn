/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// CalibrationDlg.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "CalibrationDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CalibrationDlg dialog


CalibrationDlg::CalibrationDlg(CWnd* parent /*=NULL*/)
	: CDialog(CalibrationDlg::IDD, parent)
{
	//{{AFX_DATA_INIT(CalibrationDlg)
	X_res_ = 90.0;
	Y_res_ = 90.0;
	//}}AFX_DATA_INIT
	header_bottom_ = 0;
	min_wnd_size_size_ = CSize(0, 0);
}


void CalibrationDlg::DoDataExchange(CDataExchange* DX)
{
	CDialog::DoDataExchange(DX);
	//{{AFX_DATA_MAP(CalibrationDlg)
	DDX_Control(DX, IDC_Y_RES, editY_res_);
	DDX_Control(DX, IDC_X_RES, editX_res_);
	DDX_Text(DX, IDC_X_RES, X_res_);
	DDX_Text(DX, IDC_Y_RES, Y_res_);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CalibrationDlg, CDialog)
	//{{AFX_MSG_MAP(CalibrationDlg)
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CalibrationDlg message handlers

BOOL CalibrationDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetIcon(AfxGetApp()->LoadIcon(IDI_MONITOR), false);

	CRect rect;
	GetWindowRect(rect);
	min_wnd_size_size_ = rect.Size();

	if (CWnd* ok= GetDlgItem(IDCANCEL))
	{
		WINDOWPLACEMENT wp;
		ok->GetWindowPlacement(&wp);
		header_bottom_ = wp.rcNormalPosition.bottom + 6;
	}

	circle_wnd_.Create(this);
	CSize size= circle_wnd_.CalcSize(X_res_, Y_res_);

	rect.SetRect(0, 0, size.cx, size.cy + header_bottom_);
	AdjustWindowRect(rect, GetStyle(), false);
	SetWindowPos(0, 0, 0, rect.Width(), rect.Height(), SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


BOOL CalibrationDlg::OnEraseBkgnd(CDC* dc)
{
	CRect rect;
	GetClientRect(rect);

	dc->FillSolidRect(rect.left, rect.top, rect.Width(), header_bottom_ - 1, ::GetSysColor(COLOR_3DFACE));
	dc->FillSolidRect(rect.left, header_bottom_ - 1, rect.Width(), 1, ::GetSysColor(COLOR_3DSHADOW));
	dc->FillSolidRect(rect.left, header_bottom_, rect.Width(), rect.Height() - header_bottom_, RGB(255,255,255));

	return true;
}


void CalibrationDlg::OnSize(UINT type, int cx, int cy)
{
//	CDialog::OnSize(type, cx, cy);

	Resize();
}


void CalibrationDlg::Resize()
{
	if (circle_wnd_.m_hWnd)
	{
		CRect rect;
		GetClientRect(rect);
		rect.top = header_bottom_;
		circle_wnd_.MoveWindow(rect);

		double x= 0.0, y= 0.0;
		circle_wnd_.GetResolution(x, y);

		TCHAR buf[64];
		swprintf_s(buf, _T("%.1f"), x);
		editX_res_.SetWindowText(buf);

		swprintf_s(buf, _T("%.1f"), y);
		editY_res_.SetWindowText(buf);
	}
}


void CalibrationDlg::OnGetMinMaxInfo(MINMAXINFO* MMI)
{
	CDialog::OnGetMinMaxInfo(MMI);

	MMI->ptMinTrackSize.x = min_wnd_size_size_.cx;
	MMI->ptMinTrackSize.y = min_wnd_size_size_.cy;
}

/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// InfoPaneBar.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "InfoPaneBar.h"
#include "CatchAll.h"
#include "Config.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// InfoPaneBar dialog

InfoPaneBar::InfoPaneBar() : CDialog(InfoPaneBar::IDD, 0)
{
}

InfoPaneBar::~InfoPaneBar()
{
}

void InfoPaneBar::DoDataExchange(CDataExchange* DX)
{
	CDialog::DoDataExchange(DX);
}


BEGIN_MESSAGE_MAP(InfoPaneBar, CDialog)
	ON_EN_CHANGE(IDC_FILTER, OnChangeFilter)
	ON_BN_CLICKED(IDC_HIDE, OnClickedHide)
	ON_COMMAND(ID_CANCEL, OnCancelQuickFilter)
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


bool InfoPaneBar::Create(CWnd* parent, const boost::function<void (const CString& filter, bool hide)>& filter, bool hideUnknown)
{
	if (!CDialog::Create(IDD, parent))
		return false;


	filter_bar_.SetParams(IDB_FIND_TOOLBAR, 0, ID_CANCEL, EditCombo::NONE);
	filter_bar_.SetImage(IDB_FUNNEL_SMALL);
	filter_bar_.SubclassDlgItem(IDC_FILTER, this);
	filter_bar_.SetState(1);

	filter_ = filter;

	//close_btn_.SubclassDlgItem(IDC_TOOLBAR, this);
	//close_btn_.SetOnIdleUpdateState(false);

	//const int command= ID_CANCEL;
	//close_btn_.SetPadding(6, 6);
	//if (!close_btn_.AddButtons("p", &command, IDB_INFO_BAR))
	//	return false;

	CheckDlgButton(IDC_HIDE, hideUnknown ? 1 : 0);

	OnChangeFilter();

	return true;
}


void InfoPaneBar::OnChangeFilter()
{
	try
	{
		CString str= FilterText();
		filter_bar_.EnableButton(!str.IsEmpty(), 0);
		filter_(str, IsDlgButtonChecked(IDC_HIDE) != 0);
	}
	CATCH_ALL
}


void InfoPaneBar::OnClickedHide()
{
	OnChangeFilter();
}


void InfoPaneBar::OnCancelQuickFilter()
{
	filter_bar_.GetEditCtrl().SetWindowText(_T(""));
}


bool InfoPaneBar::IsUnknownHidden()
{
	return IsDlgButtonChecked(IDC_HIDE) != 0;
}


CString InfoPaneBar::FilterText()
{
	CString str;
	filter_bar_.GetEditCtrl().GetWindowText(str);
	return str;
}


void InfoPaneBar::OnOK()
{}


void InfoPaneBar::OnCancel()
{
	OnCancelQuickFilter();
}


BOOL InfoPaneBar::OnEraseBkgnd(CDC* dc)
{
	COLORREF background = g_Settings.AppColors()[AppColors::Background];
//	MemoryDC mem_dc(*dc, this, rgb_background);
//	mem_dc.BitBlt();
	CRect rect;
	GetClientRect(rect);
	dc->FillSolidRect(rect, background);

	return true;
}

HBRUSH InfoPaneBar::OnCtlColor(CDC* dc, CWnd* wnd, UINT ctl_color)
{
	HBRUSH hbr = CDialog::OnCtlColor(dc, wnd, ctl_color);

	COLORREF background = g_Settings.AppColors()[AppColors::Background];
	COLORREF text = g_Settings.AppColors()[AppColors::Text];

//	if (wnd && wnd->GetDlgCtrlID() == IDC_EXAMPLE)
	{
		background_brush_.DeleteObject();
		background_brush_.CreateSolidBrush(background);
		hbr = background_brush_;
		dc->SetBkColor(background);
		dc->SetTextColor(text);
//		dc->SelectObject(&fnd_desc_);
	}

	return hbr;
}

/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// PrnThumbsOptionsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "PrnThumbsOptionsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// PrnThumbsOptionsDlg dialog


PrnThumbsOptionsDlg::PrnThumbsOptionsDlg(CWnd* parent /*=NULL*/)
	: CDialog(PrnThumbsOptionsDlg::IDD, parent)
{
	//{{AFX_DATA_INIT(PrnThumbsOptionsDlg)
	print_footer_ = true;
	print_footer_text_ = true;
	items_across_ = 6;
	//}}AFX_DATA_INIT
	ready_ = false;
	print_option_ = 0;
}


void PrnThumbsOptionsDlg::DoDataExchange(CDataExchange* DX)
{
	CDialog::DoDataExchange(DX);
	//{{AFX_DATA_MAP(PrnThumbsOptionsDlg)
	DDX_Control(DX, IDC_SPIN, spin_wnd_);
	DDX_Control(DX, IDC_ITEMS, edit_items_);
	DDX_Control(DX, IDC_SLIDER, slider_wnd_);
	DDX_Check(DX, IDC_FOOTER, print_footer_);
	DDX_Check(DX, IDC_FOOTER_T, print_footer_text_);
	DDX_Slider(DX, IDC_SLIDER, items_across_);
	//}}AFX_DATA_MAP
	DDX_Control(DX, IDC_FOOTER_TEXT, edit_footer_text_);
	DDX_Text(DX, IDC_FOOTER_TEXT, footer_text_);
}


BEGIN_MESSAGE_MAP(PrnThumbsOptionsDlg, CDialog)
	//{{AFX_MSG_MAP(PrnThumbsOptionsDlg)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_FOOTER, OnFooter)
	ON_BN_CLICKED(IDC_FOOTER_T, OnFooter)
	ON_EN_CHANGE(IDC_ITEMS, OnChangeItems)
	ON_EN_CHANGE(IDC_FOOTER_TEXT, OnFooter)
	ON_BN_CLICKED(IDC_FONT, OnFont)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_PRINT_OPT_1, OnPrintOptions)
	ON_BN_CLICKED(IDC_PRINT_OPT_2, OnPrintOptions)
	ON_BN_CLICKED(IDC_PRINT_OPT_3, OnPrintOptions)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// PrnThumbsOptionsDlg message handlers

BOOL PrnThumbsOptionsDlg::OnInitDialog()
{
	if (slider_wnd_.SubclassDlgItem(IDC_SLIDER, this))
	{
		slider_wnd_.SendMessage(TBM_SETRANGE, 1, MAKELONG(MIN_ITEMS, MAX_ITEMS));
		slider_wnd_.SetPos(items_across_);
	}

	CDialog::OnInitDialog();

	edit_items_.LimitText(2);
	SetDlgItemInt(IDC_ITEMS, items_across_);
	spin_wnd_.SetRange(MIN_ITEMS, MAX_ITEMS);

	edit_footer_text_.LimitText(500);

	SetFontInfo(font_);

	CheckRadioButton(IDC_PRINT_OPT_1, IDC_PRINT_OPT_3, IDC_PRINT_OPT_1 + print_option_);

	ready_ = true;

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


bool PrnThumbsOptionsDlg::Create(CWnd* parent, CWnd& placeholder_wnd)
{
	m_pParentWnd = parent;

	if (!CDialog::Create(IDD, parent))
		return false;

	SetDlgCtrlID(IDD);

	WINDOWPLACEMENT wp;
	if (placeholder_wnd.GetWindowPlacement(&wp))
	{
		CRect rect= wp.rcNormalPosition;
		SetWindowPos(&placeholder_wnd, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOACTIVATE);
		return true;
	}

	return false;
}


void PrnThumbsOptionsDlg::OnHScroll(UINT sb_code, UINT pos, CScrollBar* scroll_bar)
{
	if (slider_wnd_.m_hWnd)
	{
		int items= slider_wnd_.GetPos();
		SetDlgItemInt(IDC_ITEMS, items);
		ItemsNumberChanges(items);
	}
}


void PrnThumbsOptionsDlg::OnFooter()
{
	if (CWnd* parent= GetParent())
		if (const MSG* msg= GetCurrentMessage())
			parent->SendMessage(msg->message, msg->wParam, msg->lParam);
}


void PrnThumbsOptionsDlg::OnChangeItems()
{
	if (!ready_)
		return;	// not yet initialized, do not overwrite 'items_across_'

	int items= GetDlgItemInt(IDC_ITEMS);
	if (items < MIN_ITEMS)
		items = MIN_ITEMS;
	else if (items > MAX_ITEMS)
		items = MAX_ITEMS;

	if (slider_wnd_.m_hWnd)
		slider_wnd_.SetPos(items);

	ItemsNumberChanges(items);
}


void PrnThumbsOptionsDlg::ItemsNumberChanges(int items)
{
	items_across_ = items;

	if (CWnd* parent= GetParent())
		parent->SendMessage(WM_USER, items);
}


void PrnThumbsOptionsDlg::SetFontInfo(const LOGFONT& lf)
{
	CDC dc;
	dc.CreateIC(_T("DISPLAY"), 0, 0, 0);
	int log_inch= dc.GetDeviceCaps(LOGPIXELSY);
	if (log_inch < 1)
		log_inch = 72;
	oStringstream ost;
	ost << (abs(lf.lfHeight) * 72 + 72/2) / log_inch << _T(" pt, ") << lf.lfFaceName;
	SetDlgItemText(IDC_FONT_INFO, ost.str().c_str());
}


void PrnThumbsOptionsDlg::OnFont()
{
	CFontDialog dlg(&font_, CF_BOTH | CF_SCALABLEONLY | CF_INITTOLOGFONTSTRUCT);
	if (dlg.DoModal() != IDOK)
		return;

	dlg.GetCurrentFont(&font_);

	SetFontInfo(font_);

	if (CWnd* parent= GetParent())
		parent->SendMessage(WM_USER + 2);
}


void PrnThumbsOptionsDlg::OnPrintOptions()
{
	int opt= 0;
	if (IsDlgButtonChecked(IDC_PRINT_OPT_1))
		opt = 0;
	else if (IsDlgButtonChecked(IDC_PRINT_OPT_2))
		opt = 1;
	else
		opt = 2;

	print_option_ = opt;

	if (CWnd* parent= GetParent())
		parent->SendMessage(WM_USER + 3, opt);
}

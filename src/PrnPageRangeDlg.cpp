/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// PrnPageRangeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "PrnPageRangeDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// PrnPageRangeDlg dialog


PrnPageRangeDlg::PrnPageRangeDlg(CWnd* parent /*=NULL*/)
	: CDialog(PrnPageRangeDlg::IDD, parent)
{
	//{{AFX_DATA_INIT(PrnPageRangeDlg)
	page_range_ = 0;
	page_range_str_ = _T("");
	//}}AFX_DATA_INIT
}


void PrnPageRangeDlg::DoDataExchange(CDataExchange* DX)
{
	CDialog::DoDataExchange(DX);
	//{{AFX_DATA_MAP(PrnPageRangeDlg)
	DDX_Radio(DX, IDC_ALL_PAGES, page_range_);
	DDX_Control(DX, IDC_PAGES_BOX, edit_page_range_);
	DDX_Text(DX, IDC_PAGES_BOX, page_range_str_);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(PrnPageRangeDlg, CDialog)
	//{{AFX_MSG_MAP(PrnPageRangeDlg)
	ON_BN_CLICKED(IDC_SELECTED_PAGES, OnSelPagesClicked)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_CUR_PAGE, DisablePagesEditBox)
	ON_BN_CLICKED(IDC_ALL_PAGES, DisablePagesEditBox)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// PrnPageRangeDlg message handlers

BOOL PrnPageRangeDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	edit_page_range_.EnableWindow(page_range_ == 2);
	edit_page_range_.LimitText(32000);	// some reasonable limit

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


bool PrnPageRangeDlg::Create(CWnd* parent, int placeholder_id)
{
	m_pParentWnd = parent;

	if (!CDialog::Create(IDD, parent))
		return false;

	SetDlgCtrlID(IDD);

	if (CWnd* place= parent->GetDlgItem(placeholder_id))
	{
		WINDOWPLACEMENT wp;
		if (place->GetWindowPlacement(&wp))
		{
			CRect rect= wp.rcNormalPosition;
			SetWindowPos(place, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOACTIVATE);

			place->DestroyWindow();

			return true;
		}
	}

	return false;
}


void PrnPageRangeDlg::OnSelPagesClicked()
{
	// activate edit box instead
	if (edit_page_range_.m_hWnd)
	{
		edit_page_range_.EnableWindow();
		GotoDlgCtrl(&edit_page_range_);
	}
}


void PrnPageRangeDlg::DisablePagesEditBox()
{
	if (edit_page_range_.m_hWnd)
		edit_page_range_.EnableWindow(false);
}


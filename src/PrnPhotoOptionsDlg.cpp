/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// PrnPhotoOptionsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "PrnPhotoOptionsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// PrnPhotoOptionsDlg dialog


PrnPhotoOptionsDlg::PrnPhotoOptionsDlg(CWnd* parent /*=NULL*/)
	: CDialog(PrnPhotoOptionsDlg::IDD, parent)
{
	//{{AFX_DATA_INIT(PrnPhotoOptionsDlg)
	copies_ = 1;
	zoom_ = 100;
	//}}AFX_DATA_INIT
}


void PrnPhotoOptionsDlg::DoDataExchange(CDataExchange* DX)
{
	CDialog::DoDataExchange(DX);
	//{{AFX_DATA_MAP(PrnPhotoOptionsDlg)
	DDX_Control(DX, IDC_COPIES, edit_copies_);
	DDX_Control(DX, IDC_ZOOM, edit_zoom_);
	DDX_Control(DX, IDC_SPIN, spin_wnd_);
	DDX_Control(DX, IDC_SPIN2, spin_wnd_2_);
	DDX_Text(DX, IDC_COPIES, copies_);
	DDX_Text(DX, IDC_ZOOM, zoom_);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(PrnPhotoOptionsDlg, CDialog)
	//{{AFX_MSG_MAP(PrnPhotoOptionsDlg)
	ON_EN_CHANGE(IDC_COPIES, OnChangeCopies)
	ON_EN_CHANGE(IDC_ZOOM, OnChangeZoom)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// PrnPhotoOptionsDlg message handlers

BOOL PrnPhotoOptionsDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	edit_copies_.LimitText(2);
	spin_wnd_.SetRange(MIN_COPIES, MAX_COPIES);
	edit_zoom_.LimitText(3);
	spin_wnd_2_.SetRange(MIN_ZOOM, MAX_ZOOM);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void PrnPhotoOptionsDlg::OnChangeCopies()
{
	if (edit_copies_.m_hWnd == 0)
		return;	// not yet initialized, do not overwrite 'copies_'

	int copies= GetDlgItemInt(IDC_COPIES);
	if (copies < MIN_COPIES)
		copies = MIN_COPIES;
	else if (copies > MAX_COPIES)
		copies = MAX_COPIES;

//	if (slider_wnd_.m_hWnd)
//		slider_wnd_.SetPos(items);

	copies_ = copies;

	if (CWnd* parent= GetParent())
		parent->SendMessage(WM_USER + 1, copies);
}


void PrnPhotoOptionsDlg::OnChangeZoom()
{
	if (edit_zoom_.m_hWnd == 0)
		return;

	int zoom= GetDlgItemInt(IDC_ZOOM);
	if (zoom < MIN_ZOOM)
		zoom = MIN_ZOOM;
	else if (zoom > MAX_ZOOM)
		zoom = MAX_ZOOM;

	zoom_ = zoom;

	if (CWnd* parent= GetParent())
		parent->SendMessage(WM_USER + 4, zoom);
}


bool PrnPhotoOptionsDlg::Create(CWnd* parent, CWnd& placeholder_wnd)
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


void PrnPhotoOptionsDlg::SetZoom(int zoom)
{
	zoom_ = zoom;
	SetDlgItemInt(IDC_ZOOM, zoom);
}

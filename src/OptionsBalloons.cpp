/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// OptionsBalloons.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "OptionsBalloons.h"
#include "Columns.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// OptionsBalloons property page

//IMPLEMENT_DYNCREATE(OptionsBalloons, CPropertyPage)

OptionsBalloons::OptionsBalloons(Columns& columns) : RPropertyPage(OptionsBalloons::IDD), ColumnTree(columns)
{
	//{{AFX_DATA_INIT(OptionsBalloons)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

OptionsBalloons::~OptionsBalloons()
{
}


void OptionsBalloons::DoDataExchange(CDataExchange* DX)
{
	CPropertyPage::DoDataExchange(DX);
	//{{AFX_DATA_MAP(OptionsBalloons)
	DDX_Control(DX, IDC_BALLOON, shape_wnd_);
	//}}AFX_DATA_MAP

	TreeDoDataExchange(DX, IDC_FIELDS);
}


BEGIN_MESSAGE_MAP(OptionsBalloons, RPropertyPage)
	//{{AFX_MSG_MAP(OptionsBalloons)
	ON_WM_ERASEBKGND()
	ON_BN_CLICKED(IDC_RESET, OnReset)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// OptionsBalloons message handlers

namespace
{
	const COLORREF BTEXT= RGB(0,0,0);
	const COLORREF BALLOON= RGB(255,255,225);
}

BOOL OptionsBalloons::OnEraseBkgnd(CDC* dc)
{
	CPropertyPage::OnEraseBkgnd(dc);

	if (shape_wnd_.m_hWnd)
	{
		WINDOWPLACEMENT wp;
		shape_wnd_.GetWindowPlacement(&wp);
		CRect rect= wp.rcNormalPosition;

		int diameter= 16;// rect.Width() / 18;	// round corner
		int n= diameter * 12 / 10; // balloon's tip
		rect.bottom -= n;

		CBrush br(BALLOON);
		CBrush* old= dc->SelectObject(&br);
		dc->SelectStockObject(BLACK_PEN);
		dc->RoundRect(rect, CPoint(diameter, diameter));
		int y= rect.bottom - 1;
		int x= rect.left + n * 12 / 10;
		CPoint points[]=
		{
			CPoint(x, y),
			CPoint(x, y + n),
			CPoint(x + n, y),
			CPoint(x, y)
		};
		dc->Polygon(points, array_count(points));
		dc->FillSolidRect(x + 1, y, n - 1, 1, BALLOON);
		dc->SelectObject(old);
	}

	return true;
}


BOOL OptionsBalloons::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	InitTree();
	tree_wnd_.SetBkColor(BALLOON);
	tree_wnd_.SetTextColor(BTEXT);

	WINDOWPLACEMENT wp;
	if (CWnd* wnd= GetDlgItem(IDC_RESET))
	{
		wnd->GetWindowPlacement(&wp);
		CRect rect= wp.rcNormalPosition;
		CString text;
		wnd->GetWindowText(text);
		reset_wnd_.Create(this, rect.TopLeft(), text, _T(""), 0, IDC_RESET);
		wnd->DestroyWindow();
	}

	ResizeMgr().BuildMap(this);
	ResizeMgr().SetWndResizing(IDC_BALLOON, DlgAutoResize::RESIZE);
	ResizeMgr().SetWndResizing(IDC_FIELDS, DlgAutoResize::RESIZE);
	ResizeMgr().SetWndResizing(IDC_RESET, DlgAutoResize::MOVE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void OptionsBalloons::OnReset()
{
	Reset(0);
}


void OptionsBalloons::OnSize(UINT type, int cx, int cy)
{
	RPropertyPage::OnSize(type, cx, cy);

	if (type != SIZE_MINIMIZED)
		InvalidateRect(0);
}

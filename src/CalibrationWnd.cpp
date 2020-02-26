/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// CalibrationWnd.cpp : implementation file
//

#include "stdafx.h"
#include "CalibrationWnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CalibrationWnd

CalibrationWnd::CalibrationWnd()
{
}

CalibrationWnd::~CalibrationWnd()
{
}


BEGIN_MESSAGE_MAP(CalibrationWnd, CWnd)
	//{{AFX_MSG_MAP(CalibrationWnd)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CalibrationWnd message handlers


bool CalibrationWnd::Create(CWnd* parent)
{
	if (!CWnd::Create(AfxRegisterWndClass(CS_VREDRAW | CS_HREDRAW, ::LoadCursor(NULL, IDC_ARROW)),
		0, WS_CHILD | WS_VISIBLE, CRect(0,0,0,0), parent, -1))
		return false;


	return true;
}


void CalibrationWnd::OnPaint()
{
	CPaintDC dc(this); // device context for painting
}


BOOL CalibrationWnd::OnEraseBkgnd(CDC* dc)
{
	COLORREF rgb_black= RGB(0,0,0);

	CRect rect;
	GetClientRect(rect);
	dc->FillSolidRect(rect, RGB(255,255,255));

	CRect ellipse_rect= rect;
	ellipse_rect.DeflateRect(MARGIN, MARGIN, BOTTOM, BOTTOM);

	const int LINE= 9;
	CPoint pt(MARGIN, MARGIN / 2);
	dc->FillSolidRect(pt.x, pt.y, ellipse_rect.Width(), 1, rgb_black);
	dc->FillSolidRect(pt.x, pt.y - LINE / 2, 1, LINE, rgb_black);
	pt.x += ellipse_rect.Width();
	dc->FillSolidRect(pt.x, pt.y - LINE / 2, 1, LINE, rgb_black);

	pt = CPoint(MARGIN / 2, MARGIN);
	dc->FillSolidRect(pt.x, pt.y, 1, ellipse_rect.Height(), rgb_black);
	dc->FillSolidRect(pt.x - LINE / 2, pt.y, LINE, 1, rgb_black);
	pt.y += ellipse_rect.Height();
	dc->FillSolidRect(pt.x - LINE / 2, pt.y, LINE, 1, rgb_black);

	dc->SelectStockObject(BLACK_PEN);
	dc->SelectStockObject(WHITE_BRUSH);

	dc->Ellipse(ellipse_rect);

	return true;
}


namespace {
	const double g_multipier= 4.0;		// 4 inches
}

void CalibrationWnd::GetResolution(double& X_res, double& Y_res) const
{
	CRect rect;
	GetClientRect(rect);

	rect.DeflateRect(MARGIN, MARGIN, BOTTOM, BOTTOM);

	// resolution in dpi
	X_res = rect.Width() / g_multipier;
	Y_res = rect.Height() / g_multipier;
}


CSize CalibrationWnd::CalcSize(double X_res, double Y_res) const
{
	int width= static_cast<int>(X_res * g_multipier + MARGIN + BOTTOM);
	int height= static_cast<int>(Y_res * g_multipier + MARGIN + BOTTOM);

	return CSize(width, height);
}

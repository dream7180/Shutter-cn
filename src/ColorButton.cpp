/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// ColorButton.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "ColorButton.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// ColorButton

ColorButton::ColorButton()
{
	dx_ = ::GetSystemMetrics(SM_CXEDGE);
	if (dx_ == 0)
		dx_ = 2;

	dy_ = ::GetSystemMetrics(SM_CYEDGE);
	if (dy_ == 0)
		dy_ = 2;

	dx_ += 3;
	dy_ += 3;

	rgb_color_ = 0;
}

ColorButton::~ColorButton()
{
}


BEGIN_MESSAGE_MAP(ColorButton, CButton)
	//{{AFX_MSG_MAP(ColorButton)
	ON_WM_PAINT()
	ON_WM_ENABLE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// ColorButton message handlers


void ColorButton::SetColor(COLORREF rgb_color)
{
	rgb_color_ = rgb_color;
	Invalidate(false);
//	CreateIcon(rgb_color);
/*	CBitmap color_bmp;
	CRect rect;
	GetClientRect(rect);
	rect.right -= 10;
	rect.bottom -= 10;
	CDC dc;
	dc.CreateIC(_T("DISPLAY"), NULL, NULL, NULL);
//	CClientDC dc(this);
	SetBitmap(0);
	color_bmp_.DeleteObject();
	color_bmp_.CreateCompatibleBitmap(&dc, rect.Width(), rect.Height());
	CDC mem_dc;
	mem_dc.CreateCompatibleDC(&dc);
	mem_dc.SelectObject(&color_bmp_);
	mem_dc.FillSolidRect(rect, rgb_color);
//	mem_dc.Draw3dRect(rect, ::GetSysColor(COLOR_3DSHADOW), ::GetSysColor(COLOR_3DHIGHLIGHT));
	SetBitmap(color_bmp_);
*/
//	if (hicon_color_)
//		SetIcon(hicon_color_);
}


void ColorButton::OnPaint()
{
	CButton::OnPaint();		// call Default() to draw the button
	//int state= SendMessage(BM_GETSTATE,0,0);
//	PaintIt(state & BST_PUSHED ? 1 : 0);
	PaintIt(0);
}


void ColorButton::PaintIt(int offset)
{
	CRect rect;
	GetClientRect(&rect);

	rect.bottom -= dy_ - offset;
	rect.right -= dx_ - offset;
	rect.top += dy_ + offset;
	rect.left += dx_ + offset;

	CClientDC dc(this);
//	CBrush brush(::GetSysColor(COLOR_BTNTEXT));
	COLORREF rgb_frame= RGB(128,128,128);
	dc.Draw3dRect(rect, rgb_frame, rgb_frame);
	rect.DeflateRect(1, 1);
	dc.FillSolidRect(&rect, rgb_color_);
//	dc.FrameRect(&rect, &brush);
}


void ColorButton::OnEnable(BOOL enable)
{
	CButton::OnEnable(enable);

	Invalidate(false);
}

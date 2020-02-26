/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "resource.h"
#include "ArrowButton.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// ArrowButton

ArrowButton::ArrowButton()
{
	left_arrow_ = true;
}

ArrowButton::~ArrowButton()
{
}


BEGIN_MESSAGE_MAP(ArrowButton, CButton)
	//{{AFX_MSG_MAP(ArrowButton)
	ON_WM_PAINT()
	ON_WM_ENABLE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// ArrowButton message handlers


void ArrowButton::OnPaint()
{
	CButton::OnPaint();		// call Default() to draw the button
//	int state= SendMessage(BM_GETSTATE,0,0);
//	PaintIt(state & BST_PUSHED ? 1 : 0);
	try
	{
		PaintIt(0, !!IsWindowEnabled());
	}
	catch (...)
	{
		ASSERT(false);
	}
}


void ArrowButton::PaintIt(int offset, bool enabled)
{
	CRect rect;
	GetClientRect(&rect);

	rect.bottom += offset;
	rect.right += offset;
	rect.top += offset;
	rect.left += offset;

	CClientDC dc(this);

	COLORREF color= ::GetSysColor(enabled ? COLOR_BTNTEXT : COLOR_GRAYTEXT);

	const int margin= dc.GetTextExtent(_T("X"), 1).cx * 13 / 10;
	int x= left_arrow_ ? rect.left + margin : rect.right - margin;
	int y= rect.CenterPoint().y;
	int h= 1;
	for (int i= 0; i < 4; ++i)
	{
		dc.FillSolidRect(x, y, 1, h, color);

		if (left_arrow_)
			x++;
		else
			x--;

		h += 2;
		y--;
	}

}


void ArrowButton::OnEnable(BOOL enable)
{
	CButton::OnEnable(enable);

	Invalidate(false);
}


void ArrowButton::DrawLeftArrow(bool left)
{
	left_arrow_ = left;
	if (m_hWnd)
		Invalidate();
}

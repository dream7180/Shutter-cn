/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// SliderCtrlEx.cpp : implementation file
//

#include "stdafx.h"
#include "SliderCtrlEx.h"
#include "UIElements.h"
#include "SnapFrame/CaptionWindow.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// SliderCtrlEx

SliderCtrlEx::SliderCtrlEx()
{
	rgb_backgnd_ = ::GetSysColor(COLOR_3DFACE);
}

SliderCtrlEx::~SliderCtrlEx()
{
}


BEGIN_MESSAGE_MAP(SliderCtrlEx, CSliderCtrl)
	//{{AFX_MSG_MAP(SliderCtrlEx)
	ON_WM_ERASEBKGND()
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnCustomDraw)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// SliderCtrlEx message handlers

BOOL SliderCtrlEx::OnEraseBkgnd(CDC* dc)
{
	return false;
}


void SliderCtrlEx::OnCustomDraw(NMHDR* nmhdr, LRESULT* result)
{
	LPNMCUSTOMDRAW cd= reinterpret_cast<LPNMCUSTOMDRAW>(nmhdr);
//TRACE(L"slider: %x\n", cd->dwDrawStage);
	switch (cd->dwDrawStage)
	{
	case CDDS_PREPAINT:
		*result = CDRF_NOTIFYITEMDRAW;
		break;

	case CDDS_ITEMPREPAINT:
		if (cd->dwItemSpec == TBCD_THUMB)
		{
			*result = CDRF_DODEFAULT;
			break;
		}
		if (cd->dwItemSpec == TBCD_TICS)
		{
			*result = CDRF_DODEFAULT;
			break;
		}
		if (cd->dwItemSpec == TBCD_CHANNEL)
		{
			CRect rect;
			GetClientRect(rect);

#if 0 // this is worthy code that works fine with real ReBarCtrl:
			if (background_dc_.m_hDC == 0)
			{
				if (CWnd* parent= GetParent())
				{
					CDC* dc= CDC::FromHandle(cd->hdc);
					background_dc_.CreateCompatibleDC(dc);
					background_bmp_.CreateCompatibleBitmap(dc, rect.Width(), rect.Height());
					background_dc_.SelectObject(&background_bmp_);

					WINDOWPLACEMENT wp;
					GetWindowPlacement(&wp);

					// prepare copy of background
					background_dc_.SetWindowOrg(wp.rcNormalPosition.left, wp.rcNormalPosition.top);
					parent->Print(&background_dc_, PRF_ERASEBKGND | PRF_CLIENT);
					background_dc_.SetWindowOrg(0, 0);
				}
			}

			::BitBlt(cd->hdc, 0, 0, rect.Width(), rect.Height(), background_dc_, 0, 0, SRCCOPY);

#else // this code works only with CaptionWindow's gradient

			WINDOWPLACEMENT wp;
			GetWindowPlacement(&wp);

			rect.top = -wp.rcNormalPosition.top;
			rect.bottom = rect.top + CaptionWindow::GetHeight();
			if (CDC* dc= CDC::FromHandle(cd->hdc))
			{
				::DrawPanelBackground(*dc, rect, &rgb_backgnd_);
//				dc->FillSolidRect(&cd->rc, RGB(0,0,0));
			}
#endif
//			*result = CDRF_SKIPDEFAULT;
			*result = CDRF_DODEFAULT;

			break;
		}

	default:
		*result = 0;
		break;
	}
}


void SliderCtrlEx::SetBackgroundColor(COLORREF backgnd)
{
	rgb_backgnd_ = backgnd;

	if (m_hWnd)
		Invalidate();
}

/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// PagerCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "PagerCtrl.h"
#include "GetDefaultGuiFont.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// PagerCtrlEx

PagerCtrlEx::PagerCtrlEx() : child_size_(0, 0)
{
}

PagerCtrlEx::~PagerCtrlEx()
{
}


BEGIN_MESSAGE_MAP(PagerCtrlEx, CWnd)
	//{{AFX_MSG_MAP(PagerCtrlEx)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
	ON_NOTIFY_REFLECT(PGN_CALCSIZE, OnCalcSize)
	ON_NOTIFY_REFLECT(PGN_SCROLL, OnPageScroll)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// PagerCtrlEx message handlers


bool PagerCtrlEx::Create(CWnd* parent, int id)
{
	if (!CWnd::Create(WC_PAGESCROLLER, _T(""), WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | PGS_HORZ | PGS_AUTOSCROLL,
		CRect(0,0,0,0), parent, id))
	{
		ASSERT(false);
		return false;
	}
	LOGFONT lf;
	::GetDefaultGuiFont(lf);
	HFONT hfont = CreateFontIndirectW(&lf);
	SendMessage(WM_SETFONT, WPARAM(hfont));
	//SendMessage(WM_SETFONT, WPARAM(::GetStockObject(DEFAULT_GUI_FONT)));

	return true;
}


void PagerCtrlEx::OnCalcSize(NMHDR* hdr, LRESULT* result)
{
	NMPGCALCSIZE* pgcs= reinterpret_cast<NMPGCALCSIZE*>(hdr);
	*result = 0;

	pgcs->iHeight = child_size_.cy;
	pgcs->iWidth = child_size_.cx;
}


void PagerCtrlEx::OnPageScroll(NMHDR* hdr, LRESULT* result)
{
	NMPGSCROLL* scroll= reinterpret_cast<NMPGSCROLL*>(hdr);
	*result = 0;

	switch(scroll->iDir)
	{
	case PGF_SCROLLLEFT:
	case PGF_SCROLLRIGHT:
	case PGF_SCROLLUP:
	case PGF_SCROLLDOWN:
		{
/*			CDC dc;
			dc.CreateIC(_T("DISPLAY"), 0, 0, 0);
			dc.SelectStockObject(DEFAULT_GUI_FONT);
			TEXTMETRIC tm;
			dc.GetTextMetrics(&tm);
			int line_height= tm.tmHeight + tm.tmInternalLeading + tm.tmExternalLeading;
			scroll->iScroll = line_height; */
			scroll->iScroll = 45;
		}
		break;
	}
}



void PagerCtrlEx::SetSize(CSize child_size)
{
	child_size_ = child_size;

	if (m_hWnd)
		SendMessage(PGM_RECALCSIZE);
}


void PagerCtrlEx::SetChild(HWND child)
{
	SendMessage(PGM_SETCHILD, 0, LPARAM(child));
}

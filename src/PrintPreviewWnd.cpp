/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// PrintPreviewWnd.cpp : implementation file
//

#include "stdafx.h"
#include "PrintPreviewWnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// PrintPreviewWnd

PrintPreviewWnd::PrintPreviewWnd(VectPhotoInfo& selected, PrintEngine* print)
 : selected_(selected), print_(print)
{
}

PrintPreviewWnd::~PrintPreviewWnd()
{
}


BEGIN_MESSAGE_MAP(PrintPreviewWnd, CWnd)
	//{{AFX_MSG_MAP(PrintPreviewWnd)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// PrintPreviewWnd message handlers


bool PrintPreviewWnd::Create(CWnd* parent, CRect rect)
{
	if (!CWnd::Create(AfxRegisterWndClass(CS_VREDRAW | CS_HREDRAW, ::LoadCursor(NULL, IDC_ARROW)),
		0, WS_CHILD | WS_VISIBLE, rect, parent, -1))
		return false;

	return true;
}


BOOL PrintPreviewWnd::OnEraseBkgnd(CDC* dc)
{
	if (print_ == 0)
		return false;

	CSize page_size= print_->GetPageSize();
	if (page_size.cx == 0 || page_size.cy == 0)
		return false;

	CRect client_rect;
	GetClientRect(client_rect);
	dc->FillSolidRect(client_rect, ::GetSysColor(COLOR_3DFACE));

	CRect rect= PrepareDC(*dc);

	dc->DPtoLP(rect);

	CRect page_rect(CPoint((rect.Width() - page_size.cx) / 2, (rect.Height() - page_size.cy) / 2), page_size);
	dc->FillSolidRect(page_rect, RGB(255,255,255));

	dc->LPtoDP(page_rect);
	dc->SetMapMode(MM_TEXT);
//	page_rect.InflateRect(1, 1);
	COLORREF rgb_outline= RGB(176,176,176);
	dc->Draw3dRect(page_rect, rgb_outline, rgb_outline);

/*	COLORREF rgb_outline= RGB(192,192,192);
	dc->Draw3dRect(rect, rgb_outline, rgb_outline);
	rect.DeflateRect(1, 1);
	dc->FillSolidRect(rect, RGB(255,255,255)); */

	return true;
}


CRect PrintPreviewWnd::PrepareDC(CDC& dc)
{
	CRect rect;
	GetClientRect(rect);
//	rect.DeflateRect(1, 1);
	dc.SetMapMode(MM_ISOTROPIC);
	dc.SetWindowExt(print_->GetPageSize());
	dc.SetViewportExt(rect.Size());
	return rect;
}


void PrintPreviewWnd::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	CRect rect;
	GetClientRect(rect);

	if (print_)
		print_->Print(dc, rect, selected_, print_->GetCurPage());
}


void PrintPreviewWnd::SetPageSize(CSize size, const CRect& margins_rect)
{
	if (print_)
	{
		print_->SetPageSize(size, margins_rect);

		if (m_hWnd)
			Invalidate();
	}
}


void PrintPreviewWnd::SetItemsAcross(int count)
{
	if (print_)
	{
		print_->SetItemsAcross(count); // columns_ = count;
		if (m_hWnd)
			Invalidate();
	}
}


void PrintPreviewWnd::SetPrinableArea(const CRect& rect)
{
	if (print_)
	{
		print_->SetPrintableArea(rect);
		if (m_hWnd)
			Invalidate();
	}
}


void PrintPreviewWnd::SetMargins(const CRect& margins_rect)
{
	if (print_)
	{
		print_->SetMargins(margins_rect);
		if (m_hWnd)
			Invalidate();
	}
}


void PrintPreviewWnd::SetEngine(PrintEngine* print)
{
	print_ = print;

	if (m_hWnd)
		Invalidate();
}

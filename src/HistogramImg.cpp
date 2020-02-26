/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// HistogramImg.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "HistogramImg.h"
#include "Dib.h"
#include "HistogramCtrl.h"
#include "RString.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// HistogramImg

HistogramImg::HistogramImg(HistogramCtrl* hist, CWnd* rect_label_wnd)
 : selection_rect_(0,0,0,0), drawing_(false)
{
	image_rect_ = CRect(0,0,0,0);
	hist_view_ = hist;
	rect_label_wnd_ = rect_label_wnd;
}

HistogramImg::~HistogramImg()
{
}


BEGIN_MESSAGE_MAP(HistogramImg, CButton)
	//{{AFX_MSG_MAP(HistogramImg)
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_SETCURSOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// HistogramImg message handlers

void HistogramImg::DrawItem(LPDRAWITEMSTRUCT draw_item_struct)
{
	if (draw_item_struct->itemAction != ODA_DRAWENTIRE)
		return;

	CDC* dc= CDC::FromHandle(draw_item_struct->hDC);

	dc->SetBkMode(OPAQUE);
	COLORREF rgb_back= RGB(255,255,255);

	if (bmp_)
	{
		CRect rect;
		GetClientRect(rect);
		int width= bmp_->GetWidth();
		int height= bmp_->GetHeight();

		if (width > rect.Width())
			width = rect.Width();
		if (height > rect.Height())
			height = rect.Height();
		if (width != bmp_->GetWidth() || height != bmp_->GetHeight())
		{
			double ratio_org= double(bmp_->GetWidth()) / double(bmp_->GetHeight());
			if (double(width) / double(height) > ratio_org)
				width = static_cast<int>(height * ratio_org);
			else
				height = static_cast<int>(width / ratio_org);
		}

		CRect dest_rect= rect;

		if (width < dest_rect.Width())
			dest_rect.left = (dest_rect.Width() - width) / 2;
		else
			dest_rect.left = 0;

		if (height < rect.Height())
			dest_rect.top = (dest_rect.Height() - height) / 2;
		else
			dest_rect.top = 0;

		dest_rect.right = dest_rect.left + width;
		dest_rect.bottom = dest_rect.top + height;

		CRect fill_rect= rect;
		fill_rect.right = dest_rect.left;
		if (fill_rect.Width() > 0)
			dc->FillSolidRect(fill_rect, rgb_back);
		fill_rect.left = dest_rect.right;
		fill_rect.right = rect.right;
		if (fill_rect.Width() > 0)
			dc->FillSolidRect(fill_rect, rgb_back);

		fill_rect = rect;
		fill_rect.bottom = dest_rect.top;
		if (fill_rect.Height() > 0)
			dc->FillSolidRect(fill_rect, rgb_back);
		fill_rect.top = dest_rect.bottom;
		fill_rect.bottom = rect.bottom;
		if (fill_rect.Height() > 0)
			dc->FillSolidRect(fill_rect, rgb_back);

		bmp_->Draw(dc, dest_rect);
		image_rect_ = dest_rect;
	}

	if (!selection_rect_.IsRectEmpty())
	{
		CRect rect= selection_rect_;
		rect.NormalizeRect();
		CBitmap bmp;
		bmp.LoadBitmap(IDB_PATTERN);
		CBrush br;
		br.CreatePatternBrush(&bmp);
		dc->SetBkColor(RGB(255,255,255));
		dc->SetTextColor(RGB(255,0,0));
		dc->FrameRect(rect, &br);
	}
}


BOOL HistogramImg::OnEraseBkgnd(CDC* dc)
{
	return true;
}

///////////////////////////////////////////////////////////////////////////////
void HistogramImg::PointInside(CPoint& pos)
{
	CRect rect= image_rect_;

	if (pos.x < rect.left)
		pos.x = rect.left;
	if (pos.x > rect.right)
		pos.x = rect.right;
	if (pos.y < rect.top)
		pos.y = rect.top;
	if (pos.y > rect.bottom)
		pos.y = rect.bottom;
}

void HistogramImg::OnLButtonDown(UINT flags, CPoint pos)
{
	drawing_ = true;
	SetCapture();
	PointInside(pos);
	selection_rect_.left = selection_rect_.right = pos.x;
	selection_rect_.top = selection_rect_.bottom = pos.y;
	CClientDC dc(this);
	dc.DrawFocusRect(selection_rect_);
//	SetCursor();
}


void HistogramImg::OnLButtonUp(UINT flags, CPoint point)
{
	drawing_ = false;
	selection_rect_.NormalizeRect();
	CClientDC dc(this);
	dc.DrawFocusRect(selection_rect_);
	Invalidate();
	if (bmp_ != 0 && image_rect_.Width() > 0 && image_rect_.Height() > 0)
	{
		CRect rect= selection_rect_;
		CRect bmp_rect(0, 0, bmp_->GetWidth() - 1, bmp_->GetHeight() - 1);
		rect.left = (rect.left - image_rect_.left) * bmp_rect.Width() / image_rect_.Width();
		rect.right = (rect.right - image_rect_.left) * bmp_rect.Width() / image_rect_.Width();
		rect.top = (rect.top - image_rect_.top) * bmp_rect.Height() / image_rect_.Height();
		rect.bottom = (rect.bottom - image_rect_.top) * bmp_rect.Height() / image_rect_.Height();
		oStringstream ost;
		if (rect == bmp_rect)
			ost << _T("the whole photograph");
		else
			ost << _T("rectangle from (") << rect.left << _T(", ") << rect.top << _T(") to (") << rect.right << _T(", ") << rect.bottom << _T(")");
		rect_label_wnd_->SetWindowText(ost.str().c_str());
		//	CWaitCursor wait;
		if (selection_rect_.Width() * selection_rect_.Height() > 1000)
			::SetCursor(::LoadCursor(0, IDC_WAIT));

		// rebuild histogram
		hist_view_->Build(*bmp_, rect);
	}
	SetCursor();
	ReleaseCapture();
}


void HistogramImg::OnMouseMove(UINT flags, CPoint pos)
{
	if (drawing_)
	{
		CClientDC dc(this);
		CRect rect= selection_rect_;
		rect.NormalizeRect();
		dc.DrawFocusRect(rect);
		PointInside(pos);
		selection_rect_.right = pos.x;
		selection_rect_.bottom = pos.y;
		rect = selection_rect_;
		rect.NormalizeRect();
		dc.DrawFocusRect(rect);
		SetCursor();
		return;
	}

	CButton::OnMouseMove(flags, pos);
}


void HistogramImg::SetCursor()
{
	if (drawing_ && (selection_rect_.Width() != 0 || selection_rect_.Height() != 0))
		::SetCursor(::LoadCursor(0, IDC_CROSS));
	else
		::SetCursor(AfxGetApp()->LoadCursor(IDC_EYE_DROP));
}


BOOL HistogramImg::OnSetCursor(CWnd* wnd, UINT hit_test, UINT message)
{
	if (hit_test == HTCLIENT)
	{
		SetCursor();
		return true;
	}
	else
		return CButton::OnSetCursor(wnd, hit_test, message);
}

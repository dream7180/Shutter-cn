/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// PreviewCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "PreviewCtrl.h"
#include "CtrlDraw.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CPreviewCtrl

CPreviewCtrl::CPreviewCtrl()
{
	const TCHAR* cls_name= _T("PreviewCtrl");

	HINSTANCE inst= AfxGetResourceHandle();
	// force same dll as resources
//	HINSTANCE inst= AfxFindResourceHandle(MAKEINTRESOURCE(IDD_FORM_DOUBLE), RT_DIALOG);

	// see if the class already exists
	WNDCLASS wndcls;
	if (!::GetClassInfo(inst, cls_name, &wndcls))
	{
		// otherwise we need to register a new class
		wndcls.style = 0;
		wndcls.lpfnWndProc = ::DefWindowProc;
		wndcls.cbClsExtra = wndcls.cbWndExtra = 0;
		wndcls.hInstance = inst;
		wndcls.hIcon = 0;
		wndcls.hCursor = ::LoadCursor(NULL, IDC_ARROW);
		wndcls.hbrBackground = 0;
		wndcls.lpszMenuName = NULL;
		wndcls.lpszClassName = cls_name;

		AfxRegisterClass(&wndcls);
	}

	host_ = 0;
	image_ = 0;
	icm_ = false;
	bitmap_rect_.SetRectEmpty();

	show_selection_ = false;
	selection_rect_.SetRectEmpty();
	resizing_ = moving_ = drawing_ = false;
	start_ = CPoint(0, 0);
	side_ = 0;

	orig_bitmap_rect_.SetRectEmpty();
	grid_size_ = CSize(8, 8);
	magnetic_grid_ = false;

	ratio_constraints_ = CSize(0, 0);
}


CPreviewCtrl::~CPreviewCtrl()
{
}


BEGIN_MESSAGE_MAP(CPreviewCtrl, CWnd)
	//{{AFX_MSG_MAP(CPreviewCtrl)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_MOUSEMOVE()
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CPreviewCtrl message handlers

//bool CPreviewCtrl::Create(CWnd* parent, CRect rect)
//{
////	pattern_bmp_.LoadBitmap(IDB_GRID_PATTERN);
//
//	if (!CWnd::Create(AfxRegisterWndClass(CS_VREDRAW | CS_HREDRAW, ::LoadCursor(NULL, IDC_ARROW)),
//		0, WS_CHILD | WS_VISIBLE, rect, parent, -1))
//		return false;
//
//	return true;
//}


void CPreviewCtrl::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	Draw(dc);
}


void CPreviewCtrl::Draw(CDC& dc)
{
	CRect rect;
	GetClientRect(rect);

	CtrlDraw::DrawBorder(dc, rect);
	rect.DeflateRect(1, 1);

	COLORREF rgb_white= ::GetSysColor(COLOR_WINDOW);
	dc.Draw3dRect(rect, rgb_white, rgb_white);
	rect.DeflateRect(1, 1);

	if (image_)
	{
		bitmap_rect_ = image_->GetDisplayRect(rect);
		::DrawPreserveAspectRatio(*image_, &dc, rect, rgb_white, icm_, false);

		if (show_selection_)
			DrawSelectionRect(dc, GetSelection());
	}
	else
		bitmap_rect_.SetRectEmpty();
}


BOOL CPreviewCtrl::OnEraseBkgnd(CDC* dc)
{
	return true;
}


void CPreviewCtrl::Clear()
{
	if (m_hWnd)
		Invalidate();
}


void CPreviewCtrl::SetDib(Dib& dib)
{
	image_ = &dib;
}


/*void CPreviewCtrl::OnMouseMove(UINT flags, CPoint point)
{
	CWnd::OnMouseMove(flags, point);

	SendMouseNotification(point);

	TRACKMOUSEEVENT tme;
	tme.cbSize = sizeof tme;
	tme.flags = TME_LEAVE;
	tme.hover_time = 0;
	tme.track = *this;
	::_TrackMouseEvent(&tme);
}*/


LRESULT CPreviewCtrl::OnMouseLeave(WPARAM, LPARAM)
{
	SendMouseNotification(CPoint(-1, -1));
	return 0;
}


void CPreviewCtrl::SendMouseNotification(CPoint point)
{
	if (host_ == 0  || image_ == 0 || bitmap_rect_.IsRectEmpty())
		return;

	if (bitmap_rect_.PtInRect(point))
	{
		int x= (point.x - bitmap_rect_.left) * image_->GetWidth() / bitmap_rect_.Width();
		int y= (point.y - bitmap_rect_.top) * image_->GetHeight() / bitmap_rect_.Height();
		host_->MouseMoved(this, x, y, false);
	}
	else
		host_->MouseMoved(this, 0, 0, true);
}


void CPreviewCtrl::SetCursor()
{
	CPoint point;
	GetCursorPos(&point);
	ScreenToClient(&point);

	HCURSOR cursor= 0;

	if (show_selection_) //drawing_ && (selection_rect_.Width() != 0 || selection_rect_.Height() != 0))
	{
		if (moving_)
			cursor = AfxGetApp()->LoadCursor(IDC_SCROLL);
		else if (side_ != 0)
		{
			if ((side_ & (RIGHT | BOTTOM)) == (RIGHT | BOTTOM) || ((side_ & (LEFT | TOP)) == (LEFT | TOP)))
				cursor = ::LoadCursor(0, IDC_SIZENWSE);
			else if ((side_ & (RIGHT | TOP)) == (RIGHT | TOP) || ((side_ & (LEFT | BOTTOM)) == (LEFT | BOTTOM)))
				cursor = ::LoadCursor(0, IDC_SIZENESW);
			else if (side_ & (RIGHT | LEFT))
				cursor = ::LoadCursor(0, IDC_SIZEWE);
			else if (side_ & (TOP | BOTTOM))
				cursor = ::LoadCursor(0, IDC_SIZENS);
		}
		else if (GetSelection().PtInRect(point) && !drawing_)
			cursor = ::LoadCursor(0, IDC_SIZEALL);
		else if (bitmap_rect_.PtInRect(point))
			cursor = AfxGetApp()->LoadCursor(IDC_CROSS_CURSOR);
			//cursor = ::LoadCursor(0, IDC_CROSS);
		else
			cursor = ::LoadCursor(0, IDC_ARROW);
	}
	else if (bitmap_rect_.PtInRect(point))
		cursor = AfxGetApp()->LoadCursor(IDC_EYE_DROP);
	else
		cursor = ::LoadCursor(0, IDC_ARROW);

	ASSERT(cursor);
	::SetCursor(cursor);
}


BOOL CPreviewCtrl::OnSetCursor(CWnd* wnd, UINT hit_test, UINT message)
{
	if (hit_test == HTCLIENT)
	{
		SetCursor();
		return true;
	}
	else
		return CWnd::OnSetCursor(wnd, hit_test, message);
}


///////////////////////////////////////////////////////////////////////////////


void CPreviewCtrl::DrawSelectionRect(CDC& dc, const CRect& selection_rect)
{
	CRect rect= selection_rect;
	rect.NormalizeRect();
	if (rect.IsRectEmpty())
		return;
	CBitmap bmp;
	bmp.LoadBitmap(IDB_PATTERN);
	CBrush br;
	br.CreatePatternBrush(&bmp);
	dc.SetBkColor(RGB(255,255,255));
	dc.SetTextColor(RGB(255,0,0));
	dc.FrameRect(rect, &br);
}


void CPreviewCtrl::PointInside(CPoint& pos)
{
	CRect rect= bitmap_rect_;

	if (pos.x < rect.left)
		pos.x = rect.left;
	if (pos.x > rect.right)
		pos.x = rect.right;
	if (pos.y < rect.top)
		pos.y = rect.top;
	if (pos.y > rect.bottom)
		pos.y = rect.bottom;
}


void CPreviewCtrl::OnLButtonDown(UINT flags, CPoint pos)
{
	if (!show_selection_)
		return;

	start_ = pos;

	SetCapture();

	CRect rect= GetSelection();
	selection_rect_ = rect;

	if (side_ = CheckSide(rect, pos))
	{
		resizing_ = true;
	}
	else if (rect.PtInRect(pos))
	{
		moving_ = true;
	}
	else
	{
		PointInside(pos);

		drawing_ = true;

		selection_rect_.left = selection_rect_.right = pos.x;
		selection_rect_.top = selection_rect_.bottom = pos.y;
		selection_.Set(bitmap_rect_, selection_rect_);

		CClientDC dc(this);
		dc.DrawFocusRect(selection_rect_);
	}

	SetCursor();
}


void CPreviewCtrl::OnLButtonUp(UINT flags, CPoint point)
{
	if (!show_selection_)
		return;

	if (moving_)			// moving selection
	{
		moving_ = false;
		selection_rect_ = GetSelection();
	}
	else if (resizing_)	// resizing selection
	{
		resizing_ = false;
		selection_rect_ = GetSelection();
	}
	else if (drawing_)	// creating new selection
	{
		drawing_ = false;

		selection_rect_.NormalizeRect();
		selection_.Set(bitmap_rect_, selection_rect_);
		CClientDC dc(this);
		dc.DrawFocusRect(selection_rect_);
		Invalidate();
	}
/*	if (image_rect_.Width() > 0 && image_rect_.Height() > 0)
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
		hist_view_->BuildHistogram(bmp_, &rect);
		hist_view_->Invalidate();
	} */
	SetCursor();
	ReleaseCapture();

	if (host_)
		host_->SelectionRectChanged(GetSelectionRect());
}


void CPreviewCtrl::OnMouseMove(UINT flags, CPoint point)
{
	CWnd::OnMouseMove(flags, point);

	if (!show_selection_)
	{
		SendMouseNotification(point);

		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof tme;
		tme.dwFlags = TME_LEAVE;
		tme.dwHoverTime = 0;
		tme.hwndTrack = *this;
		::_TrackMouseEvent(&tme);
		return;
	}

	if (drawing_)
	{
		CClientDC dc(this);
		CRect rect= selection_rect_;
		rect.NormalizeRect();
		dc.DrawFocusRect(rect);
		PointInside(point);
		selection_rect_.right = point.x;
		selection_rect_.bottom = point.y;
		ConstrainAspectRatio(selection_rect_, ratio_constraints_, bitmap_rect_, 0);
		rect = selection_rect_;
		rect.NormalizeRect();
		selection_.Set(bitmap_rect_, rect);	// keep current
		dc.DrawFocusRect(rect);
		SetCursor();
	}
	else if (moving_)
	{
		CRect original_rect= GetSelection();
		CSize delta= point - start_;
		CRect rect= selection_rect_;
		rect.NormalizeRect();
		if (delta.cx < 0)
			delta.cx = -std::min(-delta.cx, selection_rect_.left - bitmap_rect_.left);
		else if (delta.cx > 0)
			delta.cx = std::min(delta.cx, bitmap_rect_.right - selection_rect_.right);

		if (delta.cy < 0)
			delta.cy = -std::min(-delta.cy, selection_rect_.top - bitmap_rect_.top);
		else if (delta.cy > 0)
			delta.cy = std::min(delta.cy, bitmap_rect_.bottom - selection_rect_.bottom);

		rect.OffsetRect(delta);

		selection_.Set(bitmap_rect_, rect);
		CRect new_rect= GetSelection();
		if (new_rect != original_rect)
		{
			CClientDC dc(this);
			Draw(dc);
		}
	}
	else if (resizing_)
	{
		CRect original_rect= GetSelection();
		CSize delta= point - start_;
		CRect rect= selection_rect_;
		rect.NormalizeRect();

		if (side_ & RIGHT)
			rect.right += delta.cx;
		if (side_ & LEFT)
			rect.left += delta.cx;
		if (side_ & TOP)
			rect.top += delta.cy;
		if (side_ & BOTTOM)
			rect.bottom += delta.cy;

		rect.NormalizeRect();
		ConstrainAspectRatio(rect, ratio_constraints_, bitmap_rect_, side_);

		CRect r= rect & bitmap_rect_;
//		if (r == rect)
		{

//		rect &= bitmap_rect_;

		selection_.Set(bitmap_rect_, rect);
		CRect new_rect= GetSelection();
		if (new_rect != original_rect)
		{
			CClientDC dc(this);
			Draw(dc);
		}
		}
	}
	else
	{
		UINT side= CheckSide(GetSelection(), point);
		if (side_ != side)
		{
			side_ = side;
			SetCursor();
		}
	}
}


void CPreviewCtrl::EnableSelection(bool enable)
{
	show_selection_ = enable;
	Invalidate();
}


void CPreviewCtrl::DRect::Set(const CRect& bmp_rect, const CRect& selection_rect)
{
	if (bmp_rect.Width() == 0 || bmp_rect.Height() == 0)
		return;

	CPoint p= selection_rect.TopLeft() - bmp_rect.TopLeft();
	x1 = double(p.x) / bmp_rect.Width();
	y1 = double(p.y) / bmp_rect.Height();

	p = selection_rect.BottomRight() - bmp_rect.TopLeft();
	x2 = double(p.x) / bmp_rect.Width();
	y2 = double(p.y) / bmp_rect.Height();
}


CRect CPreviewCtrl::DRect::Get(const CRect& bmp_rect) const
{
	CRect rect= CalcRect(bmp_rect);

	rect.OffsetRect(bmp_rect.TopLeft());

	return rect;
}


CRect CPreviewCtrl::DRect::CalcRect(const CRect& bmp_rect) const
{
	CRect rect;

	rect.left = static_cast<int>(x1 * bmp_rect.Width() + 0.5);
	rect.top = static_cast<int>(y1 * bmp_rect.Height() + 0.5);

	rect.right = static_cast<int>(x2 * bmp_rect.Width() + 0.5);
	rect.bottom = static_cast<int>(y2 * bmp_rect.Height() + 0.5);

	return rect;
}


CRect CPreviewCtrl::DRect::Get(const CRect& bmp_rect, const CRect& original_bmp_rect, CSize grid_size) const
{
	CRect rect= CalcRect(original_bmp_rect);

	CSize delta= rect.TopLeft();

	rect.left += grid_size.cx / 2;
	rect.top  += grid_size.cy / 2;
	rect.left -= rect.left % grid_size.cx;
	rect.top  -= rect.top % grid_size.cy;

	delta -= CSize(rect.TopLeft());

//	rect.BottomRight() -= delta;

	rect.right  += grid_size.cx / 2;
	rect.bottom += grid_size.cy / 2;
	rect.right  -= rect.right % grid_size.cx;
	rect.bottom -= rect.bottom % grid_size.cy;

	if (rect.right > original_bmp_rect.right)
		rect.right -= grid_size.cx;
	if (rect.bottom > original_bmp_rect.bottom)
		rect.bottom -= grid_size.cy;

	CRect out_rect;
	out_rect.left = static_cast<int>(double(rect.left) * bmp_rect.Width() / original_bmp_rect.Width() + 0.5);
	out_rect.top = static_cast<int>(double(rect.top) * bmp_rect.Height() / original_bmp_rect.Height() + 0.5);
	out_rect.right = static_cast<int>(double(rect.right) * bmp_rect.Width() / original_bmp_rect.Width() + 0.5);
	out_rect.bottom = static_cast<int>(double(rect.bottom) * bmp_rect.Height() / original_bmp_rect.Height() + 0.5);

	out_rect.OffsetRect(bmp_rect.TopLeft());

	return out_rect;
}


UINT CPreviewCtrl::CheckSide(const CRect& rect, CPoint point)
{
	const int EXTRA= 4;	// tolerance in pixels

	CRect out= rect;
	out.InflateRect(EXTRA, EXTRA);
	CRect in= rect;
	in.DeflateRect(EXTRA, EXTRA);

	if (!out.PtInRect(point) || in.PtInRect(point))
		return 0;

	UINT side= 0;

	if (point.x >= in.right)
		side |= RIGHT;
	if (point.x <= in.left)
		side |= LEFT;
	if (point.y >= in.bottom)
		side |= BOTTOM;
	if (point.y <= in.top)
		side |= TOP;

	return side;
}


void CPreviewCtrl::SetOriginalBmpSize(CSize original_size)
{
	orig_bitmap_rect_ = CRect(CPoint(0, 0), original_size);
}


void CPreviewCtrl::EnableMagneticGrid(bool enable)
{
	magnetic_grid_ = enable;
	Invalidate();
}


CRect CPreviewCtrl::GetSelection() const
{
	if (magnetic_grid_)
		return selection_.Get(bitmap_rect_, orig_bitmap_rect_, grid_size_);
	else
		return selection_.Get(bitmap_rect_);
}


CRect CPreviewCtrl::GetSelectionRect() const
{
	if (magnetic_grid_)
		return selection_.Get(orig_bitmap_rect_, orig_bitmap_rect_, grid_size_);
	else
		return selection_.Get(orig_bitmap_rect_);
}


void CPreviewCtrl::SetSelectionRect(const CRect& rect)
{
	return selection_.Set(orig_bitmap_rect_, rect);
}


// built-in rect union fn doesn't work properly for rects with zero height or width
static CRect Union(const CRect& a, const CRect& b)
{
	return CRect(
		std::max(a.left, b.left),
		std::max(a.top, b.top),
		std::min(a.right, b.right),
		std::min(a.bottom, b.bottom));
}


void CPreviewCtrl::ConstrainAspectRatio(CRect& sel_rect, CSize ratio, const CRect& bounds, UINT sides)
{
	if (ratio.cx <= 0 || ratio.cy <= 0)
		return;

	CRect rect= sel_rect;
//TRACE(L"rect: %d %d  %d %d\n", rect.left, rect.top, rect.Width(), rect.Height());
	if (sides == 0)
	{
		if (rect.Height() < 0)
			sides |= TOP;
		else
			sides |= BOTTOM;

		if (rect.Width() < 0)
			sides |= LEFT;
		else
			sides |= RIGHT;
	}

	rect.NormalizeRect();

	CSize size= rect.Size();

	int w= std::max<int>(1, abs(size.cy) * ratio.cx / ratio.cy);
	int h= std::max<int>(1, abs(size.cx) * ratio.cy / ratio.cx);

	if ((sides & (RIGHT | LEFT)) && (sides & (TOP | BOTTOM)))	// dragging corner
	{
		if (h < size.cy)
		{
			if (sides & BOTTOM)
				rect.bottom = rect.top + h;
			else
				rect.top = rect.bottom - h;
		}
		else if (w < size.cx)
		{
			if (sides & RIGHT)
				rect.right = rect.left + w;
			else
				rect.left = rect.right - w;
		}
	}
	else if (sides & (RIGHT | LEFT))	// dragging side
		rect.bottom = rect.top + h;
	else if (sides & (TOP | BOTTOM))
		rect.right = rect.left + w;

	CRect unionrect= Union(rect, bounds);

	if (unionrect != rect)
	{
		// correction necessary

		rect = unionrect;

		CSize size= rect.Size();

		int w= std::max<int>(1, abs(size.cy) * ratio.cx / ratio.cy);
		int h= std::max<int>(1, abs(size.cx) * ratio.cy / ratio.cx);

		if (h < size.cy)
		{
			//if (sides & BOTTOM)
			//	rect.bottom = rect.top + h;
			//else
			//	rect.top = rect.bottom - h;
			if (sides & TOP)
				rect.top = rect.bottom - h;
			else
				rect.bottom = rect.top + h;
		}
		else if (w < size.cx)
		{
			//if (sides & RIGHT)
			//	rect.right = rect.left + w;
			//else
			//	rect.left = rect.right - w;
			if (sides & LEFT)
				rect.left = rect.right - w;
			else
				rect.right = rect.left + w;
		}

	}

	if (sel_rect.Width() < 0)
		std::swap(rect.left, rect.right);
	if (sel_rect.Height() < 0)
		std::swap(rect.top, rect.bottom);

	sel_rect = rect;
}


void CPreviewCtrl::SetRatioConstrain(CSize ratio)
{
	ratio_constraints_ = ratio;

	//todo
}

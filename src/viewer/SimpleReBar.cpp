/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// SimpleReBar.cpp : implementation file
//

#include "stdafx.h"
#include "SimpleReBar.h"
#include "../clamp.h"
using namespace std;


struct Band
{
	Band() : rect_(0,0,0,0), id_(0), visible_(true), gripper_(true), snap_to_ideal_size_(true), wnd_(0)
	{
		minimal_size_ = CSize(50, 28);
		ideal_size_ = minimal_size_;
		ideal_size_.cx *= 3;
	}

	CRect rect_;
	CSize ideal_size_;	// fn callback?
	CSize minimal_size_;
	int id_;
	CWnd* wnd_;
	struct
	{
		unsigned visible_ : 1;
		unsigned gripper_ : 1;	// band without a gripper cannot to moved
		unsigned snap_to_ideal_size_ : 1;	// when resizing close to the ideal_size_ resizing will be frozen within small range
	};

	static const int GRIPPER_W= 9;
};


String StoreLayout(const Band& band)
{
	oStringstream ost;
	ost << band.rect_.left << _T(" ");
	ost << band.rect_.right << _T(" ");
	ost << band.rect_.top << _T(" ");
	ost << band.rect_.bottom << _T(" ");
	ost << static_cast<bool>(band.visible_) << std::endl;
	return ost.str();
}

void RestoreLayout(Band& band, iStringstream& ist)
{
	ist >> band.rect_.left;
	ist >> band.rect_.right;
	ist >> band.rect_.top;
	ist >> band.rect_.bottom;
	bool b= true;
	ist >> b;
	band.visible_ = b;
}


void SetBandWindowPos(Band& band)
{
	if (band.wnd_)
	{
		int width= band.rect_.Width();
		int x= band.rect_.left;

		if (band.gripper_)
		{
			width -= Band::GRIPPER_W;
			x += Band::GRIPPER_W;
		}

		band.wnd_->SetWindowPos(0, x, band.rect_.top, width, band.rect_.Height(),
			SWP_NOZORDER | SWP_NOACTIVATE);
	}
}


struct SimpleReBar::Impl
{
	Impl();

	bool EnterMove(CPoint pos);
	bool StopMove(CPoint pos);
	bool MoveBands(CPoint pos, bool lbtn_down);

	void Resize(int x, int y, int cx, int cy, bool force);

	void SetBand(size_t idx, CWnd* band_wnd);

	int GetHeight();

	void BandWndChanged(CWnd* band_wnd, CSize size, const CRect rect);

	bool BandMoveCursor() const		{ return moving_bands_ && sel_band_; }

	Band band1_;
	Band band2_;
	CRect limits_;
	COLORREF light_grip_;	// colors for shaded dots in bands
	COLORREF dark_grip_;

	boost::function<void ()> resize_callback_;
	boost::function<void (CDC& dc, int id, CRect band, bool in_line)> erase_callback_;

	void ValidateLayout(CSize size);

private:
	bool moving_bands_;
	Band* sel_band_;
	CSize cursor_delta_;	// offset form the top left corner of a band to the cursor at mouse click
};


SimpleReBar::Impl::Impl() : moving_bands_(false), sel_band_(0), cursor_delta_(0, 0), limits_(0,0,0,0)
{
	light_grip_ = RGB(90,90,90);	// colors for shaded dots in bands
	dark_grip_ = RGB(50,50,50);
}


void SimpleReBar::Impl::ValidateLayout(CSize size)
{
	Resize(0, 0, size.cx, size.cy, true);
}


void LimitBand(Band& band, CRect limits)
{
//	band.rect_ &= limits;
	band.rect_.left = clamp(band.rect_.left, limits.left, limits.right);
	band.rect_.right = clamp(band.rect_.right, limits.left, limits.right);
	band.rect_.top = clamp(band.rect_.top, limits.top, limits.bottom);
	band.rect_.bottom = clamp(band.rect_.bottom, limits.top, limits.bottom);
}


bool BandsInLine(const Band& b1, const Band& b2)
{
	return b1.rect_.top == b2.rect_.top;	// in line?
}


void SimpleReBar::Impl::Resize(int x, int y, int cx, int cy, bool force)
{
	limits_.SetRect(x, y, cx, cy);

	CRect org1= band1_.rect_;
	CRect org2= band2_.rect_;

	LimitBand(band1_, limits_);
	LimitBand(band2_, limits_);

	if (BandsInLine(band1_, band2_))	// in line?
	{
		// right band
		Band& r= band1_.rect_.right > band2_.rect_.right ? band1_ : band2_;
		Band& l= band1_.rect_.right > band2_.rect_.right ? band2_ : band1_;

		int right= max(limits_.right, band1_.minimal_size_.cx + band2_.minimal_size_.cx);

		if (l.rect_.Width() < l.minimal_size_.cx)
		{
			l.rect_.left = 0;
			l.rect_.right = r.rect_.left = l.minimal_size_.cx;
		}

		r.rect_.right = right;

		if (r.rect_.Width() < r.minimal_size_.cx)
			l.rect_.right = r.rect_.left = r.rect_.right - r.minimal_size_.cx;

		r.rect_.top = l.rect_.top = limits_.top;
		r.rect_.bottom = l.rect_.bottom = GetHeight();
	}
	else
	{
		band1_.rect_.left = band2_.rect_.left = limits_.left;
		band1_.rect_.right = band2_.rect_.right = limits_.right;
	}

	if (band1_.wnd_ && (force || org1 != band1_.rect_))
		SetBandWindowPos(band1_);

	if (band2_.wnd_ && (force || org2 != band2_.rect_))
		SetBandWindowPos(band2_);
}

static const int BAND_EXTRA_SPACE= 3;

void SimpleReBar::Impl::SetBand(size_t idx, CWnd* band_wnd)
{
	Band& b= idx == 0 ? band1_ : band2_;

	b.wnd_ = band_wnd;
	b.id_ = 0;
	b.visible_ = true;
	b.gripper_ = true;
	b.snap_to_ideal_size_ = true;

	int x= 0;
	if (idx > 0)
		x += band1_.rect_.Width();

	if (band_wnd)
	{
		CRect rect(0,0,0,0);
		band_wnd->GetWindowRect(rect);
		b.minimal_size_.cy = rect.Height();
		b.ideal_size_ = rect.Size();
		b.ideal_size_.cx += BAND_EXTRA_SPACE; // extra space, so bands are not too close each other in ideal size
		if (b.gripper_)
			b.ideal_size_.cx += Band::GRIPPER_W;
		b.id_ = band_wnd->GetDlgCtrlID();
		b.rect_ = CRect(CPoint(x, 0), b.ideal_size_);
	}
}


void VerticalStack(Band& b1, Band& b2, CRect limits)
{
	b2.rect_ = b1.rect_ = limits;

	b2.rect_.top = b1.rect_.bottom = b1.rect_.top + b1.minimal_size_.cy;

	b2.rect_.bottom = b2.rect_.top + b2.minimal_size_.cy;
}


void SimpleReBar::Impl::BandWndChanged(CWnd* band_wnd, CSize size, const CRect rect)
{
	Band& b1= band1_.wnd_ == band_wnd ? band1_ : band2_;

//	CRect rect(0,0,0,0);
//	band_wnd->GetWindowRect(rect);
	b1.minimal_size_.cy = size.cy;
	b1.ideal_size_ = size;
	b1.ideal_size_.cx += BAND_EXTRA_SPACE; // extra space, so bands are not too close each other in ideal size
	if (b1.gripper_)
		b1.ideal_size_.cx += Band::GRIPPER_W;
//	b1.rect_ = CRect(CPoint(x, 0), b.ideal_size_);
	b1.rect_.bottom = b1.rect_.top + b1.ideal_size_.cy;

	// second band
	Band& b2= band1_.wnd_ == band_wnd ? band2_ : band1_;

	if (BandsInLine(band1_, band2_))	// in line?
	{
		b2.rect_.bottom = b2.rect_.top + b2.ideal_size_.cy;

		int h= GetHeight();
		b1.rect_.bottom = b1.rect_.top + h;
		b2.rect_.bottom = b2.rect_.top + h;
	}
	else
	{
		if (b1.rect_.top < b2.rect_.top)
			VerticalStack(b1, b2, rect);
		else
			VerticalStack(b2, b1, rect);
	}

	SetBandWindowPos(band1_);
	SetBandWindowPos(band2_);
}


int SimpleReBar::Impl::GetHeight()
{
	int h1= band1_.visible_ ? band1_.rect_.Height() : 0;
	int h2= band2_.visible_ ? band2_.rect_.Height() : 0;

	int height= max(h1, h2);

	if (h1 > 0 && h2 > 0)
		if (band1_.rect_.top != band2_.rect_.top)	// bands in separate rows?
			height = h1 + h2;

	return height;
}


bool SimpleReBar::Impl::EnterMove(CPoint pos)
{
	sel_band_ = 0;
	moving_bands_ = false;

	if (band1_.rect_.PtInRect(pos))
	{
		if (band1_.gripper_)
			sel_band_ = &band1_;
	}
	else if (band2_.rect_.PtInRect(pos))
	{
		if (band2_.gripper_)
			sel_band_ = &band2_;
	}

	if (sel_band_ == 0)
		return false;

	cursor_delta_ = pos - sel_band_->rect_.TopLeft();

	moving_bands_ = true;
	return true;
}


bool SimpleReBar::Impl::StopMove(CPoint pos)
{
	if (!moving_bands_)
		return false;

	moving_bands_ = false;
	return true;
}


void MoveBandIntoFront(Band& b1, Band& b2, CRect limits)
{
	b1.rect_.left = limits.left;
	b1.rect_.right = b1.rect_.left + b1.ideal_size_.cx;

	b2.rect_.left = b1.rect_.right;
	b2.rect_.right = limits.right;

	if (b2.rect_.Width() < b2.minimal_size_.cx)
		b1.rect_.right = b2.rect_.left = b2.rect_.right - b2.minimal_size_.cx;
}


bool SimpleReBar::Impl::MoveBands(CPoint pos, bool lbtn_down)
{
	if (!moving_bands_)
		return false;

	if (!lbtn_down)
	{
		sel_band_ = 0;
		return false;
	}

	CPoint p= pos - cursor_delta_;

	if (sel_band_ == 0 || p == sel_band_->rect_.TopLeft())
		return false;

	// move selected band

	Band* s= sel_band_;
	Band* r= sel_band_ == &band1_ ? &band2_ : &band1_;	// second, remaining bar

	CRect org1= band1_.rect_;
	CRect org2= band2_.rect_;

	// check if r is visible
	if (!r->visible_)
		return false;

	// check new location against hard limits imposed by enclosing window
	p.x = min<int>(p.x, limits_.right - s->minimal_size_.cx);
	p.x = max<int>(p.x, limits_.left);

	bool in_line= r->rect_.top == s->rect_.top;	// bands in the same row?

	if (r != 0)
	{
		const int EDGE= 8;	// for hysteresis

		// check cursor location (not relative 'p' point)
		if (in_line && pos.y < s->rect_.top - EDGE)
		{
			// moving the band to the top row
			VerticalStack(*s, *r, limits_);
		}
		else if (in_line && pos.y > s->rect_.bottom + EDGE)
		{
			// moving the band to the bottom row
			VerticalStack(*r, *s, limits_);
		}
		else if (!in_line)	// bands in different rows?
		{
			if (pos.y >= r->rect_.top && pos.y < r->rect_.bottom)
			{
				// move bands into same the row again

				//TODO:
				MoveBandIntoFront(*s, *r, limits_);

				s->rect_.top = r->rect_.top = limits_.top;
				int bottom= max(s->minimal_size_.cy, r->minimal_size_.cy);
				s->rect_.bottom = r->rect_.bottom = bottom;
			}
		}
		else
		{
			// check new location against soft limits imposed by second band

			if (r->rect_.left >= s->rect_.left)
			{
				// remaining band is second in a row
				s->rect_.left = limits_.left;

				if (pos.x > r->rect_.left)
					MoveBandIntoFront(*r, *s, limits_);
			}

			if (r->rect_.left < s->rect_.left)
			{
				// remaining band is first in a row
				if (p.x <= r->rect_.left)	// moving selected band into front positon?
				{
					MoveBandIntoFront(*s, *r, limits_);
				}
				else
				{
					// prevent first band from becoming too narrow
					p.x = max(p.x, limits_.left + r->minimal_size_.cx);

					r->rect_.left = limits_.left;	// just in case

					if (r->snap_to_ideal_size_)
					{
						const int DELTA= 12;
						int ideal= r->rect_.left + r->ideal_size_.cx;
						if (p.x >= ideal - DELTA && p.x <= ideal + DELTA)
							p.x = ideal;
					}

					s->rect_.left = r->rect_.right = p.x;

					s->rect_.right = limits_.right;
				}
			}

/*
			p.x = min<int>(p.x, limits_.right - (s->minimal_size_.cx + r->minimal_size_.cx));

			s->rect_.OffsetRect(p.x - s->rect_.left, 0);

			r->rect_.left = s->rect_.right;

			if (r->rect_.Width() < r->minimal_size_.cx)
				r->rect_.left = s->rect_.right = r->rect_.right - r->minimal_size_.cx;
*/
			//}
		}
	}

	if (band1_.wnd_ && org1 != band1_.rect_)
		SetBandWindowPos(band1_);

	if (band2_.wnd_ && org2 != band2_.rect_)
		SetBandWindowPos(band2_);

	return org1 != band1_.rect_ || org2 != band2_.rect_;
}


///////////////////////////////////////////////////////////////////

// SimpleReBar

SimpleReBar::SimpleReBar() : pImpl_(new Impl())
{
}

SimpleReBar::~SimpleReBar()
{
}


BEGIN_MESSAGE_MAP(SimpleReBar, CWnd)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_MESSAGE(WM_PRINTCLIENT, OnPrintClient)
	ON_WM_SETCURSOR()
END_MESSAGE_MAP()

// SimpleReBar message handlers


bool SimpleReBar::Create(CWnd* parent, CWnd* band1, CWnd* band2, const boost::function<void ()>& resize_callback,
						const boost::function<void (CDC& dc, int id, CRect band, bool in_line)>& erase_callback)
{
	pImpl_->erase_callback_ = erase_callback;

	if (!CWnd::Create(AfxRegisterWndClass(CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS, AfxGetApp()->LoadStandardCursor(IDC_ARROW), 0, 0),
		0, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, CRect(0,0,0,0), parent, -1))
		return false;

	pImpl_->resize_callback_ = resize_callback;

	pImpl_->SetBand(0, band1);
	pImpl_->SetBand(1, band2);

	if (band1)
	{
		band1->SetOwner(band1->GetParent());
		band1->SetParent(this);
		SetBandWindowPos(pImpl_->band1_);
	}
	if (band2)
	{
		band2->SetOwner(band2->GetParent());
		band2->SetParent(this);
		SetBandWindowPos(pImpl_->band2_);
	}

	return true;
}


void SimpleReBar::OnLButtonDown(UINT flags, CPoint pos)
{
	if (pImpl_->EnterMove(pos))
	{
		SetCapture();
		SetCursor();
	}
}


void SimpleReBar::OnLButtonUp(UINT flags, CPoint pos)
{
	if (pImpl_->StopMove(pos))
	{
		ReleaseCapture();
		SetCursor();
	}
}


void SimpleReBar::OnLButtonDblClk(UINT flags, CPoint pos)
{
	//TODO band resizing
}


void SimpleReBar::OnMouseMove(UINT flags, CPoint pos)
{
	int h= GetHeight();

	if (pImpl_->MoveBands(pos, (flags & MK_LBUTTON) != 0))
	{
		RedrawWindow();
		if (GetHeight() != h && pImpl_->resize_callback_)
			pImpl_->resize_callback_();
	}
}


void SimpleReBar::OnSize(UINT type, int cx, int cy)
{
	if (cx > 0 && cy > 0)
		pImpl_->Resize(0, 0, cx, cy, false);
}


static void DrawGrip(CDC* dc, const CRect& rect, COLORREF rgb_light, COLORREF rgb_dark)
{
	CRect grip= rect;
	dc->FillSolidRect(grip, rgb_light);
	grip.OffsetRect(-1, -1);
	dc->FillSolidRect(grip, rgb_dark);
}


static void DrawGripper(CDC& dc, CRect rect, COLORREF light, COLORREF dark)
{
	bool big= false;
	CRect grip(0, 0, 2, 2);

	// margins 3 & 3; first two dots take 3 pixels, subsequent dots take 6
	const int count= (rect.Height() - 3 - 3 - 3) / 6;
	const int offset= (rect.Height() - count * 6 - 3 + 1) / 2;

	grip.OffsetRect(rect.left + 3, rect.top + offset); //(big ? 5 : 4));
	//COLORREF rgb_light= RGB(90,90,90); //CalcShade(rgb_caption, 40.0f);
	//COLORREF rgb_dark= RGB(50,50,50); //CalcShade(rgb_caption, is_active ? -40.0f : -20.0f);

	DrawGrip(&dc, grip, light, dark);
	grip.OffsetRect(3, 3);

	for (int i= 0; i < count; ++i)
	{
		DrawGrip(&dc, grip, light, dark);
		grip.OffsetRect(-3, 3);
		DrawGrip(&dc, grip, light, dark);
		grip.OffsetRect(3, 3);

		if (rect.bottom - grip.bottom < 5)
			break;
	}
}


void SimpleReBar::OnPaint()
{
	CPaintDC dc(this);
}


void SimpleReBar::Paint(CDC& dc)
{
	CRect rect(0,0,0,0);
	GetClientRect(rect);

	dc.FillSolidRect(rect, RGB(0,0,0));

	bool in_line= BandsInLine(pImpl_->band1_, pImpl_->band2_);

	pImpl_->erase_callback_(dc, pImpl_->band1_.id_, pImpl_->band1_.rect_, in_line);
	pImpl_->erase_callback_(dc, pImpl_->band2_.id_, pImpl_->band2_.rect_, in_line);

	// draw grippers
	if (pImpl_->band1_.gripper_)
		DrawGripper(dc, pImpl_->band1_.rect_, pImpl_->light_grip_, pImpl_->dark_grip_);
	if (pImpl_->band2_.gripper_)
		DrawGripper(dc, pImpl_->band2_.rect_, pImpl_->light_grip_, pImpl_->dark_grip_);
}


BOOL SimpleReBar::OnEraseBkgnd(CDC* dc)
{
	Paint(*dc);
	return true;
}


int SimpleReBar::GetHeight()
{
	return pImpl_->GetHeight();
}


LRESULT SimpleReBar::OnPrintClient(WPARAM wdc, LPARAM flags)
{
	if (CDC* dc= CDC::FromHandle(HDC(wdc)))
		Paint(*dc);

	return 0;
}


BOOL SimpleReBar::OnSetCursor(CWnd* wnd, UINT hitTest, UINT message)
{
	SetCursor();
	return true;
}


void SimpleReBar::SetCursor()
{
	if (pImpl_->BandMoveCursor())
		::SetCursor(AfxGetApp()->LoadStandardCursor(IDC_SIZEWE));
	else
		::SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));
}


void SimpleReBar::RestoreLayout(const String& layout)
{
	if (layout.empty())
		return;

	iStringstream ist(layout);
	CSize size(0, 0);
	ist >> size.cx >> size.cy;

	if (size.cx == 0 || size.cy == 0)
		return;

	::RestoreLayout(pImpl_->band1_, ist);
	::RestoreLayout(pImpl_->band2_, ist);

	pImpl_->ValidateLayout(size);
}


String SimpleReBar::StoreLayout()
{
	oStringstream ost;
	CRect rect(0,0,0,0);
	GetClientRect(rect);

	if (rect.IsRectEmpty())
		return String();

	ost << rect.Width() << _T(" ") << rect.Height() << std::endl;

	ost << ::StoreLayout(pImpl_->band1_);
	ost << ::StoreLayout(pImpl_->band2_);

	return ost.str();
}


bool SimpleReBar::IsBandVisible(size_t index) const
{
	switch (index)
	{
	case 0:	return !!pImpl_->band1_.visible_;
	case 1:	return !!pImpl_->band2_.visible_;
	}
	return false;
}


void SimpleReBar::ShowBand(size_t index, bool visible)
{
	switch (index)
	{
	case 0:	pImpl_->band1_.visible_ = visible;	break;
	case 1:	pImpl_->band2_.visible_ = visible;	break;
	}

	//TODO: resize....???
//	pImpl_->Resize(0, 0, cx, cy);
}


void SimpleReBar::BandResized(CWnd& band_wnd, CSize size)
{
	CRect rect(0,0,0,0);
	GetClientRect(rect);
	pImpl_->BandWndChanged(&band_wnd, size, rect);
	Invalidate();
}


void SimpleReBar::SetGripperColors(COLORREF light, COLORREF dark)
{
	pImpl_->light_grip_ = light;
	pImpl_->dark_grip_ = dark;
	if (m_hWnd)
		Invalidate();
}

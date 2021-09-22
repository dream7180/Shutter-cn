/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// PreviewBandWnd.cpp : implementation file
//

#include "stdafx.h"
#include "../resource.h"
#include "PreviewBandWnd.h"
#include "../CoolScrollBar.h"
#include <math.h>
#include "../Block.h"
#include "BmpRoutines.h"
#include "../DrawBmpEffects.h"
#include "../BmpFunc.h"
#include "../CatchAll.h"
#include "../GetDefaultGuiFont.h"
using namespace std;

extern void DrawParentBkgnd(CWnd& wnd, CDC& dc);


static const int SBAR_SIZE= 20;	// height of horz scrollbar (derived from the height of a bitmap)
COLORREF background = RGB(25,25,25);


struct Item
{
	Item() : rect_(0,0,0,0), real_size_(0, 0), key_(0)
	{}
	Item(AnyPointer key, CSize size) : rect_(0,0,0,0), real_size_(size.cx, size.cy), key_(key)
	{}

	CRect rect_;
	CSize real_size_;
	AnyPointer key_;
};


struct PreviewBandWnd::Impl
{
	Impl()
	{
		current_item_ = NONE;
		clicked_item_ = NONE;
		valid_layout_ = false;
		horz_layout_ = false;
		img_pos_range_ = 0;
		aspect_ratios_ = make_pair(1.0, 1.0);
		first_item_idx = 0;
		balloons_enabled_ = true;
		first_tooltip_idx_ = 0;
		in_update_ = false;
		img_space_ = 0;
		draw_selection_ = true;
		selection_color_ = RGB(247, 123, 0);//::GetSysColor(COLOR_HIGHLIGHT);
		smooth_scroll_speed_ = 100;
		ui_gamma_ = 1.0;
		click_location_ = CPoint(0, 0);
		keep_selected_centered_ = false;
		reserved_vert_space_ = 0;
	}

	void AddItem(AnyPointer key, CSize size);
	bool ModifyItem(AnyPointer key, CSize size);
	void ModifyItem(size_t index, AnyPointer key, CSize size);
	void RemoveAll();
	void ReserveItems(size_t count)		{ items_.reserve(count); }

	bool ValidateLayout(CWnd* wnd);

	int GetRange() const				{ return img_pos_range_; }
	int GetImgSpace() const				{ return img_space_; }

	void SizeChanged()					{ valid_layout_ = false; }
	bool IsLayoutValid() const			{ return valid_layout_; }

	const Item* GetItem(size_t i) const	{ return i < items_.size() ? &items_[i] : 0; }

	pair<size_t, size_t> FindVisibleItems(CRect rect);
	size_t FindItemAt(int pos);
	size_t FindItemAt(CPoint pos)		{ return FindItemAt(horz_layout_ ? pos.x : pos.y); }

	Item* FindItem(AnyPointer key);
	size_t FindItemIndex(AnyPointer key);

	int GetScrollPos() const			{ return scrollbar_.GetScrollPos(horz_layout_ ? SB_HORZ : SB_VERT); }
	CPoint GetScrollOffset() const		{ int pos= GetScrollPos(); return horz_layout_ ? CPoint(pos, 0) : CPoint(0, pos); }
	void GetScrollInfo(SCROLLINFO& si) const	{ scrollbar_.GetScrollInfo(horz_layout_ ? SB_HORZ : SB_VERT, si); }
	void SetScrollInfo(SCROLLINFO& si, bool draw)	{ scrollbar_.SetScrollInfo(horz_layout_ ? SB_HORZ : SB_VERT, si, draw); }
	void SetScrollPosition(int pos, bool draw)	{ scrollbar_.SetScrollPos(horz_layout_ ? SB_HORZ : SB_VERT, pos, draw); }
	void ShowScrollBar(bool show)		{ scrollbar_.ShowScrollBar(horz_layout_ ? SB_HORZ : SB_VERT, show); }
	int GetLineScroll(CRect rect, bool up_left);
	int GetItemScrollMargin() const		{ return img_space_ * 3 / 4; }

	bool IsHorzLayout() const			{ return horz_layout_; }
	void SetOrientation(bool horizontal);

	bool InUpdate() const				{ return in_update_; }

	bool IsEmpty() const				{ return items_.empty(); }
	size_t GetCount() const				{ return items_.size(); }

	void PaintToBmp(Dib& bmp, CRect rect, CPoint scrl, bool scrollbar, bool draw_shadow);
	void SmoothScroll(CDC& dc, CRect rect, CSize delta, bool scrollbar, bool repeated, int speed, int x_pos, int x_old);

	//void ScrollToCenter(PreviewBandWnd& self, size_t photo);

	CoolScrollBar scrollbar_;	// our own cool scrollbar (customized)
	//Dib backgnd_;
	//Dib backgnd_top_;
	//Dib backgnd_bottom_;
	//Dib backgnd_vert_;
	//Dib backgnd_vert_right_;
	double ui_gamma_;
	size_t first_item_idx;
	CToolTipCtrl tool_tips_;
	bool balloons_enabled_;
	size_t first_tooltip_idx_;
	boost::function<void (size_t index, AnyPointer key, PreviewBandWnd::ClickAction action)> item_clicked_;
	boost::function<void (CDC& dc, CRect rect, size_t index, AnyPointer key)> draw_item_;
	boost::function<String (size_t index, AnyPointer key)> get_tooltip_text_;
	size_t current_item_;		// index of currently highlighted item
	size_t clicked_item_;
	bool draw_selection_;
	COLORREF selection_color_;
	int smooth_scroll_speed_;	// this is how long it takes to smooth scroll, lower values means faster scrolling
	CPoint click_location_;
	bool keep_selected_centered_;
	int reserved_vert_space_;

	static const size_t NONE= ~0;

private:
	int RecalcLayout(CRect area, bool horz, int extra_vert_space);
	void ItemChanged(AnyPointer key, CSize size);

	vector<Item> items_;
	bool horz_layout_;
	bool valid_layout_;
	int img_pos_range_;
	int img_space_;
	pair<double, double> aspect_ratios_;
	bool in_update_;
};


struct Rect
{
	Rect(CRect& rect, bool horz)
		: left(horz ? rect.left : rect.top), right(horz ? rect.right : rect.bottom),
		  top(horz ? rect.top : rect.left), bottom(horz ? rect.bottom : rect.right)
	{}

	LONG& left;
	LONG& right;
	LONG& top;
	LONG& bottom;

	LONG Height() const		{ return bottom - top; }
	LONG Width() const		{ return right - left; }
	CSize Size() const		{ return CSize(right - left, bottom - top); }
};


int PreviewBandWnd::Impl::GetLineScroll(CRect rect, bool up_left)
{
	if (IsEmpty())
		return 0;

	Rect r(rect, horz_layout_);

	pair<size_t, size_t> range= FindVisibleItems(rect);

	if (const Item* item= GetItem(up_left ? range.first : range.second - 1))
	{
		CRect it= item->rect_;
		Rect ri(it, horz_layout_);

		int delta= up_left ? r.left - ri.left : ri.right - r.right;

		if (delta <= 0)
		{
			if (const Item* item= GetItem(up_left ? range.first - 1 : range.second))
			{
				CRect it= item->rect_;
				Rect ri(it, horz_layout_);

				delta = up_left ? r.left - ri.left : ri.right - r.right;
			}
			else
				delta = it.Width();
		}

//TRACE("delta: %d\n", delta);

		return max<int>(1, delta + GetItemScrollMargin());
	}

	return 10;
}


void PreviewBandWnd::Impl::SetOrientation(bool horizontal)
{
	if (horizontal == horz_layout_)
		return;

	Block update(in_update_);

	valid_layout_ = false;

	SCROLLINFO org_si;
	org_si.cbSize = sizeof org_si;
	org_si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
	GetScrollInfo(org_si);

	{	// hide current scrollbar (horizontal if switching to vert layout, and vice versa)
		SCROLLINFO si;
		si.cbSize = sizeof si;
		si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
		si.nMin = 0;
		si.nMax = 1;
		si.nPage = 2;
		si.nPos = 0;
		SetScrollInfo(si, true);
	}

	ShowScrollBar(false);

	// the only bmp used by both vert and horz layout, so rotate it
	//backgnd_bottom_.RotateInPlace(horizontal);

	horz_layout_ = horizontal;

	// show new one
	ShowScrollBar(true);
	SetScrollInfo(org_si, true);
}


bool PreviewBandWnd::Impl::ValidateLayout(CWnd* wnd)
{
	if (valid_layout_)
		return false;

	CRect rect(0,0,0,0);
	wnd->GetClientRect(rect);
	if (rect.IsRectEmpty())
	{
		img_pos_range_ = 0;
		valid_layout_ = true;
		return true;
	}

	DWORD style= wnd->GetStyle();

	Rect r(rect, horz_layout_);
	if (style & (WS_HSCROLL | WS_VSCROLL))
		r.bottom += SBAR_SIZE;

	img_pos_range_ = RecalcLayout(rect, horz_layout_, reserved_vert_space_);

	// available space: in case of the centered image, this is simplified, we need slightly less than
	// half a window of space
	int available_space= keep_selected_centered_ ? r.Width() / 2 : r.Width();

	if (img_pos_range_ > available_space)	// items don't fit?
	{
		// reposition items taking out space for a scrollbar

		// due to the different sizes of items and different amount of space they take
		// depending on the available space iterate through the different sizes to find the optimal size
		for (int i= 0; i < SBAR_SIZE; ++i)
		{
			r.bottom--;
			img_pos_range_ = RecalcLayout(rect, horz_layout_, reserved_vert_space_);

			available_space = keep_selected_centered_ ? r.Width() / 2 : r.Width();

			if (img_pos_range_ <= available_space)
				break;
		}
	}

	valid_layout_ = true;

	return true;
}


static const double MAX_RATIO= 4.0 / 2.0;
static const double MIN_RATIO= 2.0 / 4.0;

void PreviewBandWnd::Impl::ItemChanged(AnyPointer key, CSize size)
{
	valid_layout_ = false;

	double ratio= 0.0;
	if (size.cx > 0 && size.cy > 0)
	{
		ratio = size.cx;
		ratio /= size.cy;
	}
	if (ratio < MIN_RATIO)
		ratio = MIN_RATIO;
	if (ratio > MAX_RATIO)
		ratio = MAX_RATIO;

	if (ratio < aspect_ratios_.first)
		aspect_ratios_.first = ratio;
	if (ratio > aspect_ratios_.second)
		aspect_ratios_.second = ratio;
}


void PreviewBandWnd::Impl::AddItem(AnyPointer key, CSize size)
{
	items_.push_back(Item(key, size));
	ItemChanged(key, size);
}


struct cmp_key
{
	cmp_key(AnyPointer key) : key_(key)
	{}

	bool operator () (const Item& item) const
	{
		return item.key_ == key_;
	}

	AnyPointer key_;
};


void PreviewBandWnd::Impl::ModifyItem(size_t index, AnyPointer key, CSize size)
{
	if (index < items_.size())
	{
		Item& item= items_[index];
		item.key_ = key;
		item.rect_.SetRectEmpty();
		item.real_size_ = size;

		ItemChanged(key, size);
	}
}


bool PreviewBandWnd::Impl::ModifyItem(AnyPointer key, CSize size)
{
	if (Item* item= FindItem(key))
	{
		item->real_size_ = size;

		aspect_ratios_ = make_pair(1.0, 1.0);
		const size_t count= items_.size();
		for (size_t i= 0; i < count; ++i)
			ItemChanged(items_[i].key_, items_[i].real_size_);

		return true;
	}
	return false;	// no such item
}


Item* PreviewBandWnd::Impl::FindItem(AnyPointer key)
{
	vector<Item>::iterator it= find_if(items_.begin(), items_.end(), cmp_key(key));
	return it != items_.end() ? &*it : 0;
}


size_t PreviewBandWnd::Impl::FindItemIndex(AnyPointer key)
{
	vector<Item>::iterator it= find_if(items_.begin(), items_.end(), cmp_key(key));
	return it != items_.end() ? distance(items_.begin(), it) : NONE;
}


void PreviewBandWnd::Impl::RemoveAll()
{
	items_.clear();	//TODO: swap with empty?
	valid_layout_ = false;
	aspect_ratios_ = make_pair(1.0, 1.0);
	first_item_idx = 0;
	current_item_ = NONE;
	clicked_item_ = NONE;
}


namespace {

	struct cmp_right_pos
	{
		cmp_right_pos(bool horz) : horz_(horz)
		{}

		bool operator () (const Item& item, int val) const
		{
			return horz_ ? item.rect_.right < val : item.rect_.bottom < val;
		}

	private:
		bool horz_;
	};

	struct cmp_left_pos
	{
		cmp_left_pos(bool horz) : horz_(horz)
		{}

		bool operator () (int val, const Item& item) const
		{
			return horz_ ? item.rect_.left > val : item.rect_.top > val;
		}

		bool operator () (const Item& item, int val) const
		{
			return horz_ ? item.rect_.left < val : item.rect_.top < val;
		}

	private:
		bool horz_;
	};

}


size_t PreviewBandWnd::Impl::FindItemAt(int pos)
{
	ASSERT(valid_layout_);

	vector<Item>::const_iterator it= upper_bound(items_.begin(), items_.end(), pos, cmp_left_pos(horz_layout_));

	return it - items_.begin() - 1;
}


pair<size_t, size_t> PreviewBandWnd::Impl::FindVisibleItems(CRect rect)
{
	ASSERT(valid_layout_);
	ASSERT(!IsEmpty());

	Rect r(rect, horz_layout_);

	vector<Item>::const_iterator from= lower_bound(items_.begin(), items_.end(), r.left, cmp_right_pos(horz_layout_));
	vector<Item>::const_iterator to= upper_bound(items_.begin(), items_.end(), r.right, cmp_left_pos(horz_layout_));

	// return item's range <from, to) - open on the right side

	return make_pair(from - items_.begin(), to - items_.begin());
}


CRect GetDisplayRect(CRect dest_rect, CSize bmp_size)
{
	CRect bmp_rect(0,0,0,0);

	if (bmp_size.cx < 1 || bmp_size.cy < 1)
		return bmp_rect;

	CSize dest_size= dest_rect.Size();
	if (dest_size.cx <= 0 || dest_size.cy <= 0)
		return bmp_rect;

	//double bmp_ratio= double(bmp_size.cx) / double(bmp_size.cy);
	//double dest_ratio= double(dest_size.cx) / double(dest_size.cy);
	//double epsillon= 0.01;

	//// compare bmp ratio and destination ratio; if same, bmp will be simply scalled
	//bool simple_rescaling= fabs(bmp_ratio - dest_ratio) < epsillon;

	// calc how to rescale bmp to fit into dest rect
	double scaleW= double(dest_size.cx) / double(bmp_size.cx);
	double scaleH= double(dest_size.cy) / double(bmp_size.cy);

	double scale= min(scaleW, scaleH);

	// rescale bmp (but prevent it from collapsing to 0 size)
	bmp_size.cx = max(static_cast<LONG>(bmp_size.cx * scale), 1L);
	bmp_size.cy = max(static_cast<LONG>(bmp_size.cy * scale), 1L);

	ASSERT(bmp_size.cx <= dest_size.cx);
	ASSERT(bmp_size.cy <= dest_size.cy);
	CSize diff_size= dest_size - bmp_size;

	// center rescalled bitmap in destination rect
	CPoint pos(diff_size.cx / 2, diff_size.cy / 2);

	bmp_rect = CRect(pos, bmp_size);
	bmp_rect.OffsetRect(dest_rect.TopLeft());

	return bmp_rect;
}


int PreviewBandWnd::Impl::RecalcLayout(CRect areaRect, bool horz, int extra_vert_space)
{
	Rect area(areaRect, horz);

	// available size
	const int size= area.Height() - (horz ? extra_vert_space : 0);

	// left or top space
	const int TOP= 4;
	// right or bottom space
	const int BOTTOM= 4;

	const int IMG_SIZE= max(size - TOP - BOTTOM, 8);

	// space between images (grows slowly with size: sqrt(size))
	const int SPACE= max(4, static_cast<int>(sqrt(double(IMG_SIZE)) + 0.5));

	img_space_ = SPACE;

	// space for an image
	CRect img_rect(CPoint(0, TOP), CSize(IMG_SIZE, IMG_SIZE));

	// this sophisticated ratio calculation is needed to preserve relative size of
	// landscape and portrait images
	double opt_size= aspect_ratios_.first * aspect_ratios_.second;
	if (!horz)
		opt_size = 1.0 / opt_size;

	img_rect.right = img_rect.left + static_cast<LONG>(img_rect.Width() * opt_size + 0.49);

	int from= area.left + (SPACE * 3 / 4);

	const size_t count= items_.size();
	for (size_t i= 0; i < count; ++i)
	{
		Item& item= items_[i];

		item.rect_ = CRect(CPoint(0, 0), item.real_size_);

		Rect r(item.rect_, horz);

		CRect dest_size= GetDisplayRect(img_rect, r.Size());

		r.left = from;
		r.right = r.left + dest_size.Width();
		r.top = dest_size.top;
		r.bottom = dest_size.bottom;

		from += r.Width() + SPACE;

		if (!horz)
			from += extra_vert_space;
	}

	int range= from - SPACE / 4;

	return range;
}


///////////////////////////////////////////////////////////////////////////////////////////////////


PreviewBandWnd::PreviewBandWnd() : pImpl_(new Impl)
{
}

PreviewBandWnd::~PreviewBandWnd()
{
}


BEGIN_MESSAGE_MAP(PreviewBandWnd, CWnd)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_MESSAGE(WM_PRINTCLIENT, OnPrintClient)
	ON_WM_DESTROY()
	ON_NOTIFY_RANGE(CoolScrollBar::COOLSB_CUSTOMDRAW, 0, 0xffff, OnCustDrawScrollBar)
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXT, 0, 0xFFFF, OnToolTipNotify)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()


void PreviewBandWnd::SetUIBrightness(double gamma)
{
	//pImpl_->backgnd_.Load(IDB_PREVIEW_BACKGND);
	//pImpl_->backgnd_top_.Load(IDB_PREVIEW_BACKGND_TOP);
	//pImpl_->backgnd_bottom_.Load(IDB_PREVIEW_BACKGND_BTM);
	//pImpl_->backgnd_vert_.Load(IDB_PREVIEW_BACKGND_VERT);
	//pImpl_->backgnd_vert_right_.Load(IDB_PREVIEW_BACKGND_VERT_R);

	// the only bmp used by both vert and horz layout, so rotate it
	//if (!pImpl_->IsHorzLayout())
		//pImpl_->backgnd_bottom_.RotateInPlace(false);

	if (gamma != 1.0)
	{
		pImpl_->scrollbar_.SetImages(IDB_PREVIEW_SCRL_BAR, IDB_PREVIEW_SCRL_BAR_V, 0, gamma);
		//::ApplyGammaInPlace(&pImpl_->backgnd_, gamma, -1, -1);
		//::ApplyGammaInPlace(&pImpl_->backgnd_top_, gamma, -1, -1);
		//::ApplyGammaInPlace(&pImpl_->backgnd_bottom_, gamma, -1, -1);
		//::ApplyGammaInPlace(&pImpl_->backgnd_vert_, gamma, -1, -1);
		//::ApplyGammaInPlace(&pImpl_->backgnd_vert_right_, gamma, -1, -1);
	}

	Invalidate(false);
}


bool PreviewBandWnd::Create(CWnd* parent)
{
	if (!CWnd::Create(AfxRegisterWndClass(CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS, AfxGetApp()->LoadStandardCursor(IDC_ARROW), 0, 0),
		0, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, CRect(0,0,0,0), parent, 0x7ff0))
		return false;

	// just in case...
	ModifyStyleEx(WS_EX_LEFTSCROLLBAR, 0);

	SetUIBrightness(pImpl_->ui_gamma_);

	// ------------------- scrollbar ----------------------

	if (!pImpl_->scrollbar_.Create(*this, true))
		return false;

	static const int parts[]= { 14, 1, 14, 10, 1, 10 };

	vector<int> p(parts, parts + array_count(parts));

	pImpl_->scrollbar_.LoadImages(IDB_PREVIEW_SCRL_BAR, IDB_PREVIEW_SCRL_BAR_V, 0, p, pImpl_->ui_gamma_);

	pImpl_->scrollbar_.SetHotTrack(SB_BOTH, true);

	// for custom draw scrollbars set sizes
	pImpl_->scrollbar_.SetSize(SB_BOTH, parts[0], SBAR_SIZE);
	pImpl_->scrollbar_.SetMinThumbSize(SB_BOTH, 10 + 1 + 10);

	SCROLLINFO si;
	si.cbSize = sizeof si;
	si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
	si.nMin = 0;
	si.nMax = 1;
	si.nPage = 2;
	si.nPos = 0;
	pImpl_->SetScrollInfo(si, true);

	// ------------------- tooltips ----------------------

	if (!pImpl_->tool_tips_.Create(this, WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP | TTS_BALLOON))
		return false;

	TOOLINFO ti;
	ti.cbSize = sizeof ti;
	ti.hwnd   = *this;
	ti.uId    = UINT_PTR(ti.hwnd);
	ti.lpszText = const_cast<TCHAR*>(_T(""));
	ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
	ti.rect   = CRect(0,0,0,0);
	ti.hinst  = NULL;
	ti.lParam = 0;
	pImpl_->tool_tips_.SendMessage(TTM_ADDTOOL, 0, LPARAM(&ti));

	pImpl_->tool_tips_.SetDelayTime(TTDT_AUTOPOP, 15000);	// 15 sec.
	pImpl_->tool_tips_.SetMaxTipWidth(300);

	return true;
}


void PreviewBandWnd::ValidateLayout()
{
	if (pImpl_->ValidateLayout(this))
		ResetToolTips();
}


void PreviewBandWnd::ResetToolTips()
{
	if (pImpl_->tool_tips_.m_hWnd == 0)
		return;

	{
		const int count= pImpl_->tool_tips_.GetToolCount();
		for (int index= 0; index < count; ++index)
			pImpl_->tool_tips_.DelTool(this, index + 1);
	}

	if (!pImpl_->balloons_enabled_)
		return;

	if (pImpl_->IsEmpty())
		return;

	//ValidateLayout();

	CRect rect(0,0,0,0);
	GetClientRect(rect);

	if (rect.IsRectEmpty())
		return;

	CPoint scrl= pImpl_->GetScrollOffset();
	rect.OffsetRect(scrl);
	pair<size_t, size_t> range= pImpl_->FindVisibleItems(rect);

	pImpl_->first_tooltip_idx_ = range.first;

	int index= 1;
	for (size_t i= range.first; i < range.second; ++i)
	{
		if (const Item* item= pImpl_->GetItem(i))
		{
			CRect frm= item->rect_;
			frm.OffsetRect(-scrl);

			pImpl_->tool_tips_.AddTool(this, LPSTR_TEXTCALLBACK, frm, index++);
		}
	}
}


BOOL PreviewBandWnd::OnEraseBkgnd(CDC* dc)
{
	try
	{
		if (pImpl_->IsEmpty())
		{
			ValidateLayout();

			CRect rect(0,0,0,0);
			GetClientRect(rect);
			CPoint scrl= pImpl_->GetScrollOffset();

			DWORD style= GetStyle();
			bool scrollbar= !!(style & (WS_HSCROLL | WS_VSCROLL));

			Dib bmp;
			pImpl_->PaintToBmp(bmp, rect, scrl, scrollbar, true);

			bmp.Draw(dc, CPoint(0, 0));
		}
	}
	catch (...)
	{}

	return true;
}


void PreviewBandWnd::OnPaint()
{
	CPaintDC paint_dc(this);
	try
	{
		Paint(paint_dc);
	}
	catch (...)
	{}
}


static const int SHADOW_H= 5;		// height of drop shadow
static const float SHADOW_OP= 0.3f;	// shadow's opacity
static const int SHADOW_DIR= 2;		// shadow's direction

void PreviewBandWnd::Paint(CDC& paint_dc)
{
	ValidateLayout();

	if (pImpl_->IsEmpty())
		return;

	CRect rect(0,0,0,0);
	GetClientRect(rect);

	CPoint scrl= pImpl_->GetScrollOffset();

	DWORD style= GetStyle();
	bool scrollbar= !!(style & (WS_HSCROLL | WS_VSCROLL));

	Dib bmp;
	pImpl_->PaintToBmp(bmp, rect, scrl, scrollbar, true);

	bmp.Draw(&paint_dc, CPoint(0, 0));
//	paint_dc.BitBlt(0, 0, bmp.GetWidth(), bmp.GetHeight(), &dc, 0, 0, SRCCOPY);
}


void PreviewBandWnd::Impl::PaintToBmp(Dib& bmp, CRect rect, CPoint scrl, bool scrollbar, bool draw_shadow)
{
	CRect shadow_rect= rect;

	if (rect.IsRectEmpty())
		return;

	bmp.Create(rect.Width(), rect.Height(), 32);

	CDC dc;
	dc.CreateCompatibleDC(0);

	CBitmap* oldBmp= dc.SelectObject(bmp.GetBmp());

	// draw background
	if (IsHorzLayout())
	{
		CRect t= rect;
		//t.bottom = t.top + backgnd_top_.GetHeight();
		//backgnd_top_.Draw(&dc, t);

		//CRect r= rect;
		//r.top = t.bottom;
		//if (!scrollbar)
		//	r.bottom -= SBAR_SIZE;
		//if (t.Height() > 0)
			//backgnd_.Draw(&dc, r);
			dc.FillSolidRect(t, background);

		/*if (!scrollbar)
		{
			CRect b= rect;
			b.top = b.bottom - SBAR_SIZE;
			//backgnd_bottom_.Draw(&dc, b);
		}*/
	}
	else	// vertical layout
	{
		CRect t= rect;
		if (!scrollbar)
			t.right -= SBAR_SIZE;
		//backgnd_vert_.Draw(&dc, t);
		dc.FillSolidRect(t, background);
		t.bottom = t.top + SHADOW_H;
		shadow_rect = t;

		if (!scrollbar)
		{
			CRect r= rect;
			r.left = rect.right - SBAR_SIZE;
			if (r.Width() > 0)
			{
				//backgnd_bottom_.Draw(&dc, r);
				dc.FillSolidRect(r, background);
				//r.bottom = r.top + backgnd_vert_right_.GetHeight();
				//backgnd_vert_right_.Draw(&dc, r);
			}
		}
	}

	if (!IsEmpty())
	{
		dc.SelectObject(&GetDefaultGuiFont());//&_font);
		//dc.SelectStockObject(DEFAULT_GUI_FONT);
		dc.SetBkMode(TRANSPARENT);

//		CPoint scrl= GetScrollOffset();
		rect.OffsetRect(scrl);
		pair<size_t, size_t> range= FindVisibleItems(rect);

		for (size_t i= range.first; i < range.second; ++i)
		{
			if (const Item* item= GetItem(i))
			{
				CRect frm= item->rect_;
				frm.OffsetRect(-scrl);

				const int EFFECT_SIZE= 4;

				if (i == current_item_ && draw_selection_)	// selected item?
				{
					// draw selection
					DrawGlowEffect(bmp, frm, selection_color_, 1.0f, EFFECT_SIZE + 1, true);
				}
				else
				{
					// normal item
					CRect rect= frm;
					rect.InflateRect(0, 0, EFFECT_SIZE, EFFECT_SIZE);
					DrawBlackShadow(bmp, rect, 0.45f, CSize(0, 0), EFFECT_SIZE);
				}

				if (draw_item_)
					draw_item_(dc, frm, i, item->key_);
			}
			else
			{
				ASSERT(false);
				break;
			}
		}
	}

	if (!IsHorzLayout() && draw_shadow)
	{
		// add drop shadow at the top of window
		DrawBlackGradient(bmp, shadow_rect, SHADOW_OP, SHADOW_DIR);
	}

//	paint_dc.BitBlt(0, 0, bmp.GetWidth(), bmp.GetHeight(), &dc, 0, 0, SRCCOPY);

	dc.SelectObject(oldBmp);
}


LRESULT PreviewBandWnd::OnPrintClient(WPARAM wdc, LPARAM flags)
{
	if (CDC* dc= CDC::FromHandle(HDC(wdc)))
	{
		if (pImpl_->IsEmpty())
			OnEraseBkgnd(dc);
		else
			DrawParentBkgnd(*this, *dc);
	}

	return 0;
}


void PreviewBandWnd::OnSize(UINT type, int cx, int cy)
{
	if (cx > 0 && cy > 0)
	{
		if (!pImpl_->InUpdate())
		{
			pImpl_->SizeChanged();
			ResetScrollBar(true);
		}
	}
}


void PreviewBandWnd::OnDestroy()
{
	pImpl_->scrollbar_.Delete();
}


void PreviewBandWnd::OnCustDrawScrollBar(UINT id, NMHDR* hdr, LRESULT* result)
{
	*result = pImpl_->scrollbar_.HandleCustomDraw(hdr);
}


void PreviewBandWnd::RemoveAllItems()
{
	pImpl_->RemoveAll();
}


void PreviewBandWnd::ReserveItems(size_t count)
{
	pImpl_->ReserveItems(count);
}


void PreviewBandWnd::AddItem(CSize size, AnyPointer key)
{
	pImpl_->AddItem(key, size);
}


void PreviewBandWnd::ModifyItem(CSize size, AnyPointer key)
{
	if (pImpl_->ModifyItem(key, size))
	{
		Invalidate();
		ResetScrollBar(false);
	}
}


void PreviewBandWnd::ModifyItem(size_t index, CSize size, AnyPointer key)
{
	pImpl_->ModifyItem(index, key, size);
}


void PreviewBandWnd::ResetScrollBar(bool move_pos)
{
	static bool in_update= false;
	if (in_update)
		return;

	Block update(in_update);

	ValidateLayout();

	int range= pImpl_->GetRange();

	CRect rect(0,0,0,0);
	GetClientRect(rect);
	if (rect.IsRectEmpty())
		return;

	int spos= pImpl_->GetScrollPos();

	int pos= 0;
	if (const Item* item= pImpl_->GetItem(pImpl_->first_item_idx))
		pos = (pImpl_->IsHorzLayout() ? item->rect_.left : item->rect_.top) - pImpl_->GetImgSpace() + 1;

	int left= 0, right= 0;
	int page= pImpl_->IsHorzLayout() ? rect.Width() : rect.Height();

	if (pImpl_->keep_selected_centered_ && !pImpl_->IsEmpty())
	{
		// when option to keep selected item centered is on, add extra space for scrolling
		// to the left and right, such that first and last image can be centered too

		const Item* first= pImpl_->GetItem(0);
		const Item* last= pImpl_->GetItem(pImpl_->GetCount() - 1);

		ASSERT(first && last);

		int half_page= page / 2;
		int space= pImpl_->GetImgSpace();

		if (pImpl_->IsHorzLayout())
		{
			left = half_page - first->rect_.Width() / 2 - space;
			right = half_page - last->rect_.Width() / 2 - space;
		}
		else
		{
			left = half_page - first->rect_.Height() / 2;
			right = half_page - last->rect_.Height() / 2;
		}
	}

	SCROLLINFO si;
	si.cbSize = sizeof si;
	si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
	si.nMin = 0 - left;
	si.nMax = range - 1 + right;
	si.nPage = page;
	si.nPos = page < range ? pos : 0;

	if (!move_pos)
		si.nPos = page < range ? min(spos, range - page) : 0;
		//si.fMask &= ~SIF_POS;

	pImpl_->SetScrollInfo(si, true);
}


void PreviewBandWnd::OnVScroll(UINT sb_code, UINT pos, CScrollBar* scroll_bar)
{
	try
	{
		OnScroll(sb_code, pos, pImpl_->smooth_scroll_speed_);
	}
	catch (...)
	{}
}


void PreviewBandWnd::OnHScroll(UINT sb_code, UINT pos, CScrollBar* scroll_bar)
{
	try
	{
		OnScroll(sb_code, pos, pImpl_->smooth_scroll_speed_);
	}
	catch (...)
	{}
}


void PreviewBandWnd::OnScroll(UINT scroll_code, UINT pos, int speed)
{
	ValidateLayout();

	// calc new position
	int x= pImpl_->GetScrollPos();
	int xOrig= x;

	SCROLLINFO si;
	si.cbSize = sizeof si;
	si.fMask = SIF_RANGE | SIF_PAGE | SIF_TRACKPOS;
	pImpl_->GetScrollInfo(si);

	CRect rect(0,0,0,0);
	GetClientRect(rect);
	CPoint scrl= pImpl_->GetScrollOffset();
	rect.OffsetRect(scrl);

	switch (scroll_code & 0xff)
	{
	case SB_TOP:
		x = 0;
		break;
	case SB_BOTTOM:
		x = si.nMax;
		break;
	case SB_LINEUP:
		x -= pImpl_->GetLineScroll(rect, true);
		break;
	case SB_LINEDOWN:
		x += pImpl_->GetLineScroll(rect, false);
		break;
	case SB_PAGEUP:
		x -= si.nPage;
		break;
	case SB_PAGEDOWN:
		x += si.nPage;
		break;
	case SB_THUMBTRACK:
		{
			speed = 0;
			//x = pos;
			x = si.nTrackPos;
		}
		break;
	}

	bool repeated= !!(scroll_code & 0x100);
	if (OnScrollBy(x - xOrig, speed, repeated))
		UpdateWindow();
}


bool PreviewBandWnd::OnScrollBy(int scroll, int speed, bool repeated)
{
	int xOrig, x;

	// adjust current position
	xOrig = x = pImpl_->GetScrollPos();
	int xMax= 0;
	SCROLLINFO si;
	si.cbSize = sizeof si;
	si.fMask = SIF_RANGE | SIF_PAGE;
	pImpl_->GetScrollInfo(si);
	xMax = si.nMax - si.nPage + 1;

	x += scroll;
	if (x < si.nMin)
		x = si.nMin;
	else if (x > xMax)
		x = xMax;

	// has something changed?
	if (x == xOrig)
		return false;

	int delta= -(x - xOrig);
	int rest= 0;

	// do scroll and update scroll positions

	if (speed > 0)	// smooth scroll?
	{
		// if smooth-scrolling large amount (more than a page), quick scroll at first
		// and only then finish with smooth scrolling one page worth of content

		if (abs(delta) > si.nPage)
		{
			int page= static_cast<int>(si.nPage);
			rest = delta > 0 ? page : -page;
			delta -= rest;
		}
		else
		{
			rest = delta;
			delta = 0;
		}
	}

	if (delta != 0)
	{
		if (pImpl_->IsHorzLayout())
		{
			ScrollWindow(delta, 0);
		}
		else
		{
			//HACK: shadow on the top must not be scrolled
			CRect rect(0,0,0,0);
			GetClientRect(rect);
			rect.bottom = rect.top + SHADOW_H;
			InvalidateRect(rect);

			ScrollWindow(0, delta);
		}
	}

	if (speed > 0 && rest != 0)	// smooth scroll?
	{
		if (delta != 0)
		{
			// record new position after partial scroll has been done
			pImpl_->SetScrollPosition(x + rest, true);
		}

		CRect rect(0,0,0,0);
		GetClientRect(rect);
		CClientDC dc(this);
		CSize delta_scr= pImpl_->IsHorzLayout() ? CSize(rest, 0) : CSize(0, rest);
		pImpl_->SmoothScroll(dc, rect, delta_scr, true, repeated, speed, x, xOrig);
	}

	pImpl_->SetScrollPosition(x, true);
	pImpl_->first_item_idx= pImpl_->FindItemAt(x);

	ResetToolTips();

	return true;
}


static int linear(int x, int steps, int delta)
{
	// for repeated scroll use linear function for uniform move
	double pos= double(x) / steps;
	double fr= pos;
	return static_cast<int>(delta * fr + (delta > 0.0 ? 0.5 : -0.5));
}

static int accelerated(int x, int steps, int delta)
{
	// for single scroll use accelerated move with slow down at the end
	double pos= 1.0 - double(x) / steps;
// circle:	double fr= sqrt(1.0 - pos * pos);
	double fr= 1.0 - pow(pos, 2.6);
	// double fr= pos * pos * pos + 1.0;
	//	double fr= -pos * pos + 1.0;
	return static_cast<int>(delta * fr + (delta > 0.0 ? 0.5 : -0.5));
}


void PreviewBandWnd::Impl::SmoothScroll(CDC& dc, CRect rect, CSize deltaScr, bool scrollbar, bool repeated, int speed, int x_pos, int x_old)
{
	CPoint scrl= GetScrollOffset();

	bool horz= deltaScr.cx != 0;
	bool shadow= !horz;
	int delta= horz ? deltaScr.cx : deltaScr.cy;

	if (deltaScr.cx < 0)
		rect.right -= deltaScr.cx;
	else
	{
		rect.right += deltaScr.cx;
		scrl.x -= deltaScr.cx;
	}

	if (deltaScr.cy < 0)
		rect.bottom -= deltaScr.cy;
	else
	{
		rect.bottom += deltaScr.cy;
		scrl.y -= deltaScr.cy;
	}

	Dib bmp;
	PaintToBmp(bmp, rect, scrl, scrollbar, false);
	Dib tmp;
	if (shadow)
		tmp.Clone(bmp);

	if (speed < 1)
		speed = 1;

	// time budget for smooth scrolling; it depends on a distance to scroll to look right
	// for repeated scrolling minimize time budget to speed up scrolling
	int budget= (repeated ?
			4 + static_cast<int>(pow(double(abs(delta)), 0.3)) :
			8 + static_cast<int>(pow(double(abs(delta)), 0.5))	) * speed;

//TRACE("steps: %d  rep: %d\n", STEPS, repeated ? 1 : 0);

	// for repeated scroll use linear function for uniform move;
	// for single scroll use accelerated move with slow down at the end
	int (*fn)(int x, int steps, int delta)= repeated ? linear : accelerated;

//TRACE("steps: %d   ", delta);
	//int dist= 999999;
	int steps= 100;
	int calc= 5;	// calculate number of steps three times, selecting largest value, the reason
	// to repeat calculation has to do with the fact that thumbnail decoding may throw off timing
//DWORD start_s= ::GetTickCount();
	int estimated_steps= -1;

	for (int x= 1; x < steps; ++x)
	{
		DWORD start= ::GetTickCount();

		int step= fn(x, steps, delta);

		int dist= delta > 0 ? step - delta : step;
//TRACE(" %d", step);

		CPoint pos= horz ? CPoint(dist, 0) : CPoint(0, dist);

		if (shadow)
		{
			CRect shadow_rect= rect;
			shadow_rect.top -= dist;
			shadow_rect.bottom = shadow_rect.top + SHADOW_H;
			DrawBlackGradient(bmp, shadow_rect, SHADOW_OP, SHADOW_DIR);
		}

		bmp.Draw(&dc, pos);

		::GdiFlush();

		if (shadow)
			memcpy(bmp.GetBuffer(), tmp.GetBuffer(), bmp.GetBufferSize());

		if (step == delta)
			break;

		int scrl_pos= x_old + fn(x, steps, x_pos - x_old);
		SetScrollPosition(scrl_pos, true);

		::Sleep(15);	// sleep for 15 ms, since this is about the resolution of GetTickCount timer

		DWORD time= ::GetTickCount() - start;
//TRACE(L"%d, ", time);

		if (calc > 0 && time > 0)
		{
			--calc;

			int est_steps= time < budget ? budget / static_cast<int>(time) : 0;

			if (estimated_steps < est_steps)
			{
				estimated_steps = est_steps;
				steps = estimated_steps;
			}
		}
	}

//DWORD tot= ::GetTickCount() - start_s;
//TRACE(L"\ntotal scrolling time: %d ms (%d steps, %d budget)\n", tot, steps, budget);
	//TODO: verify last step was drawn correctly for linear fn

	//if (delta > 0)
	//{
	//	if (dist != 0)
	//		bmp.Draw(&dc, CPoint(0, 0));
	//}
	//else
	//{
	//	if (dist != delta)
	//		bmp.Draw(&dc, horz ? CPoint(delta, 0) : CPoint(0, delta));
	//}
//TRACE("\n");
}


BOOL PreviewBandWnd::OnToolTipNotify(UINT id, NMHDR* hdr, LRESULT* result)
{
	NMTTDISPINFO* TTT= reinterpret_cast<NMTTDISPINFO*>(hdr);

	if (pImpl_->balloons_enabled_)
	{
		try
		{
			static String buff_;
			size_t index= TTT->hdr.idFrom - 1 + pImpl_->first_tooltip_idx_;
			if (const Item* item= pImpl_->GetItem(index))
			{
				if (pImpl_->get_tooltip_text_)
					buff_ = pImpl_->get_tooltip_text_(index, item->key_);

				TTT->lpszText = const_cast<TCHAR*>(buff_.c_str());
			}
			else
			{
				ASSERT(false);
			}
		}
		catch (...)
		{}
		TTT->szText[0] = 0;
		TTT->hinst = NULL;
	}
	else
	{
		TTT->lpszText = TTT->szText;
		TTT->szText[0] = 0;
	}

	*result = 0;
	return TRUE;
}


void PreviewBandWnd::SetOrientation(bool horizontal)
{
	pImpl_->SetOrientation(horizontal);
}


bool PreviewBandWnd::IsHorizontalOrientation()
{
	return pImpl_->IsHorzLayout();
}


void PreviewBandWnd::OnMouseMove(UINT flags, CPoint pos)
{
	if ((flags & MK_LBUTTON) != 0 && pImpl_->clicked_item_ != Impl::NONE)
	{
		ValidateLayout();
		CPoint scrl= pImpl_->GetScrollOffset();
		pos += scrl;

		CRect rect(pImpl_->click_location_, pImpl_->click_location_);
		rect.InflateRect(5, 5);

		// ignore small moves
		if (!rect.PtInRect(pos))
		{
			if (const Item* item= pImpl_->GetItem(pImpl_->clicked_item_))
			{
				try
				{
					if (pImpl_->item_clicked_)
						pImpl_->item_clicked_(pImpl_->clicked_item_, item->key_, StartDragDrop);

					Invalidate();	// this is a hack to redraw indicators (if any; view pane & light table ones)
				}
				CATCH_ALL
				{}
			}
		}
	}
}


void PreviewBandWnd::OnLButtonUp(UINT flags, CPoint pos)
{
	ValidateLayout();
	CPoint scrl= pImpl_->GetScrollOffset();
	pos += scrl;
	pImpl_->click_location_ = CPoint(0, 0);
	size_t index= pImpl_->FindItemAt(pos);
	if (const Item* item= pImpl_->GetItem(index))
	{
		// mouse btn released over the same item (the clicked one)?
		if (item->rect_.PtInRect(pos) && pImpl_->clicked_item_ == index)
		{
			pImpl_->current_item_ = index;

			if (pImpl_->draw_selection_)
				RedrawWindow();

			// send item selected notification
			if (pImpl_->item_clicked_)
				pImpl_->item_clicked_(index, item->key_, MouseBtnReleased);
		}
	}

	pImpl_->clicked_item_ = Impl::NONE;
}


void PreviewBandWnd::OnLButtonDown(UINT flags, CPoint pos)
{
	ValidateLayout();
	CPoint scrl= pImpl_->GetScrollOffset();
	pos += scrl;
	pImpl_->click_location_ = pos;
	size_t index= pImpl_->FindItemAt(pos);
	if (const Item* item= pImpl_->GetItem(index))
	{
		if (item->rect_.PtInRect(pos))// && pImpl_->current_item_ != index)
		{
			pImpl_->clicked_item_ = index;

			//if (pImpl_->draw_selection_)
			//	RedrawWindow();

			// send item selected notification
			if (pImpl_->item_clicked_)
				pImpl_->item_clicked_(index, item->key_, MouseBtnPressed);
		}
	}
}


void PreviewBandWnd::SetItemDrawCallback(
		const boost::function<void (CDC& dc, CRect rect, size_t index, AnyPointer key)>& draw_item)
{
	pImpl_->draw_item_ = draw_item;
}

void PreviewBandWnd::SetCallBacks(
		const boost::function<void (size_t index, AnyPointer key, ClickAction action)>& item_clicked,
		const boost::function<void (CDC& dc, CRect rect, size_t index, AnyPointer key)>& draw_item,
		const boost::function<String (size_t index, AnyPointer key)>& get_tooltip_text)
{
	pImpl_->item_clicked_ = item_clicked;
	pImpl_->draw_item_ = draw_item;
	pImpl_->get_tooltip_text_ = get_tooltip_text;
}


void PreviewBandWnd::EnableToolTips(bool enable)
{
	pImpl_->balloons_enabled_ = enable;

	if (pImpl_->IsLayoutValid())
		ResetToolTips();
	else
		ValidateLayout();	// reset tooltips after validating layout
}


void PreviewBandWnd::SelectionVisible(size_t current, bool smooth_scroll)
{
	bool center_current = pImpl_->keep_selected_centered_;

	ValidateLayout();

	CRect rect(0,0,0,0);
	GetClientRect(rect);

	Invalidate();

	if (!pImpl_->IsEmpty())
	{
		CPoint scrl= pImpl_->GetScrollOffset();
		rect.OffsetRect(scrl);

		if (const Item* item= pImpl_->GetItem(current))
		{
			pImpl_->current_item_ = current;

			CRect itm(item->rect_);
			Rect r(itm, pImpl_->IsHorzLayout());
			Rect wnd(rect, pImpl_->IsHorzLayout());
			int delta= 0;

			if (center_current)
				delta = (r.left + r.right) / 2 - (wnd.left + wnd.right) / 2;
			else if (r.left < wnd.left)
				delta = r.left - wnd.left - pImpl_->GetItemScrollMargin();
			else if (r.right > wnd.right)
				delta = r.right - wnd.right + pImpl_->GetItemScrollMargin();

			if (delta != 0)
				OnScrollBy(delta, smooth_scroll ? pImpl_->smooth_scroll_speed_ / 2 : 0, false);

			Invalidate();
		}
		else
		{
			ASSERT(false);	// wrong index passed to the current item
			pImpl_->current_item_ = Impl::NONE;
		}
	}
}


void PreviewBandWnd::RedrawSelection()
{
//	RedrawWindow();
	UpdateWindow();
}


void PreviewBandWnd::RedrawItem(AnyPointer key)
{
	//TODO:
	RedrawWindow();
}


void PreviewBandWnd::ScrollLeft(int speed_up_factor)
{
	OnScroll(SB_LINEUP | 256, 0, speed_up_factor * pImpl_->smooth_scroll_speed_);
}

void PreviewBandWnd::ScrollRight(int speed_up_factor)
{
	OnScroll(SB_LINEDOWN | 256, 0, speed_up_factor * pImpl_->smooth_scroll_speed_);
}

size_t PreviewBandWnd::GetItemCount() const
{
	return pImpl_->GetCount();
}


void PreviewBandWnd::EnableSelectionDisp(bool enable)
{
	pImpl_->draw_selection_ = enable;
	Invalidate();
}


CSize PreviewBandWnd::GetItemsSize(AnyPointer key)
{
	ValidateLayout();

	if (Item* item= pImpl_->FindItem(key))
		return item->rect_.Size();

	ASSERT(false);	// no such item
	return CSize(0, 0);
}


CRect PreviewBandWnd::GetItemRect(AnyPointer key)
{
	ValidateLayout();

	if (Item* item= pImpl_->FindItem(key))
		return item->rect_;

	ASSERT(false);	// no such item
	return CRect(0, 0, 0, 0);
}


bool PreviewBandWnd::IsItemVisible(AnyPointer key) const
{
	if (!pImpl_->IsLayoutValid())
		return false;

	if (Item* item= pImpl_->FindItem(key))
	{
		CRect rect(0,0,0,0);
		GetClientRect(rect);

		CPoint scrl= pImpl_->GetScrollOffset();
	
		rect.OffsetRect(scrl);

		return !(item->rect_ & rect).IsRectEmpty();
	}

	ASSERT(false);	// no such item
	return false;
}


void PreviewBandWnd::SetSelectionColor(COLORREF color)
{
	pImpl_->selection_color_ = color;
	Invalidate();
}


void PreviewBandWnd::SetSmoothScrollingSpeed(int speed)
{
	ASSERT(speed >= 0 && speed < 10000);
	pImpl_->smooth_scroll_speed_ = speed;
}


void PreviewBandWnd::SetCurrent(AnyPointer photo)
{
	pImpl_->current_item_ = pImpl_->FindItemIndex(photo);

	Invalidate(false);

	//if (pImpl_->keep_selected_centered_)
	//	pImpl_->ScrollToCenter(*this, pImpl_->current_item_);
}


void PreviewBandWnd::KeepCurrentItemCentered(bool enabled)
{
	pImpl_->keep_selected_centered_ = enabled;

	Invalidate(false);

	pImpl_->SizeChanged();	// to force recalc layout
	ValidateLayout();

	ResetScrollBar(true);
}


//void PreviewBandWnd::Impl::ScrollToCenter(PreviewBandWnd& self, size_t photo)
//{
//	if (photo == NONE)
//		return;
//
//	const size_t count= items_.size();
//	if (photo >= count)
//		return;
//
//	CPoint c= items_[photo].rect_.CenterPoint();
//
//	CRect client(0,0,0,0);
//	self.GetClientRect(client);
//
//	// center of the 'photo'
//	int desired_center= horz_layout_ ? c.x : c.y;
//
//	int size= horz_layout_ ? client.Width() : client.Height();
//	int spos= GetScrollPos();
//
//	// center of the window is currently at this position:
//	int center_now= size / 2 + spos;
//
//	int delta= desired_center - center_now;
//
//	// if different, scroll
//	if (delta != 0)
//		self.OnScrollBy(delta, smooth_scroll_speed_, false);
//}


void PreviewBandWnd::ReserveVerticalSpace(int vspace)
{
	if (pImpl_->reserved_vert_space_ == vspace)
		return;

	pImpl_->reserved_vert_space_ = vspace;

	Invalidate(false);

	pImpl_->SizeChanged();	// to force recalc layout
	ValidateLayout();

	ResetScrollBar(true);
}

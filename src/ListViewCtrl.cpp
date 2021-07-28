/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// ListViewCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "ListViewCtrl.h"
#include "MemoryDC.h"
#include "UIElements.h"
#include "CtrlDraw.h"
#include "GetDefaultGuiFont.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

extern void DrawParentBkgnd(CWnd& wnd, CDC& dc);

static const int NONE= -1;
static CString s_wnd_class;	// registered window class
static void RegisterWndClass();


struct LvItem
{
	String text_;		// label
	int icon_index_;
	int label_width_;	// width of text in pixels
	int item_width_;	// width of item (text, space, image)
	int command_id_;
	int grouped_begin_;	// if this is group item, range of items
	int grouped_end_;	//   that belong to the group, -1 othwerwise
	bool enabled_;
	bool group_;		// true if this is group header, or false if regular item
	bool visible_;		// items in collapsed group are not visible
	bool collapsed_;	// groups can be collapsed
	CRect rect_;		// item's location
	size_t user_param_;

	LvItem(const TCHAR* text, int icon_index, int command_id, size_t user_param)
	{
		text_ = text;
		icon_index_ = icon_index;
		item_width_ = label_width_ = 0;
		command_id_ = command_id;
		enabled_ = true;
		visible_ = true;
		rect_.SetRectEmpty();
		group_ = false;
		grouped_begin_ = grouped_end_ = -1;
		collapsed_ = false;
		user_param_ = user_param;
	}

	LvItem()
	{
		icon_index_ = -1;
		item_width_ = label_width_ = 0;
		command_id_ = 0;
		enabled_ = true;
		visible_ = true;
		rect_.SetRectEmpty();
		group_ = false;
		grouped_begin_ = grouped_end_ = -1;
		collapsed_ = false;
		user_param_ = 0 - size_t(1);
	}
};


struct ListViewCtrl::Impl
{
	Impl()
	{
		hot_item_index_ = NONE;
		cell_size_ = CSize(0, 0);
		image_size_ = CSize(0, 0);
		img_label_space_ = 0;
		items_per_column_ = 0;
		image_list_ = 0;
		layout_valid_ = false;
		receiver_ = 0;
		draw_check_boxes_ = false;
		rgb_backgnd_ = ::GetSysColor(COLOR_WINDOW);
		rgb_normal_text_ = ::GetSysColor(COLOR_WINDOWTEXT);
		rgb_disabled_text_ = ::GetSysColor(COLOR_GRAYTEXT);
		rgb_selected_text_ = ::GetSysColor(COLOR_HIGHLIGHTTEXT);
		rgb_selection_ = ::GetSysColor(COLOR_HIGHLIGHT);
		times_text_height_ = 1.2f;
		menu_like_selection_ = false;
		intercol_space_ = cell_margin_ = 0;
		on_idle_update_state_ = false;
		display_mode_ = List;
		wnd_ = nullptr;
		current_group_ = -1;
		total_width_ = 0;
		use_parent_background_ = false;

		RegisterWndClass();
	}

	CSize GetIdealSize();
	int GetItemCount() const;
	void Paint(CDC& dc, CRect rect);
	void TrackHotItem(CPoint pos);
	int FindItemIndex(CPoint pos);
	void DrawItem(int item_index, bool hot);
	void DrawItem(CDC& dc, int item_index, bool hot);
	void DrawItems(CDC& dc);
	bool CalcLayout();
	CSize SetItemsLayout(const CRect& rect);
	void ResetLayout();
	CPoint GetScrollOffset() const;
	void PreSubclassWindow();
	void ResetToolTips();
	CPoint GetItemPoint(int index) const;
	CRect GetItemRect(int index) const;
	void ToggleGroup(int item);

	ListViewCtrl* wnd_;
	int hot_item_index_;	// for tracking item under mouse cursor
	bool layout_valid_;
	CSize cell_size_;
	int cell_margin_;		// extra space on the left and right side of an item
	int intercol_space_;	// amount of pixels between columns of cells
	int min_cell_width_;
	CSize image_size_;
	int img_label_space_;
	int items_per_column_;
	int columns_;
	CFont normal_fnt_;
	CFont underlined_fnt_;
	CImageList* image_list_;
	bool draw_check_boxes_;
	ListViewCtrlNotifications* receiver_;
	COLORREF rgb_backgnd_;
	COLORREF rgb_selection_;
	COLORREF rgb_normal_text_;
	COLORREF rgb_disabled_text_;
	COLORREF rgb_selected_text_;
	float times_text_height_;
	bool menu_like_selection_;
	Mode display_mode_;
	CToolTipCtrl tool_tips_;
	bool path_display_;
	std::vector<LvItem> items_;
	bool on_idle_update_state_;
	int current_group_;
	int total_width_;
	bool use_parent_background_;
};


// ListViewCtrl

ListViewCtrl::ListViewCtrl() : impl_(*new Impl())
{
	impl_.wnd_ = this;
}

ListViewCtrl::~ListViewCtrl()
{
	delete &impl_;
}


BEGIN_MESSAGE_MAP(ListViewCtrl, CWnd)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_MOUSEMOVE()
	ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
//	ON_WM_ACTIVATE()
	ON_WM_SIZE()
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONDOWN()
	ON_WM_HSCROLL()
	ON_MESSAGE(WM_IDLEUPDATECMDUI, OnIdleUpdateCmdUI)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXT, 0, 0xFFFF, OnToolTipNotify)
END_MESSAGE_MAP()


void RegisterWndClass()
{
	if (s_wnd_class.IsEmpty())
	{
		const TCHAR* cls_name= _T("ExtListViewCtrl");

		HINSTANCE inst= AfxGetInstanceHandle();

		// see if the class already exists
		WNDCLASS wndcls;
		if (!::GetClassInfo(inst, cls_name, &wndcls))
		{
			// otherwise we need to register a new class
			wndcls.style = CS_VREDRAW | CS_HREDRAW;
			wndcls.lpfnWndProc = ::DefWindowProc;
			wndcls.cbClsExtra = wndcls.cbWndExtra = 0;
			wndcls.hInstance = inst;
			wndcls.hIcon = 0;
			wndcls.hCursor = ::LoadCursor(NULL, IDC_ARROW);
			wndcls.hbrBackground = 0;
			wndcls.lpszMenuName = NULL;
			wndcls.lpszClassName = cls_name;

			if (AfxRegisterClass(&wndcls))
				s_wnd_class = cls_name;
		}
	}
}


// ListViewCtrl message handlers

void ListViewCtrl::PreSubclassWindow()
{
	CWnd::PreSubclassWindow();

	impl_.PreSubclassWindow();
}

void ListViewCtrl::Impl::PreSubclassWindow()
{
	//HFONT font= static_cast<HFONT>(::GetStockObject(DEFAULT_GUI_FONT));
	LOGFONT lf;
	/*::GetObject(font, sizeof(lf), &lf);
	//lf.lfHeight += 1;
	_tcscpy(lf.lfFaceName, _T("Tahoma"));*/
	::GetDefaultGuiFont(lf);
	normal_fnt_.CreateFontIndirect(&lf);

	lf.lfUnderline = true;
	underlined_fnt_.CreateFontIndirect(&lf);
}


bool ListViewCtrl::Create(CWnd* parent, CRect rect, UINT id, ListViewCtrlNotifications* receiver/*= 0*/)
{
	if (!CWnd::CreateEx(0 /*WS_EX_CLIENTEDGE*/, s_wnd_class, _T(""), WS_CHILD | WS_VISIBLE, rect, parent, id))
		return false;

	impl_.receiver_ = receiver;

	return true;
}


void ListViewCtrl::SetReceiver(ListViewCtrlNotifications* receiver)
{
	impl_.receiver_ = receiver;
}


CSize ListViewCtrl::GetIdealSize()
{
	return impl_.GetIdealSize();
}


CSize ListViewCtrl::Impl::GetIdealSize()
{
	if (GetItemCount() == 0)
		return CSize(0, 0);

	if (!layout_valid_)
		CalcLayout();

	CRect rect(0, 0, 1000, INT_MAX);
	CSize size= SetItemsLayout(rect);

	layout_valid_ = false;

	return size;
}


void ListViewCtrl::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	CRect rect;
	GetClientRect(rect);
	impl_.Paint(dc, rect);
}


void ListViewCtrl::Impl::Paint(CDC& dc, CRect rect)
{
	if (!layout_valid_)
	{
		layout_valid_ = CalcLayout();

		ResetToolTips();
	}

	if (layout_valid_)
	{
		CPoint offset= GetScrollOffset();
		dc.SetViewportOrg(-offset);

		rect.OffsetRect(offset);

		MemoryDC mem_dc(dc, rect, rgb_backgnd_);

		if (use_parent_background_)
		{
			CPoint org= mem_dc.GetViewportOrg();
			mem_dc.SetViewportOrg(CPoint(0, 0));
			DrawParentBkgnd(*wnd_, mem_dc);
			mem_dc.SetViewportOrg(org);
		}

		DrawItems(mem_dc);

		mem_dc.BitBlt();

		dc.SetViewportOrg(0, 0);
	}
	else
	{
		dc.SetViewportOrg(0, 0);

		dc.FillSolidRect(rect, rgb_backgnd_);
	}
}


BOOL ListViewCtrl::OnEraseBkgnd(CDC* dc)
{
	return true;
}


void ListViewCtrl::OnMouseMove(UINT flags, CPoint pos)
{
	impl_.TrackHotItem(pos);
}


LRESULT ListViewCtrl::OnMouseLeave(WPARAM, LPARAM)
{
	if (impl_.hot_item_index_ != NONE)
	{
		if (impl_.use_parent_background_)
			Invalidate();	// todo: optimize...
		else
			impl_.DrawItem(impl_.hot_item_index_, false);
		impl_.hot_item_index_ = NONE;
	}

	return 0;
}


void ListViewCtrl::Impl::TrackHotItem(CPoint pos)
{
	pos += GetScrollOffset();
	int item= FindItemIndex(pos);

	if (hot_item_index_ == item)
		return;

	if (hot_item_index_ != NONE)
	{
		if (use_parent_background_)
			wnd_->Invalidate();	// todo: optimize...
		else
			DrawItem(hot_item_index_, false);
		hot_item_index_ = NONE;
	}

//	if (!is_active_)
//		return;

	if (item != NONE)
	{
		if (use_parent_background_)
			wnd_->Invalidate();	// todo: optimize...
		else
			DrawItem(item, true);
		hot_item_index_ = item;

		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof tme;
		tme.dwFlags = TME_LEAVE;
		tme.dwHoverTime = 0;
		tme.hwndTrack = *wnd_;
		::_TrackMouseEvent(&tme);
	}
}


void ListViewCtrl::OnActivate(UINT state, CWnd* wnd_other, BOOL minimized)
{
	CWnd::OnActivate(state, wnd_other, minimized);

	bool is_active= state != WA_INACTIVE;

	if (is_active)
		SetFocus();

	//if (is_active_ != is_active)
	//{
	//	is_active_ = is_active;
	//	RedrawWindow();
	//}
}


void ListViewCtrl::Impl::DrawItem(int item_index, bool hot)
{
	CClientDC dc(wnd_);
	dc.SetWindowOrg(GetScrollOffset());
	DrawItem(dc, item_index, hot);
}


CPoint ListViewCtrl::Impl::GetItemPoint(int index) const
{
	if (static_cast<size_t>(index) < items_.size())
		return items_[index].rect_.TopLeft();

	ASSERT(false);
	return CPoint(-1, -1);
}


CRect ListViewCtrl::Impl::GetItemRect(int index) const
{
	if (static_cast<size_t>(index) < items_.size())
		return items_[index].rect_;

	ASSERT(false);
	return CRect(-1, -1, 0, 0);
}


void ListViewCtrl::Impl::DrawItem(CDC& dc, int item_index, bool hot)
{
	if (item_index >= items_.size())
		return;	// bogus hot item index for instance

	const LvItem& item= items_[item_index];

	if (!item.visible_)
		return;

	CRect item_rect= GetItemRect(item_index);
	CPoint pos= item_rect.TopLeft();

	bool item_enabled= item.enabled_;
	if (receiver_)
		item_enabled = receiver_->IsItemEnabled(*wnd_, item_index, item.user_param_);

	bool disabled= !!(wnd_->GetStyle() & WS_DISABLED) || !item_enabled;

	COLORREF rgb_text= disabled ? rgb_disabled_text_ : rgb_normal_text_;
	COLORREF rgb_backgnd= rgb_backgnd_;
	bool selection_rect= false;

	if (hot && menu_like_selection_)
	{
		rgb_backgnd = rgb_selection_;

		if (!disabled)
			rgb_text = rgb_selected_text_;

		selection_rect = true;
	}

	if (receiver_)
		receiver_->ItemColors(*wnd_, item_index, item.user_param_, rgb_text, rgb_backgnd);

	if (selection_rect)
		DrawItemSelection(dc, item_rect, rgb_backgnd, disabled, false);
	else if (!use_parent_background_)
		dc.FillSolidRect(item_rect, rgb_backgnd);

	if (receiver_)
		receiver_->DrawItemBackground(*wnd_, item_index, item.user_param_, dc, item_rect);

	pos.x += cell_margin_;

	bool horz_center= display_mode_ == Tiles;

	if (item.group_)
	{
		CPoint pt= pos;
		pt.y += (cell_size_.cy - image_size_.cy) / 2 - 1;
		CtrlDraw::DrawExpandBox(dc, CRect(pt, image_size_), item.collapsed_, hot);
		pos.x += image_size_.cx + img_label_space_;
	}
	else if (image_list_)
	{
		CPoint pt= pos;
		if (!horz_center)
		{
			pt.y += (cell_size_.cy - image_size_.cy) / 2;	// center img vertically
			pos.x += image_size_.cx + img_label_space_;
		}
		else
		{
			// center horizontally
			pt.x += (cell_size_.cx - image_size_.cx) / 2;
			pos.y += image_size_.cy + img_label_space_;
		}
		image_list_->Draw(&dc, item.icon_index_, pt, ILD_TRANSPARENT);
	}
	else if (draw_check_boxes_ && receiver_)
	{
		int state= 0;
		switch (receiver_->GetItemCheckState(*wnd_, item_index, item.user_param_))
		{
		case 0:	// not checked
			if (disabled)
				state = CBS_UNCHECKEDDISABLED;
			else
				state = hot ? CBS_UNCHECKEDHOT : CBS_UNCHECKEDNORMAL;
			break;
		case 1:	// checked
			if (disabled)
				state = CBS_CHECKEDDISABLED;
			else
				state = hot ? CBS_CHECKEDHOT : CBS_CHECKEDNORMAL;
			break;
		case 2:	// undetermined
			if (disabled)
				state = CBS_MIXEDDISABLED;
			else
				state = hot ? CBS_MIXEDHOT : CBS_MIXEDNORMAL;
			break;
		default:
			ASSERT(false);
			break;
		}

		CPoint pt= pos;
		pt.y += (cell_size_.cy - image_size_.cy) / 2 - 1;
		CtrlDraw::DrawCheckBox(dc, CRect(pt, image_size_), state);
		pos.x += image_size_.cx + img_label_space_;
	}

	dc.SelectObject(hot && !menu_like_selection_ && !disabled ? &underlined_fnt_ : &normal_fnt_);

	CRect rect(pos, CSize(item.label_width_, cell_size_.cy));
	dc.SetTextColor(rgb_text);
	dc.SetBkColor(rgb_backgnd);
	dc.SetBkMode(TRANSPARENT);

	DWORD flags= DT_NOPREFIX;

	if (horz_center)
		dc.DrawText(item.text_.c_str(), static_cast<int>(item.text_.length()), rect, flags | DT_CENTER | DT_TOP | DT_WORDBREAK);
	else
		dc.DrawText(item.text_.c_str(), static_cast<int>(item.text_.length()), rect, flags | DT_LEFT | DT_VCENTER | DT_SINGLELINE);
}


int ListViewCtrl::Impl::FindItemIndex(CPoint pos)
{
	if (!layout_valid_)// || cell_size_.cx < 1 || cell_size_.cy < 1 || items_per_column_ < 1)
		return NONE;

	//todo: improve

	const size_t count= items_.size();
	for (size_t i= 0; i < count; ++i)
		if (items_[i].rect_.PtInRect(pos))
			return static_cast<int>(i);

	return NONE;

/*
	int x= pos.x / cell_size_.cx;
	int y= pos.y / cell_size_.cy;
	if (y >= items_per_column_)
		y = items_per_column_ - 1;

	int index= items_per_column_ * x + y;

	if (index < 0 || index >= items_.size())
		return NONE;

	const LvItem& item= items_[index];

	CRect rect(CPoint(x * cell_size_.cx, y * cell_size_.cy), CSize(item.item_width_, cell_size_.cy));

	if (menu_like_selection_)
		rect.right = rect.left + cell_size_.cx;

	return rect.PtInRect(pos) ? index : NONE;
*/
}


void ListViewCtrl::ReserveItems(size_t count)
{
	impl_.items_.reserve(count);
}


void ListViewCtrl::AddItem(const TCHAR* text, int icon_index, int command_id, size_t user_param)
{
	impl_.items_.push_back(LvItem(text, icon_index, command_id, user_param));

	if (impl_.current_group_ >= 0)
		impl_.items_[impl_.current_group_].grouped_end_ = static_cast<int>(impl_.items_.size());

	impl_.layout_valid_ = false;
}


void ListViewCtrl::AddItem(const TCHAR* text)
{
	AddItem(text, 0, 0, 0);
}


int ListViewCtrl::OpenGroup(const TCHAR* text)
{
	impl_.current_group_ = -1;
	impl_.items_.push_back(LvItem(text, 0, 0, -1));
	LvItem& group= impl_.items_.back();
	group.group_ = true;
	int end= static_cast<int>(impl_.items_.size());
	impl_.current_group_ = end - 1;
	group.grouped_begin_ = group.grouped_end_ = end;
	return impl_.current_group_;
}


void ListViewCtrl::CloseGroup()
{
	impl_.current_group_ = -1;
}


bool ListViewCtrl::Impl::CalcLayout()
{
	if (items_.empty())
		return false;

	CDC dc;
	dc.CreateIC(_T("DISPLAY"), 0, 0, 0);
	dc.SelectObject(normal_fnt_);

	image_size_ = CSize(0, 0);

	if (image_list_)
	{
		IMAGEINFO ii;
		if (image_list_->GetImageInfo(0, &ii))
			image_size_ = CRect(ii.rcImage).Size();
	}
	else if (draw_check_boxes_)
	{
		image_size_ = CtrlDraw::GetCheckBoxSize(wnd_);
	}

	CSize space_size= dc.GetTextExtent(_T(" "), 1);

	cell_margin_ = space_size.cx;
	intercol_space_ = space_size.cx;
	img_label_space_ = space_size.cx * 2;

	int max_width= 0;

	const size_t count= items_.size();
	for (size_t i= 0; i < count; ++i)
	{
		CSize size= dc.GetTextExtent(items_[i].text_.c_str(), static_cast<int>(items_[i].text_.length()));

		items_[i].label_width_ = size.cx;
		items_[i].item_width_ = image_size_.cx + img_label_space_ + size.cx;

		if (size.cx > max_width)
			max_width = size.cx;
	}

	int height= space_size.cy;

	if (height < image_size_.cy)
		height = image_size_.cy;

	bool horz_center= display_mode_ == Tiles;
	if (horz_center)
	{
		cell_size_.cx = cell_margin_ + std::max<int>(image_size_.cx + img_label_space_, max_width + space_size.cx) + cell_margin_;
		cell_size_.cy = image_size_.cy + img_label_space_ + 2 * height;
	}
	else
	{
		cell_size_.cx = cell_margin_ + image_size_.cx + img_label_space_ + max_width + space_size.cx + cell_margin_;
		cell_size_.cy = static_cast<int>(height * times_text_height_); // extra space
	}

	min_cell_width_ = cell_size_.cx;

	ResetLayout();

	return true;
}


CSize ListViewCtrl::Impl::SetItemsLayout(const CRect& rect)
{
	if (cell_size_.cy == 0)
	{
		ASSERT(false);
		return CSize(0, 0);
	}

	const size_t count= items_.size();
	if (count == 0)
		return CSize(0, 0);

	int x= rect.left, y= rect.top;
	int rightmost= rect.left;
	int bottommost= rect.top;
	bool new_column= true;
	size_t group_begin= 0;
	size_t group_end= 0;
	bool group_collapsed= false;
	bool group_filtered_in= false;	// if group is "filtered in", then all its items are visible
	int indent= 0;
	size_t last_item= 0;

	for (size_t i= 0; i < count; ++i)
	{
		LvItem& item= items_[i];

		int height= cell_size_.cy;
		int width= cell_margin_ + item.item_width_ + cell_margin_;
		int vspace= 0;

		// detect end of expanded group, and add extra vert. space
		if (i == group_end && group_begin < group_end && !group_collapsed)
			vspace = image_size_.cy / 2; //todo-value

		item.visible_ = true;

		if (item.group_)
		{
			group_begin = item.grouped_begin_;
			group_end = item.grouped_end_;
			group_collapsed = item.collapsed_;
			group_filtered_in = false;
			indent = 0;
		}
		else if (i >= group_begin && i < group_end)
			item.visible_ = !group_collapsed;
		else
			indent = 0;

		// items can by filtered out (hidden)
		if (receiver_ && !group_filtered_in)
		{
			bool filter_in= false;
			if (receiver_->FilterItem(*wnd_, static_cast<int>(i), item.user_param_, item.group_, item.text_, filter_in))
			{
				// filter is active

				item.visible_ = filter_in;

				if (item.group_ && filter_in)
				{
					group_filtered_in = true;
					group_collapsed = false;	// expand them to see what's left after filtering
				}
			}
		}

		if (!item.visible_)
		{
			item.rect_.SetRectEmpty();
			continue;
		}

		if (y + vspace + height >= rect.bottom)
		{
			if (!new_column)
			{
				// "overflow", go to the next column

				if (menu_like_selection_)
				{
					// for menu-like selection make all items in a column have the same width
					for (size_t j= last_item; j < i; ++j)
						if (items_[j].visible_)
							items_[j].rect_.right = rightmost;

					last_item = i;
				}

				x = rightmost + intercol_space_;
				y = rect.top;
				new_column = true;
				indent = 0;	// group items in new column don't need indentation
			}
			vspace = 0;		// cancel group leading space at the top of new column
		}

		item.rect_ = CRect(CPoint(x + indent, y + vspace), CSize(width, height));

		if (item.group_)
			indent = image_size_.cx + img_label_space_; //todo-value

		if (item.rect_.right > rightmost)
			rightmost = item.rect_.right;

		if (item.rect_.bottom > bottommost)
			bottommost = item.rect_.bottom;

		y = item.rect_.bottom;

		new_column = false;
	}

	if (menu_like_selection_)
	{
		// for menu-like selection make all items in a column have the same width
		for (size_t j= last_item; j < count; ++j)
			if (items_[j].visible_)
				items_[j].rect_.right = rightmost;
	}

	total_width_ = rightmost;

	cell_size_.cx = min_cell_width_;

	ASSERT(cell_size_.cy > 0);
	items_per_column_ = std::max(1L, rect.Height() / cell_size_.cy);

	columns_ = static_cast<int>((count + items_per_column_ - 1) / items_per_column_);

	if (items_per_column_ >= count)
		if (cell_size_.cx < rect.Width())
			cell_size_.cx = rect.Width();

	return CSize(rightmost, bottommost + 1);
}


void ListViewCtrl::Impl::DrawItems(CDC& dc)
{
	// this redraw is quite simplistic: just draw every item whether it's visible or not

	size_t count= items_.size();

	for (int i= 0; i < count; ++i)
		DrawItem(dc, i, i == hot_item_index_);

	//for (int i= 0; i < count; ++i)
	//	dc.Draw3dRect(items_[i].rect_, RGB(255,0,0), RGB(0,0,255));
}


void ListViewCtrl::SetImageList(CImageList* image_list)
{
	impl_.image_list_ = image_list;
	impl_.draw_check_boxes_ = false;
	impl_.layout_valid_ = false;
}


void ListViewCtrl::SetCheckBoxes(bool enable)
{
	impl_.draw_check_boxes_ = enable;
	impl_.image_list_ = 0;
	impl_.layout_valid_ = false;
}


void ListViewCtrl::OnSize(UINT type, int cx, int cy)
{
	CWnd::OnSize(type, cx, cy);

	impl_.ResetLayout();
}


void ListViewCtrl::Impl::ResetLayout()
{
	if (cell_size_.cy == 0)
		return;

	CRect rect(0,0,0,0);
	wnd_->GetClientRect(rect);

	if (wnd_->GetStyle() & WS_HSCROLL)	// has scrollbar?
		rect.bottom += ::GetSystemMetrics(SM_CYHSCROLL);

	SetItemsLayout(rect);

	// not enough space?
	if (total_width_ /*columns_ * cell_size_.cx*/ > rect.Width())
	{
		rect.bottom -= ::GetSystemMetrics(SM_CYHSCROLL);
		SetItemsLayout(rect);
	}

	SCROLLINFO si;
	si.cbSize = sizeof si;
	si.fMask = SIF_RANGE | SIF_PAGE;
	si.nMin = 0;
	si.nMax = total_width_ - 1; //columns_ * cell_size_.cx - 1;
	si.nPage = rect.Width();
	si.nTrackPos = 0;
	wnd_->SetScrollInfo(SB_HORZ, &si, true);
}


BOOL ListViewCtrl::OnSetCursor(CWnd* wnd, UINT hit_test, UINT message)
{
	if (hit_test == HTCLIENT)
	{
		ListViewCtrl::SetCursor();
		return true;
	}
	else
		return CWnd::OnSetCursor(wnd, hit_test, message);
}


// select correct cursor (arrow or pointing hand)
//
void ListViewCtrl::SetCursor()
{
	CPoint pos;
	GetCursorPos(&pos);
	ScreenToClient(&pos);
	pos += impl_.GetScrollOffset();
	int item= impl_.FindItemIndex(pos);
	bool active_item= true;

	if (item != NONE && !impl_.items_[item].group_)
	{
		bool item_enabled= impl_.items_[item].enabled_;
		if (impl_.receiver_)
			item_enabled = impl_.receiver_->IsItemEnabled(*this, item, impl_.items_[item].user_param_);
		active_item = item_enabled;
	}
	else
		active_item = false;

	if (active_item)
	{
		HCURSOR cursor= AfxGetApp()->LoadStandardCursor(IDC_HAND);
		if (cursor == 0)
			cursor = AfxGetApp()->LoadCursor(IDC_LINK);
		::SetCursor(cursor);
	}
	else
		::SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));
}


CPoint ListViewCtrl::Impl::GetScrollOffset() const
{
	return CPoint(wnd_->GetScrollPos(SB_HORZ), wnd_->GetScrollPos(SB_VERT));
}


void ListViewCtrl::OnLButtonDown(UINT flags, CPoint pos)
{
	pos += impl_.GetScrollOffset();
	int index= impl_.FindItemIndex(pos);

	if (index != NONE)
	{
		const LvItem& item= impl_.items_[index];

		if (item.group_)
			impl_.ToggleGroup(index);
		else if (impl_.receiver_)
			impl_.receiver_->ItemClicked(*this, index, item.user_param_);
		else if (CWnd* wnd= GetOwner())
		{
			if (item.enabled_)
				wnd->SendMessage(WM_COMMAND, item.command_id_);
		}
	}
}


void ListViewCtrl::Impl::ToggleGroup(int item)
{
	if (item >= 0 && item < items_.size() && items_[item].group_)
	{
		items_[item].collapsed_ = !items_[item].collapsed_;
		ResetLayout();
		wnd_->Invalidate();
	}
}


void ListViewCtrl::SetBackgndColor(COLORREF rgb_backgnd)
{
	impl_.rgb_backgnd_ = rgb_backgnd;
	Invalidate();
}


void ListViewCtrl::SetBackgndColors(COLORREF rgb_backgnd, COLORREF selection)
{
	impl_.rgb_backgnd_ = rgb_backgnd;
	impl_.rgb_selection_ = selection;

	if (m_hWnd)
		Invalidate();
}


void ListViewCtrl::SetTextColors(COLORREF normal_text, COLORREF selected_text, COLORREF disabled_text)
{
	impl_.rgb_normal_text_ = normal_text;
	impl_.rgb_selected_text_ = selected_text;
	impl_.rgb_disabled_text_ = disabled_text;

	if (m_hWnd)
		Invalidate();
}


int ListViewCtrl::GetItemCount() const
{
	return impl_.GetItemCount();
}


int ListViewCtrl::GetVisibleItemCount() const
{
	int visible= 0;
	const size_t count= impl_.items_.size();
	for (size_t i= 0; i < count; ++i)
		if (impl_.items_[i].visible_)
			visible++;

	return visible;
}


bool ListViewCtrl::HasVisibleItems() const
{
	const size_t count= impl_.items_.size();
	for (size_t i= 0; i < count; ++i)
		if (impl_.items_[i].visible_)
			return true;

	return false;
}


int ListViewCtrl::Impl::GetItemCount() const
{
	return static_cast<int>(items_.size());
}


void ListViewCtrl::SetItemText(int item_index, const TCHAR* text)
{
	size_t index= item_index;
	if (index < impl_.items_.size())
	{
		impl_.items_[index].text_ = text;
		impl_.layout_valid_ = false;	// length of item might have changed...
		Invalidate();
	}
	else
	{ ASSERT(false); }
}

const String& ListViewCtrl::GetItemText(int item_index) const
{
	return impl_.items_.at(item_index).text_;
}

void ListViewCtrl::DeleteFromItem(int item_index)
{
	size_t index= item_index;
	if (index < impl_.items_.size())
	{
		impl_.items_.erase(impl_.items_.begin() + index, impl_.items_.end());
		impl_.layout_valid_ = false;
		impl_.current_group_ = -1;
		Invalidate();
	}
	else if (index == impl_.items_.size())
	{
		// ok
	}
	else
	{ ASSERT(false); }	// wrong index
}


// space between items
void ListViewCtrl::SetItemSpace(float times_text_height)
{
	ASSERT(times_text_height > 0.0f);

	impl_.times_text_height_ = times_text_height;
	impl_.layout_valid_ = false;
}


void ListViewCtrl::OnHScroll(UINT sb_code, UINT pos, CScrollBar* scroll_bar)
{
	OnScroll(sb_code, pos);
}


BOOL ListViewCtrl::OnScroll(UINT scroll_code, UINT pos, BOOL do_scroll/* = TRUE*/)
{
	// calc new x position
	int x= GetScrollPos(SB_HORZ);
	int xOrig= x;

	SCROLLINFO si;
	si.cbSize = sizeof si;
	si.fMask = SIF_RANGE | SIF_PAGE | SIF_TRACKPOS;
	GetScrollInfo(SB_HORZ, &si);
	int line_dx= 20; //(thumb_size_.cx + g_GAP_W) / 2;

	switch (scroll_code)
	{
	case SB_TOP:
		x = 0;
		break;
	case SB_BOTTOM:
		x = si.nMax;
		break;
	case SB_LINEUP:
		x -= line_dx;
		break;
	case SB_LINEDOWN:
		x += line_dx;
		break;
	case SB_PAGEUP:
		x -= si.nPage;
		break;
	case SB_PAGEDOWN:
		x += si.nPage;
		break;
	case SB_THUMBTRACK:
		{
			//x = pos;
			x = si.nTrackPos;
		}
		break;
	}

	do_scroll = true;
	BOOL result= OnScrollBy(CSize(x - xOrig, 0), !!do_scroll);
	if (result && do_scroll)
		UpdateWindow();

	return result;
}


bool ListViewCtrl::OnScrollBy(CSize scroll_size, bool do_scroll)
{
	int xOrig, x;

	// adjust current x position
	xOrig = x = GetScrollPos(SB_HORZ);
	int xMax= 0;
	SCROLLINFO si;
	si.cbSize = sizeof si;
	si.fMask = SIF_RANGE | SIF_PAGE;
	GetScrollInfo(SB_HORZ, &si);
	xMax = si.nMax - si.nPage + 1;
	x += scroll_size.cx;
	if (x < 0)
		x = 0;
	else if (x > xMax)
		x = xMax;

	// has something changed?
	if (x == xOrig)
		return false;

	if (do_scroll)
	{
		// do scroll and update scroll positions
		ScrollWindow(-(x-xOrig), 0);
		if (x != xOrig)
			SetScrollPos(SB_HORZ, x);

		impl_.ResetToolTips();

		if (impl_.use_parent_background_)
			Invalidate();
	}
	return TRUE;
}


LRESULT ListViewCtrl::OnIdleUpdateCmdUI(WPARAM wParam, LPARAM)
{
	if (!impl_.on_idle_update_state_)
		return 0L;

	// the list view must be visible
	if ((GetStyle() & WS_VISIBLE))
	{
		CFrameWnd* target = static_cast<CFrameWnd*>(GetOwner());
		if (target == NULL || !target->IsFrameWnd())
			target = GetParentFrame();
		if (target != NULL)
			OnUpdateCmdUI(target, !!wParam);
	}
	return 0L;
}


namespace {

class xCToolCmdUI : public CCmdUI        // class private to this file!
{
public: // re-implementations only
	virtual void Enable(BOOL on);
	virtual void SetCheck(int check);
	virtual void SetText(LPCTSTR lpszText);
};

void xCToolCmdUI::Enable(BOOL on)
{
	m_bEnableChanged = TRUE;
	ListViewCtrl* list= static_cast<ListViewCtrl*>(m_pOther);
	ASSERT(list != NULL);
	ASSERT_KINDOF(ListViewCtrl, list);
	ASSERT(m_nIndex < m_nIndexMax);
//	if (static_cast<uint32>(index_) < list->items_.size())
//	if (!on)
//		tool_bar->PressButton(m_nID, false);	// unpress btn when disabled
	list->EnableItemByIndex(m_nIndex, !!on);
}

void xCToolCmdUI::SetCheck(int check)
{
	// TODO
}
//	ASSERT(check >= 0 && check <= 2); // 0=>off, 1=>on, 2=>indeterminate
//	ToolBarWnd* tool_bar = dynamic_cast<ToolBarWnd*>(other_);
//	ASSERT(tool_bar != NULL);
////	dynamiASSERT_KINDOF(ToolBarWnd, tool_bar);
//	ASSERT(m_nIndex < m_nIndexMax);
//
//	if (tool_bar->shift_image_for_checked_btn_ != 0 && tool_bar->shift_image_for_checked_btn_ == m_nID)
//	{
//		TBBUTTONINFO tbbi;
//		tbbi.cbSize = sizeof tbbi;
//		tbbi.mask = TBIF_IMAGE;
//		tbbi.iImage = check;
//		tool_bar->SetButtonInfo(m_nID, &tbbi);
//	}
//	else
//	{
//		TBBUTTONINFO tbbi;
//		tbbi.cbSize = sizeof tbbi;
//		tbbi.mask = TBIF_STYLE;
//		// do not set checked state for normal button (non-check buttons)
//		tool_bar->GetButtonInfo(m_nID, &tbbi);
//		if ((tbbi.fsStyle & BTNS_CHECK) == 0)
//			return;
//		tool_bar->CheckButton(m_nID, check);
//	}
//}

void xCToolCmdUI::SetText(LPCTSTR)
{
	// ignore it
}

} // namespace


void ListViewCtrl::OnUpdateCmdUI(CFrameWnd* target, bool disable_if_no_hndler)
{
	xCToolCmdUI state;
	state.m_pOther = this;

	state.m_nIndexMax = static_cast<int>(impl_.items_.size());
	for (state.m_nIndex = 0; state.m_nIndex < state.m_nIndexMax; state.m_nIndex++)
	{
		// get buttons state
//		TBBUTTON button;
//		GetButton(state.m_nIndex, &button);
//		_GetButton(state.m_nIndex, &button);
		state.m_nID = impl_.items_[state.m_nIndex].command_id_;

		// ignore separators
//		if (!(button.fsStyle & TBSTYLE_SEP))
		{
			// allow reflections
//			if (CWnd::OnCmdMsg(0,
//				MAKELONG((int)CN_UPDATE_COMMAND_UI, WM_COMMAND+WM_REFLECT_BASE),
//				&state, NULL))
//				continue;

			// allow the toolbar itself to have update handlers
//			if (CWnd::OnCmdMsg(state.m_nID, CN_UPDATE_COMMAND_UI, &state, NULL))
//				continue;

			// allow the owner to process the update
			state.DoUpdate(target, disable_if_no_hndler);
		}
	}

	// update the dialog controls added to the toolbar
	UpdateDialogControls(target, disable_if_no_hndler);
}


void ListViewCtrl::EnableItemByIndex(size_t index, bool enable)
{
	if (index < impl_.items_.size())
	{
		if (impl_.items_[index].enabled_ != enable)
		{
			impl_.items_[index].enabled_ = enable;
			Invalidate();
		}
	}
}


void ListViewCtrl::SetOnIdleUpdateState(bool enable)
{
	impl_.on_idle_update_state_ = enable;
}


void ListViewCtrl::SetMode(Mode mode)
{
	if (impl_.display_mode_ != mode)
	{
		impl_.display_mode_ = mode;
		impl_.layout_valid_ = false;
		Invalidate();
	}
}


void ListViewCtrl::EnableToolTips()
{
	// ------------------- tooltips ----------------------

	if (!impl_.tool_tips_.Create(this, WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP))
		return;

	TOOLINFO ti;
	ti.cbSize = sizeof ti;
	ti.hwnd   = *this;
	ti.uId    = UINT_PTR(ti.hwnd);
	ti.lpszText = const_cast<TCHAR*>(_T(""));
	ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
	ti.rect   = CRect(0,0,0,0);
	ti.hinst  = NULL;
	ti.lParam = 0;
	impl_.tool_tips_.SendMessage(TTM_ADDTOOL, 0, LPARAM(&ti));

	impl_.tool_tips_.SetDelayTime(1000);	// 1 sec
	impl_.tool_tips_.SetDelayTime(TTDT_AUTOPOP, 15000);	// 15 sec
	impl_.tool_tips_.SetDelayTime(TTDT_RESHOW, 1000);	// 1 sec
//	tool_tips_.SetMaxTipWidth(300);
}


void ListViewCtrl::Impl::ResetToolTips()
{
	if (tool_tips_.m_hWnd == 0)
		return;

	{
		const int count= tool_tips_.GetToolCount();
		for (int index= 0; index < count; ++index)
			tool_tips_.DelTool(wnd_, index + 1);
	}

//	if (!balloons_enabled_)
//		return;

	if (items_.empty())
		return;

	//ValidateLayout();

	CRect rect(0,0,0,0);
	wnd_->GetClientRect(rect);

	CPoint scrl= GetScrollOffset();
	rect.OffsetRect(scrl);
	//TODO: visible items only
//	pair<size_t, size_t> range= FindVisibleItems(rect);

//	first_tooltip_idx_ = range.first;

	int index= 1;
	for (size_t i= 0; i < items_.size(); ++i)
	{
		const LvItem& item= items_[i];

		CRect frm= GetItemRect(static_cast<int>(i));
		frm.OffsetRect(-scrl);

		tool_tips_.AddTool(wnd_, LPSTR_TEXTCALLBACK, frm, i + 1);
	}
}


BOOL ListViewCtrl::OnToolTipNotify(UINT id, NMHDR* hdr, LRESULT* result)
{
	NMTTDISPINFO* TTT= reinterpret_cast<NMTTDISPINFO*>(hdr);

//	if (pImpl_->balloons_enabled_)
	{
		try
		{
			static String buff_;
			size_t index= TTT->hdr.idFrom - 1; // + pImpl_->first_tooltip_idx_;
			if (index < impl_.items_.size() && impl_.receiver_)
			{
				buff_ = impl_.receiver_->GetTooltipText(*this, static_cast<int>(index), impl_.items_[index].user_param_);
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
	//else
	//{
	//	TTT->lpszText = TTT->szText;
	//	TTT->szText[0] = 0;
	//}

	*result = 0;
	return TRUE;
}


void ListViewCtrl::SetMenuLikeSelection(bool on)
{
	impl_.menu_like_selection_ = on;
}


void ListViewCtrl::UseParentBackground(bool enable)
{
	impl_.use_parent_background_ = enable;
}


int ListViewCtrl::GetFirstGroup() const
{
	return GetNextGroup(-1);
}


int ListViewCtrl::GetNextGroup(int index) const
{
	size_t start= index < 0 ? 0 : index + 1;

	const size_t count= impl_.items_.size();
	for (size_t i= start; i < count; ++i)
		if (impl_.items_[i].group_)
			return static_cast<int>(i);

	return -1;
}


void ListViewCtrl::CollapseGroup(int item_index, bool collapsed)
{
	const size_t count= impl_.items_.size();
	size_t index= item_index;

	if (index < count && impl_.items_[index].group_)
	{
		if (impl_.items_[index].collapsed_ != collapsed)
			impl_.ToggleGroup(item_index);
	}
	else
	{ ASSERT(false); }
}


bool ListViewCtrl::IsGroupCollapsed(int item_index) const
{
	ASSERT(impl_.items_.at(item_index).group_);
	return impl_.items_.at(item_index).collapsed_;
}


bool ListViewCtrl::IsGroup(int item_index) const
{
	return impl_.items_.at(item_index).group_;
}


void ListViewCtrl::FilterItems()
{
	impl_.ResetLayout();
	if (m_hWnd)
		Invalidate();
}


bool ListViewCtrl::IsVisible(int item_index) const
{
	return impl_.items_.at(item_index).visible_;
}

size_t ListViewCtrl::GetUserParam(int item_index) const
{
	return impl_.items_.at(item_index).user_param_;
}

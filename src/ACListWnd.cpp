// This file comes from www.codeproject.com/combobox


#include "stdafx.h"
#include "ACListWnd.h"
#include "block.h"
#include "WhistlerLook.h"
#include "MultiMonitor.h"
#include "MemoryDC.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


void DoPaintMessageLoop()
{
	MSG message;
	while (::PeekMessage(&message, NULL, WM_PAINT, WM_PAINT, PM_REMOVE))
	{
		::TranslateMessage(&message);
		::DispatchMessage(&message);
	}
}

static const int BORDER= 1;
static const int WIDTH_GRID= 8;

/**********************************************************************/

AutoCompletePopup::AutoCompletePopup()
{
	top_index_ = 0;
	item_height_ = 16;
	selected_item_ = -1;
	visible_items_ = 0;
	edit_ctrl_ = NULL;
	last_size_.cx = 0;
	last_size_.cy = 0;
	old_wnd_proc_ = 0;
	parent_old_wnd_proc_ = 0;
	in_update_ = false;
	cur_item_ = -1;
	align_to_parent_ = false;
	auto_popup_ = true;
	auto_unhook_ = true;
	handle_up_down_key_ = true;
}

AutoCompletePopup::~AutoCompletePopup()
{
	DestroyWindow();
}


void AutoCompletePopup::RegisterTextRemoveFn(const boost::function<void (const String& text)>& remove)
{
	remove_text_ = remove;
}


/*********************************************************************/

void AutoCompletePopup::OnActivateApp(BOOL active, DWORD task)
{
#if _MSC_VER >= 1310
	CWnd::OnActivateApp(active, task);
#else
	CWnd::OnActivateApp(active, (HTASK)task);
#endif
	ShowWindow(SW_HIDE);
}


BEGIN_MESSAGE_MAP(AutoCompletePopup, CWnd)
	//{{AFX_MSG_MAP(AutoCompletePopup)
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_SIZING()
	ON_WM_ERASEBKGND()
	ON_WM_NCPAINT()
	ON_WM_KEYDOWN()
	ON_WM_NCCALCSIZE()
	ON_WM_VSCROLL()
	ON_WM_ACTIVATEAPP()
	ON_WM_NCHITTEST()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_SETCURSOR()
	ON_WM_SHOWWINDOW()
	ON_WM_MOUSEMOVE()
	ON_WM_TIMER()
	ON_WM_GETMINMAXINFO()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////

void AutoCompletePopup::DrawItem(CDC* dc, long item, long width)
{
	long y= item - top_index_;
	CRect rcLabel(2, y * item_height_, width, (y + 1) * item_height_);

	dc->SetTextColor(::GetSysColor(COLOR_WINDOWTEXT));
	dc->SetBkColor(::GetSysColor(COLOR_WINDOW));

	if (item == selected_item_)
	{
		rcLabel.left = 0;
		COLORREF highlight= RGB(247, 123, 0);//::GetSysColor(COLOR_HIGHLIGHT);
		dc->FillSolidRect(rcLabel, highlight);
		dc->SetTextColor(::GetSysColor(COLOR_HIGHLIGHTTEXT));
		dc->SetBkColor(highlight);
		rcLabel.left = 2;
	}

	String disp= *display_list_.at(item);

	for (size_t i= 0; i < disp.length(); ++i)
		if (disp[i] == '\n')
			disp[i] = ' ';
		else if (disp[i] == '\r')
			disp[i] = ' ';//'¶';

	dc->DrawText(disp.c_str(), static_cast<int>(disp.size()), rcLabel, DT_LEFT | DT_SINGLELINE | DT_NOPREFIX | DT_VCENTER | DT_END_ELLIPSIS);
}

/*********************************************************************/


void AutoCompletePopup::OnPaint()
{
	try
	{
		CPaintDC paint_dc(this);

		CRect rc(0,0,0,0);
		GetClientRect(rc);

		MemoryDC dc(paint_dc, rc, ::GetSysColor(COLOR_WINDOW));

		CRect rect= rc;

		rc.left = rc.right - GetSystemMetrics(SM_CXHSCROLL);
		rc.top = rc.bottom - GetSystemMetrics(SM_CYVSCROLL);

		long width= rect.Width() - ScrollBarWidth();

		dc.FillSolidRect(rect, ::GetSysColor(COLOR_WINDOW));
		//dc.SelectObject(GetStockObject(DEFAULT_GUI_FONT)); 
		dc.SelectObject(&fontDC);
		dc.SetBkMode(OPAQUE);

		for (int i= top_index_; i < display_list_.size(); i++)
			DrawItem(&dc, i, width);

		CPen pen1(PS_SOLID, 1, ::GetSysColor(COLOR_WINDOW));
		CPen pen2(PS_SOLID, 1, ::GetSysColor(COLOR_BTNFACE));
		CPen pen3(PS_SOLID, 1, ::GetSysColor(COLOR_3DSHADOW));

		if (vert_scrollbar_.IsWindowVisible())
			dc.FillSolidRect(rc, ::GetSysColor(COLOR_BTNFACE));

		CPen* oldPen = dc.SelectObject(&pen1);

		width = GetSystemMetrics(SM_CXHSCROLL);
		int bottom = (rect.bottom - GetSystemMetrics(SM_CXHSCROLL)) - 1;

		for (int i= 0, a= 1; i < 20; i++, a++)
		{
			if (a==1)
				dc.SelectObject(&pen1);
			else if (a==2)
				dc.SelectObject(&pen2);
			else if (a==3)
				dc.SelectObject(&pen3);
			else if (a > 3)
				a = 0;

			dc.MoveTo(rc.left + i - 1, rect.bottom);
			dc.LineTo(rc.left + i + width, bottom);
		}

		dc.SelectObject(oldPen);

		dc.BitBlt();
	}
	catch (CException*)
	{}
	catch (std::exception&)
	{}
}

/*********************************************************************/

bool AutoCompletePopup::Create()
{
	UINT classStyle= CS_SAVEBITS | (WhistlerLook::IsAvailable() ? CS_DROPSHADOW : 0);
	CRect rect(0, 0, 0, 0);
	VERIFY(CreateEx(WS_EX_TOOLWINDOW,
		AfxRegisterWndClass(CS_CLASSDC | CS_HREDRAW | CS_VREDRAW | classStyle, 0, HBRUSH(COLOR_WINDOW + 1), 0),
		NULL, WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, rect, CWnd::GetDesktopWindow(), -1, NULL));

	Init();

	return true;
}


void AutoCompletePopup::Init()
{
	VERIFY(vert_scrollbar_.Create(WS_VISIBLE | SBS_VERT, CRect(0, 0, 0, 0), this, 0));

	SetScroller();
	edit_ctrl_ = 0;

	vert_scrollbar_.SetScrollPos(0, false);
	SetProp();

	CClientDC dc(this);
	LOGFONT lf;
	HFONT hfont = static_cast<HFONT>(::GetStockObject(DEFAULT_GUI_FONT));
	::GetObject(hfont, sizeof(lf), &lf);
	lf.lfWeight = FW_NORMAL;
	lf.lfHeight += 1;
	_tcscpy(lf.lfFaceName, _T("Tahoma"));
	//lf.lfQuality = ANTIALIASED_QUALITY;
	fontDC.CreateFontIndirect(&lf);
	dc.SelectObject(&fontDC);
	item_height_ = dc.GetOutputTextExtent("X").cy * 13 / 10;

	if (last_size_.cy == 0)
		last_size_.cy = item_height_ * 20 + 2 * BORDER;	// extra space for border
}


void ReplaceText(CEdit& edit, const String& str)
{
	edit.SetSel(0, -1, true);
	edit.ReplaceSel(str.c_str(), true);
}


void AutoCompletePopup::SelectAndHide()
{
	if (selected_item_ >= 0 && selected_item_ < display_list_.size())
	{
		Block update(in_update_);
		if (const String* text= GetString())
		{
			ShowWindow(SW_HIDE);
			ReplaceText(*edit_ctrl_, *text);
		}
	}
}


void AutoCompletePopup::ShowString(const std::vector<String>& strings, int item)
{
	if (item >= 0 && item < strings.size() && edit_ctrl_ != 0)
	{
		Block update(in_update_);
		const String& str= strings[item];
		ReplaceText(*edit_ctrl_, str);
//		edit_ctrl_->SetWindowText(str.c_str());
		edit_ctrl_->SetSel(0, static_cast<int>(str.length()));
	}
}


void AutoCompletePopup::ShowAll()
{
	display_list_.clear();
	const size_t count= search_list_.size();
	display_list_.reserve(count);
	for (size_t i= 0; i < count; ++i)
		display_list_.push_back(&search_list_[i]);

	PopupList(true);
}


bool AutoCompletePopup::HandleKey(WPARAM key)
{
	if (!IsWindowVisible())
	{
		bool alt= ::GetKeyState(VK_MENU) < 0;
		if (alt && (key == VK_DOWN || key == VK_UP))
		{
			ShowAll();
			return true;
		}

		if (edit_ctrl_->GetStyle() & ES_MULTILINE)
			return false;

		switch (key)
		{
		case VK_DOWN:
			if (!handle_up_down_key_)
				return false;
			if (cur_item_ + 1 < static_cast<int>(search_list_.size()))
				cur_item_++;
			else if (cur_item_ < 0)
				cur_item_ = 0;
			ShowString(search_list_, cur_item_);
			break;

		case VK_UP:
			if (!handle_up_down_key_)
				return false;
			if (cur_item_ > 0)
				cur_item_--;
			else if (cur_item_ < 0)
				cur_item_ = 0;
			ShowString(search_list_, cur_item_);
			break;

		case VK_NEXT:
			cur_item_ = static_cast<int>(search_list_.size()) - 1;
			ShowString(search_list_, cur_item_);
			break;

		case VK_PRIOR:
			cur_item_ = 0;
			ShowString(search_list_, cur_item_);
			break;

		default:
			return false;
		}

		return true;
	}

	// popup is visible

	switch (key)
	{
	case VK_ESCAPE:
		ShowWindow(SW_HIDE);
		return true;

	case VK_TAB:
		SelectAndHide();
		if (CWnd* parent= edit_ctrl_->GetParent())
		{
			bool shift= ::GetKeyState(VK_SHIFT) < 0;
			parent->SendMessage(WM_NEXTDLGCTL, shift ? 1 : 0, 0);
		}
		return true;

	case VK_RETURN:
		SelectAndHide();
		if (!text_selected_.empty())
			text_selected_();
		return true;

	case VK_DOWN:
	case VK_UP:
	case VK_PRIOR:
	case VK_NEXT:
	case VK_HOME:
	case VK_END:
		if (IsWindowVisible())
		{
			Block update(in_update_);

			SetNextString(static_cast<int>(key));

			if (const String* str= GetString())
			{
				int len= user_text_.GetLength();

				//edit_ctrl_->SetWindowText(str->c_str());
				ReplaceText(*edit_ctrl_, *str);
				edit_ctrl_->SetSel(len, static_cast<int>(str->length()), true);
				edit_ctrl_->SetModify(true);
			}
			return true;
		}
		break;
	}

	return false;
}


void AutoCompletePopup::EditChange()
{
	if (in_update_)
		return;

	cur_item_ = -1;
	edit_ctrl_->GetWindowText(user_text_);
	FindString(-1, user_text_);
}


void AutoCompletePopup::Unhook()
{
	ShowWindow(SW_HIDE);

	if (edit_ctrl_ == 0)
		return;

	::SetWindowLongPtr(*edit_ctrl_, GWLP_USERDATA, 0);
	::SetWindowLongPtr(*edit_ctrl_, GWLP_WNDPROC, old_wnd_proc_);

	HWND parent= ::GetParent(*edit_ctrl_);
	::SetWindowLongPtr(parent, GWLP_USERDATA, 0);
	::SetWindowLongPtr(parent, GWLP_WNDPROC, parent_old_wnd_proc_);

	old_wnd_proc_ = 0;
	parent_old_wnd_proc_ = 0;
	edit_ctrl_ = 0;
	in_update_ = false;
	user_text_ = CString();
	cur_item_ = -1;
}


void AutoCompletePopup::Hook(CEdit* edit)
{
	if (edit_ctrl_ != 0)
		Unhook();

	if (edit == 0)
		return;

	HWND parent= ::GetParent(*edit);

	old_wnd_proc_ = ::GetWindowLongPtr(*edit, GWLP_WNDPROC);
	::SetWindowLongPtr(*edit, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
	::SetWindowLongPtr(*edit, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&AutoCompletePopup::CtrlWndProc));

	parent_old_wnd_proc_ = ::GetWindowLongPtr(parent, GWLP_WNDPROC);
	::SetWindowLongPtr(parent, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
	::SetWindowLongPtr(parent, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&AutoCompletePopup::ParentWindowProc));

	edit_ctrl_ = edit;
	edit_ctrl_->GetWindowText(user_text_);
}


LRESULT AutoCompletePopup::CtrlWndProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
//TRACE(L"edit msg: %d  (%d)\n", msg, wParam);
	if (AutoCompletePopup* this_ptr= reinterpret_cast<AutoCompletePopup*>(::GetWindowLongPtr(wnd, GWLP_USERDATA)))
		return this_ptr->WindowProc(wnd, msg, wParam, lParam);

	ASSERT(false);
	return 0;
}


LRESULT AutoCompletePopup::ParentWindowProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (AutoCompletePopup* this_ptr= reinterpret_cast<AutoCompletePopup*>(::GetWindowLongPtr(wnd, GWLP_USERDATA)))
	{
		WNDPROC wnd_proc= reinterpret_cast<WNDPROC>(this_ptr->parent_old_wnd_proc_);

		switch (msg)
		{
		case WM_COMMAND:
			switch (HIWORD(wParam))
			{
			case EN_CHANGE:
				if (this_ptr->auto_popup_)
					this_ptr->EditChange();
				break;

			case EN_KILLFOCUS:
TRACE(L"kill focus!\n");
//				this_ptr->ShowWindow(SW_HIDE);
				if (this_ptr->auto_unhook_)
				this_ptr->Unhook();
				break;
			}
			break;

		case WM_WINDOWPOSCHANGING:
			this_ptr->ShowWindow(SW_HIDE);
			break;
		}

		return ::CallWindowProc(wnd_proc, wnd, msg, wParam, lParam);
	}

	ASSERT(false);
	return 0;
}


LRESULT AutoCompletePopup::WindowProc(HWND wnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	ASSERT(edit_ctrl_ != 0);
	ASSERT(wnd == edit_ctrl_->m_hWnd);
	WNDPROC wnd_proc= reinterpret_cast<WNDPROC>(old_wnd_proc_);

	switch (msg)
	{
	case WM_SYSKEYDOWN:
		if (HandleKey(wParam))
			return 0;
		break;

	case WM_KEYDOWN:
		if (HandleKey(wParam))
			return 0;
		break;

	case WM_WINDOWPOSCHANGING:
		ShowWindow(SW_HIDE);
		break;

	case WM_LBUTTONDBLCLK:
		if ((wParam & (MK_RBUTTON | MK_SHIFT | MK_CONTROL | MK_MBUTTON | MK_XBUTTON1 | MK_XBUTTON2)) == 0)
		{
			// double click in an empty edit control brings up history list

			if (edit_ctrl_->GetWindowTextLength() == 0)
			{
				ShowAll();
				return 0;
			}
		}
		break;

	case WM_DESTROY:
		Unhook();
		break;

	case WM_GETDLGCODE:
		if (IsWindowVisible())
			return DLGC_WANTALLKEYS;	// give me all keys when popup visible (including Esc)
		break;
	}

	return ::CallWindowProc(wnd_proc, wnd, msg, wParam, lParam);
}


void AutoCompletePopup::ControlEditBox(CEdit* ctrl, const std::vector<String>* history)
{
	Hook(ctrl);

	display_list_.clear();
	search_list_.clear();

	if (history)
	{
		search_list_ = *history;
		SortList(search_list_);
	}
}


/*********************************************************************/

void AutoCompletePopup::SetScroller()
{
	if (vert_scrollbar_.m_hWnd == 0)
		return;

	CRect bar;
	GetClientRect(bar);

	bar.top = 0;
	bar.left = bar.Width() - ::GetSystemMetrics(SM_CYVSCROLL);
	bar.bottom -= ::GetSystemMetrics(SM_CYHSCROLL);
	vert_scrollbar_.MoveWindow(bar);

	vert_scrollbar_.SetScrollPos(top_index_, true);
}

/*********************************************************************/

void AutoCompletePopup::OnSize(UINT type, int cx, int cy)
{
	CWnd::OnSize(type, cx, cy);

	CRect rect;
	GetWindowRect(rect);
	if (!rect.IsRectEmpty())
	{
		SetScroller();
		SetProp();
	}
}


void AutoCompletePopup::OnSizing(UINT type, RECT* rc)
{
	if (item_height_ > 0)
	{
		// enforce integral height
		int h= rc->bottom - rc->top - 2 * BORDER + item_height_ / 2;
		h -= h % item_height_;
		h += 2 * BORDER;
		rc->bottom = rc->top + h;
	}

	// snap width to grid
	int w= rc->right - rc->left;
	w -= w % WIDTH_GRID;
	rc->right = rc->left + w;

	CWnd::OnSizing(type, rc);

	CRect rect;
	GetWindowRect(rect);
	if (!rect.IsRectEmpty())
		last_size_ = rect.Size();
}


/*********************************************************************/

long AutoCompletePopup::ScrollBarWidth()
{
	if (vert_scrollbar_.IsWindowVisible())
		return GetSystemMetrics(SM_CYVSCROLL);
	else
		return 0;
}

/*********************************************************************/

void AutoCompletePopup::SetProp()
{
	if (display_list_.empty())
		return;

	CRect rect;
	GetClientRect(rect);

	SCROLLINFO si;
	si.cbSize = sizeof si;
	si.fMask = SIF_PAGE | SIF_RANGE;
	si.nMin = 0;
	si.nMax = static_cast<int>(display_list_.size()) - 1;
	visible_items_ = si.nPage = rect.Height() / item_height_;
	si.nTrackPos = 0;
	vert_scrollbar_.SetScrollRange(0, static_cast<int>(display_list_.size()) - 1);
	vert_scrollbar_.SetScrollInfo(&si);

	if (visible_items_ > display_list_.size() - 1)
		vert_scrollbar_.ShowWindow(SW_HIDE);
	else
		vert_scrollbar_.ShowWindow(SW_SHOWNA);

	if (top_index_ + visible_items_ > display_list_.size())
	{
		top_index_ = static_cast<int>(display_list_.size()) - visible_items_;
		if (top_index_ < 0)
			top_index_ = 0;
		vert_scrollbar_.SetScrollPos(top_index_, true);
	}
}

/*********************************************************************/

BOOL AutoCompletePopup::OnEraseBkgnd(CDC* /*dc*/)
{
	return true;
}

/*********************************************************************/

static void FillSolidRect(HDC dc, int x, int y, int cx, int cy, COLORREF clr)
{
	::SetBkColor(dc, clr);
	CRect rect(x, y, x + cx, y + cy);
	::ExtTextOut(dc, 0, 0, ETO_OPAQUE, rect, 0, 0, 0);
}


void AutoCompletePopup::OnNcPaint()
{
	if (HDC hdc= ::GetWindowDC(m_hWnd))
//	if (HDC hdc= ::GetDCEx(m_hWnd, reinterpret_cast<HRGN>(GetCurrentMessage()->wParam), DCX_WINDOW | DCX_INTERSECTRGN))
	{
		CRect rect;
		GetWindowRect(rect);
		ScreenToClient(rect);
		rect.OffsetRect(-rect.left, -rect.top);

		COLORREF frame= ::GetSysColor(COLOR_BTNSHADOW);
		::FillSolidRect(hdc, rect.left, rect.top, 1, rect.Height(), frame);
		::FillSolidRect(hdc, rect.left, rect.top, rect.Width(), 1, frame);
		::FillSolidRect(hdc, rect.left, rect.bottom - 1, rect.Width(), 1, frame);
		::FillSolidRect(hdc, rect.right - 1, rect.top, 1, rect.Height(), frame);

		::ReleaseDC(m_hWnd, hdc);
	}
/*
	CWindowDC dc(this);
	CRect client_rect, window_rect,rcWnd;

	GetClientRect(client_rect);
	GetWindowRect(window_rect);
	ScreenToClient(window_rect);

	client_rect.OffsetRect(-window_rect.left, -window_rect.top);
	dc.ExcludeClipRect(client_rect);

	window_rect.OffsetRect(-window_rect.left, -window_rect.top);

	dc.FillSolidRect(window_rect, ::GetSysColor(COLOR_WINDOWTEXT));
*/
}

/*********************************************************************/

void AutoCompletePopup::OnKeyDown(UINT chr, UINT rep_cnt, UINT flags)
{
	CWnd::OnKeyDown(chr, rep_cnt, flags);

	if (chr == VK_ESCAPE)
		ShowWindow(SW_HIDE);
}

/*********************************************************************/

void AutoCompletePopup::OnNcCalcSize(BOOL /*calc_valid_rects*/, NCCALCSIZE_PARAMS* ncsp)
{
	::InflateRect(ncsp->rgrc, -BORDER, -BORDER);
}

/*********************************************************************/

int AutoCompletePopup::HitTest(CPoint point)
{
	CRect rcItem;
	CRect rcWnd;

	GetClientRect(rcWnd);
	long width= rcWnd.Width() - ScrollBarWidth();

	for (int i= top_index_; i < display_list_.size(); i++)
	{
		long y= i - top_index_;
		rcItem.SetRect(2, y * item_height_, width, (y + 1) * item_height_);

		if (PtInRect(&rcItem, point))
			return y + top_index_;
	}

	return -1;
}

/*********************************************************************/

LRESULT AutoCompletePopup::OnNcHitTest(CPoint point)
{
	CRect rect;
	GetWindowRect(rect);

	rect.DeflateRect(BORDER, BORDER);

	if (!rect.PtInRect(point))
		return HTNOWHERE;	// frame

	rect.left = rect.right - ::GetSystemMetrics(SM_CYVSCROLL);
	rect.top = rect.bottom - ::GetSystemMetrics(SM_CXVSCROLL);

	return rect.PtInRect(point) ? HTBOTTOMRIGHT : HTCLIENT;
}

/*********************************************************************/

void AutoCompletePopup::OnLButtonDown(UINT flags, CPoint point)
{
	if (edit_ctrl_ == 0)
		return;

	int sel= HitTest(point);

	if (sel >= 0)
	{
		SelectItem(sel);

		SelectAndHide();

		if (!text_selected_.empty())
			text_selected_();
	}
	else
	{
		CRect rc;
		GetClientRect(rc);
		if (!rc.PtInRect(point))
			ShowWindow(SW_HIDE);
	}
}

/*********************************************************************/

void AutoCompletePopup::OnRButtonDown(UINT flags, CPoint point)
{
	ShowWindow(SW_HIDE);
}

/*********************************************************************/

BOOL AutoCompletePopup::OnSetCursor(CWnd* wnd, UINT hit_test, UINT message)
{
	CRect client_rect;
	CPoint cursor;

	GetWindowRect(client_rect);
	ScreenToClient(&client_rect);

	client_rect.left = client_rect.right - GetSystemMetrics(SM_CYVSCROLL);
	client_rect.top = client_rect.bottom - GetSystemMetrics(SM_CXVSCROLL);

	GetCursorPos(&cursor);
	ScreenToClient(&cursor);

	if (client_rect.PtInRect(cursor)) // Vergrößerungs-Cursor
		return CWnd::OnSetCursor(wnd, hit_test, message);

	::SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));

	return TRUE;
}

/*********************************************************************/

void AutoCompletePopup::InvalidateAndScroll()
{
	vert_scrollbar_.SetScrollPos(top_index_, true);
	Invalidate();
	DoPaintMessageLoop();
}

/*********************************************************************/

bool AutoCompletePopup::EnsureVisible(int item, bool wait)
{
	if (item > top_index_ && item < top_index_ + visible_items_)
		return false; // is visible

	if (item > top_index_)	// scroll down
	{
		long len_ = item;
		for (int i = top_index_; i < len_; i++)
		{
			if (i >= display_list_.size() - visible_items_)
				break;
			if (i >= display_list_.size() - visible_items_ || i + visible_items_ > item)
				break;

			top_index_++;

			if (wait)
			{
				InvalidateAndScroll();
				Sleep(10);
				DoPaintMessageLoop();
			}
		}
		InvalidateAndScroll();
		return true;
	}

	if (item < top_index_)	// scroll up
	{
		while(item < top_index_)
		{
			if (top_index_ > 0)
				top_index_--;
			else
			{
				break;
			}
			if (wait)
			{
				InvalidateAndScroll();
				Sleep(10);
				DoPaintMessageLoop();
			}
		}

		InvalidateAndScroll();
		return true;
	}

	return false;
}

/*********************************************************************/

bool AutoCompletePopup::SelectItem(int item)
{
	if (item >= display_list_.size())
		return false;

	if (item < 0)
	{
		EnsureVisible(selected_item_, false);
		Invalidate();
		return false;
	}

	if (selected_item_ == item)
		return false;

	selected_item_ = item;

	if (!EnsureVisible(item, true))
		Invalidate();

	return true;
}

/*********************************************************************/

int AutoCompletePopup::FindStringExact(int start_after, LPCTSTR string)
{
	if (start_after > search_list_.size())
		return -1;

	for (size_t i= start_after + 1; i < search_list_.size(); ++i)
		if (search_list_.at(i) == string)
			return static_cast<int>(i);

	return -1;
}

/*********************************************************************/

void AutoCompletePopup::PopupList(bool resetCurSelection)
{
	if (display_list_.empty())
	{
		ShowWindow(SW_HIDE);
		return;
	}

	CRect rect(0,0,0,0);
	edit_ctrl_->GetWindowRect(rect);

	CRect wnd(0,0,0,0);
	edit_ctrl_->GetParent()->GetWindowRect(wnd);

	if (align_to_parent_)
		rect = wnd;

	int height= std::min<int>(std::max<int>(last_size_.cy, item_height_), static_cast<int>(display_list_.size()) * item_height_ + ::GetSystemMetrics(SM_CYBORDER) * 2);
	int width= rect.Width();// - 8;

	if (last_size_.cx > 0)
		width = last_size_.cx;

	width -= width % WIDTH_GRID;

	CRect area= ::GetWorkArea(wnd, true);

	CRect popup(CPoint(rect.left, rect.bottom + 1), CSize(width, height));

	if (popup.bottom > area.bottom)	// does it fit?
	{
		if (popup.Height() > area.Height() / 2)	// if it's big enough just make it smaller
		{
			popup.bottom = area.bottom;
		}
		else
		{
			// but if it is small then move it above edit ctrl

			popup.OffsetRect(0, -(height + rect.Height() + 2));

			if (popup.top < area.top)
			{
				popup.top = area.top;

				if (popup.Height() < item_height_)	// sanity check
					popup.bottom = popup.top + item_height_;
			}
		}
	}

	SetWindowPos(&CWnd::wndTopMost, popup.left, popup.top, popup.Width(), popup.Height(), 0);

	SetScroller();
	SetProp();

	ShowWindow(SW_SHOW);

	if (resetCurSelection) //actualCount != display_list_.size())
		selected_item_ = -1;
}


int AutoCompletePopup::FindString(int start_after, const TCHAR* str)
{
	if (edit_ctrl_ == 0 || str == 0 || *str == 0)
	{
		ShowWindow(SW_HIDE);
		return -1;
	}

	size_t length= _tcslen(str);

	const size_t actualCount= display_list_.size();

	if (start_after >= 0 && start_after > search_list_.size())
	{
		ShowWindow(SW_HIDE);
		return -1;
	}

	display_list_.clear();

	for (size_t i= start_after + 1; i < search_list_.size(); ++i)
	{
		const TCHAR* tmp= search_list_.at(i).c_str();

		if (_tcsncicmp(tmp, str, length) == 0)
			display_list_.push_back(&search_list_.at(i));
	}

	PopupList(actualCount != display_list_.size());

	return 1;	
}

/*********************************************************************/

int AutoCompletePopup::SelectString(LPCTSTR string)
{
	int item = FindString(-1, string);
	SelectItem(item);
	return item;
}

/*********************************************************************/

void AutoCompletePopup::OnShowWindow(BOOL show, UINT status)
{
	if (edit_ctrl_ == 0)
		return;

	if (show)
	{
		cur_item_ = -1;
		id_timer_ = SetTimer(IDTimerInstall, 100, NULL);
		edit_ctrl_->GetParent()->GetWindowRect(parent_rect_);
	}
	else
	{
		if (id_timer_)
			KillTimer(IDTimerInstall);

		id_timer_ = 0;
		selected_item_ = -1;
		top_index_ = 0;
	}

	CWnd::OnShowWindow(show, status);
	::SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));
}

/*********************************************************************/

const String* AutoCompletePopup::GetString() const
{
	if (display_list_.empty())
		return 0;

	size_t i= 0;

	if (static_cast<size_t>(selected_item_) < display_list_.size())
		i = selected_item_;

	return display_list_.at(i);
}

/*********************************************************************/

void AutoCompletePopup::OnVScroll(UINT sb_code, UINT pos, CScrollBar* scroll_bar)
{
	CWnd::OnVScroll(sb_code, pos, scroll_bar);
	long oldTopIndex= top_index_;

	switch (sb_code)
	{
	case SB_ENDSCROLL:
		break;

	case SB_PAGEUP:
		top_index_ -= visible_items_;
		break;

	case SB_PAGEDOWN:
		top_index_ += visible_items_;
		break;

	case SB_LINEUP:
		top_index_--;
		break;

	case SB_LINEDOWN:
		top_index_++;
		break;

	case SB_THUMBTRACK:
		top_index_ = pos;
		break;
	}

	if (top_index_ >= static_cast<int>(display_list_.size()) - visible_items_)
		top_index_ = static_cast<int>(display_list_.size()) - visible_items_;

	if (top_index_ < 0)
		top_index_ = 0;

	if (oldTopIndex != top_index_)
	{
		vert_scrollbar_.SetScrollPos(top_index_, true);
		Invalidate();
	}
}

/*********************************************************************/

void AutoCompletePopup::SetNextString(int key)
{
	if (display_list_.empty())
		return;

	int sel= selected_item_;

	switch (key)
	{
	case VK_DOWN:
		sel++;
		break;

	case VK_UP:
		sel--;
		break;

	case VK_PRIOR:
		sel -= visible_items_;
		break;

	case VK_NEXT:
		sel += visible_items_;
		break;

	case VK_HOME:
		sel = 0;
		break;

	case VK_END:
		sel = static_cast<int>(display_list_.size()) - 1;
		break;
	}

	if (sel < 0)
		sel = 0;
	if (sel >= static_cast<int>(display_list_.size()))
		sel = static_cast<int>(display_list_.size()) - 1;

	SelectItem(sel);
}

/*********************************************************************/

void AutoCompletePopup::OnMouseMove(UINT flags, CPoint point)
{
	CWnd::OnMouseMove(flags, point);
	int sel= HitTest(point);
	if (sel >= 0)
		SelectItem(sel);
}

/*********************************************************************/

void AutoCompletePopup::OnTimer(UINT_PTR event_id)
{
	CWnd::OnTimer(event_id);

	if (event_id != IDTimerInstall)
		return;

	if (edit_ctrl_ == 0)
		return;

	CRect rect;
	edit_ctrl_->GetParent()->GetWindowRect(rect);
	if (rect != parent_rect_)
		ShowWindow(SW_HIDE);
}

/*********************************************************************/

void AutoCompletePopup::OnGetMinMaxInfo(MINMAXINFO* minmax)
{
	if (GetSafeHwnd())
	{
		minmax->ptMinTrackSize.y = GetSystemMetrics(SM_CXHSCROLL) + GetSystemMetrics(SM_CYBORDER) * 2;
		minmax->ptMinTrackSize.x = GetSystemMetrics(SM_CXHSCROLL) * 4;
	}
	else	
		CWnd::OnGetMinMaxInfo(minmax);
}

/*********************************************************************/

struct CompareIString
{
	bool operator () (const String& s1, const String& s2) const
	{
		return _tcsicmp(s1.c_str(), s2.c_str()) < 0;
	}
};


void AutoCompletePopup::SortList(std::vector<String>& array)
{
	std::sort(array.begin(), array.end(), CompareIString());
}


CEdit* AutoCompletePopup::CurrentEditCtrl() const
{
	return edit_ctrl_;
}


void AutoCompletePopup::ShowList()
{
	if (edit_ctrl_ && search_list_.size() > 0)
		ShowAll();
}


void AutoCompletePopup::AlignToParent(bool enable)
{
	align_to_parent_ = enable;
}


void AutoCompletePopup::AutoPopup(bool enable)
{
	auto_popup_ = enable;
}

void AutoCompletePopup::AutoUnhook(bool enable)
{
	auto_unhook_ = enable;
}

void AutoCompletePopup::HandleUpDownKey(bool enable)
{
	handle_up_down_key_ = enable;
}

void AutoCompletePopup::RegisterTextSelectedFn(const boost::function<void ()>& fn)
{
	text_selected_ = fn;
}

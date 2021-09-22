/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// PhotoCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "PhotoCtrl.h"
#include "MemoryDC.h"
#include "PhotoInfoStorage.h"
#include "Color.h"
#include "PNGImage.h"
#include "WhistlerLook.h"
#include "block.h"
#include "ImageDraw.h"
#include "CatchAll.h"
#include "UIElements.h"
#include "CtrlDraw.h"
#include "Exception.h"
//#include "Color.h"
#include "GetDefaultGuiFont.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// PhotoCtrl

CImageList PhotoCtrl::img_list_group_icons_;
CImageList PhotoCtrl::img_list_indicators_;
CBitmap PhotoCtrl::ascending_bmp_;
CBitmap PhotoCtrl::descending_bmp_;
static const int OUTLINE= 3;
static const int EXTRA= 2 * (5 + OUTLINE); // 5 is a depth of shadow
const int INDICATOR_IMG_WIDTH= 40;	// this is pitch rather than width
const int INDICATOR_IMG_HEIGHT= 21;


PhotoCtrl::PhotoCtrl()
{
	item_size_ = CSize(0, 0);
	label_rect_.SetRectEmpty();
	image_rect_.SetRectEmpty();
	mode_ = THUMBNAILS;
	sticky_selection_ = false;
	enable_updating_ = true;
	enable_set_current_counter_ = 0;

	img_size_ = CSize(THUMB_SIZE, THUMB_SIZE);
	items_across_ = 2;

	header_height_ = 0;
	top_margin_ = 0;
	bottom_ = 0;

	show_label_ = true;
	show_tags_ = true;
	show_marker_ = true;

	ResetColors();

	horz_items_ = 0;

	host_ = 0;

	page_size_ = CSize(0, 0);
	max_size_ = CSize(0, 0);

	header_columns_.reserve(60);
	sort_by_column_ = 0;

	halftone_drawing_ = false;
	ballon_info_enabled_ = true;

	// tooltip support
	last_hit_ = 0;
	last_hit_code_ = -1;
	memset(&last_info_, 0, sizeof last_info_);

	// default font
	LOGFONT lf, bold;
	::GetDefaultGuiFont(lf);
	::GetDefaultGuiBoldFont(bold);
//	::GetObject(font, sizeof(lf), &lf);
	SetFont(lf, bold);

	//TODO: both img lists are color sensitive in non-XP windows
	if (img_list_group_icons_.m_hImageList == 0)
		PNGImage().LoadImageList(IDB_GROUP_HEADER_ICONS, -6, rgb_bk_color_, img_list_group_icons_);

	if (img_list_indicators_.m_hImageList == 0)
		PNGImage().LoadImageList(IDR_FILE_TYPES, INDICATOR_IMG_WIDTH, rgb_bk_color_, img_list_indicators_);

	if (ascending_bmp_.m_hObject == 0)
		ascending_bmp_.LoadMappedBitmap(IDB_ASCENDING);

	if (descending_bmp_.m_hObject == 0)
		descending_bmp_.LoadMappedBitmap(IDB_DESCENDING);
}


PhotoCtrl::~PhotoCtrl()
{
	// remove groups before deleting Items, because groups manipulate items during distruction
	groups_.RemoveAll();
}


BEGIN_MESSAGE_MAP(PhotoCtrl, CWnd)
	//{{AFX_MSG_MAP(PhotoCtrl)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_WM_MOUSEWHEEL()
	ON_WM_CREATE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_MOUSEMOVE()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_RBUTTONDBLCLK()
	ON_WM_SETCURSOR()
	ON_WM_TIMER()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_WM_CHAR()
	//}}AFX_MSG_MAP
	ON_NOTIFY(HDN_ITEMCHANGING, ID_HEADER, OnHeaderItemChanging)
	ON_NOTIFY(HDN_DIVIDERDBLCLICK, ID_HEADER, OnHeaderItemDblClick)
	ON_NOTIFY(HDN_ITEMCLICK, ID_HEADER, OnHeaderItemClick)
	ON_NOTIFY(HDN_ENDDRAG, ID_HEADER, OnHeaderItemEndDrag)
	ON_MESSAGE(WM_USER, OnHeaderItemEndDrag2)
	ON_NOTIFY(NM_RCLICK, ID_HEADER, OnHeaderRClick)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXT, 0, 0xFFFF, OnToolTipNotify)
	ON_WM_DESTROY()
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// PhotoCtrl message handlers


bool PhotoCtrl::Create(CWnd* parent, PhotoCtrlNotification* host, UINT id)
{
	host_ = host;

	if (!CWnd::CreateEx(0, AfxRegisterWndClass(CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS, ::LoadCursor(NULL, IDC_ARROW)),
		_T(""), WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN, CRect(0,0,0,0), parent, id))
		return false;

//	::SetWindowTheme(m_hWnd, L"explorer", nullptr);

	// create balloon tooltip window

	//CWnd* owner= GetParentOwner();
	if (!tool_tip_wnd_.Create(this, WS_POPUP | TTS_ALWAYSTIP | TTS_NOPREFIX | TTS_BALLOON))
		return false;

	tool_tip_wnd_.SendMessage(TTM_ACTIVATE, false);
	tool_tip_wnd_.SetMaxTipWidth(300);	// turns on multiline tool tips
	tool_tip_wnd_.SetDelayTime(1000);	// show up after 1 second
//	tool_tip_wnd_.SetDelayTime(TTDT_INITIAL, 1000);	// show up after 1 second
	tool_tip_wnd_.SetDelayTime(TTDT_AUTOPOP, 20000);	// stay on for 20 sec.
//	tool_tip_wnd_.SetDelayTime(TTDT_RESHOW, 10);

//EnableToolTips();

	return true;
}


int PhotoCtrl::OnCreate(LPCREATESTRUCT create_struct)
{
	if (CWnd::OnCreate(create_struct) == -1)
		return -1;

	if (!scroll_bar_.Create(*this, true))
		return -1;

	scroll_bar_.SetStyle(SB_BOTH, CoolScrollBar::COOLSB_FLAT);

	//	scroll_bar_.SetHotTrack(SB_BOTH, true);


	if (!header_wnd_.Create(WS_CHILD | HDS_HORZ | HDS_DRAGDROP | HDS_FULLDRAG | HDS_BUTTONS, CRect(0,0,0,0), this, ID_HEADER))
		return -1;

	header_wnd_.SetFont(&default_fnt_);

	// reposition now to calculate 'header_height_'
	SetHeader();

	return 0;
}


void PhotoCtrl::OnPaint()
{
	try
	{
		CPaintDC paint_dc(this); // device context for painting

		// rectangle to update
		Draw(paint_dc, paint_dc.m_ps.rcPaint);

		if (select_.lasso_)	//TODO: it's not perfect (may partially restore)
			DrawSelectionRect(paint_dc, false);	// restore selection rect
/*

// TODO: make sure it's not wider than window's client rect
	MemoryDC dc(paint_dc, CRect(CPoint(0, 0), item_size_), GetBkColor());

	int first= 0, last= -1;
	FindVisibleItems(first, last);

	for (int index= first; index <= last; ++index)
	{
		CRect rect= GetItemRect(index);

		if (paint_dc.RectVisible(rect))
		{
			dc.OffsetTo(rect);
			DrawItem(items_[index], dc, rect);
			dc.BitBlt();
		}
	}
*/
	}
	// do not catch anything in debug mode
#ifdef _DEBUG
	catch (float)
	{
	}
#else
	catch (...)
	{
	}
#endif
}


void PhotoCtrl::Draw(CDC& dc, const CRect& rect)
{
	if (IsEmpty())
		return;

	CRect client_rect(0,0,0,0);
	GetClientRect(client_rect);

	PrepareDC(dc);

	// off-screen bmp for a row of items
	int width= item_size_.cx;
	// item may be scrolled horizontally (even in thumbnail mode)
	int X_offset= scroll_bar_.GetScrollPos(SB_HORZ);
	if (mode_ == DETAILS)
	{
		// in detailed mode item_dc spans across window's width
		width = client_rect.Width();
	}

	MemoryDC item_dc(dc, CRect(CPoint(X_offset, 0), CSize(width, item_size_.cy)), GetBkColor());

	// off-screen bmp for a group's header
	CRect header_rect(CPoint(X_offset, 0), CSize(client_rect.Width(), Group::GetMaxHeaderHeight()));
	MemoryDC header_dc(dc, header_rect, GetBkColor());

	groups_.Draw(*this, dc, item_dc, header_dc);
}


BOOL PhotoCtrl::OnEraseBkgnd(CDC* dc)
{
	COLORREF rgb_back= GetBkColor();

	CRect rect;
	GetClientRect(rect);

	// do we have any items?
	if (IsEmpty())
	{
		dc->FillSolidRect(rect, rgb_back);
		return true;
	}

	int width= rect.Width();
	int height= rect.Height();

	// erase margin area
	if (mode_ != DETAILS)
	{
		PrepareDC(*dc);

		CRect fld_rect= rect;
		CPoint offset= GetScrollOffset();
		if (fld_rect.Width() < item_size_.cx + MARGIN_LEFT + MARGIN_RIGHT)
		{
			fld_rect.right = fld_rect.left + item_size_.cx + MARGIN_LEFT + MARGIN_RIGHT;
			fld_rect.OffsetRect(0, offset.y);
		}
		else
			fld_rect.OffsetRect(offset);

		// left side
		dc->FillSolidRect(0, fld_rect.top, MARGIN_LEFT, fld_rect.top + height, rgb_back);

		// right side
		dc->FillSolidRect(fld_rect.right - MARGIN_RIGHT, fld_rect.top, MARGIN_RIGHT, fld_rect.top + height, rgb_back);

		// top side
		dc->FillSolidRect(fld_rect.left + MARGIN_LEFT, 0, fld_rect.Width() - MARGIN_LEFT - MARGIN_RIGHT, MARGIN_TOP, rgb_back);

		// there's no bottom margin so far, but it could be added
		// bottom side
		//	dc->FillSolidRect(MARGIN_LEFT, bottom_?, width - MARGIN_LEFT - MARGIN_RIGHT, MARGIN_TOP, rgb_back);
	}

	if (bottom_ < rect.bottom)
	{
		rect.top = bottom_;
		dc->FillSolidRect(rect, rgb_back);
	}

	return true;
}


COLORREF PhotoCtrl::GetBkColor()
{
	return rgb_bk_color_ == -1 ? ::GetSysColor(COLOR_WINDOW) : rgb_bk_color_;
}

/*
void PhotoCtrl::FindVisibleItems(int& first, int& last)
{
	CRect rect= GetFieldRect();

	first = 0;
	last = -1;

	if (rect.Height() <= 0)
		return;

	rect.OffsetRect(0, -top_margin_);	

	if (horz_items_ < 1 || item_size_.cx <= 0 || item_size_.cy <= 0)
	{
		ASSERT(false);
		return;
	}

	int top= rect.top / item_size_.cy * horz_items_;
	int btm= (rect.bottom + item_size_.cy - 1) / item_size_.cy * horz_items_;

	if (top <= btm)
	{
		int count= items_.size();

		if (top < 0 || top >= count)
		{
			ASSERT(false);
			top = 0;
		}

		if (btm < 0)
			btm = top - 1;
		else if (btm >= count)
			btm = count - 1;

		first = top;
		last = btm;
	}
	else
	{
		ASSERT(false);
	}
}


CRect PhotoCtrl::GetItemRect(int index)
{
	CRect rect(CPoint(0, 0), item_size_);

	if (horz_items_ > 0)
	{
		int row= index / horz_items_;
		int col= index % horz_items_;

		rect.OffsetRect(col * item_size_.cx, row * item_size_.cy + top_margin_);
	}

	return rect;
}
*/

///////////////////////////////////////////////////////////////////////////////

CSize PhotoCtrl::SetItemsDimensions(Mode mode, bool bigItems)
{
	top_margin_ = 0;
	margins_rect_ = CRect(MARGIN_LEFT, MARGIN_TOP, MARGIN_RIGHT, MARGIN_BOTTOM);
	CSize margin_size(margins_rect_.left + margins_rect_.right, margins_rect_.top + margins_rect_.bottom);
	label_rect_.SetRect(margins_rect_.left, margins_rect_.top, 0, 0);
	const int LABEL_SPACE= 2;
	const int LABEL_OFFSET= 4;
	CSize item_size(0, 0);
	bool adjustSize= true;
	int lines= show_label_ ? (host_ ? host_->GetLabelLines(mode) : 1) : 0;

	// Note: what really counts here is final image_rect_ size, and label_rect_ size
	// label_rect_ will be used to determine label height

	switch (mode)
	{
	case THUMBNAILS:
		image_rect_.SetRect(0, 0, img_size_.cx, img_size_.cy);
margin_size.cx = 4;
margin_size.cy = 4;
		label_rect_ = image_rect_;
		label_rect_.OffsetRect(0, image_rect_.Height());
		label_rect_.bottom = label_rect_.top + (lines ? lines * M_size_.cy + 2 * LABEL_SPACE : 0);
		//item_size = CSize(image_rect_.Width(), image_rect_.Height()+label_rect_.Height());
		break;

	case DETAILS:
		margins_rect_.SetRect(MARGIN_LEFT, 0, 0, 0);
		item_size = CSize(ColumnsTotalWidth(), M_size_.cy) + CSize(0, 12);//4);
		label_rect_.SetRectEmpty();
		image_rect_.SetRectEmpty();
		top_margin_ += header_height_;
		adjustSize = false;
		break;

	case TILES:
		image_rect_.SetRect(0, 0, 88, std::max(70L, M_size_.cy * 5) + LABEL_OFFSET);
		label_rect_ = image_rect_;
		label_rect_.OffsetRect(image_rect_.Width() + LABEL_SPACE * 3, LABEL_OFFSET);
		label_rect_.right = label_rect_.top + 17 * M_size_.cx + LABEL_SPACE;
		label_rect_.bottom -= LABEL_OFFSET;
		margin_size.cx *= 2;
		margin_size.cy *= 3;
		break;

	case PREVIEWS:
		{
			std::pair<int, int> widths= CalcItemWidth(items_across_);
			//NOTE: height is adjusted in PhotoCtrl::Group::SetLocation
			item_size.cx = item_size.cy = bigItems ? widths.second : widths.first;
		}
		image_rect_.SetRect(0, 0, item_size.cx, item_size.cy);
		label_rect_ = image_rect_;
		label_rect_.OffsetRect(0, image_rect_.Height());
		label_rect_.bottom = label_rect_.top + (lines ? lines * M_size_.cy + 2 * LABEL_SPACE : 0);
		break;

	/*case LIST:
		if (bigItems)
			return CSize(0, 0);
		{
			margins_rect_.SetRect(MARGIN_LEFT, 0, 0, 0);
			image_rect_.SetRectEmpty();
			CRect img(0, 0, M_size_.cy * 3 / 2, M_size_.cy);
			CClientDC dc(this);
			dc.SelectObject(&default_fnt_);
			int maxWidth= dc.GetTextExtent(_T("XXXXXXXXXX"), 10).cx * 10;
			int width= 0;
			if (host_)
				for (ItemList::const_iterator it= items_list_.Begin(); it != items_list_.End(); ++it)
				{
					String label= host_->GetItemLabel(it->photo_, dc, maxWidth);
					int w= dc.GetTextExtent(label.c_str(), static_cast<int>(label.length())).cx;
					if (w > width && w < maxWidth)
						width = w;
				}
			item_size = CSize(img.Width() + 2 + width + 2, img.Height()) + CSize(0, 4);
			label_rect_.SetRect(img.Width() + 2, 0, item_size.cx - 2, item_size.cy);
			image_rect_.right = img.Width();
		}
		adjustSize = false;
		break;

	case MOSAIC:
		image_rect_.SetRect(0, 0, img_size_.cx, img_size_.cy);
		margin_size = CSize(0, 0);
		margins_rect_.SetRectEmpty();
		label_rect_.SetRectEmpty();
		item_size = image_rect_.Size();
		adjustSize = false;
		break;
*/
	default:
		ASSERT(false);
		return CSize(0, 0);
	}

	if (adjustSize)
	{
		item_size = (image_rect_ | label_rect_).Size();
		item_size += margin_size;

		image_rect_.OffsetRect(MARGIN_LEFT, MARGIN_TOP);
		label_rect_.OffsetRect(MARGIN_LEFT, MARGIN_TOP);
	}
//TRACE(L"item rect: %d x %d\n", image_rect_.Width(), image_rect_.Height());
	return item_size;
}


void PhotoCtrl::ResetItemsLocation()
{
	ResetMode(mode_, true);
}


void PhotoCtrl::SetMode(Mode mode)
{
	ResetMode(mode, true);
}


void PhotoCtrl::ResetMode(Mode mode, bool resetLocations)
{
//if (mode == TILES) mode = LIST;
//if (mode == TILES) mode = MOSAIC;

	// big item
	CSize item_big_size= SetItemsDimensions(mode, true);
	// normal item
	CSize item_size= SetItemsDimensions(mode, false);

	mode_ = mode;

	if (header_wnd_.m_hWnd)
	{
		header_wnd_.ShowWindow(mode_ == DETAILS ? SW_SHOWNA : SW_HIDE);

		// calc item width
		//header_wnd_.
	}

	if (m_hWnd == 0)
	{
		item_size_ = item_size;
		return;
	}

	if (resetLocations)
	{
		// TODO: try to preserve current scroll offset
		scroll_bar_.SetScrollPos(SB_HORZ, 0);
		scroll_bar_.SetScrollPos(SB_VERT, 0);
	}

	RemoveToolTips();

	Invalidate();

	// finally set layout of items, calculate height of rows as needed
	if (item_size.cx > 0 && item_size.cy > 0)
	{
		bool useBigItem= SetLocation(item_size, item_big_size);
		item_size_ = SetItemsDimensions(mode, useBigItem);
	}

	SetHeader();

	Invalidate();

	if (resetLocations)
	{
		// scroll to the current item
		EnsureVisible(current_item_);
	}
}


void PhotoCtrl::GetClientRects(CRect& small_rect, CRect& big_rect)
{
	CRect rect(0,0,0,0);
	GetClientRect(rect);

	small_rect = rect;
	big_rect = rect;

	DWORD dwStyle= GetStyle();

	CSize scroll_bars_size= CSize(::GetSystemMetrics(SM_CXVSCROLL), ::GetSystemMetrics(SM_CXVSCROLL));

	// add extra space occupied by scrollbars
	if (dwStyle & WS_VSCROLL)
		big_rect.right += scroll_bars_size.cx;
	else
		small_rect.right -= scroll_bars_size.cx;

	if (dwStyle & WS_HSCROLL)
		big_rect.bottom += scroll_bars_size.cy;
	else
		small_rect.bottom -= scroll_bars_size.cy;
}


std::pair<int, int> PhotoCtrl::CalcItemWidth(int items_across)
{
	CRect small_rect, big_rect;
	GetClientRects(small_rect, big_rect);

	const int MIN_WIDTH= 220;
	//TODO: those calculations are not precise enough
	int small_width= small_rect.Width() - MARGIN_LEFT - MARGIN_RIGHT;
	int big_width= big_rect.Width() - MARGIN_LEFT - MARGIN_RIGHT;
	int img_extra= EXTRA / 2 - 1;	// extra space per item (approx.)

	return std::make_pair(std::max(MIN_WIDTH, small_width / items_across - img_extra), std::max(MIN_WIDTH, big_width / items_across - img_extra));
}


void PhotoCtrl::ChangeMode(Mode mode)
{
	if (mode != mode_)
		SetMode(mode);
}


void PhotoCtrl::SetFont(LOGFONT normal, LOGFONT bold)
{
	if (default_fnt_.m_hObject)
		default_fnt_.DeleteObject();

	default_fnt_.CreateFontIndirect(&normal);

	CDC dc;
	dc.CreateDC(_T("DISPLAY"), 0, 0, 0);
	CFont* old= dc.SelectObject(&default_fnt_);

	M_size_ = dc.GetTextExtent(_T("M"), 1);

	dc.SelectObject(old);

	SetMode(mode_);

	if (header_wnd_.m_hWnd)
		header_wnd_.SetFont(&default_fnt_);

	if (group_fnt_.m_hObject)
		group_fnt_.DeleteObject();

	bold.lfUnderline = true;
//	lf.lfWeight = FW_BOLD;
	group_fnt_.CreateFontIndirect(&bold);

	if (tags_fnt_.m_hObject)
		tags_fnt_.DeleteObject();

	bold.lfUnderline = false;
//	lf.lfWeight = FW_BOLD;
	tags_fnt_.CreateFontIndirect(&bold);

	if (m_hWnd)
		Invalidate();
}


void PhotoCtrl::SetTagFont(/*const */LOGFONT& font)
{
	if (tags_fnt_.m_hObject)
		tags_fnt_.DeleteObject();
	//font.lfHeight += 1;
	tags_fnt_.CreateFontIndirect(&font);

	if (m_hWnd)
		Invalidate();
}

///////////////////////////////////////////////////////////////////////////////

void PhotoCtrl::OnSize(UINT type, int cx, int cy)
{
	try
	{
		//	CWnd::OnSize(type, cx, cy);

		static bool in_update= false;

		if (in_update)
		{
			//TRACE(L"OnSize blocked, vbar: %d\n", !!(GetStyle() & WS_VSCROLL));
			return;
		}

		Block update(in_update);

		//TRACE(L"OnSize, vbar: %d\n", !!(GetStyle() & WS_VSCROLL));

		if (mode_ == PREVIEWS)
			SetMode(mode_);
		else
			Resize();
	}
	CATCH_ALL
}


CRect PhotoCtrl::GetFieldRect(bool shifted/*= true*/) const
{
	CRect rect(0,0,0,0);
	if (m_hWnd == 0)
		return rect;

	GetClientRect(rect);

	if (rect.IsRectEmpty())
		return rect;

	rect.top += top_margin_;

	if (shifted)
		rect.OffsetRect(GetScrollOffset());

	return rect;
}


void PhotoCtrl::SetHeader()
{
	if (header_wnd_.m_hWnd == 0)
		return;

	CRect rect;
	GetClientRect(rect);

	HDLAYOUT  hdl;
	WINDOWPOS wpos;
	CRect header_rect= rect;
	//TODO: this is problematic in multimonitor configurations
	header_rect.right = 10000;
	hdl.prc = header_rect;
	hdl.pwpos = &wpos;

	if (header_wnd_.Layout(&hdl))
	{
		// Reposition the header control so that it is placed at 
		// the top of its parent window's client area
		::SetWindowPos(header_wnd_, wpos.hwndInsertAfter, wpos.x - scroll_bar_.GetScrollPos(SB_HORZ), wpos.y, wpos.cx, wpos.cy, wpos.flags);

		header_height_ = wpos.cy;
	}
}


void PhotoCtrl::Resize()
{
	CRect rect;
	GetClientRect(rect);

	if (rect.IsRectEmpty())
		return;

//	TRACE(L"resizing: vbar %d\n", !!(GetStyle() & WS_VSCROLL));

	if (item_size_.cx > 0 && item_size_.cy > 0)
		ResetMode(mode_, false);
	else
		SetHeader();
}


void PhotoCtrl::SetScrollBar(int bar, SCROLLINFO& si)
{
	if (bar == SB_VERT)
	{
		// change scrollbar only if page/max value has changed
		if (page_size_.cy != si.nPage || max_size_.cy != si.nMax)
		{
			page_size_.cy = si.nPage;
			max_size_.cy = si.nMax;
			scroll_bar_.SetScrollInfo(SB_VERT, si, true);
//TRACE(L"page %d  max %d\n", page_size_.cy, max_size_.cy);
//SetWindowPos(0, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
		}
	}
	else if (bar == SB_HORZ)
	{
		// change scrollbar only if page/max value has changed
		if (page_size_.cx != si.nPage || max_size_.cx != si.nMax)
		{
			page_size_.cx = si.nPage;
			max_size_.cx = si.nMax;
			scroll_bar_.SetScrollInfo(SB_HORZ, si, true);
		}
	}
	else
		ASSERT(false);
}


void PhotoCtrl::ResetScrollSize()
{
#if 0
	bool details= mode_ == DETAILS;

	CRect rect= GetFieldRect(false);
	int height= rect.Height();

//	int count= GetItemCount(); // items_.size();
//	int items_vert= 0;
//	if (horz_items_ > 0)
//		items_vert = (count + horz_items_ - 1) / horz_items_;

	// set vertical scrollbar range
	{
		SCROLLINFO si;
		si.cbSize = sizeof si;
		si.nPage = height;
		si.nMin = 0;
		si.nMax = max(0, bottom_ - top_margin_);
		si.fMask = SIF_RANGE | SIF_PAGE;
TRACE(L"scrollbar: %d\n", si.nMax);
		// change scrollbar only if page/max value has changed
		if (page_size_.cy != si.nPage || max_size_.cy != si.nMax)
		{
			page_size_.cy = si.nPage;
			max_size_.cy = si.nMax;
			scroll_bar_.SetScrollInfo(SB_VERT, &si);
		}
	}

	// set horizontal scrollbar range
	if (details)
	{
		ResetHorzScrollBar();
	}
	else
	{
		SCROLLINFO si;
		si.cbSize = sizeof si;
		si.nPage = max(0, rect.Width() - MARGIN_LEFT - MARGIN_RIGHT);
		si.nMin = 0;
		si.nMax = item_size_.cx - 1;
		si.fMask = SIF_RANGE | SIF_PAGE;

		// change scrollbar only if page/max value has changed
		if (page_size_.cx != si.nPage || max_size_.cx != si.nMax)
		{
			page_size_.cx = si.nPage;
			max_size_.cx = si.nMax;
			scroll_bar_.SetScrollInfo(SB_HORZ, &si);
		}
	}
#endif
}

///////////////////////////////////////////////////////////////////////////////

void PhotoCtrl::RemoveAll()
{
	current_item_.Clear();
	anchor_item_.Clear();
	if (host_)
		host_->CurrentItemChanged(0);

	select_.selected_count_ = 0;

	groups_.RemoveAll();

	items_list_.RemoveAll();

	bottom_ = 0;

	if (m_hWnd)
	{
//		ResetScrollSize();
		SetLocation(item_size_);
		Invalidate();
	}
}


void PhotoCtrl::MoveCurrentToUnselected()
{
	if (!current_item_)
		return;

	anchor_item_.Clear();

	int current= current_item_.Group()->FindItemIndex(current_item_.Item());

	int index= current - 1;

	while (index >= 0)
	{
		Item* item= current_item_.Group()->GetItem(index--);

		if (!item->IsSelected())
		{
			current_item_.SetItem(item);
			return;
		}
	}

	index = current + 1;

	const int count= current_item_.Group()->GetItemCount();

	while (index < count)
	{
		Item* item= current_item_.Group()->GetItem(index++);

		if (!item->IsSelected())
		{
			current_item_.SetItem(item);
			return;
		}
	}

	//TODO: change group

	current_item_.Clear();
}


void PhotoCtrl::RemoveSelected()
{
	// remove selected items; reset current (it's necessary); send selection changed notification

	bool curChanged= false;

	if (current_item_ && current_item_.Item()->IsSelected())
	{
		MoveCurrentToUnselected();
		curChanged = true;
	}

	// remove from all groups
	groups_.RemoveSelected();

	// now remove items from the storage
	items_list_.RemoveSelected();

	if (m_hWnd)
	{
		Invalidate();
		Resize();
	}

	// all selected items are removed now; send selection changed notification
	if (host_)
	{
		// no selection
		VectPhotoInfo selected_photos;
		// send it
		host_->SelectionChanged(selected_photos);

		if (curChanged)
			host_->CurrentItemChanged(current_item_.Item() != 0 ? current_item_.Item()->photo_ : 0);
	}
}


void PhotoCtrl::RemoveItem(Item* item, bool notify)
{
	ASSERT(item);

	if (item->IsSelected())
	{
		item->SetSelection(false);
		if (notify)
			SelectionChanged();
	}

	bool cur_change= false;
	if (current_item_.Item() == item)
	{
		current_item_.Clear();
		anchor_item_.Clear();
		cur_change = true;
	}

	VERIFY(groups_.RemoveItem(this, item));
	VERIFY(items_list_.Remove(item->photo_));

	if (notify && cur_change && host_ != 0)
		host_->CurrentItemChanged(0);
}


// remove one item
bool PhotoCtrl::RemoveItem(PhotoInfoPtr photo, bool notify)
{
	Item* item= items_list_.FastFind(photo);

	if (item == 0)
	{
		ASSERT(false);
		return false;
	}

	RemoveItem(item, notify);

	if (m_hWnd)
	{
		Invalidate();
		Resize();
	}

	return true;
}


void PhotoCtrl::RemoveGroups()
{
	// current item is group/item pair so it has to be reset
	current_item_.Clear();
	anchor_item_.Clear();
	if (host_)
		host_->CurrentItemChanged(0);

	groups_.RemoveAll();

	bottom_ = 0;

	if (m_hWnd)
	{
//		ResetScrollSize();
		SetLocation(item_size_);
		Invalidate();
	}
}


void PhotoCtrl::RemoveGroup(Group* group)
{
	if (current_item_.Group() == group)
		current_item_.Clear();
	if (anchor_item_.Group() == group)
		anchor_item_.Clear();

	groups_.Remove(group);
}


void PhotoCtrl::InsertItem(PhotoInfoPtr photo, size_t index, Group* group)
{
	ASSERT(group != 0);

	group->InsertItem(items_list_, photo, index);
	{
//		InvalidateGroup(group);
		// invalidate all: groups below might need refresh too
		if (enable_updating_)
		{
			Invalidate();
			Resize();
		}
	}
}


void PhotoCtrl::UpdateItems(PhotoInfoPtr* begin, PhotoInfoPtr* end, Group* group)
{
	ASSERT(group != 0);

	// allocate space; if successful no exception can occur later while adding items
	// if exception is thrown items_ and sorted_ will still remain in sync
//	group->Reserve(end - begin);

	bool change= false;

	// check photos from begin to end to see if there's any new ones

//	for (PhotoInfoPtr* photo= begin; photo != end; ++photo)
	{

/*		if (group->FindItem(*photo) == 0)	// new photo?
		{
			// add photo to internal storage
			items_list_.push_back(*photo);

			// add item to the group
			group->AddItem(&items_list_.back());

			change = true;
		} */
	}

	auto changes= group->UpdateItems(items_list_, begin, end);

	// go over all itmes that:
	// - were just added to this 'group' and
	// - were present in the control already
	// we want them removed from all other groups
	for (auto item : changes.second)
		if (auto gr= groups_.RemoveItem(nullptr, item, group))
			if (gr->GetItemCount() == 0)
				RemoveGroup(gr);

	if (changes.first)
	{
//		InvalidateGroup(group);
		// invalidate all: groups below might need refresh too
		Invalidate();
		Resize();
	}
}


// invalidate window's rect at group location
void PhotoCtrl::InvalidateGroup(Group* group)
{
	ASSERT(group);
	if (group)
		InvalidateRect(group->GetLocation() - GetScrollOffset());
}


void PhotoCtrl::Group::InsertItem(ItemList& items_list, PhotoInfoPtr photo, size_t index)
{
	if (Item* item= items_list.FastFind(photo))
	{
		ASSERT(false);
		return;
	}

	auto new_item= items_list.Insert(photo);

	if (index <= items_.size())
	{
		items_.insert(items_.begin() + index, new_item);
		new_item->SetGroup(this);
	}
	else
	{
		ASSERT(false);	// index out of range
		THROW_EXCEPTION(L"Photo UI control error", L"Internal error: inserting items to the list control failed due to the wrong index specified");
	}
}

	//ItemVector items;
	//items.reserve(end - begin);
	//for (PhotoInfoPtr* photo= begin; photo != end; ++photo)
	//{
	//	if (Item* item= items_list.FastFind(*photo))
	//		items.push_back(item);
	//	else
	//		items.push_back(items_list.Insert(*photo));
	//}
	//items_.swap(items);


// set group items; return true if changes were detected; return vector of items there were already present in this control
std::pair<bool, PhotoCtrl::ItemVector> PhotoCtrl::Group::UpdateItems(ItemList& items_list, PhotoInfoPtr* begin, PhotoInfoPtr* end)
{
	size_t count= end - begin;
	if (count < items_.size())	// removed items?
	{
		return std::make_pair(true, ResetItems(items_list, begin, end));
	}
	else if (count == items_.size())
	{
		// same amount of items; just check if order is same
		for (size_t i= 0; i < count; ++i)
			if (items_[i]->photo_ != begin[i])
			{
				// order has changed--reset items in the group
				return std::make_pair(true, ResetItems(items_list, begin, end));
			}

		return std::make_pair(false, ItemVector());	// no changes detected
	}
	else	// more items (some new items were added to the group)--simply reset group's items
	{
		return std::make_pair(true, ResetItems(items_list, begin, end));
/*
		items_.reserve(count);
		ItemVector::iterator it= items_.begin();

		for (int i= 0; i < count; ++i)
		{
			if (it != items_.end())
			{
				if ((*it)->photo_ != begin[i])
				{
					// make sure this is new item being added; if this is an existing
					// photo UpdateItems() is invoked to change sort order
					if (items_list.FastFind(begin[i]) != 0)
					{
						// order has changed--reset items in the group
						ResetItems(items_list, begin, end);
						return true;
					}

					// add photo to internal storage and then insert into the group
					it = items_.insert(it, items_list.Insert(begin[i]));
				}
			}
			else
			{
				if (items_list.FastFind(begin[i]) != 0)
				{
					// order has changed--reset items in the group
					ResetItems(items_list, begin, end);
					return true;
				}

				// add photo to internal storage
				items_list.Insert(begin[i]);
//				items_list.push_back(begin[i]);

				items_.push_back(items_list.Back());//&items_list.back());
				it = items_.end();
			}
		}

		// if order of items in a group was different than begin..end then size won't match--simply reset items
		// that should not happen though
		if (items_.size() != count)
		{
			ASSERT(false);
			ResetItems(items_list, begin, end);
		}
*/
	}

//	return true;	// group has changed
}


PhotoCtrl::ItemVector PhotoCtrl::Group::ResetItems(ItemList& items_list, PhotoInfoPtr* begin, PhotoInfoPtr* end)
{
	ItemVector existing_items;
	ItemVector items;
	items.reserve(end - begin);
	for (PhotoInfoPtr* photo= begin; photo != end; ++photo)
	{
		if (Item* item= items_list.FastFind(*photo))
		{
			items.push_back(item);
			existing_items.push_back(item);
		}
		else
			items.push_back(items_list.Insert(*photo));
	}
	items_.swap(items);

	return existing_items;
}


// helper fn: collects Item on corresponding to the range of photos in vector; those
// items are first searched for in local storage (items_list_), if not found
// they are created as 'items_list' elements
std::vector<PhotoCtrl::Item*> PhotoCtrl::CollectItems(PhotoInfoPtr* begin, PhotoInfoPtr* end, std::list<Item>& items_list, std::vector<Item*>& items)
{
	items.clear();
	items.reserve(end - begin);
	items_list.clear();
	std::vector<Item*> existing_items;

	for (PhotoInfoPtr* photo= begin; photo != end; ++photo)
	{
		ASSERT(*photo);

		// find item inside storage list
		Item* item= items_list_.FastFind(*photo);

		if (item == 0)	// new item?
		{
			// add it to the list; this list will be appended to the storage list 
			items_list.push_back(Item(*photo));
			item = &items_list.back();
		}
		else
			existing_items.push_back(item);

		items.push_back(item);
	}

	return existing_items;
}


void PhotoCtrl::InsertItem(PhotoInfoPtr photo, size_t index, const String& group_name, Icons group_icon, int group_id)
{
	if (Group* group= groups_.Find(group_id))
	{
		// insert item into the existing group
		InsertItem(photo, index, group);
	}
	else
		AddItems(&photo, &photo + 1, group_name, group_icon, group_id, true);
}


void PhotoCtrl::AddItems(PhotoInfoPtr* begin, PhotoInfoPtr* end, const String& group_name, Icons group_icon, int group_id, bool)
{
	// only new groups expected here
	if (Group* group= groups_.Find(group_id))
	{
		UpdateItems(begin, end, group);

		//// remove empty group
		//if (group->GetItemCount() == 0)
		//	RemoveGroup(group);

		return;
	}

	// temp list with new 'Items'
	std::list<Item> items_list;
	std::vector<Item*> items;
	auto existing_items= CollectItems(begin, end, items_list, items);

	// reserve space (this operation may throw)
	items_list_.ReserveSpace(items_list.size());

	// new group (may throw)
	std::auto_ptr<Group> new_group(new Group(group_name, group_id, group_icon, items));

	// add new group to the list of groups at the right place (may throw)
	Group* group= groups_.Insert(new_group);

#if 0 // this code is not needed if groups either do not change at all (like during sort), or they have been all removed and are being recreated

	// this is a new group we are adding, so if some photos it contains are already present, they have to be removed
	// but only from other groups, not this one (this operation doesn't throw)
	for (auto item : existing_items)
		if (auto gr= groups_.RemoveItem(nullptr, item, group))
		{
			if (gr->GetItemCount() == 0)
				RemoveGroup(gr);
		}
#endif
	// store 'Items' in items_list_ (this operation will not throw if ReserveSpace() was used)
	items_list_.Splice(items_list);

	// current item
	if (enable_set_current_counter_ >= 0 && group != 0 && !current_item_ && !items.empty())
	{
		// there's no current item: set it now
		current_item_.Set(group, items.front());
		if (host_)
			host_->CurrentItemChanged(current_item_.Item()->photo_);
	}

	if (enable_updating_)
	{
		Resize();

		if (group != 0 && m_hWnd)
			InvalidateGroup(group);
	}
}


void PhotoCtrl::AddItems(VectPhotoInfo::iterator begin, VectPhotoInfo::iterator end, const String& group_name, Icons group_icon, int group_id)
{
	if (begin != end)
		AddItems(&*begin, &*--end + 1, group_name, group_icon, group_id, false);
}


void PhotoCtrl::AddItems(VectPhotoInfo& photos, const String& group_name, Icons group_icon, int group_id)
{
	if (!photos.empty())
		AddItems(photos.begin(), photos.end(), group_name, group_icon, group_id);
}


// move item from one group into another
//
void PhotoCtrl::MoveItem(PhotoInfoPtr photo, const String& group_name, Icons group_icon, int group_id)
{
	if (ItemGroupPair igp= FindItem(photo))	// existing item?
	{
		if (igp.Group()->GetId() != group_id)	// moving item to a different group?
		{
			if (Group* group= groups_.Find(group_id))	// moving to the existing group?
			{
				group->AddItem(igp.Item());
				igp.Group()->RemoveItem(igp.Item());

				Resize();
			}
			else	// moving existing item to the new group
			{
				// new group
				std::auto_ptr<Group> new_group(new Group(group_name, group_id, group_icon, igp.Item()));

				// add to the groups at the right place
				groups_.Insert(new_group);

//				AddItems(&photo, &photo + 1, group_name, group_icon, group_id);
				igp.Group()->RemoveItem(igp.Item());
			}

			Resize();
		}
	}
	else	// new item
	{
		if (Group* group= groups_.Find(group_id))
		{
			group->AddItem(items_list_.Insert(photo));
			//
			Resize();
		}
		else	// new group
		{
			AddItems(&photo, &photo + 1, group_name, group_icon, group_id, false);
		}
	}

	Invalidate();
}


/*
void PhotoCtrl::AddItems(PhotoInfoStorage& photos)
{
	ASSERT(m_hWnd != 0);
	RemoveAll();

	items_.reserve(photos.size());

	for (PhotoInfoStorage::iterator it= photos.begin(); it != photos.end(); ++it)
		items_.push_back(Item(it->get()));

	// reset current item
	if (!items_.empty())
		current_item_ = &items_.front();

	Resize();
}


void PhotoCtrl::AddItem(PhotoInfoPtr photo)
{
	ASSERT(m_hWnd != 0);
	ASSERT(photo != 0);

	items_.push_back(Item(photo));

//	InvalidateItem(items_.back());

	Resize();
}
*/

void PhotoCtrl::SetCurrentItem(PhotoInfoPtr photo, bool select_current, bool scroll/*= true*/, bool notify/*= false*/)
{
	GoTo(FindItem(photo), select_current, false, false, notify, scroll);
}


CRect PhotoCtrl::GetItemRect(PhotoInfoPtr photo, bool bounding_box) const
{
	if (ItemGroupPair itg= const_cast<PhotoCtrl*>(this)->FindItem(photo))
		return itg.GetItemRect(bounding_box);
	else
		return CRect(0, 0, 0, 0);
}


///////////////////////////////////////////////////////////////////////////////


void PhotoCtrl::OnHScroll(UINT sb_code, UINT pos, CScrollBar* scroll_bar)
{
	if (sb_code == SB_THUMBPOSITION || sb_code == SB_THUMBTRACK)
	{
		SCROLLINFO si;
		memset(&si, 0, sizeof si);
		si.cbSize = sizeof si;
		si.fMask = SIF_TRACKPOS;
		scroll_bar_.GetScrollInfo(SB_HORZ, si);
		pos = si.nTrackPos;
	}
	OnScroll(MAKEWORD(sb_code, -1), pos);
}


void PhotoCtrl::OnVScroll(UINT sb_code, UINT pos, CScrollBar* scroll_bar)
{
	if (sb_code == SB_THUMBPOSITION || sb_code == SB_THUMBTRACK)
	{
		SCROLLINFO si;
		memset(&si, 0, sizeof si);
		si.cbSize = sizeof si;
		si.fMask = SIF_TRACKPOS;
		scroll_bar_.GetScrollInfo(SB_VERT, si);
		pos = si.nTrackPos;
	}
	OnScroll(MAKEWORD(-1, sb_code), pos);
}


void PhotoCtrl::OnScroll(UINT scroll_code, UINT pos)
{
	CRect rect;
	GetClientRect(rect);

	CSize line_size(30, item_size_.cy / 2);
	if (mode_ == DETAILS/* || mode_ == LIST*/)
		line_size.cy = item_size_.cy;
	else if (mode_ == PREVIEWS)
		line_size.cy = item_size_.cy / 5;	//TODO: improve to line up photos at the top

	// calc new x position
	int x= scroll_bar_.GetScrollPos(SB_HORZ);
	int xOrig= x;

	switch (LOBYTE(scroll_code))
	{
	case SB_TOP:
		x = 0;
		break;
	case SB_BOTTOM:
		x = INT_MAX;
		break;
	case SB_LINEUP:
		x -= line_size.cx;
		break;
	case SB_LINEDOWN:
		x += line_size.cx;
		break;
	case SB_PAGEUP:
		x -= rect.Width();
		break;
	case SB_PAGEDOWN:
		x += rect.Width();
		break;
	case SB_THUMBTRACK:
		x = pos;
		break;
	}

	// calc new y position
	int y= scroll_bar_.GetScrollPos(SB_VERT);
	int yOrig= y;

	switch (HIBYTE(scroll_code))
	{
	case SB_TOP:
		y = 0;
		break;
	case SB_BOTTOM:
		y = INT_MAX;
		break;
	case SB_LINEUP:
		y -= line_size.cy;
		break;
	case SB_LINEDOWN:
		y += line_size.cy;
		break;
	case SB_PAGEUP:
		y -= rect.Height();
		break;
	case SB_PAGEDOWN:
		y += rect.Height();
		break;
	case SB_THUMBTRACK:
		y = pos;
		break;
	case 98:
		y -= pos;
		break;
	case 99:
		y += pos;
		break;
	}

	bool do_scroll= true;
	BOOL result = OnScrollBy(CSize(x - xOrig, y - yOrig), do_scroll);
	if (result && do_scroll)
		UpdateWindow();
}


BOOL PhotoCtrl::OnScrollBy(CSize scroll_size, BOOL do_scroll)
{
	// don't scroll if there is no valid scroll range (ie. no scroll bar)
	DWORD dwStyle = GetStyle();
	CScrollBar* bar= GetScrollBarCtrl(SB_VERT);
	if ((bar != NULL && !bar->IsWindowEnabled()) || (bar == NULL && !(dwStyle & WS_VSCROLL)))
	{
		// vertical scroll bar not enabled
		scroll_size.cy = 0;
	}
	bar = GetScrollBarCtrl(SB_HORZ);
	if ((bar != NULL && !bar->IsWindowEnabled()) || (bar == NULL && !(dwStyle & WS_HSCROLL)))
	{
		// horizontal scroll bar not enabled
		scroll_size.cx = 0;
	}

	// adjust current x position
	int x;
	int xOrig = x = scroll_bar_.GetScrollPos(SB_HORZ);
	int xMax = scroll_bar_.GetScrollLimit(SB_HORZ);
	x += scroll_size.cx;
	if (x < 0)
		x = 0;
	else if (x > xMax)
		x = xMax;

	// adjust current y position
	int y;
	int yOrig = y = scroll_bar_.GetScrollPos(SB_VERT);
	int yMax = scroll_bar_.GetScrollLimit(SB_VERT);
	y += scroll_size.cy;
	if (y < 0)
		y = 0;
	else if (y > yMax)
		y = yMax;

	// did anything change?
	if (x == xOrig && y == yOrig)
		return FALSE;

	if (do_scroll)
	{
		// do scroll and update scroll positions (in detailed mode exclude header control)
		const RECT* scroll_area= 0;
		if (mode_ == DETAILS)
		{
			if (yOrig != y)
				ScrollWindow(0, yOrig - y, GetFieldRect(false));
			if (xOrig != x)
				ScrollWindow(xOrig - x, 0);
		}
		else
			ScrollWindow(xOrig - x, yOrig - y);

		if (x != xOrig)
			scroll_bar_.SetScrollPos(SB_HORZ, x);

		if (y != yOrig)
		{
			scroll_bar_.SetScrollPos(SB_VERT, y);
			RemoveToolTips();
		}

		if (y != yOrig && host_)	// let host know about content scroll
		{
			PhotoCtrlNotification::Shift dir;

			if (y == 0)
				dir = PhotoCtrlNotification::TOP;
			else if (y == yMax)
				dir = PhotoCtrlNotification::BOTTOM;
			else if (y > yOrig)
				dir = PhotoCtrlNotification::DOWN;
			else
				dir = PhotoCtrlNotification::UP;

			host_->ContentScrolled(dir);
		}
	}
	return TRUE;
}


CPoint PhotoCtrl::GetScrollOffset() const
{
	return CPoint(scroll_bar_.GetScrollPos(SB_HORZ), scroll_bar_.GetScrollPos(SB_VERT));
}


BOOL PhotoCtrl::OnMouseWheel(UINT flags, short delta, CPoint pt)
{
	if (delta)
	{
		int dy= item_size_.cy;
		if (mode_ == DETAILS/* || mode_ == LIST*/)
			dy *= 3;
		else if (mode_ == PREVIEWS)
			dy /= 2;

		OnScroll(MAKEWORD(-1, delta > 0 ? 98 : 99), dy);

		// verify cursor (group label might have been scrolled from under the cursor)
		SetCursor();
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////

// draw selection rectangle at stored position
//
void PhotoCtrl::DrawSelectionRect(CDC& dc, bool erasing)
{
	dc.SetBkMode(OPAQUE);
	dc.SetTextColor(RGB(0,0,0));
	dc.SetBkColor(RGB(255,255,255));
	dc.DrawFocusRect(select_.selection_rect_);
/*
extern bool DrawAlphaRect(CDC& dc, const CRect& rect, int alpha);

CRect rect= select_.selection_rect_;

rect.DeflateRect(1, 1);
if (rect.Width() > 0 && rect.Height() > 0)
{
	dc.IntersectClipRect(rect);
	Draw(dc, rect);
	if (!erasing)
		DrawAlphaRect(dc, rect, 0x40);
	dc.SelectClipRgn(0);
}
*/
}

// send selection changed notification
//
void PhotoCtrl::SelectionChanged()
{
	if (host_)
	{
		// collect current selection
		VectPhotoInfo selected_photos;
		CollectSelected(selected_photos);

		// send it
		host_->SelectionChanged(selected_photos);
	}
}


BOOL PhotoCtrl::OnSetCursor(CWnd* wnd, UINT hit_test, UINT message)
{
	if (hit_test == HTCLIENT)
	{
		PhotoCtrl::SetCursor();
		return true;
	}
	else
		return CWnd::OnSetCursor(wnd, hit_test, message);
}

// select correct cursor (arrow or pointing hand for group labels)
//
void PhotoCtrl::SetCursor()
{
	if (select_.lasso_ && ::GetKeyState(VK_CONTROL) < 0)
		::SetCursor(AfxGetApp()->LoadCursor(IDC_POINTING_INV));
	else if (select_.lasso_)
		::SetCursor(AfxGetApp()->LoadCursor(IDC_POINTING));
	else
	{
		CPoint pos(0, 0);
		GetCursorPos(&pos);
		ScreenToClient(&pos);
		pos += GetScrollOffset();
		Group* group= HitTest(pos);

		if (group && group->HitTest(pos) == Group::GROUP_LABEL)
		{
			HCURSOR cursor= AfxGetApp()->LoadStandardCursor(IDC_HAND);
			if (cursor == 0)
				cursor = AfxGetApp()->LoadCursor(IDC_LINK);
			::SetCursor(cursor);
		}
		else
			::SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));
	}
}


bool PhotoCtrl::SelectionExtend(ItemGroupPair anchor_item, ItemGroupPair current_item, bool desel_current)
{
	if (!anchor_item || !current_item)
		return false;

	bool change= false;

	//TODO: this unselecting cries for improvements
	if (!sticky_selection_ && desel_current)
		change = SelectItems(NONE, false);

	if (anchor_item.Group() != current_item.Group())
	{
		int i1= groups_.FindGroupIndex(anchor_item.Group());
		int i2= groups_.FindGroupIndex(current_item.Group());

		ItemGroupPair first= i1 < i2 ? anchor_item : current_item;
		ItemGroupPair last=  i1 > i2 ? anchor_item : current_item;
		if (i1 > i2)
			std::swap(i1, i2);

		if (first.Group()->SelectItems(first.Item(), true))
		{
			InvalidateGroup(first.Group());
			change = true;
		}

		for (int i= i1 + 1; i < i2 ; ++i)
		{
			if (Group* group= groups_.Get(i))
				if (group->SelectAll(true, false))
				{
					InvalidateGroup(group);
					change = true;
				}
		}

		if (last.Group()->SelectItems(last.Item(), false))
		{
			InvalidateGroup(last.Group());
			change = true;
		}
	}
	else
	{
		ASSERT(anchor_item.Group() == current_item.Group());

		if (current_item.Group()->SelectItems(current_item.Item(), anchor_item.Item()))
		{
			InvalidateGroup(current_item.Group());
			return true;
		}
	}

	return change;
}


// move current item (taking care of current selection)
void PhotoCtrl::GoTo(ItemGroupPair igp, bool select_current, bool extend_sel_key, bool toggle_sel_key,
					  bool notify/*= true*/, bool scroll/*= true*/)
{
	bool sel_changed= false;

	CClientDC dc(this);
	PrepareDC(dc);

	if (!extend_sel_key)
		anchor_item_.Clear();

	ItemGroupPair old= current_item_;

	if (igp == current_item_ && current_item_)	// no move?
	{
		if (select_current)
		{
			bool select= igp.Item()->IsSelected();
			bool deselect_all= false;

			if (!sticky_selection_ && !extend_sel_key && !toggle_sel_key)
			{
				if (GetSelectedCount() == 1 && igp.Item()->IsSelected())
					select = false;
				else
					deselect_all = true;	// clear current selection
			}

			if (extend_sel_key)
			{
				if (!anchor_item_)
					anchor_item_ = igp;
				if (SelectionExtend(anchor_item_, current_item_, !toggle_sel_key))
					sel_changed = true;
			}
			else if (toggle_sel_key || sticky_selection_)
				select = !igp.Item()->IsSelected();
			else
				select = true;

			if (deselect_all)
				sel_changed = SelectItems(NONE, false);	// clear current selection
			if (select != igp.Item()->IsSelected())
				igp.Item()->SetSelection(select), sel_changed = true;
		}

		current_item_.Group()->DrawSelection(*this, dc, current_item_.Item());

		if (scroll)
		{
			// scroll to the current item
			EnsureVisible(current_item_);
		}
	}
	else
	{
		current_item_ = igp;

		if (old)
		{
			if (!sticky_selection_ && select_current && !extend_sel_key && !toggle_sel_key)
			{
				if (GetSelectedCount() == 1 && old.Item()->IsSelected())
					sel_changed = old.Item()->SetSelection(false);
				else
					sel_changed = SelectItems(NONE, false);	// clear current selection
			}

			old.Group()->DrawSelection(*this, dc, old.Item());
		}

		if (current_item_)
		{
			if (select_current)
			{
				if (extend_sel_key)
				{
					if (!anchor_item_)
						anchor_item_ = old;
					if (SelectionExtend(anchor_item_, current_item_, !toggle_sel_key))
						sel_changed = true;
				}
				else if (toggle_sel_key || sticky_selection_)
					current_item_.Item()->ToggleSelection(), sel_changed = true;
				else
					if (current_item_.Item()->SetSelection(true))
						sel_changed = true;
			}

			current_item_.Group()->DrawSelection(*this, dc, current_item_.Item());

			if (scroll)
			{
				// scroll to the current item
				EnsureVisible(current_item_);
			}
		}
	}

	if (sel_changed)
		SelectionChanged();

	if (host_ != 0 && /*old != igp && */ notify)
		host_->CurrentItemChanged(current_item_.Item() != 0 ? current_item_.Item()->photo_ : 0);

	// experimental call: otherwise scrolling after pressing key up/down looks nasty; now it's slow
	UpdateWindow();
}


// scroll window's content if needed to make sure item 'igp' is visible
void PhotoCtrl::EnsureVisible(ItemGroupPair igp)
{
	if (!igp)
		return;

	// Scrolling to the current item

	int item_index= current_item_.Group()->FindItemIndex(current_item_.Item());
	CRect item_rect= current_item_.Group()->GetItemRect(item_index);

	// if first item in a row modify top to match the top of whole group
	// (then scrolling will reveal group's header)
	if (item_index < current_item_.Group()->HorzItems())	// item in first row?
		item_rect.top = current_item_.Group()->GetLocation().top;

	CRect rect= GetFieldRect();

	int delta_y= rect.top - item_rect.top;

	if (delta_y > 0)
	{
		OnScrollBy(CSize(0, -delta_y), true);
		return;
	}

	delta_y = item_rect.bottom - rect.bottom;

	if (delta_y > 0)
	{
		OnScrollBy(CSize(0, delta_y), true);
		return;
	}
}


void PhotoCtrl::GoTo(Direction dir, bool select_current, bool extend_sel_key, bool toggle_sel_key)
{
	if (!current_item_)
		return;

	if (ItemGroupPair igp= FindNextItem(current_item_, dir))
		GoTo(igp, select_current, extend_sel_key, toggle_sel_key);
}


PhotoCtrl::ItemGroupPair PhotoCtrl::FindNextItem(ItemGroupPair igp, Direction dir)
{
	if (!igp)
		return igp;

	// item's index in a group
	int index= igp.Group()->FindItemIndex(igp.Item());
	int new_index= index;

	switch (dir)
	{
	case GO_LEFT:
		new_index--;
		break;
	case GO_RIGHT:
		new_index++;
		break;
	case GO_UP:
		new_index -= igp.Group()->HorzItems();
		break;
	case GO_DOWN:
		new_index += igp.Group()->HorzItems();
		break;
	case GO_TOP:
		if (Group* group= groups_.FindFirstGroup())
			return ItemGroupPair(group, group->GetItem(0));
		return igp;
	case GO_BOTTOM:
		if (Group* group= groups_.FindLastGroup())
			return ItemGroupPair(group, group->GetItem(group->GetItemCount() - 1));
		return igp;
	case GO_PG_UP:
		return FindPage(igp, false);
	case GO_PG_DOWN:
		return FindPage(igp, true);
	default:
		ASSERT(false);
		break;
	}

	if (new_index < 0)
	{
		Group* prev_group= groups_.FindPrevGroup(igp.Group());

		if (prev_group == 0)
			return igp;

		int last= prev_group->GetItemCount() - 1;
		int last_row= last - last % prev_group->HorzItems();
		new_index += last_row + prev_group->HorzItems();
		if (new_index < 0)
			new_index = 0;
		else if (new_index > last)
			new_index = last;

		return ItemGroupPair(prev_group, prev_group->GetItem(new_index));
	}
	else if (new_index >= igp.Group()->GetItemCount())
	{
		Group* next_group= groups_.FindNextGroup(igp.Group());

		if (next_group == 0)
			return igp;

		if (dir == GO_RIGHT)
			new_index = 0;
		else
			new_index %=  next_group->HorzItems();

		return ItemGroupPair(next_group, next_group->GetItem(std::min(new_index, next_group->GetItemCount() - 1)));
	}
	else
	{
		// next position found (within the same group)
		return ItemGroupPair(igp.Group(), igp.Group()->GetItem(new_index));
	}
}


// find next/prev page
PhotoCtrl::ItemGroupPair PhotoCtrl::FindPage(ItemGroupPair igp, bool next_page)
{
	if (m_hWnd == 0)
		return igp;

	// Page up (down) is a two step process. First I'm locating top (bottom) item in a current view rect.
	// If current item is different, then go there first. If current item is already at the top (bottom)
	// of the view rect then view rect gets offset by a page height and top (bottom) item is being located again.

	// current rect
	CRect rect= GetFieldRect();

	// find first/last item in the current rect

	Group* group= next_page ? groups_.FindLastGroupInRect(rect) : groups_.FindFirstGroupInRect(rect);

	if (group == 0)
		return igp;

	int item_index= igp.Group()->FindItemIndex(igp.Item());

	Item* item= next_page ? group->FindLastItemInRect(rect, item_index) : group->FindFirstItemInRect(rect, item_index);

	if (item == 0)
	{
		ASSERT(false);
		return igp;
	}

	if (igp.Group() != group || igp.Item() != item)
		return ItemGroupPair(group, item);

	// igp already points to the first/last visible element

	// scrolling page up/down occurs here =============================

	CRect item_rect= igp.GetItemRect(true);

	if (next_page)
		rect.OffsetRect(0, item_rect.bottom - rect.top);
	else
		rect.OffsetRect(0, item_rect.top - rect.bottom);

	// repeat same process

	group = next_page ? groups_.FindLastGroupInRect(rect) : groups_.FindFirstGroupInRect(rect);

	if (group == 0)
		return igp;

	item = next_page ? group->FindLastItemInRect(rect, item_index) : group->FindFirstItemInRect(rect, item_index);

	if (item == 0)
	{
		ASSERT(false);
		return igp;
	}

	return ItemGroupPair(group, item);
}


// find last group falling into the 'rect'
PhotoCtrl::Group* PhotoCtrl::GroupVector::FindLastGroupInRect(const CRect& rect)
{
	Group* group= FindLastGroup();

	while (group && group->GetLocation().top >= rect.bottom)
		group = FindPrevGroup(group);

	if (group == 0)
		return 0;

	CRect item_rect= group->GetItemRect(0);

	// check if item is visible in whole or only partially (grace margin is set to 1/8-th
	// of the items height meaning it mast be at least in 7/8 visible)
	if (item_rect.bottom - item_rect.Height() / 8 > rect.bottom)
	{
		Group* prev_group= FindPrevGroup(group);
		if (prev_group != 0)
			group = prev_group;
	}

	return group;
}

// find first group falling into the 'rect'
PhotoCtrl::Group* PhotoCtrl::GroupVector::FindFirstGroupInRect(const CRect& rect)
{
	Group* group= FindFirstGroup();

	while (group && group->GetLocation().bottom <= rect.top)
		group = FindNextGroup(group);

	if (group == 0)
		return 0;

	CRect item_rect= group->GetItemRect(group->GetItemCount() - 1);

	// check if item is visible in whole or only partially (grace margin is set to 1/8-th
	// of the items height meaning it mast be at least in 7/8 visible)
	if (item_rect.top + item_rect.Height() / 8 < rect.top)
	{
		Group* next_group= FindNextGroup(group);
		if (next_group != 0)
			group = next_group;
	}

	return group;
}


// perform drag & drop operation
void PhotoCtrl::DoDragDrop()
{
	if (host_)
	{
		try
		{
			VectPhotoInfo selected;
			// get selected
			CollectSelected(selected);

			host_->DoDragDrop(selected);
		}
		CATCH_ALL
	}
}


// start scrolling if user is lasso-selecting images outside of the window
//
void PhotoCtrl::LassoSelectScroll()
{
	CRect rect;
	GetClientRect(rect);

	CPoint pos;
	::GetCursorPos(&pos);
	ScreenToClient(&pos);

	const int MAX_SCROLL_AMOUNT= 200;

	CSize scroll_size(0, 0);

	int delta_y= pos.y - rect.Height();

	// check if we are scrolling outside the control
	if (delta_y > 0)
		scroll_size.cy = std::min(delta_y, MAX_SCROLL_AMOUNT);
	else if (pos.y < 0)
		scroll_size.cy = -std::min(static_cast<int>(-pos.y), MAX_SCROLL_AMOUNT);

	int delta_x= pos.x - rect.Width();
	if (delta_x > 0)
		scroll_size.cx = std::min(delta_x, MAX_SCROLL_AMOUNT);
	else if (pos.x < 0)
		scroll_size.cx = -std::min(static_cast<int>(-pos.x), MAX_SCROLL_AMOUNT);

	select_.scroll_amount_size_ = scroll_size;

	if (scroll_size.cy != 0 || scroll_size.cx != 0)
		SetTimer(SCROLL_TIMER_ID, 10, 0);
	else	// no auto scrolling; mouse cursor inside client rect
		KillTimer(SCROLL_TIMER_ID);
}


void PhotoCtrl::OnTimer(UINT_PTR id_event)
{
	if (id_event != SCROLL_TIMER_ID)
	{
		CWnd::OnTimer(id_event);
		return;
	}

	if (!select_.selecting_ || (select_.scroll_amount_size_.cy == 0 && select_.scroll_amount_size_.cx == 0))
		return;

	CClientDC dc(this);
	PrepareDC(dc);

	if (select_.lasso_)
		DrawSelectionRect(dc, true);				// rect off
	select_.selection_rect_.SetRectEmpty();

	OnScrollBy(select_.scroll_amount_size_, true);	// scroll window contents
	UpdateWindow();							// and update uncovered areas

	CPoint pos;
	::GetCursorPos(&pos);
	ScreenToClient(&pos);
	pos.Offset(GetScrollOffset());

	select_.selection_rect_.SetRect(select_.start_, pos);
	select_.selection_rect_.NormalizeRect();

	PrepareDC(dc);
	SelectTemporarily(dc, select_.selection_rect_, ::GetKeyState(VK_CONTROL) < 0);

	if (select_.lasso_)
		DrawSelectionRect(dc, false);				// rect on
}


///////////////////////////////////////////////////////////////////////////////

struct GroupIdLess
{
	bool operator () (PhotoCtrl::Group* group, int id) const
	{
		if (group->GetId() < id)
			return true;
		return false;
	}
};


PhotoCtrl::Group* PhotoCtrl::GroupVector::Insert(std::auto_ptr<Group> group)
{
	iterator it= lower_bound(begin(), end(), group->GetId(), GroupIdLess());
	ASSERT(it == end() || (*it)->GetId() != group->GetId());
	Group* empty= nullptr;
	it = insert(it, empty);	// resize (may throw exception)
	*it = group.release();	// now release auto_ptr
	return *it;
}


struct PointYInGroup
{
	bool operator () (PhotoCtrl::Group* group, long y_pos) const
	{
		if (group->GetLocation().bottom < y_pos)
			return true;
		return false;
	}
};


PhotoCtrl::Group* PhotoCtrl::GroupVector::HitTest(CPoint pos) const
{
	const_iterator it= lower_bound(begin(), end(), pos.y, PointYInGroup());
	if (it != end() && (*it)->GetLocation().PtInRect(pos))
		return *it;

	return 0;
}


struct MatchingGroupId : std::binary_function<const PhotoCtrl::Group*, int, bool>
{
	bool operator () (const PhotoCtrl::Group* group, int id) const
	{
		return group->GetId() == id;
	}
};


PhotoCtrl::Group* PhotoCtrl::GroupVector::Find(int group_id)
{
	//TODO: groups are sorted by id; binary search here

	iterator it= lower_bound(begin(), end(), group_id, GroupIdLess());

//	iterator it= find_if(begin(), end(), bind2nd(MatchingGroupId(), group_id));

	if (it == end() || (*it)->GetId() != group_id)
		return 0;

	return *it;
}

// next not empty group
PhotoCtrl::Group* PhotoCtrl::GroupVector::FindNextGroup(Group* group)
{
	for (int index= FindGroupIndex(group) + 1; index < size(); ++index)
		if ((*this)[index]->GetItemCount() > 0)
			return (*this)[index];

	return 0;
}

// previous not empty group
PhotoCtrl::Group* PhotoCtrl::GroupVector::FindPrevGroup(Group* group)
{
	for (int index= FindGroupIndex(group) - 1; index >= 0; --index)
		if ((*this)[index]->GetItemCount() > 0)
			return (*this)[index];

	return 0;
}

// first not empty group
PhotoCtrl::Group* PhotoCtrl::GroupVector::FindFirstGroup()
{
	for (int index= 0; index < size(); ++index)
		if ((*this)[index]->GetItemCount() > 0)
			return (*this)[index];

	return 0;
}

// last not empty group
PhotoCtrl::Group* PhotoCtrl::GroupVector::FindLastGroup()
{
	for (int index= static_cast<int>(size()) - 1; index >= 0; --index)
		if ((*this)[index]->GetItemCount() > 0)
			return (*this)[index];

	return 0;
}


int PhotoCtrl::GroupVector::FindGroupIndex(Group* group)
{
	iterator it= find(begin(), end(), group);
	if (it == end())
	{
		ASSERT(false);
		return 0;
	}
	return static_cast<int>(distance(begin(), it));
}


//PhotoCtrl::ItemGroupPair PhotoCtrl::GroupVector::FindItem(PhotoInfoPtr photo)
//{
//	for (iterator it= begin(); it != end(); ++it)
//		if (Item* item= (*it)->FindItem(photo))
//			return ItemGroupPair(*it, item);
//
//	return ItemGroupPair();
//}


PhotoCtrl::Group* PhotoCtrl::GroupVector::RemoveItem(PhotoCtrl* ctrl, Item* item, Group* skip_group/*= nullptr*/)
{
	for (iterator it= begin(); it != end(); ++it)
		if (*it != skip_group && (*it)->RemoveItem(item))
		{
			if (ctrl)
				ctrl->InvalidateGroup(*it);

			return *it;
		}

	return nullptr;
}


void PhotoCtrl::ItemVector::AssignGroup(Group* group)
{
	for (auto& item : *this)
		item->SetGroup(group);
}


bool PhotoCtrl::Group::RemoveSelected()
{
	auto new_end= remove_if(items_.begin(), items_.end(), std::mem_fun(&Item::IsSelected));

	for (auto it= new_end; it != items_.end(); ++it)
		(*it)->SetGroup(nullptr);

	items_.erase(new_end, items_.end());

	return true;
}

void PhotoCtrl::GroupVector::RemoveSelected()
{
	for_each(begin(), end(), std::mem_fun(&Group::RemoveSelected));
}

/*
const PhotoCtrl::Group* PhotoCtrl::GroupVector::HitTest(CPoint pos) const
{
	return 0;
}
*/

void PhotoCtrl::Group::CollectSelected(VectPhotoInfo& selected)
{
//	for (ItemVector::iterator it= items_.begin(); it != items_.end(); ++it)
//		it->CollectSelected(selected);
//TODO	copy_if
	for (ItemVector::iterator it= items_.begin(); it != items_.end(); ++it)
		if ((*it)->IsSelected())
			selected.push_back((*it)->photo_);
}

void PhotoCtrl::GroupVector::CollectSelected(VectPhotoInfo& selected)
{
	selected.clear();
	selected.reserve(400);

//	for_each(begin(), end(), bind2nd(mem_fun_ref_v<Group*, VectPhotoInfo>(&Group::CollectSelected), selected));

	for (iterator it= begin(); it != end(); ++it)
		(*it)->CollectSelected(selected);
}


// collect all selected photos
void PhotoCtrl::CollectSelected(VectPhotoInfo& selected)
{
	groups_.CollectSelected(selected);
	select_.selected_count_ = static_cast<int>(selected.size());
}


CSize PhotoCtrl::TryLocation(const CRect& rect, CSize item_size, int& width, int& height, int& horz_items, int& bottom, int height_limit)
{
	//TODO: margins_rect_ is really item's margin, not field view margin, but using
	// it is better than hardcoded MARGIN_LEFT/RIGHT (for detailed view mode at least);
	// introduce field view margin?
	width = std::max<int>(0, rect.Width() - margins_rect_.left - margins_rect_.right);
	height = std::max(0, rect.Height() - top_margin_);

	horz_items = mode_ == DETAILS ? 1 : std::max<int>(1, width / item_size.cx);

	bottom = SetLocation(rect, horz_items, item_size, height_limit);

	CSize range_size(item_size.cx, std::max(0, bottom - top_margin_));

	return range_size;
}


bool PhotoCtrl::SetLocation(CSize item_size, CSize item_big_size/*= CSize(0, 0)*/)
{
	if (item_size.cx == 0 && item_size.cy == 0)
		item_size = item_size_;

	if (item_size.cx <= 0 || item_size.cy <= 0)
	{
		ASSERT(false);
		return false;
	}

	// client rect with & without scrollbars
	CRect small_rect, rect;
	GetClientRects(small_rect, rect);

	int width= 0, height= 0, horz_items= 0, bottom= 0;
	CSize range_size(0, 0);
	bool useBigItem= false;

	if (item_big_size.cx > 0 && item_big_size.cy > 0 && item_big_size != item_size)
	{
		// first try bigger item, to see if it fits

		range_size = TryLocation(rect, item_big_size, width, height, horz_items, bottom, rect.Height());

		if (width >= range_size.cx && height >= range_size.cy)
		{
			useBigItem = true;	// it fits without scrollbars, use it
			item_size = item_big_size;
		}
	}

	if (!useBigItem)
	{
		// calc size for normal item

		range_size = TryLocation(rect, item_size, width, height, horz_items, bottom, rect.Height());
	}

	if (width >= range_size.cx && height >= range_size.cy)
	{
		// enough space to hide both scrollbars

		SCROLLINFO si;
		si.cbSize = sizeof si;
		si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
		si.nMin = 0;
		si.nMax = 1;
		si.nPage = 2;
		si.nPos = 0;
		SetScrollBar(SB_HORZ, si);
		SetScrollBar(SB_VERT, si);
		// scrollbars are hidden now
	}
	else
	{
		CSize scroll_bars_size= rect.Size() - small_rect.Size();
		// at least one or both scrollbars have to be displayed

		// TODO: this is not yet bullet proof
		if (width < range_size.cx)
			rect.bottom -= scroll_bars_size.cy, height -= scroll_bars_size.cy;
		if (height < range_size.cy)
			rect.right -= scroll_bars_size.cx, width -= scroll_bars_size.cx;

		range_size = TryLocation(rect, item_size, width, height, horz_items, bottom, 0);
/*
		if (mode_ != DETAILS)
			horz_items = max<int>(1, width / item_size.cx);

		bottom = SetLocation(rect, horz_items, item_size);

		range_size = CSize(item_size.cx, max(0, bottom - top_margin_));
*/
		int x = scroll_bar_.GetScrollPos(SB_HORZ);
		SCROLLINFO si;
		si.cbSize = sizeof si;
		si.fMask = SIF_RANGE | SIF_PAGE;
		si.nMin = 0;
		si.nMax = range_size.cx - 1;
		si.nPage = width;
		if (x > si.nMax - si.nPage)
		{
			si.nPos = std::max<int>(0, si.nMax - si.nPage);
			si.fMask |= SIF_POS;
		}
		SetScrollBar(SB_HORZ, si);

		int y = scroll_bar_.GetScrollPos(SB_VERT);
		si.cbSize = sizeof si;
		si.fMask = SIF_RANGE | SIF_PAGE;
		si.nMin = 0;
		si.nMax = range_size.cy - 1;
		si.nPage = height;
		if (y > si.nMax - si.nPage)
		{
			si.nPos = std::max<int>(0, si.nMax - si.nPage);
			si.fMask |= SIF_POS;
		}
		SetScrollBar(SB_VERT, si);
	}

	if (horz_items != horz_items_)
	{
		horz_items_ = horz_items;
		Invalidate();
	}

	bottom_ = std::max(0, bottom - 1);	// last used line

	return useBigItem;
}


int PhotoCtrl::SetLocation(const CRect& rect, int horz_items, CSize item_size, int height_limit)
{
	int width= rect.Width();
	CPoint start(MARGIN_LEFT, MARGIN_TOP);

	int label_height= GetLabelHeight();
	bool same_height= false;

	if (mode_ == DETAILS)
	{
		width = std::max(rect.Width(), ColumnsTotalWidth());
//		start.x = 0;
		start.y = top_margin_;
		same_height = true;		// in detailed mode all rows are to have the same height
	}
	else
	{
		width -= MARGIN_LEFT + MARGIN_RIGHT;
		if (width < item_size.cx)
			width = item_size.cx;		// group rect shouldn't be narrower than an item or problems with horz scrolling arise
	}

	CSize size= item_size;
	size.cy -= label_height;
//	size.cy -= M_size_.cy;

/*	if ((mode_ == THUMBNAILS || mode_ == PREVIEWS) && !show_label_ || mode_ == MOSAIC)
		label_height = 0;
	else*/ if (mode_ == TILES)
	{
		label_height = item_size.cy;		// there are multiple rows of text
		size.cy = 0;
	}

	return groups_.SetLocation(*this, start, width, horz_items, size, label_height, same_height, height_limit);
}


// set layout of groups
//
int PhotoCtrl::GroupVector::SetLocation(PhotoCtrl& ctrl, CPoint start, int width, int horz_items, CSize item_size, int label_height, bool same_height, int height_limit)
{
	bool separator= false;

	for (iterator it= begin(); it != end(); ++it)
	{
		Group* group= *it;
		bool visible= group->GetItemCount() > 0;

		// existing location
		CRect old_rect= group->GetLocation();

		start.y = group->SetLocation(start, width, horz_items, item_size, label_height, ctrl.margins_rect_, separator && visible, same_height);

		// detect location change
		if (old_rect != group->GetLocation())
			ctrl.InvalidateGroup(group);

		if (visible)
			separator = true;

		// quit early if height limit already reached
		if (height_limit > 0 && start.y > height_limit)
			break;
	}

	return start.y;
}

int PhotoCtrl::Group::GetMaxHeaderHeight()
{
	return Pixels(HEADER_HEIGHT + SEPARATOR_HEIGHT);
}

// set single group layout
//
int PhotoCtrl::Group::SetLocation(CPoint left_top, int field_width, int horz_items, CSize item_size, int label_height, const CRect& margins_rect, bool separator, bool same_height)
{
	location_rect_ = CRect(left_top, CSize(field_width, 0));

	if (items_.empty())	// if there's no items leave group's location empty (height = 0)
		return location_rect_.bottom;

	if (icon_ != NO_ICON || !name_.empty())	// has an icon or label?
	{
		flags_ &= ~NO_HEADER;
		location_rect_.bottom += Pixels(HEADER_HEIGHT);
	}
	else
	{
		flags_ |= NO_HEADER;
	}

	if (separator)
	{
		location_rect_.bottom += Pixels(SEPARATOR_HEIGHT);
		flags_ |= SEPARATOR;
	}
	else
		flags_ &= ~SEPARATOR;

	item_size_ = item_size;
	item_size_.cy += label_height;
	horz_items_ = horz_items;

//TODO: modify for hidden/collapsed groups

	int item_count= static_cast<int>(items_.size());

	ASSERT(horz_items >= 1);

	int item_rows= (item_count + horz_items - 1) / horz_items;
	//TODO: that's a compromise: for 3:4 ratio photos 'cy * 4 / 5' is optimal, for 1:1.5 it's 'cy * 3 / 5'
	int small_row= item_size.cy * 36 / 50;
	// note that above statement does not apply to the small row value directly due to the extra image elements:
	// label, outline, shadow

	// height for a tall row (with portrait photos); experimentally adjusted to make landscape and portrait images the same size
	int tall_row= item_size.cy;// * 98 / 100;

	row_heights_.resize(item_rows);
	int first_item_in_row= 0;
	int group_height= 0;
	for (int row= 0; row < item_rows; ++row)
	{
		long row_height= 0;

		if (same_height)	// make rows the same height
			row_height = item_size.cy;
		else
		{
			// if all images in a row are oriented horizontally this row can be smaller
			row_height = ItemsOrientedHorizontally(first_item_in_row, horz_items) ? small_row : tall_row;
		}

		row_height += label_height;

		if (row == 0)
			row_heights_[0] = row_height;
		else
			row_heights_[row] = row_heights_[row - 1] + row_height;

		group_height += row_height;
		first_item_in_row += horz_items;
	}

	location_rect_.bottom += group_height;

	margins_rect_ = margins_rect;

	return location_rect_.bottom;
}


bool PhotoCtrl::Group::ItemsOrientedHorizontally(int first_item, int count)
{
	if (count <= 0)
		return false;

	if (first_item < 0)
		return false;

	int item_count= static_cast<int>(items_.size());

	if (first_item >= item_count)
		first_item = item_count - 1;

	if (item_count == 0)
		return false;

	int last_item= first_item + count;
	if (last_item > item_count)
		last_item = item_count;

	for (int i= first_item; i < last_item; ++i)
		if (!items_[i]->HorizontalOrientation())
			return false;

	return true;
}


// redraw item's selection
//
void PhotoCtrl::Group::DrawSelection(PhotoCtrl& ctrl, CDC &dc, Item* item)
{
	if (item == 0)
	{
		ASSERT(false);
		return;
	}
	ASSERT(!items_.empty());
	ItemVector::iterator it= find(items_.begin(), items_.end(), item);
	if (it == items_.end())
	{
		ASSERT(false);
		return;
	}
	CRect rect= GetItemRect(static_cast<int>(distance(items_.begin(), it)));
	MemoryDC item_dc(dc, rect, ctrl.GetBkColor());
	item->DrawItem(item_dc, rect, ctrl, ctrl.current_item_.Item() == item, ctrl.ShowPhotoMarker(item), ctrl.ShowNoExifIndicator(item));
	item_dc.BitBlt();
}


struct ItemsByPhotos
{
	bool operator () (PhotoCtrl::Item* item1, PhotoCtrl::Item* item2) const
	{
		return item1->photo_ < item2->photo_;
	}

	bool operator () (PhotoCtrl::Item* item, const PhotoInfoPtr& photo) const
	{
		return item->photo_ < photo;
	}
};


void PhotoCtrl::Group::AddItem(Item* item)
{
//	ASSERT(!binary_search(sorted_.begin(), sorted_.end(), item, ItemsByPhotos()));

	items_.push_back(item);
	item->SetGroup(this);

//	sorted_.insert(lower_bound(sorted_.begin(), sorted_.end(), item->photo_), item->photo_);
//	sorted_.insert(lower_bound(sorted_.begin(), sorted_.end(), item->photo_, ItemsByPhotos()), item);
}


bool PhotoCtrl::Group::RemoveItem(Item* item)
{
	// find item
	ItemVector::iterator it= find(items_.begin(), items_.end(), item);
	if (it == items_.end())
		return false;

	items_.erase(it);
	item->SetGroup(nullptr);
	return true;

//	items_.erase(remove(items_.begin(), items_.end(), item), items_.end());
//	sorted_.erase(lower_bound(sorted_.begin(), sorted_.end(), item, ItemsByPhotos()));
}


void PhotoCtrl::Group::SyncSorted()	// keep sorted_ in sync with items_
{
//	sorted_.clear();
//	sorted_.reserve(items_.capacity());

//	for (ItemVector::iterator it= items_.begin(); it != items_.end(); ++it)
//		sorted_.insert(lower_bound(sorted_.begin(), sorted_.end(), *it, ItemsByPhotos()), *it);
}


// keep sorted_ in sync with lists
void PhotoCtrl::ItemList::SyncSorted()
{
	const auto count= size();

	sorted_.clear();
	sorted_.reserve(count);

	//lookup_.clear();
	//lookup_.reserve(count);

//	for (iterator it= begin(); it != end(); ++it)
	for (auto& item : *this)
	{
		Item* p= &item;
		//TODO: revise
//		sorted_.insert(upper_bound(sorted_.begin(), sorted_.end(), &*it, ItemsByPhotos()), &*it);
		sorted_.insert(lower_bound(sorted_.begin(), sorted_.end(), p, ItemsByPhotos()), p);
		//lookup_.insert(std::make_pair(item.photo_.get(), p));
	}
}


// add new item to the list; keep sorted_ sorted
PhotoCtrl::Item* PhotoCtrl::ItemList::Insert(PhotoInfoPtr photo)
{
	ASSERT(photo);

	const auto reserve= (sorted_.size() & ~0x7) + 8;
	sorted_.reserve(reserve);
	//lookup_.reserve(reserve);

	push_back(Item(photo));
	//TODO: revise
//	sorted_.insert(upper_bound(sorted_.begin(), sorted_.end(), &back(), ItemsByPhotos()), &back());
	sorted_.insert(lower_bound(sorted_.begin(), sorted_.end(), &back(), ItemsByPhotos()), &back());
	//lookup_.insert(std::make_pair(back().photo_.get(), &back()));

	return &back();
}


// add new items to the list; keep sorted_ sorted
void PhotoCtrl::ItemList::Insert(PhotoInfoPtr* begin, PhotoInfoPtr* end)
{
	ASSERT(begin && end && end > begin);

	const auto count= sorted_.size() + (end - begin);
	sorted_.reserve(count);
	//lookup_.reserve(count);

	for (PhotoInfoPtr* photo= begin; photo != end; ++photo)
	{
		push_back(Item(*photo));
	//TODO: revise
		sorted_.insert(lower_bound(sorted_.begin(), sorted_.end(), &back(), ItemsByPhotos()), &back());
//		sorted_.insert(upper_bound(sorted_.begin(), sorted_.end(), &back(), ItemsByPhotos()), &back());
		//lookup_.insert(std::make_pair(back().photo_.get(), &back()));
	}
}


void PhotoCtrl::ItemList::ReserveSpace(const size_t count)
{
	const auto reserve= sorted_.size() + count;
	sorted_.reserve(reserve);
	//lookup_.reserve(reserve);
}

void PhotoCtrl::ItemList::Splice(list<Item>& items_list)
{
	const auto count= items_list.size();
	const auto reserve= sorted_.size() + count;
	sorted_.reserve(reserve);
	//lookup_.reserve(reserve);

//	for (list<Item>::iterator it= items_list.begin(); it != items_list.end(); ++it)
	for (auto& item : items_list)
	{
	//TODO: revise
//		sorted_.insert(upper_bound(sorted_.begin(), sorted_.end(), &*it, ItemsByPhotos()), &*it);
		sorted_.insert(lower_bound(sorted_.begin(), sorted_.end(), &item, ItemsByPhotos()), &item);
		//lookup_.insert(std::make_pair(item.photo_.get(), &item));
	}

	splice(end(), items_list);
}


// find item (binary search)
PhotoCtrl::Item* PhotoCtrl::ItemList::FastFind(PhotoInfoPtr photo)
{
	PhotoCtrl::ItemVector::iterator it= lower_bound(sorted_.begin(), sorted_.end(), photo, ItemsByPhotos());

	if (it == sorted_.end() || (*it)->photo_ != photo)
		return 0;

	return *it;
}


const PhotoCtrl::Item* PhotoCtrl::ItemList::FastFind(PhotoInfoPtr photo) const
{
	return const_cast<PhotoCtrl::ItemList*>(this)->FastFind(photo);
}


struct SameItem
{
	SameItem(PhotoCtrl::Item* item) : item_(item)
	{}

	bool operator () (const PhotoCtrl::Item& item) const
	{
		return item_ == &item;
	}

	PhotoCtrl::Item* item_;
};


bool PhotoCtrl::ItemList::Remove(PhotoInfoPtr photo)
{
	PhotoCtrl::ItemVector::iterator it= lower_bound(sorted_.begin(), sorted_.end(), photo, ItemsByPhotos());

	if (it == sorted_.end() || (*it)->photo_ != photo)
		return false;

	// remove instance of 'Item' from list
	iterator list= find_if(begin(), end(), SameItem(*it));
	if (list != end())
		erase(list);
	else
	{ ASSERT(false); }

	// and remove pointer to it
	sorted_.erase(it);

	return true;
}

// remove all selected items
void PhotoCtrl::ItemList::RemoveSelected()
{
	// remove from the list and sorted vector
	for (list<Item>::iterator it= begin(); it != end(); ++it)
	{
		if (it->IsSelected())
		{
			ItemVector::iterator el= lower_bound(sorted_.begin(), sorted_.end(), it->photo_, ItemsByPhotos());
			if (el != sorted_.end() && (*el)->photo_ == it->photo_)
				sorted_.erase(el);
			else
			{ ASSERT(false); }
		}
	}

	remove_if(std::mem_fun_ref(&Item::IsSelected));
}


PhotoCtrl::Group::Group(const String& name, int id, Icons icon, Item* item)
	: name_(name), id_(id), items_(item), flags_(NORMAL), location_rect_(0,0,0,0), item_size_(0,0), horz_items_(0), icon_(icon)
{
	items_.AssignGroup(this);
	label_rect_.SetRectEmpty();
	margins_rect_.SetRectEmpty();
	SyncSorted();
}


PhotoCtrl::Group::Group(const String& name, int id, Icons icon, std::vector<Item*>& items)
	: name_(name), id_(id), items_(items), flags_(NORMAL), location_rect_(0,0,0,0), item_size_(0,0), horz_items_(0), icon_(icon)
{
	items_.AssignGroup(this);
	label_rect_.SetRectEmpty();
	margins_rect_.SetRectEmpty();
	SyncSorted();
}


PhotoCtrl::Group::~Group()
{
	for (auto& item : items_)
		item->SetGroup(nullptr);
}


int PhotoCtrl::Group::FindItemIndex(Item* item)
{
	ASSERT(item);
	ASSERT(!items_.empty());
	ItemVector::iterator it= find(items_.begin(), items_.end(), item);
	if (it == items_.end())
	{
//		ASSERT(false);
		return 0;
	}
	return static_cast<int>(distance(items_.begin(), it));
}


void PhotoCtrl::GroupVector::SelectTemporarily(PhotoCtrl& ctrl, CDC& dc, const CRect& selection_rect, bool toggle)
{
	for (iterator it= begin(); it != end(); ++it)
		(*it)->SelectTemporarily(ctrl, dc, selection_rect, toggle);
}


void PhotoCtrl::Group::SelectTemporarily(PhotoCtrl& ctrl, CDC& dc, const CRect& selection_rect, bool toggle)
{
//	CRect result_rect;
//	if (!result_rect.IntersectRect(location_rect_, selection_rect))
//		return;		// selection and whole group do not intersect

	bool select= true;
	const size_t count= items_.size();

	for (size_t i= 0; i < count; ++i)
	{
		Item& item= *items_[i];
		bool state_changed= false;	// redrawing necessary?

		CRect item_rect= GetItemRect(static_cast<int>(i), false);
		CRect item_bbox_rect= GetItemRect(static_cast<int>(i), true);

		CRect result_rect;
		// check if item's location intersects with given rect
		if (result_rect.IntersectRect(item_rect, selection_rect) &&
			result_rect.IntersectRect(ctrl.GetImageRect(item_bbox_rect, item.photo_, true), selection_rect))
		{
			if (toggle)
			{
				// change temp selection state to the opposite of regular selection state
				select = !item.IsSelected();
			}

			if (item.IsTempSelected() != select)
			{
				// change temp selection state to requested state 'select'
				item.SetTempSelection(select);
				state_changed = true;
			}
		}
		else	// non intersecting images here
		{
			// remove temporary selection for them
			if (item.IsTempSelected() != item.IsSelected())
			{
				item.SynchTempSelection();
				state_changed = true;
			}
		}

		if (state_changed)
			DrawSelection(ctrl, dc, &item);
//			item->DrawItem(&dc, item->temp_selected_);
	}
}


bool PhotoCtrl::Group::SelectAll(bool select, bool toggle_if_all_same)
{
	bool state_changed= false;

	const size_t count= items_.size();

	if (count == 0)
		return false;

	if (toggle_if_all_same)
	{
		// check if items in group are all selected or all deselected
		bool state= items_.front()->IsSelected();
		bool same= true;

		for (size_t i= 1; i < count; ++i)
			if (items_[i]->IsSelected() != state)
			{
				same = false;
				break;
			}

		// if all defect same, toggle their state
		if (same)
			select = !state;
	}

	for (size_t i= 0; i < count; ++i)
		if (items_[i]->SetSelection(select))
			state_changed = true;

	// report back whether something has really changed
	return state_changed;
}


bool PhotoCtrl::Group::SelectItems(Item* first_item, bool select_to_end)
{
	int index= FindItemIndex(first_item);
	bool changed= false;

	if (select_to_end)
	{
		size_t count= items_.size();
		for (size_t i= index; i < count; ++i)
			if (items_[i]->SetSelection(true))
				changed = true;
	}
	else
	{
		for (int i= index; i >= 0; --i)
			if (items_[i]->SetSelection(true))
				changed = true;
	}

	return changed;
}

// select from item1 to item2 (swap 1 & 2 if necessary)
bool PhotoCtrl::Group::SelectItems(Item* item1, Item* item2)
{
	int i1= FindItemIndex(item1);
	int i2= FindItemIndex(item2);

	if (i1 > i2)
		std::swap(i1, i2);

	ASSERT(i1 <= i2);

	bool changed= false;

	for (int i= i1; i <= i2; ++i)
		if (items_[i]->SetSelection(true))
			changed = true;

	return changed;
}


void PhotoCtrl::Group::ToggleSelection()
{
	const size_t count= items_.size();
	for (size_t i= 0; i < count; ++i)
		items_[i]->ToggleSelection();
}


void PhotoCtrl::GroupVector::ToggleSelection()
{
	for (iterator it= begin(); it != end(); ++it)
		(*it)->ToggleSelection();
}


bool PhotoCtrl::GroupVector::SelectAll(bool select, bool toggle_if_all_same)
{
	bool change= false;

	for (iterator it= begin(); it != end(); ++it)
		if ((*it)->SelectAll(select, toggle_if_all_same))
			change = true;

	return change;
}


void PhotoCtrl::GroupVector::Draw(PhotoCtrl& ctrl, CDC& paint_dc, MemoryDC& item_dc, MemoryDC& header_dc)
{
//	for (iterator it= begin(); it != end(); ++it)	// <- this version crashes sometimes
	for (size_t i= 0; i < size(); ++i)
	{
		// check is groups is visible
		//if () ;

		Group* grp= (*this)[i]; //*it;

		grp->Draw(ctrl, paint_dc, item_dc, header_dc);
	}
}


void PhotoCtrl::GroupVector::RemoveAll()
{
	for (iterator it= begin(); it != end(); ++it)
		delete *it;
	clear();
}


void PhotoCtrl::GroupVector::Remove(Group* group)
{
	auto it= find(begin(), end(), group);
	if (it == end())
	{
		ASSERT(false);
		return;
	}

	delete *it;

	erase(it);
}


BOOL PhotoCtrl::IsRectVisible(CDC& dc, CRect rect) const
{
	// PhotoCtrl uses PrepareDC() shifting viewport's origin; location_rect_ itself
	// is properly positioned in Y coordinates (vertically), but is left intact
	// during horizontal shifts (except for DETAILED view mode at least);
	// thus regular RectVisible() couldn't determine rect visibility if it wasn't shifted:

//	rect.OffsetRect(GetScrollOffset().x, 0); // fixed
//TODO: lousy, but does the work; improve
rect.InflateRect(100, 100);
	return dc.RectVisible(rect);
}


void PhotoCtrl::Group::Draw(PhotoCtrl& ctrl, CDC& paint_dc, MemoryDC& item_dc, MemoryDC& header_dc)
{
	if (location_rect_.IsRectEmpty())
		return;

	if (!ctrl.IsRectVisible(paint_dc, location_rect_))
	{
//		TRACE(_T("invisible\n"));
		return;
	}

	// draw group's header
	CRect rect= location_rect_;
	rect.bottom = GetItemTop();

	COLORREF rgb_back= ctrl.GetBkColor();

	if (!rect.IsRectEmpty() && ctrl.IsRectVisible(paint_dc, rect))
	{
		header_dc.OffsetYTo(rect.top);
		header_dc.FillSolidRect(rect, ctrl.rgb_bk_color_);

		int height= 0;

		if (flags_ & SEPARATOR)
		{
			// draw separator
			//header_dc.FillSolidRect(rect.left, rect.top, std::max(rect.Width(), 0), 1, ctrl.rgb_separator_);
			int sep = Pixels(SEPARATOR_HEIGHT);
			rect.top += sep;
			height += sep;
		}
		if (icon_ != NO_ICON)
		{
			// draw icon
			CPoint pos= rect.TopLeft();
			pos.y += 1;
			//if (icon_ == BOOK)
			//	pos.y--;
			int hh = Pixels(HEADER_HEIGHT);
			img_list_group_icons_.Draw(&header_dc, static_cast<int>(icon_), pos, ILD_TRANSPARENT);
			rect.top += (hh - ctrl.M_size_.cy) / 2;
			int dx = 38;
			//if (icon_ == FILM_ROLL)
			//	dx += 34;
			//else if (icon_ == SIMILARITY)
			//	dx += 42;
			//else if (icon_ == BOOK)
			//	dx += 43;
			//else if (icon_ == STAR)
			//	dx += 37;
			//else if (icon_ == CALENDAR)
			//	dx += 38;
			//else
			//	dx += 54;

			rect.left += Pixels(dx);
			height += Pixels(HEADER_HEIGHT);
		}
		if (!name_.empty())
		{
			// draw group name

			header_dc.SelectObject(&ctrl.group_fnt_);

			header_dc.SetTextColor(ctrl.rgb_text_color_);
			header_dc.SetBkColor(rgb_back);

			header_dc.SetBkMode(OPAQUE);

			//OLD comment
			// rect is as wide as a window, but if window is narrower when an item and
			// and view is shifted text would be cut of without DT_NOCLIP
			//NEW comment
			// I no longer see this text being cut of, but with NOCLIP flag I can see some garbage
			// pixels lit on when label text is long enough (text right side is short 2 pixels due to the margin?)
			header_dc.DrawText(name_.c_str(), static_cast<int>(name_.size()), rect,
				DT_LEFT | DT_TOP | DT_SINGLELINE | DT_EXPANDTABS | DT_NOPREFIX);// | DT_NOCLIP);

			label_rect_ = CRect(rect.TopLeft(), header_dc.GetTextExtent(name_.c_str(), static_cast<int>(name_.size())));
		}
		else
			label_rect_.SetRectEmpty();

		header_dc.BitBlt(height);
	}

	// draw items

	int width= horz_items_ * item_size_.cx;
	if (width < location_rect_.right)
		paint_dc.FillSolidRect(CRect(CPoint(width, rect.bottom), CPoint(location_rect_.BottomRight())), rgb_back);

	int first= 0, last= -1;
//TODO
//	FindVisibleItems(first, last);
last = int(items_.size()) - 1;

	for (int index= first; index <= last; ++index)
	{
		CRect rect= GetItemRect(index);

		if (ctrl.IsRectVisible(paint_dc, rect)) //paint_dc.RectVisible(rect))
		{
			if (ctrl.mode_ == DETAILS)
				item_dc.OffsetYTo(rect.top);
			else
				item_dc.OffsetTo(rect);
			Item* item= items_[index];
			item->DrawItem(item_dc, rect, ctrl, ctrl.current_item_.Item() == item, ctrl.ShowPhotoMarker(item), ctrl.ShowNoExifIndicator(item));
			item_dc.BitBlt(rect.Height());
		}
	}

	if (!items_.empty() && last >= 0)
		if ((last + 1) % horz_items_)
		{
			CRect rect= GetItemRect(last + 1);
			rect.right = location_rect_.right;
			paint_dc.FillSolidRect(rect, rgb_back);
		}
}


int PhotoCtrl::Group::GetItemTop() const
{
	int top= location_rect_.top;

	if (flags_ & SEPARATOR)
		top += Pixels(SEPARATOR_HEIGHT);

	if ((flags_ & NO_HEADER) == 0)
		top += Pixels(HEADER_HEIGHT);

	return top;
}


int PhotoCtrl::Group::GetRowHeight(int row) const
{
	if (row < 0 || row >= row_heights_.size())
	{
		ASSERT(false);
		return 0;
	}
	return row > 0 ? row_heights_[row] - row_heights_[row - 1] : row_heights_[0];
}


int PhotoCtrl::Group::GetRowPosition(int row) const
{
	if (row < 0 || row >= row_heights_.size())
	{
		ASSERT(false);
		return 0;
	}
	return row > 0 ? row_heights_[row - 1] : 0;
}


CRect PhotoCtrl::Group::GetItemRect(int index, bool bounding_box/*= true*/) const
{
	if (index < 0 || index > items_.size())
	{
		ASSERT(false);
		return CRect(0,0,0,0);
	}

	// note: index == items_.size() is fine!

//	ASSERT(item_size_.cx > 0 && item_size_.cy > 0);

	CRect rect(CPoint(0, 0), item_size_);

	if (horz_items_ > 0)
	{
		int row= index / horz_items_;
		int col= index % horz_items_;

		rect.OffsetRect(col * item_size_.cx + location_rect_.left, GetRowPosition(row) + GetItemTop());

		rect.bottom = rect.top + GetRowHeight(row);
	}

	if (!bounding_box)
		rect.DeflateRect(margins_rect_);

	return rect;
}


PhotoCtrl::Group::GroupElements PhotoCtrl::Group::HitTest(CPoint pos) const
{
	if (!location_rect_.PtInRect(pos))
		return NONE;

	if (label_rect_.PtInRect(pos))
		return GROUP_LABEL;

	if (flags_ & SEPARATOR)
		if (pos.y < location_rect_.top + Pixels(SEPARATOR_HEIGHT))
			return GROUP_SEPARATOR;

	if (pos.y < GetItemTop())
		return GROUP_HEADER;

	//TODO: rest (or skip)

	return NONE;
}


PhotoCtrl::ItemGroupPair PhotoCtrl::FindItem(PhotoInfoPtr photo)
{
	Item* item= items_list_.FastFind(photo);
	if (item == nullptr || item->GetGroup() == nullptr)
		return ItemGroupPair();

	return ItemGroupPair(item->GetGroup(), item);
}


struct MatchingPhotoInfo : std::binary_function<PhotoCtrl::Item*, PhotoInfoPtr, bool>
{
	bool operator () (PhotoCtrl::Item* item, PhotoInfoPtr photo) const
	{
		return item->photo_ == photo;
	}
};

//PhotoCtrl::Item* PhotoCtrl::Group::FindItem(PhotoInfoPtr photo)
//{
///*
//	ASSERT(items_.size() == sorted_.size());
//	ItemVector::iterator it= lower_bound(sorted_.begin(), sorted_.end(), photo, ItemsByPhotos());
//	if (it != sorted_.end() && (*it)->photo_ == photo)
//		return *it;
//	return 0; */
//
//	ItemVector::iterator it= find_if(items_.begin(), items_.end(), bind2nd(MatchingPhotoInfo(), photo));
//	if (it != items_.end())
//		return *it;
//	return 0;
//}


PhotoCtrl::Item* PhotoCtrl::Group::FindFirstItemInRect(const CRect& rect, int current_index, bool full_row/*= true*/)
{
	if (items_.empty())
		return 0;

	CRect item_rect= GetItemRect(0);

	int y= rect.top - item_rect.top;
	if (full_row)	// row should be fully visible one?
		y += item_size_.cy / 2;
	int row= FindRow(y);
	if (row < 0)
		row = 0;
	int col= current_index % horz_items_;
	int index= row * horz_items_ + col;

	if (index >= items_.size())
	{
		row = static_cast<int>(items_.size()) / horz_items_;
		index = row * horz_items_ + col;

		if (index >= items_.size())
			return items_.back();
	}

	return items_[index];
}


PhotoCtrl::Item* PhotoCtrl::Group::FindLastItemInRect(const CRect& rect, int current_index, bool full_row/*= true*/)
{
	if (items_.empty() || horz_items_ == 0)
		return 0;

	CRect item_rect= GetItemRect(0);

	int y= rect.bottom - item_rect.top;
	if (full_row)	// row should be fully visible one?
		y -= item_size_.cy / 2;
	int row= FindRow(y);
	if (row < 0)
		row = 0;
	int col= current_index % horz_items_;
	int index= row * horz_items_ + col;

	if (index >= items_.size())
	{
		row = static_cast<int>(items_.size()) / horz_items_;
		index = row * horz_items_ + col;

		if (index >= items_.size())
			return items_.back();
	}

	return items_[index];
}


int PhotoCtrl::Group::FindRow(int y_pos) const
{
	std::vector<int>::const_iterator it= lower_bound(row_heights_.begin(), row_heights_.end(), y_pos);
	if (it != row_heights_.end())
		return static_cast<int>(it - row_heights_.begin());
	if (!row_heights_.empty() && y_pos >= row_heights_.back())
		return static_cast<int>(row_heights_.size());	// 'y' pos is below the last row
	return -1;
}


PhotoCtrl::Item* PhotoCtrl::Group::FindItem(PhotoCtrl& ctrl, CPoint pos)
{
	if (items_.empty())
		return 0;

	CRect rect= GetItemRect(0);

	if (pos.x < rect.left || pos.y < rect.top)
		return 0;

	int row= FindRow(pos.y - rect.top);
	int col= (pos.x - rect.left) / item_size_.cx;

	if (col >= horz_items_ || row < 0)
		return 0;

	int index= col + row * horz_items_;
	if (index < 0)
	{
		ASSERT(false);
		return 0;
	}

	if (index >= items_.size())
		return 0;

	// check point against item's raw rect (not bounding box)
	rect = GetItemRect(index, false);

	if (!rect.PtInRect(pos))
		return 0;

	/*rect = GetItemRect(index);

	// now examine bmp area (important for preview and thumbnails modes)
	CRect img_rect= ctrl.GetImageRect(rect, items_[index]->photo_, true);

	if (!img_rect.PtInRect(pos))
		return 0;
*/
	return items_[index];
}


PhotoCtrl::Item* PhotoCtrl::GroupVector::FindItem(PhotoCtrl& ctrl, CPoint pos)
{
	if (Group* group= HitTest(pos))
		return group->FindItem(ctrl, pos);

	return 0;
}


PhotoCtrl::ItemGroupPair PhotoCtrl::GroupVector::FindItemGroup(PhotoCtrl& ctrl, CPoint pos)
{
	if (Group* group= HitTest(pos))
		return ItemGroupPair(group, group->FindItem(ctrl, pos));

	return ItemGroupPair();
}


void PhotoCtrl::Group::SynchTempSelection()
{
	for_each(items_.begin(), items_.end(), std::mem_fun(&Item::SynchTempSelection));
}


struct OredItemTempSelToPerm
{
	bool operator () (bool result, PhotoCtrl::Item* item)
	{
		return item->TempSelectionToPermanent() || result;
	}
};

bool PhotoCtrl::Group::TempSelectionToPermanent()
{
	bool changed= false;
	// temp selection to permanent; accumultes changes
	return accumulate(items_.begin(), items_.end(), changed, OredItemTempSelToPerm());
}


void PhotoCtrl::GroupVector::SynchTempSelection()
{
	for_each(begin(), end(), std::mem_fun(&Group::SynchTempSelection));
}

struct OredTempSelToPerm
{
	bool operator () (bool result, PhotoCtrl::Group* group)
	{
		return group->TempSelectionToPermanent() || result;
	}
};

bool PhotoCtrl::GroupVector::TempSelectionToPermanent()
{
	bool changed= false;
	return accumulate(begin(), end(), changed, OredTempSelToPerm());
}

struct AccCountOfItems
{
	int operator () (int count, const PhotoCtrl::Group* group) const
	{
		return count + group->GetItemCount();
	}
};

int PhotoCtrl::GroupVector::GetItemCount() const
{
	int count= 0;
	return accumulate(begin(), end(), count, AccCountOfItems());
}


COLORREF CalcColor(COLORREF rgb_color1, COLORREF rgb_color2, float bias)
{
	ASSERT(bias >= 0.0f && bias <= 1.0f);

	int red1= GetRValue(rgb_color1);
	int green1= GetGValue(rgb_color1);
	int blue1= GetBValue(rgb_color1);

	int red2= GetRValue(rgb_color2);
	int green2= GetGValue(rgb_color2);
	int blue2= GetBValue(rgb_color2);

	return RGB(red2 + bias * (red1 - red2), green2 + bias * (green1 - green2), blue2 + bias * (blue1 - blue2));
}


void PhotoCtrl::Item::DrawItem(CDC& dc, CRect rect, PhotoCtrl& ctrl, bool is_current, bool show_marker, bool no_exif_marker)
{
	dc.FillSolidRect(rect, ctrl.rgb_bk_color_); //RGB(rand(), rand(), rand()));

	if (photo_ == 0 || ctrl.host_ == 0)
		return;

	bool selected= IsTempSelected();
	bool focus= ::GetFocus() == ctrl.m_hWnd;	// do we have focus?
	COLORREF rgb_text= selected ? ctrl.rgb_sel_text_color_ : ctrl.rgb_text_color_;
	COLORREF rgb_back= selected ? ctrl.rgb_sel_color_ : ctrl.rgb_bk_color_;
	COLORREF rgb_image_back= rgb_back;
	COLORREF rgb_destbg = RGB(225, 225, 225);
	float sel_to_back_color_ratio= 0.4f; // 40% of selection color and 60% of background
	float outline_sel_to_back_color_ratio= 0.4f; // 40% of selection color and 60% of background
	int label_height= ctrl.GetLabelHeight();
	CRect photo_rect(0,0,0,0);
	bool preview_mode= ctrl.mode_ == PREVIEWS;

	if (is_current && !selected && focus)
	{
		// calculate light variation of selection color
		rgb_image_back = CalcColor(ctrl.rgb_sel_color_, ctrl.rgb_bk_color_, sel_to_back_color_ratio);
	}

	// draw image (and current item outline)
	if (!ctrl.image_rect_.IsRectEmpty())
	{
		ASSERT(ctrl.mode_ != DETAILS);
//dc.Draw3dRect(rect, 0xc0c0c0, 0xa0a0a0);
		CRect img_rect= ctrl.GetImageRect(rect, photo_, false);

		// current item outline
		COLORREF rgb_focus_outline;
		if (focus)
			rgb_focus_outline = CalcColor(ctrl.rgb_sel_color_, ctrl.rgb_bk_color_, outline_sel_to_back_color_ratio);
		else
		{
			COLORREF backgnd= ctrl.rgb_bk_color_;

			float brightness = CalcColorBrightness(backgnd);

			rgb_focus_outline = brightness > 128 ? CalcShade(backgnd, -10.0f) : CalcShade(backgnd, 12.0f);
		}

		UINT flags= PhotoInfo::DRAW_BACKGND | PhotoInfo::DRAW_SHADOW;

		// for big preview images use requested method: halftone or point sample;
		// for remaining modes use point sample--cache should already contain resized smooth images
		if (preview_mode)
			flags |= ctrl.halftone_drawing_ ? PhotoInfo::DRAW_HALFTONE : PhotoInfo::DRAW_FAST;
		else
			flags |= PhotoInfo::DRAW_FAST;

		//if (ctrl.mode_ == MOSAIC)
		//	flags &= ~PhotoInfo::DRAW_SHADOW;

		if (selected)
			flags |= PhotoInfo::DRAW_SELECTION;
		if (is_current && !selected)
			flags |= PhotoInfo::DRAW_OUTLINE;

		dc.SelectObject(&ctrl.default_fnt_);

		bool draw_label= (ctrl.mode_ == THUMBNAILS || preview_mode) && ctrl.show_label_;

		photo_rect = img_rect;

		String label;

		if (draw_label)
			label = ctrl.host_->GetItemLabel(photo_, dc, photo_rect.Width());

		bool draw_thumbnail= true;

		if (preview_mode)
		{
			Dib* bmp= ctrl.host_->RequestImage(photo_, photo_rect.Size(), false);

			if (bmp && bmp->IsValid())
			{
				AutoPtr<Dib> transBmp;
				try
				{
					transBmp = ctrl.host_->CorrectImageColors(photo_, bmp);
					if (transBmp.get())
						bmp = transBmp.get();
				}
				catch (...)
				{
					ASSERT(false);
				}

				draw_thumbnail = false;

				ImageDraw::Draw(bmp, &dc, photo_rect, ctrl.rgb_bk_color_, ctrl.rgb_sel_color_, rgb_focus_outline, rgb_text, rgb_back, flags, draw_label ? &label : 0);
			}
		}

		if (draw_thumbnail)
		{
			CSize dest_size= ImageDraw::GetImageSize(0, photo_rect, flags).Size();

			Dib* dib= ctrl.host_->RequestThumbnail(photo_, dest_size, false);
			//Dib* dib= photo_->GetThumbnail(dest_size);

			AutoPtr<Dib> trans_bmp;
			if (dib && dib->IsValid() && ctrl.host_)
			{
				trans_bmp = ctrl.host_->CorrectImageColors(photo_, dib);
				if (trans_bmp.get())
					dib = trans_bmp.get();
			}

			// img draw handles invalid bmp too
			ImageDraw::Draw(dib, &dc, photo_rect, ctrl.rgb_bk_color_, ctrl.rgb_sel_color_, rgb_focus_outline, rgb_text, rgb_back, flags, draw_label ? &label : 0);

			if (dib == 0 || !dib->IsValid())
				PhotoInfo::DrawErrIcon(&dc, photo_rect);
		}

//dc.Draw3dRect(img_rect, RGB(255,0,0), RGB(0, 200,0));
//CString info;
//info.Format(L"%d x %d", rect.Width(), rect.Height());
//dc.TextOut(img_rect.left, img_rect.top, info);
//info.Format(L"%d x %d", img_rect.Width(), img_rect.Height());
//dc.TextOut(img_rect.left, img_rect.top + 14, info);

		if (preview_mode)
			img_rect = photo_rect;	// for img marker use real image position

		img_rect.top = photo_rect.top;
		img_rect.bottom = photo_rect.bottom + 1;

		if (no_exif_marker && !photo_->IsExifDataPresent())
			DrawNoExifIndicator(dc, img_rect);
		if (ctrl.show_marker_  && show_marker)
			DrawTypeIndicator(dc, rect, photo_->GetFileTypeIndex());
	}
	else
	{
	//	ASSERT(ctrl.mode_ == DETAILS);

		//int state= LISS_NORMAL;
		//if (selected)
		//	state = is_current ? LISS_SELECTED : LISS_SELECTEDNOTFOCUS;
		//else if (is_current)
		//	state = LISS_NORMAL;

/*		if (CtrlDraw::DrawListItem(dc, rect, state))
		{
//			return;//
		}
		else */ if (is_current)		// draw current item outline
		{
			if (!selected)
				rgb_back = rgb_image_back;

			dc.FillSolidRect(rect, rgb_back);

			COLORREF rgb_focus_outline= ctrl.rgb_sel_color_;
			if (selected)
				rgb_focus_outline = CalcColor(ctrl.rgb_sel_color_, ctrl.rgb_bk_color_, 0.5f);
			else if (!focus)
				rgb_focus_outline = CalcColor(ctrl.rgb_sel_color_, ctrl.rgb_bk_color_, outline_sel_to_back_color_ratio);

			dc.Draw3dRect(rect, rgb_focus_outline, rgb_focus_outline);
		}
		else
		{
			dc.FillSolidRect(rect, rgb_back);
		}
	}

	// draw text label
	if (!ctrl.label_rect_.IsRectEmpty())
	{
		CRect text_rect= ctrl.label_rect_ + rect.TopLeft();
		CRect rect_frame= text_rect;
		//rect_frame.left-= 6;
		//rect_frame.top = rect.top;
//text_rect = rect;
//text_rect.top = text_rect.bottom - label_height + 2;

		dc.SelectObject(&ctrl.default_fnt_);

		dc.SetTextColor(rgb_text);
		dc.SetBkColor(rgb_back);

		dc.SetBkMode(OPAQUE);

		if (ctrl.mode_ == TILES)
		{	
			text_rect.top-= MARGIN_BOTTOM;
			rect_frame.top-= MARGIN_TOP;
			rect_frame.left-= ITEM_X_MARGIN + 1;
			if (selected)
			{
				dc.FillSolidRect(rect_frame, rgb_image_back);
				dc.SetBkColor(rgb_image_back);
				//Gdiplus::Graphics g(dc);
				//g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
				//g.TranslateTransform(-0.5f, -0.5f);
				//int radius= Pixels(dc, 4);

				//Gdiplus::RectF area= CRectToRectF(text_rect.left - 3, text_rect.top - 2, text_rect.Width() + 2, text_rect.Height() + 2);
				//{
				//	Gdiplus::GraphicsPath frame;
				//	RoundRect(frame, area, static_cast<float>(radius));
				//	Gdiplus::SolidBrush brush(c2c(rgb_image_back));
				//	g.FillPath(&brush, &frame);
				//}
			}
			else{
				dc.FillSolidRect(rect_frame, rgb_destbg);//rgb_image_back);
				dc.SetBkColor(rgb_destbg);
			}

			dc.SetTextColor(rgb_text);

			text_rect.bottom = text_rect.top + ctrl.M_size_.cy;

			UINT format= DT_LEFT | DT_TOP | DT_WORDBREAK| DT_EXPANDTABS | DT_NOPREFIX | DT_END_ELLIPSIS;
			dc.DrawText(photo_->GetName().c_str(), static_cast<int>(photo_->GetName().length()), text_rect, format);

			if (!selected)
				dc.SetTextColor(ctrl.rgb_tile_dim_text_color_);

			text_rect.OffsetRect(0, ctrl.M_size_.cy + 2);

			String date_time= photo_->DateTimeStr();
			dc.DrawText(date_time.c_str(), static_cast<int>(date_time.length()), text_rect, format);

			text_rect.OffsetRect(0, ctrl.M_size_.cy);
			String str= photo_->Size();
			dc.DrawText(str.c_str(), static_cast<int>(str.length()), text_rect, format);

			text_rect.OffsetRect(0, ctrl.M_size_.cy);
			str = photo_->FileSize();
			dc.DrawText(str.c_str(), static_cast<int>(str.length()), text_rect, format);

			text_rect.OffsetRect(0, ctrl.M_size_.cy);
			str = _T("F");
			str += photo_->FNumber();
			str += _T(", ");
			str += photo_->ExposureTime();
			str += _T(", ");
			str += photo_->FocalLengthInt();
			str += _T("mm");
			if(str == _T("F-, -, -mm")) str=_T(" ");
			dc.DrawText(str.c_str(), static_cast<int>(str.length()), text_rect, format);

		}
		/*else if (ctrl.mode_ == LIST)
		{
			dc.FillSolidRect(text_rect, rgb_image_back);

			dc.SetBkColor(rgb_image_back);
			dc.SetTextColor(rgb_text);

//			text_rect.bottom = text_rect.top + ctrl.M_size_.cy;

			UINT format= DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_EXPANDTABS | DT_NOPREFIX | DT_END_ELLIPSIS;
			if (ctrl.host_)
			{
				String label= ctrl.host_->GetItemLabel(photo_, dc, ctrl.label_rect_.Width());
				dc.DrawText(label.c_str(), static_cast<int>(label.length()), text_rect, format);
			}

			CRect img(0, 0, ctrl.image_rect_.Width(), ctrl.label_rect_.Height());
			img.OffsetRect(rect.TopLeft());
			photo_->Draw(&dc, img, rgb_image_back, 0, 0, 0, 0, 0, 0);
		}*/
		else
		{
/*
			CSize text_size= dc.GetTextExtent(photo_->GetName().c_str(), photo_->GetName().size());

			UINT flags= DT_CENTER | DT_TOP | DT_SINGLELINE | DT_NOPREFIX;

			if (text_size.cx < text_rect.Width())
			{
				text_rect.left += (text_rect.Width() - text_size.cx) / 2;
				text_rect.right = text_rect.left + text_size.cx;
			}
			else
				flags |= DT_END_ELLIPSIS;

			dc.FillSolidRect(text_rect.left, text_rect.top, text_rect.Width() + 1, text_rect.Height() - 1, rgb_back);
			dc.DrawText(photo_->GetName().c_str(), photo_->GetName().size(), text_rect, flags);
*/
		}
	}

	// details mode
	if (ctrl.mode_ == DETAILS)
	{
		const size_t count= ctrl.header_columns_.size();

		dc.SelectObject(&ctrl.default_fnt_);

		dc.SetTextColor(rgb_text);
		dc.SetBkColor(rgb_destbg);
		
		//dc.SetBkMode(OPAQUE);
		dc.SetBkMode(TRANSPARENT);

		String buffer;
		CRect text_rect= rect;
		int sort_column= abs(ctrl.sort_by_column_) - 1;
		///////////////////
		if(!is_current){
			CRect row_rect = rect;
			row_rect.DeflateRect(0, 1, 0, 1);
			dc.FillSolidRect(row_rect, rgb_destbg);
		}
////////////////////////////////////
		for (size_t i= 0; i < count; ++i)
		{
			int col_index= ctrl.header_columns_[i].order;
			text_rect.right = text_rect.left + ctrl.header_columns_[col_index].width;

			if (col_index == 0)		// 'photo' column
			{
				CRect rect= ctrl.GetFirstColImgRect(text_rect);
				text_rect.left = rect.right;		// ensure that text is shifted by same amount in all items

				//photo_->Draw(&dc, rect, rgb_image_back, 0, 0, 0, 0, 0, 0);

				Dib* dib= ctrl.host_->RequestThumbnail(photo_, rect.Size(), false);

				if (dib && dib->IsValid())
					ImageDraw::Draw(dib, &dc, rect, rgb_image_back, 0, 0, 0, 0, 0, 0);
				else
				{
					rect.DeflateRect(1, 1);
					if (!photo_->HorizontalOrientation())
					{
						CSize size= Dib::SizeToFit(rect.Size(), CSize(rect.Height(), rect.Width()));
						rect.left += (rect.Width() - size.cx) / 2;
						rect.right = rect.left + size.cx;
					}
					//dc.FillSolidRect(rect, RGB(200,200,200));
				}

//				text_rect.left = rect.right;	<- this line here is image width dependent; not good for details, causes text shift

				// shift subsequent columns left by left margin value (there's no way to shift header
				// control to counter this margin, so first column has to include it...)
				text_rect.right -= ctrl.margins_rect_.left;
			}

			dc.SetTextColor(rgb_text);
			if (col_index == sort_column && !selected)
			{
				if (!is_current)
				{	
					CRect rect= text_rect;
					rect.DeflateRect(0, 1, 0, 1);
					dc.FillSolidRect(rect, ctrl.rgb_sort_color_);
					//dc.SetBkColor(ctrl.rgb_sort_color_);
				}
				else	// is current
				{
					if (!focus)
					{
						CRect rect= text_rect;
						//COLORREF rgb_outline= CalcColor(ctrl.rgb_sel_color_, ctrl.rgb_sort_color_, outline_sel_to_back_color_ratio);
						//dc.FillSolidRect(rect.left, rect.top, rect.Width(), 1, rgb_outline);
						//dc.FillSolidRect(rect.left, rect.bottom - 1, rect.Width(), 1, rgb_outline);
						rect.DeflateRect(0, 1, 0, 1);
						dc.FillSolidRect(rect, ctrl.rgb_sort_color_);
						//dc.SetBkColor(ctrl.rgb_sort_color_);
					}
					else	// has focus
					{
						CRect rect= text_rect;
						rect.DeflateRect(0, 1, 0, 1);
						COLORREF rgb_current= CalcColor(ctrl.rgb_sel_color_, ctrl.rgb_sort_color_, sel_to_back_color_ratio);
						dc.FillSolidRect(rect, rgb_current);
						//dc.SetBkColor(rgb_current);
					}
				}
			}
			//else if (!selected){
				//CRect rect= text_rect;
				//rect.DeflateRect(0, 1, 0, 1);
				//dc.FillSolidRect(rect, rgb_destbg);
				//dc.SetBkColor(rgb_back);
			//}

			buffer.clear();
			ctrl.host_->GetItemText(photo_, ctrl.header_columns_[col_index].sub_item, buffer);

			if (!buffer.empty())
			{
				CRect rect= text_rect;
				rect.DeflateRect(ITEM_X_MARGIN, 0, ITEM_X_MARGIN, 0);	// make narrower

				int format= ctrl.header_columns_[col_index].format |
					DT_VCENTER | DT_SINGLELINE | DT_EXPANDTABS | DT_NOPREFIX | DT_END_ELLIPSIS;

				dc.DrawText(buffer.c_str(), static_cast<int>(buffer.size()), rect, format);
			}

			text_rect.OffsetRect(text_rect.Width(), 0);
		}
	}
	else if (ctrl.show_tags_ && (!photo_->GetTags().empty() || photo_->GetRating()) && ctrl.mode_ != TILES)
	{
		//TODO: replace by a call to DrawPhotoTags, update DrawPhotoTags with below changes  ------------------

		// tags (in all but detail mode)

		CRect img_rect= ctrl.image_rect_ + rect.TopLeft();

		const int MARGIN= 3;
		int tag_w= MARGIN;

		tag_w += img_rect.Width() * 2 / 3;

		// in preview mode draw tag on the photo
		if (preview_mode)
		{
			const int PROTRUDE= ctrl.M_size_.cx / 2;
			CRect rect(photo_rect.left, photo_rect.top - PROTRUDE, photo_rect.right + PROTRUDE + 1, img_rect.bottom);
			img_rect = rect & img_rect;
		}

		dc.SelectObject(&ctrl.tags_fnt_);

		CSize text_size= dc.GetTextExtent(_T("X"), 1);	// TODO: Msize here

		const size_t tags_count= photo_->GetTags().size();

		//int tag_h= min<long>(text_size.cy * tags_count + MARGIN, img_rect.Height());

		CPoint tags(img_rect.right - tag_w, img_rect.top);

		CBrush br(ctrl.rgb_tag_bkgnd_);
		CBrush* old= dc.SelectObject(&br);
		dc.SelectStockObject(NULL_PEN);
		//dc.FillSolidRect(tags.x, tags.y, tag_w, tag_h, ctrl.rgb_tag_bkgnd_);

		tags.x += MARGIN;
		tag_w -= MARGIN;

		dc.SetTextColor(ctrl.rgb_tag_text_);
		dc.SetBkColor(ctrl.rgb_tag_bkgnd_);

//		dc.SetBkMode(OPAQUE);

		CRect text_rect(tags, CSize(tag_w - 2, text_size.cy));
		text_rect.top++;

		int tag_backgnd_alpha= 200;	// transparency of tag background

		// draw stars
		int rating= photo_->GetRating();	// draw stars for ratings > 0
		if (rating > 0 && rating <= 5)
		{
			CFont wingdings;
			LOGFONT lf;
			ctrl.default_fnt_.GetLogFont(&lf);
			_tcscpy(lf.lfFaceName, _T("WingDings"));
			lf.lfCharSet = SYMBOL_CHARSET;
			if (lf.lfHeight < 0)
				lf.lfHeight -= 4; // larger stars
			else
				lf.lfHeight += 4;
			lf.lfPitchAndFamily = DEFAULT_PITCH;
			lf.lfWeight = FW_DONTCARE;
			wingdings.CreateFontIndirect(&lf);
			TCHAR stars[]=
#ifdef _UNICODE
			{ 0xf0ab, 0 };	// star
#else
			{ 0xab, 0 };	// star
#endif
			int len= rating;
			int h= dc.GetTextExtent(_T("X"), 1).cy;
			int round= h / 3;

			CFont* old= dc.SelectObject(&wingdings);
			CSize size= dc.GetTextExtent(stars, 1);
			const int right_margin= 3;
			int tag_width= round + size.cx * len + right_margin;
			if (img_rect.Width() < tag_width)
				size.cx = (img_rect.Width() - round - right_margin) / len;

			if (size.cx > 0)
			{
				int width= round + size.cx * len + right_margin;
				int x= text_rect.right - width;

				::DrawTagBackground(dc, CRect(x, text_rect.top, text_rect.right, text_rect.top + size.cy - 1), ctrl.rgb_tag_bkgnd_, tag_backgnd_alpha);

				CRect rect(x, text_rect.top, text_rect.right, text_rect.top + size.cy - 1);

				dc.SetBkMode(TRANSPARENT);

				x += round;
				for (int i= 0; i < len; ++i)
				{
					dc.TextOut(x, text_rect.top, stars, 1);
					x += size.cx;
				}

				dc.SelectObject(old);
				dc.SetTextCharacterExtra(0);
				text_rect.OffsetRect(0, size.cy);
			}
		}

		dc.SetBkMode(OPAQUE);

		// draw tags
		for (size_t i= 0; i < tags_count; ++i)
		{
			bool fit= text_rect.bottom < photo_rect.bottom;
			const String& str= photo_->GetTags()[i];
			const TCHAR* text= fit ? str.c_str() : _T("...");
			int length= static_cast<int>(fit ? str.size() : _tcslen(text));
			CSize size= dc.GetTextExtent(text, length);
			size.cy++;
			int round= size.cy;
			int width= std::min<long>(text_rect.Width(), size.cx + round / 2 + 3);
			int x_pos= text_rect.right - width;
			::DrawTagBackground(dc, CRect(x_pos, text_rect.top, text_rect.right, text_rect.top + size.cy - 1), ctrl.rgb_tag_bkgnd_, tag_backgnd_alpha);
			CRect rect(x_pos + round / 2, text_rect.top, text_rect.right, text_rect.top + size.cy - 1);
			rect.right -= 3;
			dc.SetBkMode(TRANSPARENT);
			dc.DrawText(text, length, rect, DT_RIGHT | DT_VCENTER | DT_SINGLELINE | DT_EXPANDTABS | DT_END_ELLIPSIS);
			text_rect.OffsetRect(0, size.cy);
			if (!fit)
				break;
		}

		dc.SelectObject(old);
	}
}

// draw 'no EXIF' indicator in the right bottom corner of rect
void PhotoCtrl::Item::DrawNoExifIndicator(CDC& dc, CRect rect)
{
	DrawTypeIndicator(dc, rect, -1);
}


void PhotoCtrl::Item::DrawTypeIndicator(CDC& dc, CRect rect, int index)
{
	const int OVERHANG= 2;
	rect.top = rect.top + Pixels(OVERHANG + 2);//rect.bottom - Pixels(INDICATOR_IMG_HEIGHT + OVERHANG);
	rect.left = rect.left + Pixels(OVERHANG);// + Pixels(OVERHANG);
	index++;
	if (index == 0){
		rect.left = rect.left;
		rect.top = rect.bottom - Pixels(INDICATOR_IMG_HEIGHT);
	}
	img_list_indicators_.Draw(&dc, index, rect.TopLeft(), ILD_TRANSPARENT);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Mouse related operations
///////////////////////////////////////////////////////////////////////////////////////////////////


void PhotoCtrl::OnMouseButtonDown(UINT flags, CPoint pos, bool left)
{
	bool extend_sel_key= !!(flags & MK_SHIFT);
	bool toggle_sel_key= !!(flags & MK_CONTROL);

	SetFocus();

	if (IsEmpty())
		return;

	pos += GetScrollOffset();
	bool eaten= false;

	select_.go_to_done_ = false;

	if (Group* group= HitTest(pos))
	{
		Group::GroupElements el= group->HitTest(pos);

		if (el == Group::GROUP_LABEL)
		{
			if (left)
			{
				// group label clicked--toggle items selection
				if (group->SelectAll(true, true))
				{
					InvalidateGroup(group);
					SelectionChanged();
				}

				eaten = true;	// do not process any further
			}
		}

		if (ItemGroupPair igp= ItemGroupPair(group, group->FindItem(*this, pos)))
		{
			// select clicked item only if it's not yet selected; otherwise wait
			// till mouse btn up because of possible drag&drop operation
			bool select_now= !igp.Item()->IsSelected();
			GoTo(igp, select_now, extend_sel_key, toggle_sel_key);
			select_.go_to_done_ = select_now;
		}
	}

	if (!eaten)
	{
		if (select_.selecting_)	// selecting in progress?
		{
			// it can only happen when mouse btn down message has no btn up msg
			StopSelecting(true, false, false, false);
			//
		}
		else
		{
			SetCapture();

			// starting mouse items selection
			select_.selecting_ = true;
			// no lasso selection yet
			select_.lasso_ = false;
			// starting point
			select_.start_ = pos;

			// synchronize temp selection with regular selection
			SynchTempSelection();
		}
	}
}


void PhotoCtrl::StopSelecting(bool cancel, bool select_current, bool extend_sel_key, bool toggle_sel_key)
{
	if (select_.selecting_)
	{
		KillTimer(SCROLL_TIMER_ID);

		// if there was lasso selection used convert temp selection to regular one
		if (select_.lasso_)
		{
			CClientDC dc(this);
			PrepareDC(dc);
			DrawSelectionRect(dc, true);	// erase selection rect

			if (cancel)	// discard current lasso selection?
			{
				SynchTempSelection();
				Invalidate();
			}
			else			// change temporary selection into permanent
			{
				// change current item to the first one newly selected (but don't scroll to it)
//				if (Item* defect= gallery_ctrl_->defects_->FindFirstTempSelected())
//					GoTo(defect, false);

				if (TempSelectionToPermanent())
					SelectionChanged();
			}
		}
		else if (!select_.drag_and_drop_)	// no lasso and no drag&drop--just point and click
		{
			if (!select_.go_to_done_)
			{
				if (Group* group= HitTest(select_.start_))
				{
					if (ItemGroupPair igp= ItemGroupPair(group, group->FindItem(*this, select_.start_)))
					{
						// point click on the item
						GoTo(igp, select_current, extend_sel_key, toggle_sel_key);
						select_.go_to_done_ = true;
					}
/*					else if (group->HitTest(select_.start_) == Group::GROUP_HEADER)
					{
						// if there was only point and click on group's header
						// then move current item to the first group's item

						//suppressed for now; it's silly
						//GoTo(ItemGroupPair(group, group->GetItem(0)));
					} */
				}
			}

			if (!select_.go_to_done_)
				if (select_current && !sticky_selection_ && !extend_sel_key && !toggle_sel_key)
					SelectItems(NONE, true);	// clear current selection

		}

		ReleaseCapture();
	}

	select_.Stop();
	SetCursor();
}


void PhotoCtrl::OnMouseButtonUp(UINT flags, CPoint pos, bool left)
{
	try
	{
		bool extend_sel_key= !!(flags & MK_SHIFT);
		bool toggle_sel_key= !!(flags & MK_CONTROL);

		// select image if it's left click, selecting is enabled (ctrl key overwrites this flag)
		bool select_img= left && (!sticky_selection_ || select_.click_and_select_ != toggle_sel_key);

		StopSelecting(false, select_img, extend_sel_key, toggle_sel_key);
	}
	CATCH_ALL
}


void PhotoCtrl::OnLButtonDown(UINT flags, CPoint pos)
{
	try
	{
		OnMouseButtonDown(flags, pos, true);
	}
	CATCH_ALL
}


void PhotoCtrl::OnLButtonUp(UINT flags, CPoint pos)
{
	try
	{
		OnMouseButtonUp(flags, pos, true);
	}
	CATCH_ALL
}


void PhotoCtrl::OnLButtonDblClk(UINT flags, CPoint pos)
{
	try
	{
		if (IsEmpty())
			return;

		if (select_.selecting_)
			return;

		pos += GetScrollOffset();

		if (Item* item= FindItem(pos))
			if (host_)
				host_->ItemDblClicked(item->photo_);
	}
	CATCH_ALL
}


void PhotoCtrl::OnMouseMove(UINT flags, CPoint pos)
{
	try
	{
		bool extend_sel_key= !!(flags & MK_SHIFT);
		bool toggle_sel_key= !!(flags & MK_CONTROL);

		if (select_.selecting_)	// selection mode?
		{
			pos += GetScrollOffset();

			if (!select_.lasso_)	// no lasso yet?
			{
				// check if mouse moved in this small area (10 by 10 pixels)
				CRect rect(select_.start_, select_.start_);
				rect.InflateRect(5, 5);
				if (rect.PtInRect(pos))
				{
					// disregard such a small move--it'll help unselecting
					// images by not entering drag&drop routine
					return;
				}
				else
				{
					// if mouse cursor moved further, check image state
					// and if it's selected start drag&drop operation
					if (ItemGroupPair igp= FindItemGroup(select_.start_))
						if (igp.Item()->IsSelected())
						{
							select_.drag_and_drop_ = true;
							DoDragDrop();					// attempt to drag selected images detected
							OnLButtonUp(0, CPoint(0, 0));	// WM_LBUTTONUP was eaten, so send it now
							return;
						}
				}

				if (!sticky_selection_ && !extend_sel_key && !toggle_sel_key)
				{
					SelectItems(NONE);	// clear current selection
					UpdateWindow();
				}
			}

			CClientDC dc(this);
			PrepareDC(dc);

			// is there a lasso already?
			if (select_.lasso_)
				DrawSelectionRect(dc, true);	// then erase old selection rect

			LassoSelectScroll();

			select_.selection_rect_.SetRect(select_.start_, pos);
			select_.selection_rect_.NormalizeRect();

			SelectTemporarily(dc, select_.selection_rect_, toggle_sel_key);

			// draw selection rect in new size/location
			DrawSelectionRect(dc, false);
			select_.lasso_ = true;
			SetCursor();
		}
		else
		{
			// TODO: tooltip notification
			/*		if (CGalleryWnd* wnd= dynamic_cast<CGalleryWnd*>(GetParent()))
			{
			CDefect* defect= gallery_ctrl_->defects_->FindDefect(pos + GetScrollOffset());
			wnd->MouseHoverNotify(defect);
			} */
		}
	}
	CATCH_ALL
}


void PhotoCtrl::LassoReselect(bool toggle)
{
	if (select_.lasso_)
	{
		SetCursor();

		CClientDC dc(this);
		PrepareDC(dc);

		DrawSelectionRect(dc, true);	// erase lasso

		SelectTemporarily(dc, select_.selection_rect_, toggle);

		DrawSelectionRect(dc, false);	// restore lasso
	}
}


void PhotoCtrl::OnRButtonDown(UINT flags, CPoint pos)
{
	OnMouseButtonDown(flags, pos, false);
}


void PhotoCtrl::OnRButtonUp(UINT flags, CPoint pos)
{
	OnMouseButtonUp(flags, pos, false);

	if (host_)
	{
		CPoint p= pos;
		p += GetScrollOffset();
		Group* group= HitTest(p);

		Group::GroupElements elem= Group::NONE;

		if (group)
			elem = group->HitTest(p);

		host_->ContextMenu(pos, true, group ? group->GetId() : -1, elem);
	}
}


void PhotoCtrl::OnRButtonDblClk(UINT flags, CPoint pos)
{
}


///////////////////////////////////////////////////////////////////////////////////////////////////
// Keyboard
///////////////////////////////////////////////////////////////////////////////////////////////////


void PhotoCtrl::OnKeyDown(UINT chr, UINT rep_cnt, UINT flags)
{
	try
	{
		if (chr == VK_CONTROL && select_.lasso_)
			LassoReselect(true);

		bool extend_sel_key= ::GetKeyState(VK_SHIFT) < 0;
		bool toggle_sel_key= ::GetKeyState(VK_CONTROL) < 0;
		bool alt_key=		 ::GetKeyState(VK_MENU) < 0;
		bool select_current= !toggle_sel_key && !sticky_selection_ || extend_sel_key;

		switch (chr)
		{
		case VK_LEFT:
			// in detailed mode cursor left scrolls
			if (mode_ == DETAILS)
				OnScroll(MAKEWORD(SB_LINEUP, -1), 0);
			else
				GoTo(GO_LEFT, select_current, extend_sel_key, toggle_sel_key);
			break;

		case VK_RIGHT:
			// in detailed mode cursor right scrolls
			if (mode_ == DETAILS)
				OnScroll(MAKEWORD(SB_LINEDOWN, -1), 0);
			else
				GoTo(GO_RIGHT, select_current, extend_sel_key, toggle_sel_key);
			break;

		case VK_UP:
			GoTo(GO_UP, select_current, extend_sel_key, toggle_sel_key);
			break;

		case VK_DOWN:
			GoTo(GO_DOWN, select_current, extend_sel_key, toggle_sel_key);
			break;

		case VK_HOME:
			GoTo(GO_TOP, select_current, extend_sel_key, toggle_sel_key);
			break;

		case VK_END:
			GoTo(GO_BOTTOM, select_current, extend_sel_key, toggle_sel_key);
			break;

		case VK_PRIOR:
			GoTo(GO_PG_UP, select_current, extend_sel_key, toggle_sel_key);
			break;

		case VK_NEXT:
			GoTo(GO_PG_DOWN, select_current, extend_sel_key, toggle_sel_key);
			break;

		case VK_SPACE:
			SelectItems(TOGGLE_CURRENT);
			break;

			/*	case VK_DELETE:
			if (host_ != 0 && !extend_sel_key && !toggle_sel_key && !alt_key)
			host_->DeleteSelected();
			else
			CWnd::OnKeyDown(chr, rep_cnt, flags); */

		default:
			if (host_ != 0)
				host_->KeyDown(chr, rep_cnt, flags);
			else
				CWnd::OnKeyDown(chr, rep_cnt, flags);
			break;
		}
	}
	CATCH_ALL
}


void PhotoCtrl::OnKeyUp(UINT chr, UINT rep_cnt, UINT flags)
{
	try
	{
		if (chr == VK_CONTROL && select_.lasso_)
			LassoReselect(false);

		CWnd::OnKeyUp(chr, rep_cnt, flags);
	}
	CATCH_ALL
}


///////////////////////////////////////////////////////////////////////////////

// set new image size for thumbnail view mode
void PhotoCtrl::SetImageSize(int width)
{
	//if (mode_ == MOSAIC)
	//	img_size_ = CSize(width, width);
	//else
	//{
		img_size_.cx = width + EXTRA - 5;	// cheesy: counter the margin taken out in DrawItem()
		img_size_.cy = width + EXTRA - 5;
	//}

	if (mode_ == THUMBNAILS/* || mode_ == MOSAIC*/)
		SetMode(mode_);	// reset items' size
}


// selecting images
bool PhotoCtrl::SelectItems(Selection sel, bool notify/*= true*/)
{
	bool change= false;

	if (sel == INVERT)
	{
		groups_.ToggleSelection();
		change = true;
	}
	else if (sel == TOGGLE_CURRENT)
	{
		// toggle current item's selection
		if (current_item_)
		{
			CClientDC dc(this);
			PrepareDC(dc);
			current_item_.ToggleSelection();
			current_item_.DrawSelection(*this, dc);
			change = true;
		}
	}
	else
	{
		change = groups_.SelectAll(sel == ALL, false);
	}

	if (change)
	{
		if (sel != TOGGLE_CURRENT)
			Invalidate();
	}

	if (notify)
		SelectionChanged();		// send notification even when no changes have been detected

	return change;
}


bool PhotoCtrl::SelectItem(PhotoInfoPtr photo, bool notify/*= true*/)
{
	Item* item= items_list_.FastFind(photo);

	if (item == 0)
	{
		ASSERT(false);
		return false;
	}

	if (!item->IsSelected())
	{
		item->SetSelection(true);

		if (notify)
			SelectionChanged();

		if (m_hWnd)
		{
			//TODO: optimize
			Invalidate();
		}
		return true;
	}

	return false;
/*
	bool cur_change= false;
	if (current_item_.Item() == item)
	{
		current_item_.Clear();
		anchor_item_.Clear();
		cur_change = true;
	}

	VERIFY(groups_.RemoveItem(*this, item));
	VERIFY(items_list_.Remove(photo));

	if (cur_change && host_ != 0)
		host_->CurrentItemChanged(0);

	if (m_hWnd)
	{
		Invalidate();
		Resize();
	}

	return true; */
}


///////////////////////////////////////////////////////////////////////////////

// insert column in header ctrl
int PhotoCtrl::InsertColumn(int col_index, const TCHAR* column_heading, int format, int width, int sub_item/*= -1*/)
{
	if (header_wnd_.m_hWnd == 0)
	{
		ASSERT(false);
		return -1;
	}
//	header_columns_.reserve(header_columns_.size() + 1)

	HDITEM hdi;
	memset(&hdi, 0, sizeof hdi);
	hdi.mask = HDI_FORMAT | HDI_TEXT | HDI_WIDTH;
	hdi.cxy = width;
	hdi.pszText = const_cast<TCHAR*>(column_heading);
	hdi.fmt = format;

	col_index = header_wnd_.InsertItem(col_index, &hdi);

	if (col_index >= 0)
	{
		int flags= DT_LEFT;

		if (format & HDF_RIGHT)
			flags = DT_RIGHT;
		else if (format & HDF_CENTER)
			flags = DT_CENTER;

//		header_columns_.push_back(ColumnInfo(sub_item < 0 ? col_index : sub_item, width, col_index, flags));
//		header_columns_.insert(header_columns_.begin() + col_index, ColumnInfo(sub_item < 0 ? col_index : sub_item, width, col_index, flags));
		SynchColumnsInfo();
	}

	if (mode_ == DETAILS)
	{
		ResetHorzScrollBar();
		Invalidate();
	}

	return col_index;
}


// delete column
void PhotoCtrl::DeleteColumn(int col_index)
{
	if (header_wnd_.m_hWnd == 0 || col_index < 0 || col_index >= header_columns_.size())
	{
		ASSERT(false);
		return;
	}

	if (header_wnd_.DeleteItem(col_index))
	{
//		header_columns_.erase(header_columns_.begin() + col_index);
		SynchColumnsInfo();

		if (mode_ == DETAILS)
		{
			ResetHorzScrollBar();
			Invalidate();
		}
	}
	else
	{
		ASSERT(false);
	}

}

// modify column's caption
void PhotoCtrl::UpdateColumnHeading(int col_index, const TCHAR* heading)
{
	if (header_wnd_.m_hWnd == 0 || col_index < 0 || col_index >= header_columns_.size())
	{
		ASSERT(false);
		return;
	}

	HDITEM hdi;
	memset(&hdi, 0, sizeof hdi);
	hdi.mask = HDI_TEXT;
	hdi.pszText = const_cast<TCHAR*>(heading);

	header_wnd_.SetItem(col_index, &hdi);
}


// set width of column in detailed view
void PhotoCtrl::SetColumnWidth(int col, int width)
{
	ASSERT(header_wnd_.m_hWnd != 0);

	HDITEM hdi;
	memset(&hdi, 0, sizeof hdi);
	hdi.mask = HDI_WIDTH;
	hdi.cxy = width;

	header_wnd_.SetItem(col, &hdi);

	if (col >= 0 && col < header_columns_.size())
		header_columns_[col].width = width;

	if (mode_ == DETAILS)
		Invalidate();
}


// get columns order
void PhotoCtrl::GetColumnOrderArray(std::vector<int>& col_order)
{
	col_order.clear();

	if (int count= header_wnd_.GetItemCount())
	{
		col_order.resize(count);
		header_wnd_.GetOrderArray(&col_order.front(), static_cast<int>(col_order.size()));
	}
}

// set columns order
void PhotoCtrl::SetColumnOrderArray(const std::vector<int>& col_order)
{
	if (col_order.empty())
		header_wnd_.SetOrderArray(0, 0);
	else
		header_wnd_.SetOrderArray(static_cast<int>(col_order.size()), const_cast<int*>(&col_order.front()));

	// synchronize our copy
	SynchColumnsOrder();

	if (mode_ == DETAILS)
		Invalidate();
}

void PhotoCtrl::SynchColumnsOrder()
{
	std::vector<int> col_order;
	GetColumnOrderArray(col_order);
	const size_t count= col_order.size();
	header_columns_.resize(count);
	for (size_t i= 0; i < count; ++i)
		header_columns_[i].order = col_order[i];
}


void PhotoCtrl::SynchColumnsInfo()
{
	int count= header_wnd_.GetItemCount();
	header_columns_.clear();

	for (int col= 0; col < count; ++col)
	{
		HDITEM hdi;
		memset(&hdi, 0, sizeof hdi);
		hdi.mask = HDI_WIDTH | HDI_FORMAT | HDI_ORDER;

		header_wnd_.GetItem(col, &hdi);

		int format= DT_LEFT;
		if ((hdi.fmt & HDF_JUSTIFYMASK) == HDF_RIGHT)
			format = DT_RIGHT;
		else if ((hdi.fmt & HDF_JUSTIFYMASK) == HDF_CENTER)
			format = DT_CENTER;

		header_columns_.push_back(ColumnInfo(col, hdi.cxy, hdi.iOrder, format));
	}

	SynchColumnsOrder();
}


// get column width
int PhotoCtrl::GetColumnWidth(int col) const
{
	ASSERT(header_wnd_.m_hWnd != 0);

	HDITEM hdi;
	memset(&hdi, 0, sizeof hdi);
	hdi.mask = HDI_WIDTH;
	hdi.cxy = 0;

	header_wnd_.GetItem(col, &hdi);

	return hdi.cxy;

//	header_wnd_.GetIt.GetOrderArray(&col_widths.front(), col_widths.size());
//	col_widths
}


void PhotoCtrl::OnHeaderItemChanging(NMHDR* nm_hdr, LRESULT* result)
{
	NMHEADER* nm_header= reinterpret_cast<NMHEADER*>(nm_hdr);
	*result = 0;

	if (nm_header->pitem)
	{
		if (nm_header->pitem->mask & HDI_WIDTH)
		{
			// new column width
			if (nm_header->pitem->cxy > 2000 || nm_header->pitem->cxy < 8)
			{
				// preventing from being too wide or too narrow
				*result = 1;
			}
			else
			{
				ASSERT(nm_header->iItem >= 0 && nm_header->iItem < header_columns_.size());
				header_columns_[nm_header->iItem].width = nm_header->pitem->cxy;
				ResetHorzScrollBar();
				Invalidate();
			}
		}
	}
}


struct AccColumnWidth
{
	int operator () (int width, const PhotoCtrl::ColumnInfo& col) const
	{
		return width + col.width;
	}
};

int PhotoCtrl::ColumnsTotalWidth()
{
	int width= 0;
	return accumulate(header_columns_.begin(), header_columns_.end(), width, AccColumnWidth());
}


void PhotoCtrl::ResetHorzScrollBar()
{
	int width= ColumnsTotalWidth();
	static bool update= false;
	if (!update)
	{
		Block update(update);

		int total_columns_width= width;
		if (mode_ == DETAILS)
			item_size_.cx = width;
		int x= scroll_bar_.GetScrollPos(SB_HORZ);
		SetLocation(item_size_);

//		int x= scroll_bar_.GetScrollPos(SB_HORZ);

		CRect rect;
		GetClientRect(rect);

		SCROLLINFO si;
		si.cbSize = sizeof si;
		si.nPage = rect.Width();
		si.nMin = 0;
		si.nMax = total_columns_width;
		si.fMask = SIF_RANGE | SIF_PAGE;

		if (page_size_.cx != si.nPage || max_size_.cx != si.nMax)
		{
			page_size_.cx = si.nPage;
			max_size_.cx = si.nMax;
			scroll_bar_.SetScrollInfo(SB_HORZ, si, true);
		}

		// reset horizontal position if necessary
		int X_max= scroll_bar_.GetScrollLimit(SB_HORZ);
		if (x > X_max)
		{
			ScrollWindow(x - X_max, 0, GetFieldRect(false));
			SetHeader();
			UpdateWindow();
			::Sleep(10);
		}

//		update = false;
	}
}


CRect PhotoCtrl::GetFirstColImgRect(const CRect& text_rect)
{
	CRect rect= text_rect;
	rect.DeflateRect(1, 1);
	rect.right = rect.left + 3 * rect.Height() / 2;
//	if (rect.right > text_rect.right)
//		rect.right = text_rect.right;
	return rect;
}


void PhotoCtrl::OnHeaderItemDblClick(NMHDR* nm_hdr, LRESULT* result)
{
	NMHEADER* nm_header= reinterpret_cast<NMHEADER*>(nm_hdr);
	*result = 0;

	// find optimal column width

	const size_t count= header_columns_.size();
	int col_index= nm_header->iItem;

	if (col_index < 0 || col_index >= count || host_ == 0)
		return;

	int max_width= 0;
	int extra= 2 * ITEM_X_MARGIN;

//	col_index = header_columns_[col_index].order;

	if (col_index == 0)		// account for an image in first column
	{
		extra += margins_rect_.left;
		extra += GetFirstColImgRect(CRect(CPoint(0, 0),  item_size_)).right;
	}

	CClientDC dc(this);
	dc.SelectObject(&default_fnt_);

	String buffer;

	for (ItemList::const_iterator it= items_list_.Begin(); it != items_list_.End(); ++it)
	{
		buffer.clear();
		host_->GetItemText(it->photo_, header_columns_[col_index].sub_item, buffer);

		if (!buffer.empty())
		{
			int width= dc.GetTextExtent(buffer.c_str(), static_cast<int>(buffer.length())).cx;
			if (width > max_width && width < 2000)	// limit to 2000 pixels for safety
				max_width = width;
		}
	}

	SetColumnWidth(nm_header->iItem, max_width + extra);
}


void PhotoCtrl::OnHeaderItemClick(NMHDR* nm_hdr, LRESULT* result)
{
	NMHEADER* nm_header= reinterpret_cast<NMHEADER*>(nm_hdr);
	*result = 0;

	if (host_)
		host_->SortByColumn(nm_header->iItem, ::GetKeyState(VK_SHIFT) < 0);
}


void PhotoCtrl::OnHeaderItemEndDrag(NMHDR* nm_hdr, LRESULT* result)
{
	NMHEADER* nm_header= reinterpret_cast<NMHEADER*>(nm_hdr);
	*result = 0;
	// postpone reordering; control has to be returned from this notification handler
	// for order array to be updated inside header ctrl
	PostMessage(WM_USER);
}

LRESULT PhotoCtrl::OnHeaderItemEndDrag2(WPARAM, LPARAM)
{
	SynchColumnsOrder();
	Invalidate();
	return 0;
}


void PhotoCtrl::OnHeaderRClick(NMHDR* nm_hdr, LRESULT* result)
{
	*result = 1;
	if (host_)
		host_->ColumnRClick(-1);
}


///////////////////////////////////////////////////////////////////////////////


void PhotoCtrl::OnSetFocus(CWnd* old_wnd)
{
	if (current_item_ && item_size_.cx > 0 && item_size_.cy > 0)
		InvalidateRect(current_item_.GetItemRect(true) - GetScrollOffset(), false);
}

void PhotoCtrl::OnKillFocus(CWnd* new_wnd)
{
	if (current_item_ && item_size_.cx > 0 && item_size_.cy > 0)
		InvalidateRect(current_item_.GetItemRect(true) - GetScrollOffset(), false);
}


void PhotoCtrl::SetSortColumn(int col)
{
	if (sort_by_column_ != col)
	{
		sort_by_column_ = col;
		SetSortingIndicator(sort_by_column_);

		if (mode_ == DETAILS)
			Invalidate();		// sorting column has different background--it has to be repainted
	}
}


#ifndef HDF_SORTUP
 #define HDF_SORTUP     0x0400
 #define HDF_SORTDOWN   0x0200
#endif

// display triangle bitmap in column providing sorting order
//
void PhotoCtrl::SetSortingIndicator(int column_sort)
{
	if (WhistlerLook::IsAvailable())
	{
		int count= header_wnd_.GetItemCount();

		for (int i= 0; i < count; ++i)
		{
			HDITEM hdi;
			hdi.mask = HDI_FORMAT;
			header_wnd_.GetItem(i, &hdi);
			if (hdi.fmt & (HDF_SORTUP | HDF_SORTDOWN))
			{
				hdi.fmt &= ~(HDF_SORTUP | HDF_SORTDOWN);
				header_wnd_.SetItem(i, &hdi);
			}
		}

		if (column_sort != 0)
		{
			int col= abs(column_sort) - 1;
			HDITEM hdi;
			hdi.mask = HDI_FORMAT;
			header_wnd_.GetItem(col, &hdi);
			hdi.fmt |= column_sort > 0 ? HDF_SORTUP : HDF_SORTDOWN;
			header_wnd_.SetItem(col, &hdi);
		}
	}
	else
	{
		int count= header_wnd_.GetItemCount();

		for (int i= 0; i < count; ++i)
		{
			HDITEM hdi;
			hdi.mask = HDI_FORMAT | HDI_BITMAP;
			header_wnd_.GetItem(i, &hdi);
			if (hdi.fmt & HDF_BITMAP)
			{
				hdi.fmt &= ~(HDF_BITMAP | HDF_BITMAP_ON_RIGHT);
				header_wnd_.SetItem(i, &hdi);
			}
		}

		if (column_sort)
		{
			int col= abs(column_sort) - 1;
			HDITEM hdi;
			hdi.mask = HDI_FORMAT | HDI_BITMAP;
			header_wnd_.GetItem(col, &hdi);
			hdi.hbm = column_sort > 0 ? ascending_bmp_ : descending_bmp_;
			hdi.fmt |= HDF_BITMAP;
			if (!(hdi.fmt & HDF_RIGHT))
				hdi.fmt |= HDF_BITMAP_ON_RIGHT;
			header_wnd_.SetItem(col, &hdi);
		}
	}
}


///////////////////////////////////////////////////////////////////////////////

// set all colors
void PhotoCtrl::SetColors(const std::vector<COLORREF>& colors)
{
	if (colors.size() < C_MAX_COLORS)
	{
		ASSERT(false);
		return;
	}

	rgb_bk_color_				= colors[C_BACKGND];
	rgb_text_color_				= colors[C_TEXT];
	rgb_sel_color_				= colors[C_SELECTION];
	rgb_sel_text_color_			= colors[C_SEL_TEXT];
								// C_DISABLED_TEXT not used here
	rgb_tag_bkgnd_				= colors[C_TAG_BACKGND];
	rgb_tag_text_				= colors[C_TAG_TEXT];
	rgb_sort_color_				= colors[C_SORT];
	rgb_separator_				= colors[C_SEPARATOR];
	rgb_tile_dim_text_color_	= colors[C_DIM_TEXT];
	rgb_active_bg_				= colors[C_ACTIVEBG];

	//scroll_bar_.SetColors(colors[C_BACKGND], colors[C_DIM_TEXT], colors[C_TEXT]);
	scroll_bar_.SetColors(RGB(190,190,190), RGB(150,150,150), RGB(75,75,75));
	
	if (m_hWnd)
		Invalidate();
}

// get all colors
void PhotoCtrl::GetColors(std::vector<COLORREF>& colors) const
{
	colors.resize(C_MAX_COLORS);

	colors[C_BACKGND]		= rgb_bk_color_;
	colors[C_TEXT]			= rgb_text_color_;
	colors[C_SELECTION]		= rgb_sel_color_;
	colors[C_SEL_TEXT]		= rgb_sel_text_color_;
	// C_DISABLED_TEXT
	colors[C_TAG_BACKGND]	= rgb_tag_bkgnd_;
	colors[C_TAG_TEXT]		= rgb_tag_text_;
	colors[C_SORT]			= rgb_sort_color_;
	colors[C_SEPARATOR]		= rgb_separator_;
	colors[C_DIM_TEXT]		= rgb_tile_dim_text_color_;
	colors[C_ACTIVEBG]		= rgb_active_bg_;
}

// reset colors based on system colors and hardcoded values
void PhotoCtrl::ResetColors()
{
	//rgb_bk_color_		= RGB(230, 230, 230);//::GetSysColor(COLOR_WINDOW);
	rgb_sel_color_		= RGB(145, 201, 247);//RGB(247, 123, 0);//::GetSysColor(COLOR_HIGHLIGHT);
	//rgb_text_color_		= RGB(20, 20, 20);//::GetSysColor(COLOR_WINDOWTEXT);
	//rgb_sel_text_color_	= RGB(255, 255, 255);//::GetSysColor(COLOR_HIGHLIGHTTEXT);
	//rgb_separator_		= rgb_sel_color_;//::GetSysColor(COLOR_HIGHLIGHT);
	rgb_tag_bkgnd_		= rgb_sel_color_;//RGB(247, 123, 0);
	rgb_tag_text_		= RGB(255, 255, 255);
	//rgb_tile_dim_text_color_ = CalcNewColor(rgb_text_color_, 50.0f);
	//rgb_sort_color_ = RGB(230, 230, 230);

	//{	// calculate sorting indicator color
	//	COLORREF rgb_wnd= ::GetSysColor(COLOR_WINDOW);
	//	int r= GetRValue(rgb_wnd);
	//	int g= GetGValue(rgb_wnd);
	//	int b= GetBValue(rgb_wnd);
	//	if (r > 0xf0 && g > 0xf0 && b > 0xf0)
	//		rgb_sort_color_ = RGB(r - 8, g - 8, b - 8);
	//	else
	//		rgb_sort_color_ = RGB(std::min(r + 8, 0xff), std::min(g + 8, 0xff), std::min(b + 8, 0xff));
	//}

	if (m_hWnd)
		Invalidate();
}


void PhotoCtrl::OnChar(UINT chr, UINT rep_cnt, UINT flags)
{
	if (host_ != 0)
		host_->OnChar(chr, rep_cnt, flags);
//	CWnd::OnChar(chr, rep_cnt, flags);
}


void PhotoCtrl::SetHalftoneDrawing(bool on)
{
	halftone_drawing_ = on;
	if (m_hWnd)
		Invalidate();
}


bool PhotoCtrl::ShowPhotoMarker(Item* item)
{
	if (item && item->photo_ && host_)
		return host_->ShowPhotoMarker(item->photo_->GetFileTypeIndex());

	return false;
}


bool PhotoCtrl::ShowNoExifIndicator(Item* item)
{
	if (item && item->photo_ && host_)
		return host_->ShowNoExifIndicator(item->photo_->GetFileTypeIndex());

	return false;
}


bool PhotoCtrl::SelectItems(const VectPhotoInfo& photos, bool notify/*= true*/)
{
	bool change= SelectItems(NONE, false);

	const size_t count= photos.size();

	for (size_t i= 0; i < count; ++i)
	{
		Item* item= items_list_.FastFind(photos[i]);

		if (item == 0)
		{
			ASSERT(false);
			continue;
		}

		if (!item->IsSelected())
		{
			change = true;
			item->SetSelection(true);
		}
	}

	if (change && m_hWnd)
	{
		//TODO: optimize?
		Invalidate();
	}

	if (notify)
		SelectionChanged();

	return change;
}


void PhotoCtrl::ShowItemLabel(bool show)
{
	show_label_ = show;

	if (mode_ == THUMBNAILS || mode_ == PREVIEWS)
	{
		Resize();
		Invalidate();
	}
}


void PhotoCtrl::ShowTagText(bool show)
{
	if (show_tags_ == show)
		return;

	show_tags_ = show;

	if (mode_ != DETAILS)		// in details mode there are no tags displayed
		Invalidate();
}

void PhotoCtrl::ShowMarker(bool show)
{
	if (show_marker_ == show)
		return;

	show_marker_ = show;

	if (mode_ != DETAILS)		// in details mode there are no marker info displayed
		Invalidate();
}

///////////////////////////////////////////////////////////////////////////////
//
// Tool tip support methods


INT_PTR PhotoCtrl::OnToolHitTest(CPoint pos, TOOLINFO* ti) const
{
	if (ti == 0)
		return -1;

	CPoint offset= GetScrollOffset();

	if (Item* item= const_cast<PhotoCtrl*>(this)->FindItem(pos + offset))
	{
		ti->hwnd = m_hWnd;
		ti->uId = UINT_PTR(GetRawPointer(item->photo_));
		ti->uFlags = 0;
		ti->lpszText = LPSTR_TEXTCALLBACK;
		CRect rect= const_cast<PhotoCtrl*>(this)->GetItemRect(item->photo_, false);
		rect.OffsetRect(-offset.x, -offset.y);
		ti->rect = rect;
		return ti->uId;
	}

	return -1;
}


BOOL PhotoCtrl::OnToolTipNotify(UINT id, NMHDR* nmhdr, LRESULT* result)
{
	*result = 0;
	TOOLTIPTEXT* ttt= reinterpret_cast<TOOLTIPTEXT*>(nmhdr);

	if (Item* item= items_list_.FastFind(PhotoInfoPtr(reinterpret_cast<PhotoInfo*>(nmhdr->idFrom))))
	{
		static String str;

		if (host_)
			str = host_->GetToolTipText(item->photo_);

		ttt->lpszText = const_cast<TCHAR*>(str.c_str());
		return true;
	}

	return false;
}


BOOL PhotoCtrl::PreTranslateMessage(MSG* msg)
{
	if (ballon_info_enabled_)
		FilterToolTipMessage(msg);

	return CWnd::PreTranslateMessage(msg);
}


// verbatim copy from MFC
static void AFXAPI _AfxRelayToolTipMessage(CToolTipCtrl* tool_tip, MSG* m)
{
	// transate the message based on TTM_WINDOWFROMPOINT
	MSG msg= *m;
	msg.hwnd = (HWND)tool_tip->SendMessage(TTM_WINDOWFROMPOINT, 0, (LPARAM)&msg.pt);
	CPoint pt = m->pt;
	if (msg.message >= WM_MOUSEFIRST && msg.message <= WM_MOUSELAST)
		::ScreenToClient(msg.hwnd, &pt);
	msg.lParam = MAKELONG(pt.x, pt.y);

//TRACE(L"relaying %d %d\n", pt.x, pt.y);

	// relay mouse event before deleting old tool
	tool_tip->SendMessage(TTM_RELAYEVENT, 0, (LPARAM)&msg);
}


// verbatim copy from MFC + little changes to use balloon tooltip
void PhotoCtrl::FilterToolTipMessage(MSG* msg)
{
	// this CWnd has tooltips enabled
	UINT message = msg->message;

	if ((message == WM_MOUSEMOVE || message == WM_NCMOUSEMOVE ||
		 message == WM_LBUTTONUP || message == WM_RBUTTONUP ||
		 message == WM_MBUTTONUP) &&
		(GetKeyState(VK_LBUTTON) >= 0 && GetKeyState(VK_RBUTTON) >= 0 &&
		 GetKeyState(VK_MBUTTON) >= 0))
	{
		// make sure that tooltips are not already being handled
		CWnd* wnd = CWnd::FromHandle(msg->hwnd);
		while (wnd != NULL && !(wnd->m_nFlags & (WF_TOOLTIPS|WF_TRACKINGTOOLTIPS)) && wnd != this)
		{
			wnd = wnd->GetParent();
		}
		if (wnd != this)
		{
			if (wnd == NULL)
			{
				// tooltips not enabled on this CWnd, clear last state data
				//_AFX_THREAD_STATE* thread_state = _afxThreadState.GetData();
				last_hit_ = NULL;
				last_hit_code_ = -1;
			}
			return;
		}

		//_AFX_THREAD_STATE* thread_state = _afxThreadState.GetData();
		CToolTipCtrl* tool_tip = &tool_tip_wnd_; //thread_state->tool_tip_;
/*		CWnd* owner = GetParentOwner();
		if (tool_tip != NULL && tool_tip->GetOwner() != owner)
		{
			tool_tip->DestroyWindow();
			delete tool_tip;
			thread_state->tool_tip_ = NULL;
			tool_tip = NULL;
		}
		if (tool_tip == NULL)
		{
			tool_tip = new CToolTipCtrl;
			if (!tool_tip->Create(owner, TTS_ALWAYSTIP))
			{
				delete tool_tip;
				return;
			}
			tool_tip->SendMessage(TTM_ACTIVATE, FALSE);
			thread_state->tool_tip_ = tool_tip;
		} */

		ASSERT_VALID(tool_tip);
		ASSERT(::IsWindow(tool_tip->m_hWnd));

/*		typedef struct tagAFX_OLDTOOLINFO
		{
			UINT cbSize;
			UINT flags;
			HWND hwnd;
			UINT uId;
			RECT rect;
			HINSTANCE hinst;
			LPTSTR lpszText;
		} AFX_OLDTOOLINFO; */
		typedef TOOLINFO AFX_OLDTOOLINFO;

		// add a "dead-area" tool for areas between toolbar buttons
		TOOLINFO ti; memset(&ti, 0, sizeof ti);
		ti.cbSize = sizeof ti;
		ti.uFlags = TTF_IDISHWND;
		ti.hwnd = m_hWnd;
		ti.uId = UINT_PTR(m_hWnd);
		ti.lpszText = 0;
		ti.hinst = NULL;
		ti.lParam = 0;
		if (!tool_tip->SendMessage(TTM_GETTOOLINFO, 0, (LPARAM)&ti))
		{
			ASSERT(ti.uFlags == TTF_IDISHWND);
			ASSERT(ti.hwnd == m_hWnd);
			ASSERT(ti.uId == (UINT_PTR)m_hWnd);
			VERIFY(tool_tip->SendMessage(TTM_ADDTOOL, 0, (LPARAM)&ti));
		}

		// determine which tool was hit
		CPoint point = msg->pt;
		::ScreenToClient(m_hWnd, &point);
		TOOLINFO tiHit; memset(&tiHit, 0, sizeof(TOOLINFO));
		tiHit.cbSize = sizeof(AFX_OLDTOOLINFO);
		INT_PTR hit = OnToolHitTest(point, &tiHit);

		// build new toolinfo and if different than current, register it
		CWnd* hit_wnd = hit == -1 ? NULL : this;
		if (last_hit_code_ != hit || last_hit_ != hit_wnd)
		{
			if (hit != -1)
			{
//CPoint pt; ::GetCursorPos(&pt); ScreenToClient(&pt);
//TRACE(L"add tool %x %d %d %d %d  %d %d\n", tiHit.uId, tiHit.rect.left,tiHit.rect.right,tiHit.rect.top,tiHit.rect.bottom,point.x,point.y);
				// add new tool and activate the tip
				ti = tiHit;
				ti.uFlags &= ~(TTF_NOTBUTTON|TTF_ALWAYSTIP);
				if (m_nFlags & WF_TRACKINGTOOLTIPS)
					ti.uFlags |= TTF_TRACK;
//				VERIFY(tool_tip->SendMessage(TTM_ADDTOOL, 0, (LPARAM)&ti));
				tool_tip->AddTool(this, LPSTR_TEXTCALLBACK, &ti.rect, ti.uId);
				if ((tiHit.uFlags & TTF_ALWAYSTIP) || IsTopParentActive())
				{
					// allow the tooltip to popup when it should
					tool_tip->SendMessage(TTM_ACTIVATE, TRUE);
					if (m_nFlags & WF_TRACKINGTOOLTIPS)
						tool_tip->SendMessage(TTM_TRACKACTIVATE, TRUE, (LPARAM)&ti);

					// bring the tooltip window above other popup windows
					::SetWindowPos(tool_tip->m_hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOMOVE|SWP_NOOWNERZORDER);
				}
			}
			else
			{
				tool_tip->SendMessage(TTM_ACTIVATE, FALSE);
			}

			// relay mouse event before deleting old tool
			_AfxRelayToolTipMessage(tool_tip, msg);

			// now safe to delete the old tool
			if (last_info_.cbSize >= sizeof(AFX_OLDTOOLINFO) && last_info_.uId)
			{
//				TRACE(L"del tool %x\n", last_info_.uId);
				tool_tip->SendMessage(TTM_DELTOOL, 0, (LPARAM)&last_info_);
			}
			last_hit_ = hit_wnd;
			last_hit_code_ = hit;
			last_info_ = tiHit;
		}
		else
		{
			if (m_nFlags & WF_TRACKINGTOOLTIPS)
			{
				POINT pt;

				::GetCursorPos( &pt );
				tool_tip->SendMessage(TTM_TRACKPOSITION, 0, MAKELPARAM(pt.x, pt.y));
			}
			else
			{
				// relay mouse events through the tooltip
				if (hit != -1)
					_AfxRelayToolTipMessage(tool_tip, msg);
			}
		}

		if ((tiHit.lpszText != LPSTR_TEXTCALLBACK) && (tiHit.hinst == 0))
			free(tiHit.lpszText);
	}
	else //MiK: my changes here to cancel tooltip /if (m_nFlags & (WF_TOOLTIPS|WF_TRACKINGTOOLTIPS))
	{
		// make sure that tooltips are not already being handled
		CWnd* wnd = CWnd::FromHandle(msg->hwnd);
		while (wnd != NULL && wnd != this && !(wnd->m_nFlags & (WF_TOOLTIPS|WF_TRACKINGTOOLTIPS)))
			wnd = wnd->GetParent();
		if (wnd != this)
			return;

#define WM_SYSKEYFIRST  WM_SYSKEYDOWN
#define WM_SYSKEYLAST   WM_SYSDEADCHAR

		BOOL keys = (message >= WM_KEYFIRST && message <= WM_KEYLAST) ||
			(message >= WM_SYSKEYFIRST && message <= WM_SYSKEYLAST);
		if ((m_nFlags & WF_TRACKINGTOOLTIPS) == 0 &&
			(keys ||
			 (message == WM_LBUTTONDOWN || message == WM_LBUTTONDBLCLK) ||
			 (message == WM_RBUTTONDOWN || message == WM_RBUTTONDBLCLK) ||
			 (message == WM_MBUTTONDOWN || message == WM_MBUTTONDBLCLK) ||
			 (message == WM_NCLBUTTONDOWN || message == WM_NCLBUTTONDBLCLK) ||
			 (message == WM_NCRBUTTONDOWN || message == WM_NCRBUTTONDBLCLK) ||
			 (message == WM_NCMBUTTONDOWN || message == WM_NCMBUTTONDBLCLK)))
		{
			//CWnd::CancelToolTips(keys);
			tool_tip_wnd_.Activate(false);
		}

	}
#undef WM_SYSKEYFIRST
#undef WM_SYSKEYLAST
}


void PhotoCtrl::RemoveToolTips()	// remove (invalidate) tooltips after scrolling
{
	if (tool_tip_wnd_.m_hWnd == 0)
		return;

	if (last_info_.cbSize >= 0)
		tool_tip_wnd_.SendMessage(TTM_DELTOOL, 0, reinterpret_cast<LPARAM>(&last_info_));

	last_info_.cbSize = 0;
}


void PhotoCtrl::EnableBalloonInfo(bool enable)
{
	if (ballon_info_enabled_ == enable)
		return;

	if (!enable && tool_tip_wnd_.m_hWnd)
	{
		RemoveToolTips();
		tool_tip_wnd_.SendMessage(TTM_ACTIVATE, false);
	}

	ballon_info_enabled_ = enable;
}


void PhotoCtrl::SetItemsAcross(int items)
{
	if (items < 1 || items > 100)
	{
		ASSERT(false);
		return;
	}

	if (items_across_ == items)
		return;

	items_across_ = items;

	if (mode_ == PREVIEWS)	// reset image size
		SetMode(mode_);
}


bool PhotoCtrl::IsItemVisible(PhotoInfoPtr photo) const
{
	CRect rect= GetFieldRect();
	rect &= GetItemRect(photo, true);
	return !rect.IsRectEmpty();
}


void PhotoCtrl::RedrawItem(PhotoInfoPtr photo)
{
	//TODO: invalidate/redraw background? currently not needed...

	if (ItemGroupPair igp= FindItem(photo))
	{
		CClientDC dc(this);
		PrepareDC(dc);
		igp.Group()->DrawSelection(*this, dc, igp.Item());
	}
	else
	{ ASSERT(false); } // bogus photo?
	//	InvalidateRect(GetItemRect(photo, true) - GetScrollOffset());
}


PhotoInfoPtr PhotoCtrl::FindVisibleItem(bool first)
{
	if (horz_items_ < 1)
		return 0;

	CRect rect= GetFieldRect();

	if (rect.IsRectEmpty())
		return 0;

	Group* group= first ? groups_.FindFirstGroupInRect(rect) : groups_.FindLastGroupInRect(rect);

	if (group == 0)
		return 0;

	Item* item= first ? group->FindFirstItemInRect(rect, 0, false) : group->FindLastItemInRect(rect, horz_items_ - 1, false);

	if (item == 0)
		return 0;

	return item->photo_;
}


PhotoInfoPtr PhotoCtrl::GetFirstItem(int groupId)
{
	if (Group* group= groups_.Find(groupId))
		if (Item* item= group->GetItem(0))
			return item->photo_;

	return 0;
}


PhotoInfoPtr PhotoCtrl::GetItem(ItemGet get, PhotoInfoPtr photo/*= 0*/)
{
	switch (get)
	{
	case FIRST_VISIBLE:
		return FindVisibleItem(true);

	case LAST_VISIBLE:
		return FindVisibleItem(false);

	case NEXT_ITEM:
		if (ItemGroupPair igp= FindItem(photo))
			if (ItemGroupPair next= FindNextItem(igp, GO_RIGHT))
				return next.Item()->photo_;
		break;

	case PREV_ITEM:
		if (ItemGroupPair igp= FindItem(photo))
			if (ItemGroupPair prev= FindNextItem(igp, GO_LEFT))
				return prev.Item()->photo_;
		break;

	default:
		ASSERT(false);
		break;
	}

	return 0;
}


int PhotoCtrl::GetLabelHeight() const
{
	return label_rect_.Height();
}


// current img size--for preview mode; thumbnail size otherwise
CSize PhotoCtrl::GetImageSize(PhotoInfoPtr photo) const
{
	if (mode_ == PREVIEWS)
	{
		int label_height= GetLabelHeight();
		CSize img= image_rect_.Size();
		if (show_label_)
			img.cy -= label_height;
		return img;
	}
	else
	{
		if (photo == 0)
			return img_size_;

		CRect rect= GetItemRect(photo, true); //(CPoint(0, 0), img_size_);

		if (!image_rect_.IsRectEmpty())
		{
			ASSERT(mode_ != DETAILS);
			CRect rect_img= GetImageRect(rect, photo, false);
			UINT flags= PhotoInfo::DRAW_SHADOW;
			CSize size= ImageDraw::GetImageSize(0, rect_img, flags).Size();
			return size;
		}
		else
		{
			if (mode_ == DETAILS)
			{
				CRect rect_img= GetFirstColImgRect(rect);
				return rect_img.Size();
			}
		}
		return img_size_;
	}
}


CRect PhotoCtrl::GetImageRect(const CRect& item_rect, PhotoInfoPtr photo, bool bmp_outline) const
{
	if (host_ == 0)
		return item_rect;

	if (!image_rect_.IsRectEmpty())
	{
		ASSERT(mode_ != DETAILS);

		CRect img_rect;
		if (mode_ == THUMBNAILS || mode_ == PREVIEWS)
		{
			img_rect = item_rect;
			img_rect.DeflateRect(OUTLINE, OUTLINE);
			img_rect.bottom -= GetLabelHeight();
		}
		else
			img_rect = image_rect_ + item_rect.TopLeft();

		if (mode_ == TILES)
		{
			//TODO
			// add text area to the left of img
			//...

			// for the time being:
			if (bmp_outline)
				return item_rect;
		}
		//else if (mode_ == MOSAIC)
		//	return item_rect;

		UINT flags= PhotoInfo::DRAW_SHADOW;
		CRect photo_rect= img_rect;

		if (bmp_outline && photo != 0)
		{
			bool get_thumbnail= true;

			if (mode_ == PREVIEWS)
			{
				if (Dib* bmp= host_->RequestImage(photo, photo_rect.Size(), true))
				{
					get_thumbnail = false;
					img_rect = ImageDraw::GetImageSize(bmp, img_rect, flags);
				}
			}

			if (get_thumbnail)
			{
				CSize dest_size= ImageDraw::GetImageSize(0, photo_rect, flags).Size();
				//Dib* bmp= photo->GetThumbnail(dest_size);
				if (Dib* bmp= photo->IsThumbnailAvailable(dest_size))
					img_rect = ImageDraw::GetImageSize(bmp, img_rect, flags);
			}
		}

		return img_rect;
	}

	// detailed mode
	return item_rect;
}


void PhotoCtrl::EnableUpdate(bool enable)
{
	enable_updating_ = enable;

	if (enable)
	{
		Invalidate();
		Resize();
	}
}


bool PhotoCtrl::HasGroup(int group_id) const
{
	return const_cast<PhotoCtrl*>(this)->groups_.Find(group_id) != 0;
}


bool PhotoCtrl::HasItem(PhotoInfoPtr photo) const
{
	const Item* item= items_list_.FastFind(photo);
	return item != 0;
}


void PhotoCtrl::EnableSettingCurrentItem(bool enable)
{
	if (enable)
		++enable_set_current_counter_;
	else
		--enable_set_current_counter_;
}


void PhotoCtrl::OnDestroy()
{
	scroll_bar_.Delete();
}

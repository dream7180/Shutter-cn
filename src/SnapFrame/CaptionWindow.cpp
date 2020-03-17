/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// CaptionWindow.cpp : implementation file
//

#include "stdafx.h"
#include "CaptionWindow.h"
#include "SnapView.h"
#include "SnapFrame.h"
#include "../Resource.h"
#include "../MemoryDC.h"
#include "../PNGImage.h"
#include "../ToolBarPopup.h"
#include "../WhistlerLook.h"
#include "../Config.h"
#include "../UIElements.h"
#include "../ColorConfiguration.h"
#include "../Color.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CaptionWindow

namespace {
	//const int TBAR_WIDTH= 46;		// toolbar width (including right margin)
	//const int TBAR_WIDTH= 52;	// toolbar width (including right margin)
	const int CHEVRON= 20;			// chevron toolbar width
	//const int CHEVRONBIG= 20;		// chevron toolbar width
	const int CHEVRONSPACE= CHEVRON + 12;
	//const int CHEVRONBIGSPACE= CHEVRONBIG + 12;
	//const TCHAR* REG_KEY= _T("Caption");
	//const TCHAR* REG_ENTRY= _T("Large");
}

//CImageList CaptionWindow::img_list_tab_;
//CImageList CaptionWindow::img_list_tab_end_;
//CImageList CaptionWindow::img_list_tab_big_;
//CImageList CaptionWindow::img_list_tab_end_big_;
//int CaptionWindow::tab_part_width_= 0;
//int CaptionWindow::tab_end_width_= 0;
//int CaptionWindow::min_caption_width_= 0;
//int CaptionWindow::tab_part_width_big_= 0;
//int CaptionWindow::tab_end_width_big_= 0;
//int CaptionWindow::min_caption_width_big_= 0;
//const int CaptionWindow::tab_part_divider_= 6;
int CaptionWindow::initialized_= 0;
//bool CaptionWindow::big_= true;
//COLORREF CaptionWindow::static_active_caption_color= 0;
//COLORREF CaptionWindow::static_inactive_caption_color= 0;
const int LEFT_MARGIN= Pixels(4);	// space before caption's text

/*
int CaptionWindow::CreateImgList(CImageList& img_list_tab, int tab_bmp_id, int shadow_bmp_id, int parts, COLORREF active_color, COLORREF inactive_color)
{
	CBitmap tab_shadow_bmp;
	if (!PNGImage().Load(shadow_bmp_id, tab_shadow_bmp))
		return 0;

	CBitmap tab_shape_bmp;
	if (!PNGImage().Load(tab_bmp_id, tab_shape_bmp))
		return 0;

	CBitmap tab_mask_bmp;
	if (!WhistlerLook::IsAvailable())
		if (!PNGImage().Load(tab_bmp_id, tab_mask_bmp))
			return 0;

	BITMAP bm;
	tab_shape_bmp.GetBitmap(&bm);

	ASSERT(bm.bmBitsPixel == 32); // RGBA expected even if no transparency is used

	// width must be multiple of 4 (line alignment)
	ASSERT((bm.bmWidth & 3) == 0);

//	COLORREF rgb_color1= ::GetSysColor(COLOR_ACTIVECAPTION);
//	COLORREF rgb_color2= ::GetSysColor(COLOR_INACTIVECAPTION);

	// prepare color table
	RGBQUAD* base= (RGBQUAD*)bm.bmBits;
	RGBQUAD* colors= base;
	RGBQUAD* end= colors + bm.bmWidth * bm.bmHeight;

	int b= GetBValue(active_color);
	int r= GetRValue(active_color);
	int g= GetGValue(active_color);

	for ( ; colors < end; ++colors)
	{
		colors->rgbBlue = b;
		colors->rgbGreen = g;
		colors->rgbRed = r;
	}

	int tab_part_width= bm.bmWidth / parts;
	UINT flags= WhistlerLook::IsAvailable() ? ILC_COLOR32 : ILC_COLOR24 | ILC_MASK;
	HIMAGELIST img_list= ImageList_Create(tab_part_width, bm.bmHeight, flags, 3 * parts, 0);
	ASSERT(img_list != 0);

	if (img_list == 0)
		return 0;

	img_list_tab.DeleteImageList();
	img_list_tab.Attach(img_list);
	img_list_tab.Add(&tab_shape_bmp, WhistlerLook::IsAvailable() ? static_cast<CBitmap*>(0) : &tab_mask_bmp);

	b = GetBValue(inactive_color);
	r = GetRValue(inactive_color);
	g = GetGValue(inactive_color);

	for (colors = base; colors < end; ++colors)
	{
		colors->rgbBlue = b;
		colors->rgbGreen = g;
		colors->rgbRed = r;
	}

	img_list_tab.Add(&tab_shape_bmp, WhistlerLook::IsAvailable() ? static_cast<CBitmap*>(0) : &tab_mask_bmp);
	img_list_tab.Add(&tab_shadow_bmp, static_cast<CBitmap*>(0));

	return tab_part_width;
} */


//void CaptionWindow::InitializeImageLists(COLORREF active_color, COLORREF inactive_color)
//{
//	tab_part_width_ = CreateImgList(img_list_tab_, IDR_TAB_SHAPE, IDB_TAB_CAPTION, tab_part_divider_, active_color, inactive_color);
//	min_caption_width_ = tab_part_width_ * tab_part_divider_;
//	tab_end_width_ = CreateImgList(img_list_tab_end_, IDR_TAB_SHAPE_END, IDB_TAB_CAPTION_END, 1, active_color, inactive_color);
//	min_caption_width_ += tab_end_width_;
//
//	tab_part_width_big_ = CreateImgList(img_list_tab_big_, IDR_TAB_SHAPE_BIG, IDB_TAB_CAPTION_BIG, tab_part_divider_, active_color, inactive_color);
//	min_caption_width_big_ = tab_part_width_big_ * tab_part_divider_;
//	tab_end_width_big_ = CreateImgList(img_list_tab_end_big_, IDR_TAB_SHAPE_END_BIG, IDB_TAB_CAPTION_END_BIG, 1, active_color, inactive_color);
//	min_caption_width_big_ += tab_end_width_big_;
//}


//void CaptionWindow::ReinitializeImageLists(COLORREF active_color, COLORREF inactive_color)
//{
//	if (static_active_caption_color == active_color && static_inactive_caption_color == inactive_color)
//		return;
//
//	static_active_caption_color = active_color;
//	static_inactive_caption_color = inactive_color;
//
////	CaptionWindow::InitializeImageLists(active_color, inactive_color);
//}



CaptionWindow::CaptionWindow()
{
	active_ = false;
	dragging_ = false;
	block_dragging_ = false;
	trace_rect_.SetRectEmpty();
	frame_ = 0;
	parent_ = 0;
	start_ = CPoint(-1, -1);
	hosted_bar_ = 0;
	hosted_bar_owner_ = 0;
	hosted_bar_resizable_ = false;
//	hosted_bar_rect_.SetRectEmpty();
	no_close_btn_ = false;
	no_maximize_btn_ = false;
	show_context_menu_ = true;
//	use_local_colors_ = false;
	faint_bottom_edge_ = false;
	maximized_wnd_toolbar_ = false;

	SetTabColors(g_Settings.pane_caption_colors_);

	if (initialized_ == 0)
	{
		++initialized_;

		//InitializeImageLists(static_active_caption_color, static_inactive_caption_color);

		// create bold font
		//HFONT font= static_cast<HFONT>(::GetStockObject(DEFAULT_GUI_FONT));
		//LOGFONT lf;
		//::GetObject(font, sizeof(lf), &lf);
		//lf.lfWeight = FW_NORMAL;
		//_tcscpy(lf.lfFaceName, _T("Segoe UI Semibold"));
		//title_fnt_.CreateFontIndirect(&lf);

		//big_ = AfxGetApp()->GetProfileInt(REG_KEY, REG_ENTRY, big_) != 0;
	}
}

/*
CaptionWindow::~CaptionWindow()
{
	//if (--initialized_ == 0)
	{
//		AfxGetApp()->WriteProfileInt(REG_KEY, REG_ENTRY, big_);
	}
}
*/

//CImageList& CaptionWindow::GetTabImg()
//{
//	if (use_local_colors_)
//		return tab_imglist_;
//
//	return big_ ? img_list_tab_big_ : img_list_tab_;
//}


//CImageList& CaptionWindow::GetTabEnd()
//{
//	if (use_local_colors_)
//		return tab_end_imglist_;
//
//	return big_ ? img_list_tab_end_big_ : img_list_tab_end_;
//}

//int CaptionWindow::GetTabPartWidth()
//{
//	if (use_local_colors_)
//		return tab_part_width_;
//
//	return big_ ? tab_part_width_big_ : tab_part_width_;
//}


//int CaptionWindow::GetTabEndWidth()
//{
//	if (use_local_colors_)
//		return tab_end_width_;
//
//	return big_ ? tab_end_width_big_ : tab_end_width_;
//}


//int CaptionWindow::GetMinCaptionWidth()
//{
//	if (use_local_colors_)
//		return min_caption_width_;
//
//	return big_ ? min_caption_width_big_ : min_caption_width_;
//}


void CaptionWindow::SetTabColors(const ColorConfiguration& colors)
{
	ASSERT(colors.size() >= SnapFrame::C_MAX_COLORS);

	bar_color_ = colors[SnapFrame::C_BAR].SelectedColor();
	active_caption_color_ = colors[SnapFrame::C_ACTIVE_CAPTION].SelectedColor();
	inactive_caption_color_ = colors[SnapFrame::C_INACTIVE_CAPTION].SelectedColor();
	use_inactive_caption_colors_ = colors[SnapFrame::C_INACTIVE_CAPTION].use_custom;
	active_caption_text_color_ = colors[SnapFrame::C_ACTIVE_TEXT].SelectedColor();
	inactive_caption_text_color_ = colors[SnapFrame::C_INACTIVE_TEXT].SelectedColor();

	//CreateImgList(tab_imglist_, IDR_TAB_SHAPE, IDB_TAB_CAPTION, tab_part_divider_, active_caption_color_, inactive_caption_color_);
	//CreateImgList(tab_end_imglist_, IDR_TAB_SHAPE_END, IDB_TAB_CAPTION_END, 1, active_caption_color_, inactive_caption_color_);

//	use_local_colors_ = true;
	if (m_hWnd)
		Invalidate();
	if (tool_bar_wnd_.m_hWnd)
		tool_bar_wnd_.Invalidate();
	if (chevron_wnd_.m_hWnd)
		chevron_wnd_.Invalidate();
}


BEGIN_MESSAGE_MAP(CaptionWindow, CWnd)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_COMMAND(ID_CHEVRON, OnChevron)
	ON_MESSAGE(WM_PRINT, OnPrint)
//	ON_COMMAND(ID_SMALL_ICONS, OnSmallIcons)
//	ON_COMMAND(ID_LARGE_ICONS, OnLargeIcons)
	ON_WM_INITMENUPOPUP()
	ON_WM_CONTEXTMENU()
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CaptionWindow message handlers

CString CaptionWindow::wnd_class_;
//CFont CaptionWindow::title_fnt_;


bool CaptionWindow::Create(CWnd* parent, const TCHAR* title)
{
	parent_ = dynamic_cast<SnapView*>(parent);

	if (wnd_class_.IsEmpty())
		wnd_class_ = AfxRegisterWndClass(CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS,
			::LoadCursor(NULL, IDC_ARROW));

	if (!CWnd::Create(wnd_class_, title, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN,
		// WS_CLIPCHILDREN causes redrawing to be nasty but it's necessary: otherwise tb can sometimes disappear
		CRect(0, 0, 0, GetHeight()), parent, -1))
		return false;

//	if (parent_)
//	{
	static const int commands[]= { ID_PANE_MAXIMIZE, ID_PANE_RESTORE, ID_PANE_CLOSE };
	tool_bar_wnd_.SetPadding(1, 1);
	tool_bar_wnd_.SetOnIdleUpdateState(false);
	tool_bar_wnd_.Create("ppp", commands, /*big_ ? IDB_PANE_TOOLBAR_BIG : */IDB_PANE_TOOLBAR, 0, this);
	//tool_bar_wnd_.SetHotImageList(big_ ? IDB_PANE_TOOLBAR_BIG_HOT : IDB_PANE_TOOLBAR_HOT);
	tool_bar_wnd_.HideButton(ID_PANE_RESTORE);
	tool_bar_wnd_.SetOwner(parent);
	tool_bar_wnd_.AutoResize();

	return true;
}


void CaptionWindow::OnPaint()
{
	CPaintDC dc(this); // device context for painting
}


//extern COLORREF CalcShade(COLORREF rgb_color, float shade);
/*
{
	shade /= 100.0f;

	int red= GetRValue(rgb_color);
	int green= GetGValue(rgb_color);
	int blue= GetBValue(rgb_color);

	if (shade > 0.0f)	// lighter
	{
		return RGB(red + shade * (0xff - red), green + shade * (0xff - green), blue + shade * (0xff - blue));
	}
	else if (shade < 0.0f)	// darker
	{
		shade = 1.0f + shade;

		return RGB(red * shade, green * shade, blue * shade);
	}

	return rgb_color;
}*/


//void DrawGrip(CDC* dc, const CRect& rect, COLORREF rgb_light, COLORREF rgb_dark)
//{
//	CRect grip= rect;
//	dc->FillSolidRect(grip, rgb_light);
//	grip.OffsetRect(-1, -1);
//	dc->FillSolidRect(grip, rgb_dark);
//}


extern RGBQUAD CalcShade(RGBQUAD rgbqColor, float shade)
{
	shade /= 80.0f;

	int red= rgbqColor.rgbRed; // GetRValue(rgb_color);
	int green= rgbqColor.rgbGreen; //GetGValue(rgb_color);
	int blue= rgbqColor.rgbBlue; // GetBValue(rgb_color);
	RGBQUAD out;

	if (shade > 0.0f)	// lighter
	{
		out.rgbRed = std::min(255, int(red + shade * (0xff - red)));
		out.rgbGreen = std::min(255, int(green + shade * (0xff - green)));
		out.rgbBlue = std::min(255, int(blue + shade * (0xff - blue)));
		out.rgbReserved = 0;
	}
	else if (shade < 0.0f)	// darker
	{
		shade = 1.0f + shade;

		out.rgbRed = std::min(255, int(red * shade));
		out.rgbGreen = std::min(255, int(green * shade));
		out.rgbBlue = std::min(255, int(blue * shade));
		out.rgbReserved = 0;
	}
	else
		return rgbqColor;

	return out;
}


void CaptionWindow::DrawTab(CDC* dc, int image, const CRect& rect, int text_width, COLORREF base_color)
{
	//if (GetTabImg().m_hImageList == 0 || GetTabPartWidth() == 0 ||
	//	GetTabEnd().m_hImageList == 0 || GetTabEndWidth() == 0)
	//	return;

	int w= LEFT_MARGIN;
	CRect r= rect;
	r.right = w + rect.left + text_width + 3;
//	DrawCaptionGradient(dc, r, ::GetSysColor(image == 0 ? COLOR_3DSHADOW : COLOR_BTNFACE));

	//if (image != 0)
	//	return;

	COLORREF base= base_color;//::GetSysColor(COLOR_BTNFACE);
	DrawPanelCaption(*dc, r, &base);

	//r.left = r.right;
	//r.right++;
	//COLORREF shade= CalcShade(base, -1.0f);
	//DrawPanelCaption(*dc, r, &shade);
	//r.OffsetRect(1, 0);
	//shade = CalcShade(base, -3.0f);
	//DrawPanelCaption(*dc, r, &shade);
	//r.OffsetRect(1, 0);
	//shade = CalcShade(base, -7.0f);
	//DrawPanelCaption(*dc, r, &shade);

	//r = rect;
	//r.right = r.left + 1;
	//shade = CalcShade(base, -1.0f);
	//DrawPanelCaption(*dc, r, &shade);

//	dc->FillSolidRect(rect.left, rect.top, w+text_width, rect.Height(), RGB(255,0,0));

	/*
	CPoint pos= rect.TopLeft();

	CSize offset_size(GetTabPartWidth(), 0);

	UINT style= WhistlerLook::IsAvailable() ? ILD_NORMAL : ILD_TRANSPARENT;

	int image_index= image * 6;
	if (text_width > 0)
	{
		GetTabImg().Draw(dc, image_index + 0, pos, style);
		pos += offset_size;
	}

	int repeat= text_width / offset_size.cx;
	while (repeat > 0)
	{
		GetTabImg().Draw(dc, image_index + 1, pos, style);
		pos += offset_size;
		--repeat;
	}

	if (repeat == 0)	// if repeat < 0 then do not draw next img (there's no space)
	{
		GetTabImg().Draw(dc, image_index + 2, pos, style);
		pos += offset_size;
	}
	GetTabImg().Draw(dc, image_index + 3, pos, style);
	pos += offset_size;
	GetTabImg().Draw(dc, image_index + 4, pos, style);
	pos += offset_size;
	GetTabImg().Draw(dc, image_index + 5, pos, style);

	pos = CPoint(rect.right - GetTabEndWidth(), rect.top);
	GetTabEnd().Draw(dc, image, pos, style);
*/
}


void CaptionWindow::EraseBackground(CDC& dc, const CRect& rect)
{
	CRect client;
	GetClientRect(client);
	CRect r = rect;

	if (client.Height() > rect.Height())
	{
		r.top -= (client.Height() - rect.Height()) / 2;
		r.bottom = r.top + client.Height();
	}

	DrawCaptionGradient(&dc, r);
}


void CaptionWindow::DrawCaptionGradient(CDC* dc, const CRect& rect)
{
	dc->SetBkMode(OPAQUE);
	::DrawPanelBackground(*dc, rect, faint_bottom_edge_, &bar_color_);
}


int CaptionWindow::GetTollBarWidth()
{
	//int width= /*big_ ? TBARBIG_WIDTH : */TBAR_WIDTH;
	CDC dc;
	CDC* pdc = nullptr;
	dc.CreateIC(_T("DISPLAY"), 0, 0, 0);
	pdc = &dc;
	int log_inch_x = pdc->GetDeviceCaps(LOGPIXELSX);
	int width= 25/static_cast<Gdiplus::REAL>(log_inch_x)*96;

	if (maximized_wnd_toolbar_)
		//return Pixels(width / 2 + 4);
		return Pixels(width + 1);

	if (no_close_btn_ && no_maximize_btn_)
		return 0;
	
	return Pixels(no_close_btn_ ? width + 1: width*2 + 1);
}


CRect CaptionWindow::GetTextRect(CDC* dc, const CRect& rect, const TCHAR* title, CSize& text_size)
{
	//CGdiObject* old = dc->SelectStockObject(DEFAULT_GUI_FONT); // dc->SelectObject(&title_fnt_);
	CGdiObject* old = g_Settings.SelectDefaultFont(*dc, true);// dc->SelectObject(&title_fnt_);

	CRect text_rect(rect.left + LEFT_MARGIN, rect.top,
		rect.right - GetTollBarWidth() , // - (tab_part_divider_ * GetTabPartWidth()) / 3,
		rect.bottom);

	if (chevron_wnd_.m_hWnd)
		text_rect.right -= /*big_ ? CHEVRONBIGSPACE : */CHEVRONSPACE;

	if (text_rect.right < rect.left)
		text_rect.right = rect.left;

	text_size = dc->GetTextExtent(title, static_cast<int>(_tcslen(title)));
	if (text_size.cx > text_rect.Width())
		text_size.cx = text_rect.Width();

	dc->SelectObject(old);

	return text_rect;
}


void CaptionWindow::DrawCaptionBar(CDC* dc, CRect& rect, const TCHAR* title, bool is_active)
{
	dc->SetBkMode(OPAQUE);

	DrawCaptionGradient(dc, rect);

	//int min_width= GetMinCaptionWidth();
	//if (chevron_wnd_.m_hWnd)
	//	min_width += big_ ? CHEVRONBIGSPACE : CHEVRONSPACE;

	//bool narrow= rect.Width() < min_width;

	CSize text_size(0, 0);
	CRect text_rect= GetTextRect(dc, rect, title, text_size);

	//CFont* old= dc->SelectObject(&title_fnt_);
	CGdiObject* old = g_Settings.SelectDefaultFont(*dc, true);
//	CGdiObject* old= dc->SelectStockObject(DEFAULT_GUI_FONT);

	int length= static_cast<int>(_tcslen(title));

	COLORREF rgb_caption= is_active ? active_caption_color_ : inactive_caption_color_;

	if (is_active || use_inactive_caption_colors_)
		DrawTab(dc, -1/*is_active ? 0 : 1*/, rect, text_size.cx, rgb_caption);
	else
		rgb_caption = bar_color_;

	//CRect grip= rect;
	//grip.OffsetRect(LEFT_MARGIN / 3, big_ ? 2 : 1);
	//grip.bottom -= 2;
	//DrawResizingGrip(*dc, grip, rgb_caption, is_active);
/*
//	CRect grip(0, 0, 2, 2);
	grip.OffsetRect(rect.left + 8, rect.top + (big_ ? 4 : 3));
	COLORREF base= rgb_caption; // CalcShade(::GetSysColor(COLOR_3DFACE), 0.0f);
	COLORREF rgb_light= CalcShade(base, is_active ? 40.0f : 80.0f);// CalcShade(rgb_caption, 40.0f);
	COLORREF rgb_dark= CalcShade(base, is_active ? -20.0f : -12.0f); //rgb_caption, is_active ? -40.0f : -20.0f);

	DrawGrip(dc, grip, rgb_light, rgb_dark);
	grip.OffsetRect(3, 3);
	DrawGrip(dc, grip, rgb_light, rgb_dark);
	grip.OffsetRect(-3, 3);
	DrawGrip(dc, grip, rgb_light, rgb_dark);
	grip.OffsetRect(3, 3);
	DrawGrip(dc, grip, rgb_light, rgb_dark);
	grip.OffsetRect(-3, 3);
	DrawGrip(dc, grip, rgb_light, rgb_dark);
	grip.OffsetRect(3, 3);
	DrawGrip(dc, grip, rgb_light, rgb_dark);
	grip.OffsetRect(-3, 3);
	DrawGrip(dc, grip, rgb_light, rgb_dark);
	if (big_)
	{
		grip.OffsetRect(3, 3);
		DrawGrip(dc, grip, rgb_light, rgb_dark);
		grip.OffsetRect(-3, 3);
		DrawGrip(dc, grip, rgb_light, rgb_dark);
		grip.OffsetRect(3, 3);
		DrawGrip(dc, grip, rgb_light, rgb_dark);
		grip.OffsetRect(-3, 3);
		DrawGrip(dc, grip, rgb_light, rgb_dark);
	}
*/
	dc->SetBkMode(TRANSPARENT);

	UINT format= DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS;

	COLORREF rgb_text_color= /*::GetSysColor(COLOR_BTNTEXT);*/ is_active ? active_caption_text_color_ : inactive_caption_text_color_;
/*
	// hack: no shadow if text color isn't light
	if (GetRValue(rgb_text_color) > 0xb0 && GetGValue(rgb_text_color) > 0xb0 && GetBValue(rgb_text_color) > 0xb0)
	{
		text_rect.OffsetRect(1, 1);
		dc->SetTextColor(rgb_dark);
		dc->DrawText(title, length, text_rect, format);
		text_rect.OffsetRect(-1, -1);
	}
*/
	dc->SetTextColor(rgb_text_color);
	dc->DrawText(title, length, text_rect, format);

	dc->SetBkMode(OPAQUE);

	// shadow
	//if (WhistlerLook::IsAvailable())
	//	DrawTab(dc, 2, rect, text_size.cx);

	dc->SelectObject(old);
}


BOOL CaptionWindow::OnEraseBkgnd(CDC* dc)
{
	CRect rect;
	GetClientRect(rect);

	if (rect.Width() <= 0)
		return true;

	MemoryDC mem_dc(*dc, this);

	CString title;
	GetWindowText(title);

	DrawCaptionBar(&mem_dc, rect, title, active_);

	if (chevron_wnd_.m_hWnd)
	{
		WINDOWPLACEMENT wp;
		chevron_wnd_.GetWindowPlacement(&wp);
		CRect rect= wp.rcNormalPosition;
		rect.left -= 2;
		rect.right = rect.left + 1;
		mem_dc.FillSolidRect(rect, ::GetSysColor(COLOR_3DSHADOW));
	}

	mem_dc.BitBlt();

	return true;
}


void CaptionWindow::SetPosition(const CRect& rect)
{
	SetWindowPos(0, rect.left, rect.top, rect.Width(), GetHeight(), SWP_NOZORDER | SWP_NOACTIVATE);
}


void CaptionWindow::OnSize(UINT type, int cx, int cy)
{
	CWnd::OnSize(type, cx, cy);
	Resize();
}


void CaptionWindow::Resize()
{
	CRect rect;
	GetClientRect(rect);

	if (tool_bar_wnd_.m_hWnd)
	{
		int y= 5;//big_ ? 7 : 3;
		//y=0;
		tool_bar_wnd_.SetWindowPos(0, rect.Width() - GetTollBarWidth() - 1, y, GetTollBarWidth(), GetHeight() - 1, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
	}

	SetChevronPos();

	if (hosted_bar_)
		PositionHostedBar();
}


void CaptionWindow::SetChevronPos()
{
	if (chevron_wnd_.m_hWnd)
	{
		CRect rect;
		GetClientRect(rect);
		int width= /*big_ ? CHEVRONBIG : */CHEVRON;
		chevron_wnd_.SetWindowPos(0, rect.Width() - GetTollBarWidth() - width, 0, width, GetHeight(), SWP_NOZORDER | SWP_NOACTIVATE);
	}
}


void CaptionWindow::OnLButtonDown(UINT flags, CPoint pos)
{
	UpdateWindow();

	SetCapture();

	start_ = pos;
	block_dragging_ = false;
}


void CaptionWindow::OnLButtonUp(UINT flags, CPoint point)
{
	ReleaseCapture();
	SetCursor();

	if (dragging_)
	{
		frame_->ExitDragging(true);
		dragging_ = false;
	}

	start_ = CPoint(-1, -1);
}

void CaptionWindow::OnMouseMove(UINT flags, CPoint pos)
{
	if (frame_ == 0)
		return;

	if (block_dragging_)
		return;

	if (flags & MK_LBUTTON)
	{
		if (start_ == CPoint(-1, -1))
			return;					// there was no left button down message sent

		CSize delta_size= pos - start_;

		if (!dragging_)
		{
			if (abs(delta_size.cx) > 2 || abs(delta_size.cy) > 2)
				if (frame_->EnterDragging(parent_))
				{
					dragging_ = true;
					SetCursor();
				}
		}

/*		if (dragging_)
		{
			GetCursorPos(&pos);
			::SetCursor(AfxGetApp()->LoadCursor(frame_->TrackDraggedPane(pos) ? IDC_MOVE_PANE : IDC_NODROP));
		} */
	}
}

void CaptionWindow::SetCursor()
{
	if (dragging_)
		::SetCursor(AfxGetApp()->LoadCursor(IDC_MOVE_PANE));
	else
		::SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));
}


BOOL CaptionWindow::OnSetCursor(CWnd* wnd, UINT hit_test, UINT message)
{
	if (hit_test == HTCLIENT)
	{
		SetCursor();
		return true;
	}
	return CWnd::OnSetCursor(wnd, hit_test, message);
}


void CaptionWindow::ModifyToolbar(bool wnd_maximized)
{
	maximized_wnd_toolbar_ = wnd_maximized;
	tool_bar_wnd_.HideButton(ID_PANE_MAXIMIZE, wnd_maximized || no_maximize_btn_);
	if (!no_close_btn_)
		tool_bar_wnd_.HideButton(ID_PANE_CLOSE, wnd_maximized);
	tool_bar_wnd_.HideButton(ID_PANE_RESTORE, !wnd_maximized);
	tool_bar_wnd_.AutoResize();
}


void CaptionWindow::Activate(bool active)
{
	if (active_ == active)
		return;

	active_ = active;

	// different color for close/maximize buttons
/*
	HBITMAP bmp1= active_ ? toolbar1_bmp_ : toolbar2_bmp_;
	HBITMAP bmp2= active_ ? toolbar2_bmp_ : toolbar1_bmp_;

	TBREPLACEBITMAP tbrb;
	tbrb.inst_old = 0;
	tbrb.id_old = UINT(bmp1);
	tbrb.inst_new = 0;
	tbrb.id_new = UINT(bmp2);
	tbrb.buttons = 3;
	tool_bar_wnd_.SendMessage(TB_REPLACEBITMAP, 0, LPARAM(&tbrb));
*/
	Invalidate();
}


void CaptionWindow::OnLButtonDblClk(UINT flags, CPoint point)
{
	if (frame_)
	{
		block_dragging_ = true;
		if (!no_maximize_btn_)
			frame_->PaneZoom(parent_);
	}
}


void CaptionWindow::OnRButtonDown(UINT flags, CPoint point)
{
	if (dragging_)	// right mouse click while dragging pane?
	{
		show_context_menu_ = false;

		// then cancel dragging
		frame_->CancelDraggingPane();
		dragging_ = false;
		block_dragging_ = true;
		SetCursor();
		return;
	}
	else
		show_context_menu_ = true;
}


void CaptionWindow::OnRButtonUp(UINT flags, CPoint point)
{
	if (show_context_menu_)
	{
		ClientToScreen(&point);
		OnContextMenu(this, point);
	}
}


void CaptionWindow::OnContextMenu(CWnd* wnd, CPoint point)
{
#if 0	// todo: small & big images
	CMenu menu;
	if (!menu.LoadMenu(IDR_CAPTION_CONTEXT))
		return;

	if (CMenu* popup= menu.GetSubMenu(0))
	{
		if (point.x == -1 && point.y == -1)
		{
			CRect rect;
			GetClientRect(rect);
			ClientToScreen(rect);
			point = rect.CenterPoint();
		}

		popup->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, point.x, point.y, this);
	}
#endif
}


void CaptionWindow::SetTitle(const TCHAR* title)
{
	SetWindowText(title);
	Invalidate();
}


void CaptionWindow::AddBand(CWnd* toolbar, CWnd* owner, std::pair<int, int> pairMinMaxWidth, bool resizable)
{
	hosted_bar_ = toolbar;
	hosted_bar_owner_ = owner;
	hosted_bar_resizable_ = resizable;
	pair_hosted_bar_min_max_width_ = pairMinMaxWidth;

	if (hosted_bar_)
	{
		CRect bar_rect;
		hosted_bar_->GetWindowRect(bar_rect);
		hosted_bar_->GetWindowRect(bar_rect);
		hosted_bar_->SetParent(this);
		if (CToolBarCtrl* tool_bar= dynamic_cast<CToolBarCtrl*>(hosted_bar_))
			tool_bar->SetOwner(hosted_bar_owner_);
		if (pair_hosted_bar_min_max_width_.first == 0)
			pair_hosted_bar_min_max_width_.first = bar_rect.Width();
		if (pair_hosted_bar_min_max_width_.second == 0)
			pair_hosted_bar_min_max_width_.second = bar_rect.Width();
	}
	//else
	//	hosted_bar_rect_.SetRectEmpty();

	CRect rect;
	GetClientRect(rect);
	if (rect.Width() > 0)
		PositionHostedBar();
}


void CaptionWindow::ResetBandsWidth(std::pair<int, int> pairMinMaxWidth)
{
	pair_hosted_bar_min_max_width_ = pairMinMaxWidth;
}


void CaptionWindow::ResetBandsWidth()
{
	if (hosted_bar_ != 0)
	{
		CRect rect;
		hosted_bar_->GetWindowRect(rect);
		pair_hosted_bar_min_max_width_ = std::make_pair(rect.Width(), rect.Width());
	}
	else
		pair_hosted_bar_min_max_width_ = std::make_pair(0, 0);
}


void CaptionWindow::PositionHostedBar()
{
	if (hosted_bar_ != 0)
	{
		CRect rect= GetHostedBarRect();
		if (rect.Width() >= pair_hosted_bar_min_max_width_.first)
		{
			if (chevron_wnd_.m_hWnd)
			{
				chevron_wnd_.Destroy();
				Invalidate();
			}

			if (rect.Width() >= pair_hosted_bar_min_max_width_.second)
				rect.right = rect.left + pair_hosted_bar_min_max_width_.second; // limit this width
		}
		else	// not enough space
		{
			if (!hosted_bar_resizable_)
			{
				if (chevron_wnd_.m_hWnd == 0)
				{
					CreateChevron();
					SetChevronPos();
					Invalidate();
				}
				rect.right -= CHEVRON + 2;//(big_ ? CHEVRONBIG : CHEVRON) + 2;	// place for chevron
			}
		}

		CRect bar_rect;
		hosted_bar_->GetWindowRect(bar_rect);
		if (rect.Height() > bar_rect.Height())
			rect.top += (rect.Height() - bar_rect.Height()) / 2;

		// this is lame...
		//if (CComboBox* combo = dynamic_cast<CComboBox*>(hosted_bar_))
		//	rect.bottom = rect.top + bar_rect.Height(); // important for combobox

		rect.bottom = rect.top + bar_rect.Height();

		hosted_bar_->MoveWindow(rect);
	}
}


void CaptionWindow::CreateChevron()
{
	int chevron_cmd= ID_CHEVRON;
	chevron_wnd_.SetOnIdleUpdateState(false);
	chevron_wnd_.SetPadding(0, 8);

	int bmp= /*big_ ? IDB_CHEVRON_BIG : */IDB_CHEVRON;
	chevron_wnd_.Create("p", &chevron_cmd, bmp, 0, this);

	int hot= /*big_ ? IDB_CHEVRON_BIG_HOT : */IDB_CHEVRON_HOT;
	chevron_wnd_.SetHotImageList(hot);
}


CRect CaptionWindow::GetHostedBarRect()
{
	CRect rect;
	GetClientRect(rect);

	CClientDC dc(this);

	CString title;
	GetWindowText(title);

	CSize text_size(0, 0);
	CRect text_rect= GetTextRect(&dc, rect, title + L"N", text_size);

	//int parts= text_size.cx / GetTabPartWidth() + tab_part_divider_ - 1;
	rect.left = text_rect.left + text_size.cx;// parts * GetTabPartWidth() - 1;
	rect.right -= GetTollBarWidth() + 1;

	if (rect.Width() < 0)
		rect.right = rect.left;

	return rect;
}


void CaptionWindow::OnChevron()
{
	if (hosted_bar_ == 0)
		return;

	CRect bar_rect;
	hosted_bar_->GetWindowRect(bar_rect);
	CSize size= bar_rect.Size();
	if (!hosted_bar_resizable_)
		size.cx = pair_hosted_bar_min_max_width_.first;

	new CToolBarPopup(*hosted_bar_, size, hosted_bar_owner_);
}


LRESULT CaptionWindow::OnPrint(WPARAM wdc, LPARAM flags)
{
	if (HDC hdc= reinterpret_cast<HDC>(wdc))
		if (CDC* dc= CDC::FromHandle(hdc))
		{
			dc->SetBkMode(OPAQUE);
			// fixed width here
			CRect rect(0, 0, 1999, GetHeight());
			DrawCaptionGradient(dc, rect);
		}

	return 0;
}


void CaptionWindow::HideCloseButton()
{
	tool_bar_wnd_.HideButton(ID_PANE_CLOSE);
	no_close_btn_ = true;
	tool_bar_wnd_.AutoResize();
	Resize();
}


void CaptionWindow::HideMaximizeButton()
{
	tool_bar_wnd_.HideButton(ID_PANE_MAXIMIZE);
	no_maximize_btn_ = true;
	tool_bar_wnd_.AutoResize();
	Resize();
}

/*
void CaptionWindow::OnSmallIcons()
{
	if (big_)
		if (frame_)
		{
			big_ = false;
			frame_->ChangeCaptionHeight(big_);
		}
}


void CaptionWindow::OnLargeIcons()
{
	if (!big_)
		if (frame_)
		{
			big_ = true;
			frame_->ChangeCaptionHeight(big_);
		}
}
*/
/*
void CaptionWindow::OnInitMenuPopup(CMenu* popup_menu, UINT index, BOOL sys_menu)
{
	popup_menu->CheckMenuRadioItem(ID_SMALL_ICONS, ID_LARGE_ICONS, !big_ ? ID_SMALL_ICONS : ID_LARGE_ICONS, MF_BYCOMMAND);
}
*/
/*
void CaptionWindow::ChangeHeight(bool big)
{
	tool_bar_wnd_.ReplaceImageList(big ? IDB_PANE_TOOLBAR_BIG : IDB_PANE_TOOLBAR);
	//tool_bar_wnd_.ReplaceImageList(big ? IDB_PANE_TOOLBAR_BIG_HOT : IDB_PANE_TOOLBAR_HOT, 0, ToolBarWnd::HOT);

	if (chevron_wnd_.m_hWnd)
		chevron_wnd_.Destroy();

//	chevron_wnd_.ReplaceImageList(big ? IDB_CHEVRON_BIG : IDB_CHEVRON);
//	chevron_wnd_.ReplaceImageList(big ? IDB_CHEVRON_BIG_HOT : IDB_CHEVRON_HOT, 0, ToolBarWnd::HOT);
}
*/

void CaptionWindow::SetBottomEdge(bool faint)
{
	faint_bottom_edge_ = faint;
	Invalidate();
}


bool CaptionWindow::HasMaximizeButton() const
{
	return !no_maximize_btn_;
}

int CaptionWindow::GetHeight()
{
	//CDC dc;
	//dc.CreateIC(_T("DISPLAY"), 0, 0, 0);
	//auto dpi = dc.GetDeviceCaps(LOGPIXELSY);
	auto h = 32;//big_ ? 38 : 28;
	// caption window height
	return Pixels(h);
//	return h * dpi / 96;
}

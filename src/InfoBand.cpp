/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// InfoBand.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "InfoBand.h"
#include "Columns.h"
#include "DrawFields.h"
#include "CatchAll.h"
#include "MemoryDC.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern void DrawParentBkgnd(CWnd& wnd, CDC& dc);

/////////////////////////////////////////////////////////////////////////////
// InfoBand

InfoBand::InfoBand(const Columns& columns) : columns_(columns)
{
	show_zoom_ = true;
	show_name_ = true;
	recipient_ = 0;
	fields_.push_back(COL_DATE_TIME);
	show_field_names_ = true;
	text_color_ = RGB(255,255,255);
	label_color_ = RGB(160, 160, 160);
}

InfoBand::~InfoBand()
{}

const int POPUP_CMD= IDS_INFO_BAND_OPT;

BEGIN_MESSAGE_MAP(InfoBand, CWnd)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_CONTEXTMENU()
//	ON_NOTIFY(TBN_DROPDOWN, AFX_IDW_TOOLBAR, OnTbDropDown)
	ON_COMMAND(POPUP_CMD, OnPopup)
	ON_MESSAGE(WM_PRINTCLIENT, OnPrintClient)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// InfoBand message handlers

bool InfoBand::Create(CWnd* parent, InfoBandNotification* recipient)
{
	CDC dc;
	dc.CreateIC(_T("DISPLAY"), 0, 0, 0);
	LOGFONT lf;
	HFONT hfont = static_cast<HFONT>(::GetStockObject(DEFAULT_GUI_FONT));
	::GetObject(hfont, sizeof(lf), &lf);
	lf.lfWeight = FW_NORMAL;
	lf.lfHeight += 1;
	_tcscpy(lf.lfFaceName, _T("Tahoma"));
	//lf.lfQuality = ANTIALIASED_QUALITY;
	_font.CreateFontIndirect(&lf);
	dc.SelectObject(&_font);
	//dc.SelectStockObject(DEFAULT_GUI_FONT);
	TEXTMETRIC tm;
	dc.GetTextMetrics(&tm);
	int h= tm.tmHeight + tm.tmInternalLeading + 3;	// yes, internal leading is already in tmHeight

	if (!CWnd::Create(AfxRegisterWndClass(CS_VREDRAW | CS_HREDRAW, ::LoadCursor(NULL, IDC_ARROW)),
		0, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN, CRect(0, 0, 40, h), parent, -1))
		return false;

	FancyToolBar::Params p;
	p.shade = -0.25f;
	if (!options_.Create(this, "p", &POPUP_CMD, IDB_INFO_BAND_OPT, &p))
		return false;

	options_.SetPadding(CRect(2,2,2,2));
//	options_.SetOption(FancyToolBar::BEVEL_LOOK, false);
	options_.SetOption(FancyToolBar::HOT_OVERLAY, false);
//	options_.SetOption(FancyToolBar::SHIFT_BTN, false);
	options_.SetOnIdleUpdateState(false);

	tool_bar_width_ = options_.Size().cx + 1;
	recipient_ = recipient;

	Resize();

	return true;
}


void InfoBand::OnPopup()
{
	CRect rect= options_.GetCmdButtonRect(POPUP_CMD);
	CPoint pos(rect.right, rect.bottom);
	options_.ClientToScreen(&pos);
	OptionsPopup(pos);
}


BOOL InfoBand::OnEraseBkgnd(CDC* dc)
{
	try
	{
		CRect rect(0,0,0,0);
		GetClientRect(rect);

		MemoryDC dc(*dc, rect);

		DrawParentBkgnd(*this, dc);

		dc.SetBkMode(TRANSPARENT);
		dc.SetTextColor(::GetSysColor(COLOR_BTNTEXT));
		//dc.SelectStockObject(DEFAULT_GUI_FONT);
		dc.SelectObject(&_font);

		CString text;
		GetWindowText(text);
		// trim spaces
		text.TrimRight(_T(' '));
		text.Insert(0, _T("   "));

		//COLORREF text_color= RGB(220,220,220);
		//COLORREF label_color= RGB(139,139,147);

		CRect text_rect= rect;
		text_rect.right -= tool_bar_width_;
//		text_rect.left += 5;
		if (text_rect.Width() > 0)
			DrawFields::Draw(dc, text, text_rect, text_color_, label_color_, text_color_);
		//		::GetSysColor(COLOR_BTNTEXT),
		//		::GetSysColor(COLOR_3DDKSHADOW), ::GetSysColor(COLOR_BTNTEXT));

		dc.BitBlt();
	}
	catch (...)
	{}

	return true;
/*
	CRect rect;
	GetClientRect(rect);

	if (CWnd* parent= GetParent())
	{
		CDC background_dc;
		background_dc.CreateCompatibleDC(dc);

		CBitmap background_bmp;
		background_bmp.CreateCompatibleBitmap(dc, rect.Width(), rect.Height());
		background_dc.SelectObject(&background_bmp);

		WINDOWPLACEMENT wp;
		GetWindowPlacement(&wp);

		// prepare copy of background
		background_dc.SetWindowOrg(wp.rcNormalPosition.left, wp.rcNormalPosition.top);
		parent->Print(&background_dc, PRF_ERASEBKGND | PRF_CLIENT);
		background_dc.SetWindowOrg(0, 0);

		background_dc.SetBkMode(TRANSPARENT);
		background_dc.SetTextColor(::GetSysColor(COLOR_BTNTEXT));
		background_dc.SelectStockObject(DEFAULT_GUI_FONT);

		CString text;
		GetWindowText(text);
		// trim spaces
		text.TrimRight(_T(' '));

		CRect text_rect= rect;
		text_rect.right -= tool_bar_width_;
		if (text_rect.Width() > 0)
			DrawFields::Draw(background_dc, text, text_rect, ::GetSysColor(COLOR_BTNTEXT),
				::GetSysColor(COLOR_3DDKSHADOW), ::GetSysColor(COLOR_BTNTEXT));

		dc->BitBlt(0, 0, rect.Width(), rect.Height(), &background_dc, 0, 0, SRCCOPY);

		background_dc.DeleteDC();
	}
	
	return true; */
}


void InfoBand::OnPaint()
{
	CPaintDC paint_dc(this); // device context for painting
}


void InfoBand::OnSize(UINT type, int cx, int cy)
{
	CWnd::OnSize(type, cx, cy);
	Resize();
}


void InfoBand::Resize()
{
	CRect rect;
	GetClientRect(rect);

	if (rect.IsRectEmpty())
		return;

//	if (options_wnd_.m_hWnd)
//		options_wnd_.SetWindowPos(0, rect.right - tool_bar_width_, 0, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

	int cx= rect.Width();
	int cy= rect.Height();

	if (options_.m_hWnd)
	{
		CSize s= options_.Size();
		int x= cx - s.cx - 2;
		int y= (cy - s.cy - 1) / 2;
		options_.SetWindowPos(0, x, y, s.cx, s.cy, SWP_NOZORDER | SWP_NOACTIVATE);
	}
}


LRESULT InfoBand::OnPrintClient(WPARAM wdc, LPARAM flags)
{
	if (CDC* dc= CDC::FromHandle(HDC(wdc)))
		DrawParentBkgnd(*this, *dc);

	return 0;
}


//void InfoBand::OnTbDropDown(NMHDR* nmhdr, LRESULT* result)
//{
//	NMTOOLBAR* info_tip= reinterpret_cast<NMTOOLBAR*>(nmhdr);
//	switch (info_tip->iItem)
//	{
//	case ID_INFO_BAR_OPTIONS:
//		{
//			CRect rect;
//			options_wnd_.GetRect(ID_INFO_BAR_OPTIONS, rect);
//			CPoint pos(rect.left, rect.bottom);
//			options_wnd_.ClientToScreen(&pos);
//			OptionsPopup(pos);
//		}
//		break;
//	}
//	*result = TBDDRET_DEFAULT;
//}


void InfoBand::OptionsPopup(CPoint pos)
{
	CMenu menu;
	if (!menu.CreatePopupMenu())
		return;

	try
	{
		columns_.GetPopupMenu(menu, fields_);

		const int ID_SHOW_FIELD_NAMES= 99999;
		const int ID_RESET_COLUMNS= 99998;
		const int ID_ZOOM= 99997;
		const int ID_NAME= 99996;

		menu.InsertMenu(0, MF_BYPOSITION | MF_STRING, ID_ZOOM, _T("缩放"));
		if (show_zoom_)
			menu.CheckMenuItem(ID_ZOOM, MF_BYCOMMAND | MF_CHECKED);

		menu.InsertMenu(1, MF_BYPOSITION, ID_NAME, _T("名称和数量"));
		if (show_name_)
			menu.CheckMenuItem(ID_NAME, MF_BYCOMMAND | MF_CHECKED);

		menu.InsertMenu(2, MF_BYPOSITION | MF_SEPARATOR);
		menu.AppendMenu(MF_SEPARATOR);
		menu.AppendMenu(MF_STRING, ID_RESET_COLUMNS, _T("重置到默认值"));
		menu.AppendMenu(MF_STRING | (show_field_names_ ? MF_CHECKED : 0), ID_SHOW_FIELD_NAMES, _T("显示字段名称"));

		if (pos.x == -1 && pos.y == -1)
			GetCursorPos(&pos);

		if (recipient_)
			recipient_->PopupMenu(true);

		int field= menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD | TPM_NONOTIFY, pos.x, pos.y, this);

		if (recipient_)
			recipient_->PopupMenu(false);

		if (field <= 0)
			return;

		std::vector<uint16> sel= fields_;

		if (field == ID_RESET_COLUMNS)
		{
			show_zoom_ = show_name_ = true;
			fields_.clear();
			fields_.push_back(COL_DATE_TIME);
		}
		else if (field == ID_SHOW_FIELD_NAMES)
		{
			show_field_names_ = !show_field_names_;
		}
		else if (field == ID_ZOOM)
			show_zoom_ = !show_zoom_;
		else if (field == ID_NAME)
			show_name_ = !show_name_;
		else
		{
			--field;	// 0..n-1 range now (field numbers are not consecutive though)

			std::vector<uint16>::iterator it= remove(sel.begin(), sel.end(), field);
			if (it != sel.end())
				sel.erase(it);
			else
				//TODO: revise
				sel.insert(lower_bound(sel.begin(), sel.end(), field), field);

			fields_.swap(sel);
		}

		// notify parent
		if (recipient_)
			recipient_->FieldSelectionChanged();
	}
	CATCH_ALL
}


void InfoBand::OnContextMenu(CWnd*, CPoint pos)
{
	OptionsPopup(pos);
}

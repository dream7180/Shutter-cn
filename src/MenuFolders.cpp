/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// MenuFolders.cpp: implementation of the CMenuFolders class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "MenuFolders.h"
#include "ItemIdList.h"
#include "GetDefaultGuiFont.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMenuFolders::CMenuFolders()
{
	Init(0);
}

void CMenuFolders::Init(HMENU popup)
{
	label_size_ = shcut_size_ = img_size_ = CSize(0, 0);

	tab_stop_ = 0;

	if (popup == 0)
	{
		if (!CreatePopupMenu())
		{
			ASSERT(false);
			return;
		}
		own_menu_ = true;
	}
	else
	{
		if (!Attach(popup))
		{
			ASSERT(false);
			return;
		}
		own_menu_ = false;
	}

	items_.reserve(50);

	SHFILEINFO info;
	DWORD_PTR image_list= ::SHGetFileInfo(_T(""),
		NULL, &info, sizeof info, SHGFI_SMALLICON | SHGFI_PIDL | SHGFI_SYSICONINDEX);

	if (!image_list)
	{
		ASSERT(false);
//		return;
	}

//	images_.Attach(reinterpret_cast<HIMAGELIST>(image_list));
	images_ = reinterpret_cast<HIMAGELIST>(image_list);

	IMAGEINFO ii;
	if (images_ && ::ImageList_GetImageInfo(images_, 0, &ii))
	{
		img_size_.cx = ii.rcImage.bottom - ii.rcImage.top;
		img_size_.cy = ii.rcImage.right - ii.rcImage.left;
	}
}


CMenuFolders::CMenuFolders(HMENU popup)
{
	Init(popup);
}


CMenuFolders::~CMenuFolders()
{
	if (!own_menu_)
		Detach();

	//images_.Detach();
}


bool CMenuFolders::InsertItem(const ItemIdList& idlFolder, UINT id, const TCHAR* text)
{
	// add menu item
	if (!AppendMenu(MF_BYPOSITION | MF_STRING, id, text))
	{
		ASSERT(false);
		return false;
	}

	items_.push_back(&idlFolder);

	// and modify it's param (store index)
	MENUITEMINFO mi;
	memset(&mi, 0, sizeof mi);
	mi.cbSize = sizeof mi;
	mi.fMask = MIIM_TYPE;
	if (!GetMenuItemInfo(id, &mi))
	{
		ASSERT(false);
		return false;
	}
	mi.fMask = MIIM_TYPE | MIIM_DATA;
	mi.fType |= MFT_OWNERDRAW;
	mi.dwItemData = items_.size() - 1;
	::SetMenuItemInfo(*this, id, false, &mi);

	CalcStringLengths(text);

	return true;
}


void CMenuFolders::CalcStringLengths(CString text)
{
	CDC dc;
	dc.CreateDC(_T("DISPLAY"), 0, 0, 0);
	dc.SelectObject(&GetDefaultGuiFont());//&_font);
	//dc.SelectStockObject(DEFAULT_GUI_FONT);

	// check if there is a tab char (preceding shortcut text)
	int tab_pos= text.Find(_T('\t'), 0);
	if (tab_pos >= 0 && tab_pos < text.GetLength() - 1)
	{
		CSize label_size= dc.GetTextExtent(text.Left(tab_pos) + _T(' '));
		CSize shcut_size= dc.GetTextExtent(text.Right(text.GetLength() - tab_pos - 1));

		// find the widest label string
		if (label_size.cx > label_size_.cx)
			label_size_.cx = label_size.cx;
		if (label_size.cy > label_size_.cy)
			label_size_.cy = label_size.cy;

		// find the widest shortcut string
		if (shcut_size.cx > shcut_size_.cx)
			shcut_size_.cx = shcut_size.cx;
		if (shcut_size.cy > shcut_size_.cy)
			shcut_size_.cy = shcut_size.cy;

		TEXTMETRIC tm;
		if (dc.GetTextMetrics(&tm) && tm.tmAveCharWidth > 0)
		{
			int tab_stops= (label_size_.cx + tm.tmAveCharWidth - 1) / tm.tmAveCharWidth;
			tab_stop_ = tab_stops;
		}
	}
	else
	{
		// find the widest string
		CSize ext_size= dc.GetTextExtent(text);
		if (ext_size.cx > label_size_.cx)
			label_size_.cx = ext_size.cx;
		if (ext_size.cy > label_size_.cy)
			label_size_.cy = ext_size.cy;
	}
}


bool CMenuFolders::InsertItem(UINT id, const TCHAR* text)
{
	// add menu item
	if (!AppendMenu(MF_BYPOSITION | MF_STRING, id, text))
	{
		ASSERT(false);
		return false;
	}

	// and modify it's param (store index)
	MENUITEMINFO mi;
	memset(&mi, 0, sizeof mi);
	mi.cbSize = sizeof mi;
	mi.fMask = MIIM_TYPE;
	if (!GetMenuItemInfo(id, &mi))
	{
		ASSERT(false);
		return false;
	}
	mi.fMask = MIIM_TYPE | MIIM_DATA;
	mi.fType |= MFT_OWNERDRAW;
	mi.dwItemData = -1;
	::SetMenuItemInfo(*this, id, false, &mi);

	CalcStringLengths(text);

	return true;
}



static const int MARGINX= 2;

// draw item: bmp and text
//
void CMenuFolders::DrawItem(LPDRAWITEMSTRUCT draw_item_struct)
{
	ASSERT(draw_item_struct->itemData == -1 || draw_item_struct->itemData < items_.size());
	bool idl_present= draw_item_struct->itemData != -1;

	CDC dc;
	dc.Attach(draw_item_struct->hDC);

	int save= dc.SaveDC();
	bool selected= !!(draw_item_struct->itemState & ODS_SELECTED);
	CRect rect(draw_item_struct->rcItem);

	dc.SetBkMode(OPAQUE);
	dc.SelectObject(&GetDefaultGuiFont());//&_font);
	//dc.SelectStockObject(DEFAULT_GUI_FONT);

	COLORREF rgb_back= !selected ? ::GetSysColor(COLOR_MENU) : ::GetSysColor(COLOR_HIGHLIGHT);
	COLORREF rgb_text= !selected ? ::GetSysColor(COLOR_MENUTEXT) : ::GetSysColor(COLOR_HIGHLIGHTTEXT);

	dc.FillSolidRect(rect, rgb_back);
	dc.SetTextColor(rgb_text);
	dc.SetBkColor(rgb_back);

	if (draw_item_struct->itemState & ODS_CHECKED)
	{
		CRect bullet_rect(0, 0, MARGINX, rect.Height());
		CDC mem_dc;
		mem_dc.CreateCompatibleDC(&dc);
		CBitmap bmp;
		bmp.CreateBitmap(bullet_rect.Width(), bullet_rect.Height(), 1, 1, 0);
		mem_dc.SelectObject(&bmp);
		mem_dc.DrawFrameControl(bullet_rect, DFC_MENU, DFCS_MENUBULLET);

		dc.BitBlt(rect.left, rect.top, bullet_rect.Width(), bullet_rect.Height(), &mem_dc, 0, 0, SRCCOPY);
		mem_dc.DeleteDC();
		bmp.DeleteObject();
	}

	// draw bitmap
	CPoint pos= rect.TopLeft() + CSize(MARGINX, 2);
	if (idl_present)
	{
		int img_index= items_[draw_item_struct->itemData]->GetIconIndex();
		if (img_index >= 0 && images_ != 0)
		{
			COLORREF grbBack= ::ImageList_GetBkColor(images_);
			::ImageList_SetBkColor(images_, rgb_back);
			::ImageList_Draw(images_, img_index, dc, pos.x, pos.y, ILD_NORMAL);
			::ImageList_SetBkColor(images_, grbBack);
		}
	}

	// draw text
	rect.left += MARGINX + img_size_.cx + 5;
	CString item;
	GetMenuString(draw_item_struct->itemID, item, MF_BYCOMMAND);
	const TCHAR* text= item;
	DRAWTEXTPARAMS dp;
	dp.cbSize = sizeof dp;
	dp.iLeftMargin = 0;
	dp.iRightMargin = 0;
	dp.iTabLength = tab_stop_;
	dp.uiLengthDrawn = 0;
	UINT format= DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_EXPANDTABS | DT_TABSTOP;
	if (idl_present)
		format |= DT_NOPREFIX;		// no prefixes in folders' names
	::DrawTextEx(dc, const_cast<TCHAR*>(text), item.GetLength(), rect, format, &dp);

	dc.RestoreDC(save);
	dc.Detach();
}


void CMenuFolders::MeasureItem(LPMEASUREITEMSTRUCT measure_item_struct)
{
	measure_item_struct->itemWidth = MARGINX + img_size_.cx + label_size_.cx + shcut_size_.cx;
	measure_item_struct->itemHeight = std::max(img_size_.cy + 4L, label_size_.cy);
}

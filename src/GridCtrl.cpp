/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "GridCtrl.h"
#include "resource.h"
#include "Color.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


GridCtrl::GridCtrl(bool show_add_new_item)
{
	show_add_new_item_ = show_add_new_item;
	host_ = 0;
	edit_row_ = edit_col_ = -1;
	sel_column_ = 0;

	menu_down_btn_.Create(IDB_MENU_BTN, 8, 0, RGB(255,0,255));
	menu_right_btn_.Create(IDB_MENU_BTN_RIGHT, 8, 0, RGB(255,0,255));
}


GridCtrl::~GridCtrl()
{}


BEGIN_MESSAGE_MAP(GridCtrl, CListCtrl)
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnCustomDraw)
	ON_NOTIFY_REFLECT_EX(LVN_GETDISPINFO, OnGetDispInfo)
	ON_NOTIFY_REFLECT(NM_CLICK, OnNMClick)
	ON_EN_KILLFOCUS(EDIT_ID, OnEditKillFocus)
	ON_MESSAGE(WM_USER, OnFinishEdit)
	ON_WM_KEYDOWN()
	ON_WM_CHAR()
//	ON_WM_GETDLGCODE()
END_MESSAGE_MAP()


void GridCtrl::PreSubclassWindow()
{
	SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP);
	ModifyStyle(LVS_EDITLABELS, WS_CLIPCHILDREN | LVS_SINGLESEL);
	image_list_.Create(IDB_EMPTY_BMP, 1, 0, RGB(255,255,255));
	SetImageList(&image_list_, LVSIL_SMALL);
}


//static COLORREF CalcShade(COLORREF rgb_color, float shade)
//{
//	shade /= 100.0f;
//
//	int red= GetRValue(rgb_color);
//	int green= GetGValue(rgb_color);
//	int blue= GetBValue(rgb_color);
//
//	if (shade > 0.0f)	// lighter
//	{
//		return RGB(	min(255, int(red + shade * (0xff - red))),
//					min(255, int(green + shade * (0xff - green))),
//					min(255, int(blue + shade * (0xff - blue))));
//	}
//	else if (shade < 0.0f)	// darker
//	{
//		shade = 1.0f + shade;
//
//		return RGB(	min(255, int(red * shade)),
//					min(255, int(green * shade)),
//					min(255, int(blue * shade)));
//	}
//	else
//		return rgb_color;
//}


void GridCtrl::OnCustomDraw(NMHDR* nm_hdr, LRESULT* result)
{
	NMLVCUSTOMDRAW* NM_custom_draw= reinterpret_cast<NMLVCUSTOMDRAW*>(nm_hdr);
	*result = CDRF_DODEFAULT;
	size_t row= NM_custom_draw->nmcd.dwItemSpec;

	if (NM_custom_draw->nmcd.dwDrawStage == CDDS_PREPAINT)
	{
		*result = CDRF_NOTIFYITEMDRAW | CDRF_NOTIFYPOSTPAINT;
	}
	else if (NM_custom_draw->nmcd.dwDrawStage == CDDS_ITEMPREPAINT)
	{
		if (row & 1)	// odd item?
			NM_custom_draw->clrTextBk = CalcShade(GetBkColor(), -3.0f);

		if (show_add_new_item_ && row == GetItemCount() - 1)	// last line?
			NM_custom_draw->clrText = CalcShade(GetTextColor(), 60.0f);

		*result = CDRF_NOTIFYSUBITEMDRAW | CDRF_NOTIFYPOSTPAINT;
	}
	else if (NM_custom_draw->nmcd.dwDrawStage == (CDDS_ITEMPREPAINT | CDDS_SUBITEM))
	{
		// leave selection only for one selected column rather than the whole row
		if (NM_custom_draw->nmcd.uItemState & CDIS_SELECTED)
			if (NM_custom_draw->iSubItem != sel_column_)
				NM_custom_draw->nmcd.uItemState &= ~CDIS_SELECTED;

		// don't draw focus frame
		NM_custom_draw->nmcd.uItemState &= ~CDIS_FOCUS;
	}
	else if (NM_custom_draw->nmcd.dwDrawStage == CDDS_ITEMPOSTPAINT)
	{
		CDC* dc= CDC::FromHandle(NM_custom_draw->nmcd.hdc);

		if (host_ != 0 && dc != 0)
		{
			int saved= dc->SaveDC();

			const int columns= GetHeaderCtrl()->GetItemCount();
			for (int subitem= 1; subitem < columns; ++subitem)
			{
				host_->PostPaintCell(*this, row, subitem, *dc);

				UINT flags= host_->GetItemFlags(*this, row, subitem);

				if (flags & (GridCtrlNotification::DRAW_BTN_DOWN | GridCtrlNotification::DRAW_BTN_RIGHT | GridCtrlNotification::DRAW_ELLIPSIS))
				{
					CRect rect;
					if (GetSubItemRect(static_cast<int>(row), subitem, LVIR_BOUNDS, rect))
					{
						if (rect.Width() > 20)
						{
							const int img= GetItemState(static_cast<int>(row), LVIS_SELECTED) && sel_column_ == subitem ? 1 : 0;
							if (flags & GridCtrlNotification::DRAW_BTN_DOWN)
							{
								// little arrow pointing down
								int x= rect.right - 10;
								int y= rect.CenterPoint().y - 4;
								menu_down_btn_.Draw(dc, img, CPoint(x, y), ILD_TRANSPARENT);
							}
							else if (flags & GridCtrlNotification::DRAW_BTN_RIGHT)
							{
								// little arrow pointing right
								int x= rect.right - 8;
								int y= rect.CenterPoint().y - 4;
								menu_right_btn_.Draw(dc, img, CPoint(x, y), ILD_TRANSPARENT);
							}
							else if (flags & GridCtrlNotification::DRAW_ELLIPSIS)
							{
								// ellipsis: …

							}
						}
					}
				}
			}

			dc->RestoreDC(saved);
		}

		*result = CDRF_SKIPDEFAULT;
	}
}


BOOL GridCtrl::OnGetDispInfo(NMHDR* nmhdr, LRESULT* result)
{
	LV_DISPINFO* disp_info= reinterpret_cast<LV_DISPINFO*>(nmhdr);
	*result = 0;

	if ((disp_info->item.mask & LVIF_TEXT) == 0)
		return false;

	disp_info->item.pszText[0] = _T('\0');

	int line= disp_info->item.iItem;
	if (show_add_new_item_ && line == GetItemCount() - 1)	// last line?
	{
		if (disp_info->item.iSubItem == 0)
			_tcscpy(disp_info->item.pszText, _T("<add new>"));
	}
	else if (host_)
	{
		host_->GetCellText(*this, line, disp_info->item.iSubItem, buffer_);
		const TCHAR* text= buffer_;
		disp_info->item.pszText = const_cast<TCHAR*>(text);
	}

	return true;
}


void GridCtrl::OnNMClick(NMHDR* nmhdr, LRESULT* result)
{
	LVHITTESTINFO hit;
	hit.iItem = -1;
	hit.iSubItem = -1;
	::GetCursorPos(&hit.pt);
	ScreenToClient(&hit.pt);
	hit.flags = 0;

	int item= SubItemHitTest(&hit);

	if (item >= 0)
		EnterEdit(item, hit.iSubItem);

	*result = 0;
}


void GridCtrl::EnterEdit(int row, int col)
{
	if (row < 0)
		return;

	if (sel_column_ != col)
	{
		sel_column_ = col;
		int item= GetNextItem(-1, LVIS_FOCUSED);
		RedrawItems(item, item);
	}
//	sel_column_ = col;

	if (host_)
	{
		CString str;
		host_->GetCellText(*this, row, col, str);
		EnterEdit(row, col, str);
	}
}


void GridCtrl::EnterEdit(int row, int col, const TCHAR* text)
{
	if (edit_box_.m_hWnd != 0)
		return;

	EnsureVisible(row, false);
	EnsureColVisible(col);

	if (host_ && !host_->StartEditing(*this, row, col))
		return;

	edit_row_ = row;
	edit_col_ = col;

	LVCOLUMN column;
	column.mask = LVCF_FMT;
	GetColumn(col, &column);
	bool right= !!(column.fmt & LVCFMT_RIGHT);

	CRect cell_rect;
	GetSubItemRect(row, col, LVIR_LABEL, cell_rect);

	CRect rect;
	GetClientRect(rect);
	cell_rect &= rect;

	if (cell_rect.Width() <= 0 || cell_rect.Height() <= 0)
		return;

	cell_rect.InflateRect(0, 1, 0, 0);
	DWORD dwStyle= WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT | ES_AUTOHSCROLL;
	if (right)
		dwStyle |= ES_RIGHT;
	if (!edit_box_.Create(dwStyle, cell_rect, this, EDIT_ID))
		return;

	edit_box_.SendMessage(WM_SETFONT, WPARAM(SendMessage(WM_GETFONT)), 0);

	if (right)
		edit_box_.SetMargins(0, 3);

	edit_box_.SetWindowText(text);

	edit_box_.SetLimitText(TEXT_LIMIT);

	edit_box_.SetSel(0x7fff, 0x7fff);
	edit_box_.SetFocus();

	InstallMouseHook();
}


LRESULT GridCtrl::OnFinishEdit(WPARAM key, LPARAM)
{
	if (edit_box_.m_hWnd != 0)
	{
		int row= GetNextItem(-1, LVIS_FOCUSED);

		FinishEditing(key != VK_ESCAPE);

		if (row != GetNextItem(-1, LVIS_FOCUSED))
		{
			UINT state= LVIS_FOCUSED | LVIS_SELECTED;
			SetItemState(row, state, state);
		}

		switch (key)
		{
		case VK_RETURN:
		case VK_TAB:
			GoTo(GO_NEXT, true);
			break;
		case VK_DOWN:
			GoTo(GO_DOWN, false);
			break;
		case VK_UP:
			GoTo(GO_UP, false);
			break;
		default:
			break;
		}
	}

	return 0;
}


void GridCtrl::FinishEditing(bool ok)
{
	if (edit_box_.m_hWnd != 0)
	{
		UninstallMouseHook();

		if (ok && edit_row_ >= 0 && edit_col_ >= 0)
		{
			CString text;
			edit_box_.GetWindowText(text);

			if (host_ != 0)
			{
				host_->CellTextChanged(*this, edit_row_, edit_col_, text);
				RedrawItems(edit_row_, edit_row_);
			}
		}

		edit_row_ = edit_col_ = -1;

		edit_box_.DestroyWindow();
	}
}


void GridCtrl::OnEditKillFocus()
{
	if (edit_row_ >= 0 && edit_col_ >= 0)
		FinishEditing(true);
}


void GridCtrl::SetItemCount(size_t count)
{
	CListCtrl::SetItemCount(static_cast<int>(count + (show_add_new_item_ ? 1 : 0)));
}


void GridCtrl::OnKeyDown(UINT chr, UINT rep_cnt, UINT flags)
{
	switch (chr)
	{
	case VK_F2:
	case VK_SPACE:
	case VK_RETURN:
		{
			// enter edit mode
			int row= GetNextItem(-1, LVIS_FOCUSED);
			EnterEdit(row, sel_column_);
		}
		break;

	case VK_LEFT:
		GoTo(GO_LEFT, false);
		break;

	case VK_RIGHT:
		GoTo(GO_RIGHT, false);
		break;

	case VK_DELETE:
		if (host_)
		{
			int row= GetNextItem(-1, LVIS_FOCUSED);
			host_->Delete(*this, row, sel_column_);
		}
		break;

	default:
		Default();
		break;
	}
}


void GridCtrl::OnChar(UINT chr, UINT rep_cnt, UINT flags)
{
	if (isalnum(chr)) // >= 'A' && chr <= 'Z' || chr >= '0' && chr <= '9')
	{
		int row= GetNextItem(-1, LVIS_FOCUSED);
		if (show_add_new_item_ && row == GetItemCount() - 1)
		{
			TCHAR buf[]= { chr, 0 };
			EnterEdit(row, sel_column_, buf);
		}
	}
	else
		Default();
}


void GridCtrl::GoTo(Dir dir, bool edit)
{
	int row= GetNextItem(-1, LVIS_FOCUSED);
	if (row < 0)
		return;
	int col= sel_column_;

	int columns= 0;
	if (CHeaderCtrl* header= GetHeaderCtrl())
		columns = header->GetItemCount();

	switch (dir)
	{
	case GO_NEXT:
		col++;
		if (col >= columns)
		{
			col = 0;
			row++;
			edit = false;
		}
		break;

	case GO_DOWN:
		row++;
		break;

	case GO_UP:
		row--;
		break;

	case GO_RIGHT:
		col++;
		break;

	case GO_LEFT:
		col--;
		break;
	}

	if (col < 0)
		col = 0;
	else if (col >= columns)
		col = std::max(0, columns - 1);

	if (sel_column_ != col)
	{
		sel_column_ = col;
		int item= GetNextItem(-1, LVIS_FOCUSED);
		RedrawItems(item, item);
	}

	if (row >= 0 && row < GetItemCount())
	{
		UINT state= LVIS_FOCUSED | LVIS_SELECTED;
		SetItemState(row, state, state);

		if (edit)
			EnterEdit(row, sel_column_);
	}
}


void GridCtrl::EnsureColVisible(int col)
{
	CRect cell_rect;
	GetSubItemRect(0, col, LVIR_LABEL, cell_rect);

	CRect rect;
	GetClientRect(rect);

	if (cell_rect.left < rect.left)
		Scroll(CSize(cell_rect.left - rect.left, 0));
	else if (cell_rect.right > rect.right)
	{
		int scroll= std::min(cell_rect.right - rect.right, cell_rect.left - rect.left);
		Scroll(CSize(scroll, 0));
	}
}


//UINT GridCtrl::OnGetDlgCode()
//{
//	return DLGC_WANTALLKEYS;
//}


//=============================================================================

static HHOOK next_proc_= 0;
static GridCtrl* grid_ctrl_= 0;

static LRESULT CALLBACK MouseProc(int code, WPARAM wParam, LPARAM lParam)
{
	if (next_proc_ == 0)
		return 0;

	if (code < 0)  // do not process the message
		return CallNextHookEx(next_proc_, code, wParam, lParam);
//TRACE(_T("mouse: %x\n"), wParam);
	switch (wParam)
	{
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_NCLBUTTONDOWN:
	case WM_MOUSEWHEEL:
		if (grid_ctrl_ != 0)
			grid_ctrl_->PostMessage(WM_USER, 0);
		break;
	}

	return CallNextHookEx(next_proc_, code, wParam, lParam);
}


void GridCtrl::InstallMouseHook()
{
	ASSERT(next_proc_ == 0);
	if (next_proc_ == 0)
	{
		grid_ctrl_ = this;
		next_proc_ = ::SetWindowsHookEx(WH_MOUSE, MouseProc, 0, ::GetCurrentThreadId());
	}
}


void GridCtrl::UninstallMouseHook()
{
	if (next_proc_)
	{
		::UnhookWindowsHookEx(next_proc_);
		next_proc_ = 0;
		grid_ctrl_ = 0;
	}
}


//=============================================================================


BEGIN_MESSAGE_MAP(CGridEditBox, CEdit)
	ON_WM_KEYDOWN()
	ON_WM_GETDLGCODE()
	ON_WM_CHAR()
END_MESSAGE_MAP()


UINT CGridEditBox::OnGetDlgCode()
{
	return DLGC_WANTALLKEYS;
}


void CGridEditBox::OnKeyDown(UINT chr, UINT rep_cnt, UINT flags)
{
	CWnd* parent= GetParent();
	if (parent == 0)
		return;

	switch (chr)
	{
	case VK_ESCAPE:
	case VK_RETURN:
	case VK_DOWN:
	case VK_UP:
	case VK_TAB:
		parent->PostMessage(WM_USER, chr);
		break;

	default:
		Default();
		break;
	}
}


void CGridEditBox::OnChar(UINT chr, UINT rep_cnt, UINT flags)
{
	if (chr == VK_ESCAPE)
	{
		// no-op
		// this is to suppress 'ding' sound
	}
	else
		CEdit::OnChar(chr, rep_cnt, flags);
}

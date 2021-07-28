/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// ExtTreeCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "ExtTreeCtrl.h"
#include "ExtTreeRow.h"
#include "GetDefaultGuiFont.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifndef TVS_NOHSCROLL
	#define TVS_NOHSCROLL           0x8000  // TVS_NOSCROLL overrides this
	#define CCM_SETVERSION          (CCM_FIRST + 0x7)
#endif

/////////////////////////////////////////////////////////////////////////////
// ExtTreeCtrl

ExtTreeCtrl::ExtTreeCtrl()
{
	header_height_ = 0;
	RegisterWndClass();
}

ExtTreeCtrl::~ExtTreeCtrl()
{
}


BEGIN_MESSAGE_MAP(ExtTreeCtrl, CWnd)
	//{{AFX_MSG_MAP(ExtTreeCtrl)
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
	ON_NOTIFY(NM_CUSTOMDRAW, CTR_ID_TREE, OnDrawTree)
	ON_NOTIFY(HDN_ENDTRACK, CTR_ID_HEADER, OnEndTrack)
	ON_NOTIFY(HDN_ITEMCHANGED, CTR_ID_HEADER, OnHeaderTrack)
	ON_NOTIFY(NM_CLICK, CTR_ID_TREE, OnClickTreeItem)
	ON_NOTIFY(NM_RCLICK, CTR_ID_TREE, OnRClickTreeItem)
	ON_NOTIFY(TVN_KEYDOWN, CTR_ID_TREE, OnKeyDownTreeItem)
	ON_NOTIFY(TVN_SELCHANGED, CTR_ID_TREE, OnSelChangedTreeItem)
	ON_NOTIFY(TVN_GETDISPINFO, CTR_ID_TREE, OnGetDispInfo)
	ON_NOTIFY(NM_DBLCLK, CTR_ID_TREE, OnDblClickTreeItem)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// ExtTreeCtrl message handlers

CString ExtTreeCtrl::wnd_class_;	// registered window class


void ExtTreeCtrl::RegisterWndClass()
{
	if (wnd_class_.IsEmpty())
	{
		const TCHAR* cls_name= _T("ExtTreeCtrl");

		HINSTANCE inst = AfxGetInstanceHandle();

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
				wnd_class_ = cls_name;
		}
	}
}


void ExtTreeCtrl::PreSubclassWindow()
{
	SendMessage(CCM_SETVERSION, 5);

	if (CFont* font= GetParent()->GetFont())
		SetFont(font);

	ModifyStyleEx(0, WS_EX_CONTROLPARENT);

	CreateTree();

	CWnd::PreSubclassWindow();
}


bool ExtTreeCtrl::CreateTree()
{
	CFont* font= GetParent()->GetFont();

	{
		CClientDC dc(this);
		CFont* old= dc.SelectObject(font);
		TEXTMETRIC tm;
		dc.GetTextMetrics(&tm);
		header_height_ = tm.tmHeight + 3;		// header height: text height + extra space
		dc.SelectObject(old);
	}

	SendMessage(CCM_SETVERSION, 5);

	// create bold font
/*	LOGFONT lf;
	::GetObject(::GetStockObject(DEFAULT_GUI_FONT), sizeof lf, &lf);
	lf.lfWeight = FW_BOLD;
	bold_fnt_.CreateFontIndirect(&lf); */

	CRect empty_rect(0,0,0,0);

	VERIFY(header_wnd_.Create(WS_CHILD | WS_VISIBLE | HDS_HORZ | HDS_FULLDRAG, empty_rect, this, CTR_ID_HEADER));

	DWORD tree_style= WS_CHILD | WS_VISIBLE | WS_TABSTOP | TVS_HASBUTTONS | TVS_LINESATROOT | TVS_NOHSCROLL |
		TVS_NONEVENHEIGHT | TVS_DISABLEDRAGDROP | TVS_FULLROWSELECT | TVS_SHOWSELALWAYS;

//	if (check_boxes)
//		tree_style |= TVS_CHECKBOXES;

	VERIFY(tree_wnd_.Create(tree_style, empty_rect, this, CTR_ID_TREE));

//	img_list_check_box_.Create(IDB_CHECKBOXES, CHECKBOX_SIZE, 0, -1);

	//HGDIOBJ hfont= font ? font->m_hObject : ::GetStockObject(DEFAULT_GUI_FONT);
	
	LOGFONT lf;
	/*HFONT hfont = static_cast<HFONT>(::GetStockObject(DEFAULT_GUI_FONT));
	::GetObject(hfont, sizeof(lf), &lf);
	//lf.lfQuality = ANTIALIASED_QUALITY;
	//lf.lfHeight += 1;
	_tcscpy(lf.lfFaceName, _T("Tahoma"));*/
	::GetDefaultGuiFont(lf);
	HFONT hfont = CreateFontIndirectW(&lf);

	header_wnd_.SendMessage(WM_SETFONT, WPARAM(hfont));
	tree_wnd_.SendMessage(WM_SETFONT, WPARAM(hfont));

	Resize();

	return true;
}


bool ExtTreeCtrl::Create(CWnd* parent, CImageList* img_list, UINT id, bool check_boxes)
{
	if (m_hWnd == 0)
	{
		CRect empty_rect(0,0,0,0);

		if (!CWnd::CreateEx(WS_EX_CLIENTEDGE | WS_EX_CONTROLPARENT, wnd_class_, _T(""), WS_CHILD | WS_CLIPCHILDREN | WS_VISIBLE | WS_TABSTOP,
			empty_rect, parent, id))
			return false;
		LOGFONT lf;
		::GetDefaultGuiFont(lf);
		HFONT hfont = CreateFontIndirectW(&lf);
		SendMessage(WM_SETFONT, WPARAM(hfont));//::GetStockObject(DEFAULT_GUI_FONT)));
	}

	CreateTree();

	if (img_list)
		tree_wnd_.SetImageList(img_list, TVSIL_NORMAL);

	return true;
}


void ExtTreeCtrl::SetImageList(CImageList* image_list)
{
	tree_wnd_.SetImageList(image_list, TVSIL_NORMAL);
}


void ExtTreeCtrl::OnSize(UINT type, int cx, int cy)
{
	CWnd::OnSize(type, cx, cy);
	Resize();
}


void ExtTreeCtrl::Resize()
{
	CRect rect;
	GetClientRect(rect);

	if (header_wnd_.m_hWnd)
		header_wnd_.SetWindowPos(0, 0, 0, rect.Width(), header_height_, SWP_NOZORDER | SWP_NOACTIVATE);

	if (tree_wnd_.m_hWnd)
		tree_wnd_.SetWindowPos(0, 0, header_height_, rect.Width(), std::max(rect.Height() - header_height_, 0), SWP_NOZORDER | SWP_NOACTIVATE);
}


void ExtTreeCtrl::OnPaint()
{
	CPaintDC dc(this); // device context for painting
}


BOOL ExtTreeCtrl::OnEraseBkgnd(CDC* /*dc*/)
{
	return true;
}


void ExtTreeCtrl::InsertColumn(int col_index, const TCHAR* col_text, int width, UINT format/*= HDF_LEFT*/)
{
	ASSERT(header_wnd_.m_hWnd);

	HDITEM hdi;
	hdi.mask = HDI_TEXT | HDI_FORMAT | HDI_WIDTH;
	hdi.pszText = const_cast<TCHAR*>(col_text);
	hdi.cxy = width;
	hdi.cchTextMax = static_cast<int>(_tcslen(col_text));
	hdi.fmt = format | HDF_STRING;
//	hdi.iImage = 0;
//	hdi.iOrder = col_index;
	header_wnd_.InsertItem(col_index, &hdi);
}

// set order of columns
//
void ExtTreeCtrl::SetColumnOrderArray(int count, int* order_array)
{
	ASSERT(header_wnd_.m_hWnd);
	header_wnd_.SetOrderArray(count, order_array);
}

// get column order info
//
void ExtTreeCtrl::GetColumnOrderArray(int count, int* order_array)
{
	ASSERT(header_wnd_.m_hWnd);
	ASSERT(header_wnd_.GetItemCount() == count);
	header_wnd_.GetOrderArray(order_array, count);
}

// set column width
//
void ExtTreeCtrl::SetColumnWidth(int col, int width)
{
	ASSERT(header_wnd_.m_hWnd);
	HDITEM hdi;
	hdi.mask = HDI_WIDTH;
	hdi.cxy = width;
	header_wnd_.SetItem(col, &hdi);
}

// get column width
//
int ExtTreeCtrl::GetColumnWidth(int col)
{
	ASSERT(header_wnd_.m_hWnd);
	HDITEM hdi;
	hdi.mask = HDI_WIDTH;
	header_wnd_.GetItem(col, &hdi);
	return hdi.cxy;
}


HTREEITEM ExtTreeCtrl::InsertItem(const TCHAR* item, int image, int selected_image,
		HTREEITEM parent/*= TVI_ROOT*/, HTREEITEM insert_after/*= TVI_LAST*/)
{
	return tree_wnd_.InsertItem(item, image, selected_image, parent, insert_after);
}


void ExtTreeCtrl::OnDrawTree(NMHDR* nmhdr, LRESULT* result)
{
	*result = CDRF_DODEFAULT;

	if (nmhdr->idFrom != CTR_ID_TREE)
		return;

	NMTVCUSTOMDRAW* NM_custom_draw= reinterpret_cast<NMTVCUSTOMDRAW*>(nmhdr);

	if (NM_custom_draw->nmcd.dwDrawStage == CDDS_PREPAINT)
	{
		*result = CDRF_NOTIFYITEMDRAW | CDRF_NOTIFYPOSTPAINT;
	}
	else if (NM_custom_draw->nmcd.dwDrawStage == CDDS_ITEMPREPAINT)
	{
		*result = CDRF_NOTIFYITEMDRAW | CDRF_NOTIFYPOSTPAINT;
	}
	else if (NM_custom_draw->nmcd.dwDrawStage == CDDS_ITEMPOSTPAINT)
	{
		CDC* dc= CDC::FromHandle(NM_custom_draw->nmcd.hdc);

		int mode= dc->SetBkMode(OPAQUE);

		CRect rect= NM_custom_draw->nmcd.rc;
		if (rect.IsRectEmpty())
			return;

		rect.bottom--;

		COLORREF rgb_back= dc->GetBkColor();
		dc->SetBkColor(::GetSysColor(COLOR_3DFACE));
		int columns= header_wnd_.GetItemCount();
		if (columns == 0)
			return;

		CRect col_rect;
		header_wnd_.GetItemRect(0, col_rect);

		int x_pos= col_rect.Width();
		CRect bar= rect;
		CRect cell= rect;

		int state= tree_wnd_.GetItemState(HTREEITEM(NM_custom_draw->nmcd.dwItemSpec), TVIS_EXPANDED | TVIS_DROPHILITED);
		bool selected= NM_custom_draw->clrTextBk == ::GetSysColor(COLOR_HIGHLIGHT);

		ExtTreeRow* item= reinterpret_cast<ExtTreeRow*>(NM_custom_draw->nmcd.lItemlParam);
		ASSERT(item != 0);

		std::vector<std::pair<int, int>> column_info(columns);
		for (int i= 0; i < columns; ++i)
		{
			HDITEM hdi;
			hdi.mask = HDI_FORMAT | HDI_WIDTH;
			header_wnd_.GetItem(i, &hdi);
			column_info[i] = std::make_pair(hdi.cxy, hdi.fmt);
		}

		item->Draw(dc, column_info, NM_custom_draw->clrText, NM_custom_draw->clrTextBk, rect);

		rect.top = rect.bottom;
		rect.bottom++;
		dc->SetBkColor(::GetSysColor(COLOR_3DFACE));
		dc->ExtTextOut(0, 0, ETO_OPAQUE, rect, _T(""), 0, 0);

		dc->SetBkColor(rgb_back);
		dc->SetBkMode(mode);

		*result = CDRF_SKIPDEFAULT;
	}
}


// notification msg from header control: end of column resizing
//
void ExtTreeCtrl::OnEndTrack(NMHDR* /*nmhdr*/, LRESULT* result)
{
	tree_wnd_.Invalidate();
	*result = 0;
}


void ExtTreeCtrl::OnHeaderTrack(NMHDR* nmhdr, LRESULT* result)
{
//	NMHEADER* nm_header= reinterpret_cast<NMHEADER*>(nmhdr);

//	if (nm_header->pitem && (nm_header->pitem->mask & HDI_WIDTH))
	{
//		header_wnd_.Set
		tree_wnd_.Invalidate();
	}

	*result = 0;
}



// insert ext tree item
//
HTREEITEM ExtTreeCtrl::InsertItem(ExtTreeItem* row, HTREEITEM parent, int image, bool bold, bool check_box)
{
	ASSERT(row);
//	int selected_image= image + 1;
//	CString name;
//	bool bold_dummy;
//	row->GetDisplayText(0, name, bold_dummy);
	LPARAM lParam= reinterpret_cast<LPARAM>(row);
	row->item_ = tree_wnd_.InsertItem(TVIF_PARAM | TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_STATE,
		LPSTR_TEXTCALLBACK, I_IMAGECALLBACK, I_IMAGECALLBACK, bold ? TVIS_BOLD : 0, -1, lParam, parent, TVI_LAST);

	if (!check_box)
	{
		//tree_wnd_.SetCheck(row->item_
		TVITEM item;
		item.mask = TVIF_HANDLE | TVIF_STATE;
		item.hItem = row->item_;
		item.stateMask = TVIS_STATEIMAGEMASK;
		item.state = 0;
		tree_wnd_.SendMessage(TVM_SETITEM, 0, reinterpret_cast<LPARAM>(&item));
	}

	return row->item_;
}


void ExtTreeCtrl::OnRClickTreeItem(NMHDR* nmhdr, LRESULT* result)
{
	*result = 0;

	CPoint pos(-1, -1);
	GetCursorPos(&pos);
	tree_wnd_.ScreenToClient(&pos);

	UINT flags= 0;
	HTREEITEM item= tree_wnd_.HitTest(pos, &flags);

	if (item == 0 || (flags & (TVHT_ONITEM | TVHT_ONITEMRIGHT)) == 0)
		return;

	tree_wnd_.SelectItem(item);
}


void ExtTreeCtrl::OnDblClickTreeItem(NMHDR* nmhdr, LRESULT* result)
{
	*result = 0;
	OnClickTreeItem(true);
}


void ExtTreeCtrl::OnClickTreeItem(NMHDR* nmhdr, LRESULT* result)
{
	*result = 0;
	OnClickTreeItem(false);
}

void ExtTreeCtrl::OnClickTreeItem(bool double_clk)
{
	if (header_wnd_.m_hWnd == 0)
		return;

	CPoint pos(-1, -1);
	GetCursorPos(&pos);
	tree_wnd_.ScreenToClient(&pos);

	UINT flags= 0;
	HTREEITEM item= tree_wnd_.HitTest(pos, &flags);

	if (item == 0 || (flags & (TVHT_ONITEM | TVHT_ONITEMRIGHT)) == 0)
		return;

//	if (tree_wnd_.GetItemState(item, TVIS_SELECTED) == 0)	// if not selected yet, eat this mouse click
//		return;

	int col_count= header_wnd_.GetItemCount();
	int column= -1;
	for (int col= 0; col < col_count; ++col)
	{
		CRect rect;
		header_wnd_.GetItemRect(col, rect);
		if (pos.x >= rect.left && pos.x < rect.right)
		{
			column = col;
			break;
		}
	}

	if (CWnd* wnd= GetParent())
		wnd->PostMessage(double_clk ? NOTIFY_ITEM_DLBCLICKED : NOTIFY_ITEM_CLICKED, WPARAM(item), LPARAM(column));
}


void ExtTreeCtrl::OnKeyDownTreeItem(NMHDR* nmhdr, LRESULT* result)
{
	NMTVKEYDOWN* key_down= reinterpret_cast<NMTVKEYDOWN*>(nmhdr);
	*result = 0;

	if (key_down->wVKey == VK_SPACE)
	{
		if (HTREEITEM item= tree_wnd_.GetSelectedItem())
		{
			*result = 1;
			if (CWnd* wnd= GetParent())
				wnd->PostMessage(NOTIFY_SPACE_PRESSED, WPARAM(item), LPARAM(0));
		}
	}
}


void ExtTreeCtrl::OnSelChangedTreeItem(NMHDR* nmhdr, LRESULT* result)
{
	NMTREEVIEW* tree_view= reinterpret_cast<NMTREEVIEW*>(nmhdr);
	*result = 0;

	if (CWnd* wnd= GetParent())
		wnd->PostMessage(NOTIFY_SEL_CHANGED, WPARAM(tree_view->itemNew.hItem), LPARAM(0));
}


CRect ExtTreeCtrl::GetCellRect(HTREEITEM item, int column, bool screen)
{
	CRect rect;
	tree_wnd_.GetItemRect(item, rect, false);

	if (column >= 0)
	{
		CRect col_rect;
		header_wnd_.GetItemRect(column, col_rect);
		rect.left = col_rect.left;
		rect.right = col_rect.right;
	}

	if (screen)
		tree_wnd_.ClientToScreen(rect);

	return rect;
}


HTREEITEM ExtTreeCtrl::GetSelectedItem() const
{
	if (tree_wnd_.m_hWnd == 0)
		return 0;

	return tree_wnd_.GetSelectedItem();
}


DWORD_PTR ExtTreeCtrl::GetItemData(HTREEITEM item) const
{
	if (tree_wnd_.m_hWnd == 0)
		return 0;

	return tree_wnd_.GetItemData(item);
}


// redraw given item
//
void ExtTreeCtrl::RedrawItem(HTREEITEM item)
{
	tree_wnd_.InvalidateRect(GetCellRect(item, -1, false));
}

void ExtTreeCtrl::RedrawItem(ExtTreeItem* item)
{
	if (item && item->item_)
		RedrawItem(item->item_);
}


void ExtTreeCtrl::SetItemBold(HTREEITEM item, bool bold)
{
	tree_wnd_.SetItemState(item, bold ? TVIS_BOLD : 0, TVIS_BOLD);
}


void ExtTreeCtrl::OnGetDispInfo(NMHDR* nmhdr, LRESULT* result)
{
	NMTVDISPINFO* disp_info= reinterpret_cast<NMTVDISPINFO*>(nmhdr);
	*result = 0;

	// image request?
	if (disp_info->item.mask & (TVIF_SELECTEDIMAGE | TVIF_IMAGE))
	{
		disp_info->item.iImage = 0;

		if (const ExtTreeItem* item= reinterpret_cast<const ExtTreeItem*>(disp_info->item.lParam))
		{
			disp_info->item.iImage = disp_info->item.iSelectedImage = item->GetImageIndex();
		}
	}

	if (disp_info->item.mask & TVIF_TEXT)
	{
		static CString buffer;
		bool bold_dummy;

		disp_info->item.pszText = 0;
		disp_info->item.cchTextMax = 0;

		if (const ExtTreeItem* item= reinterpret_cast<const ExtTreeItem*>(disp_info->item.lParam))
		{
			item->GetDisplayText(0, buffer, bold_dummy);
		}

		disp_info->item.pszText = const_cast<TCHAR*>(static_cast<const TCHAR*>(buffer));
		disp_info->item.cchTextMax = buffer.GetLength();
	}
}


void ExtTreeCtrl::Invalidate()
{
	tree_wnd_.Invalidate();
}

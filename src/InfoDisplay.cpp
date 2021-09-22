/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// InfoDisplay.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "InfoDisplay.h"
#include "MemoryDC.h"
#include "ProfileVector.h"
#include "PhotoInfo.h"
#include "Block.h"
#include "UIElements.h"
#include "GetDefaultGuiFont.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

namespace {
	static const TCHAR* REG_INFO_PANE	= _T("InfoPane");
	static const TCHAR* REG_COL_WIDTH	= _T("ColumnWidth");
}

/////////////////////////////////////////////////////////////////////////////
// InfoDisplay

InfoDisplay::InfoDisplay()
{
	rgb_background_dark_ = /*rgb_background_ =*/ 0;
	text_color_ = dim_text_color_ = 0;
}

InfoDisplay::~InfoDisplay()
{
}


BEGIN_MESSAGE_MAP(InfoDisplay, CListCtrl)
	ON_WM_DESTROY()
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnCustomDraw)
	ON_NOTIFY_REFLECT_EX(LVN_GETDISPINFO, OnGetDispInfo)
	ON_NOTIFY(HDN_ENDTRACK, 0, OnEndTrack)
	ON_WM_CHAR()
	ON_WM_SIZE()
END_MESSAGE_MAP()


BEGIN_MESSAGE_MAP(CustomHeaderCtrl, CHeaderCtrl)
//	ON_WM_DESTROY()
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnCustomDraw)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// InfoDisplay message handlers


bool InfoDisplay::Create(CWnd* parent, const GetTextFn& get_text, COLORREF rgb_back, COLORREF rgb_back_dark)
{
	get_text_ = get_text;
	//rgb_background_ = rgb_back;
	rgb_background_dark_ = rgb_back_dark;
#if 0
	if (wnd_class_.IsEmpty())
		wnd_class_ = AfxRegisterWndClass(0/*CS_VREDRAW | CS_HREDRAW*/, ::LoadCursor(NULL, IDC_ARROW));

	if (!CWnd::CreateEx(0, wnd_class_, _T(""), WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN,
		CRect(0,0,0,0), parent, -1))
		return false;

#endif
	if (!CListCtrl::Create(WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL | LVS_NOSORTHEADER | LVS_OWNERDATA,
		CRect(0,0,0,0), parent, IDC_LIST))
		return false;
	
	SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER | LVS_EX_LABELTIP);
	SetFont(&::GetDefaultGuiFont());
	SetBkColor(rgb_back);
	SetTextBkColor(rgb_back);

	auto hWnd = (HWND)::SendMessage(m_hWnd, LVM_GETHEADER, 0, 0);
	if (hWnd == nullptr)
		return false;

	header_.SubclassWindow(hWnd);
	header_.SetFont(&::GetDefaultGuiFont());

	InsertColumn(0, _T("标签"), LVCFMT_LEFT, Pixels(40));
	InsertColumn(1, _T("名称"), LVCFMT_LEFT, Pixels(130));
	InsertColumn(2, _T("值"), LVCFMT_LEFT, Pixels(150));

	// restore column widths

	{
		int count= header_.GetItemCount();

/*		vector<INT> col_order;
		if (GetProfileVector(frame_->GetRegSection(), REG_VIEW_COL_ORDER, col_order) &&
			count == col_order.size())
		{
			SetColumnOrderArray(count, &col_order.front());
		} */

		std::vector<int16> width_array;
		if (GetProfileVector(REG_INFO_PANE, REG_COL_WIDTH, width_array) && width_array.size() == count)
			for (int i= 0; i < count; ++i)
				SetColumnWidth(i, width_array[i]);
	}

	return true;
}

//const int g_LEFT_MARGIN= 1;

/*
void InfoDisplay::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	MemoryDC mem_dc(dc, this, rgb_background_);

	CString text;
	GetWindowText(text);

	if (text.IsEmpty())
	{
		mem_dc.BitBlt();
		return;
	}

	CRect rect;
	GetClientRect(rect);
	rect.left += g_LEFT_MARGIN;

	mem_dc.SetBkMode(OPAQUE);
	mem_dc.SelectStockObject(DEFAULT_GUI_FONT);
	mem_dc.SetBkColor(rgb_background_);
	mem_dc.SetTextColor(::GetSysColor(COLOR_WINDOWTEXT));

	TEXTMETRIC tm;
	mem_dc.GetTextMetrics(&tm);
	int line_height= tm.tmHeight + tm.tmInternalLeading + tm.tmExternalLeading;

	INT tabs1[]= { 8 * tm.tmAveCharWidth, 33 * tm.tmAveCharWidth };
	INT tabs2[]= { 27 * tm.tmAveCharWidth };

	INT* tabs= two_tabs_ ? tabs1 : tabs2;
	int tab_count= two_tabs_ ? array_count(tabs1) : array_count(tabs2);

	int bold_line_index= 2;

	for (int line= 0; ; ++line)
	{
		CString line;
		if (!AfxExtractSubString(line, text, line))
			break;

		const TCHAR* line= line;

		if (line == bold_line_index)
		{
			mem_dc.SelectObject(&bold_fnt_);
		}
		else if (line - 1 == bold_line_index)
		{
			mem_dc.SelectStockObject(DEFAULT_GUI_FONT);
		}

		if (line.GetLength() > 1)
			mem_dc.TabbedTextOut(rect.left, rect.top, line, line.GetLength() - 1, tab_count, tabs, 0);

		rect.top += line_height;

		if (rect.top >= rect.bottom)
			break;
	}

	mem_dc.BitBlt();
}


BOOL InfoDisplay::OnEraseBkgnd(CDC* dc)
{
	return true;
}


int InfoDisplay::GetTotalHeight()
{
return 1;

	CString text;
	GetWindowText(text);

	// count no of lines
	int line_count= text.Replace(_T('\n'), _T('x'));

	CClientDC dc(this);
	dc.SelectStockObject(DEFAULT_GUI_FONT);
	TEXTMETRIC tm;
	dc.GetTextMetrics(&tm);
	int line_height= tm.tmHeight + tm.tmInternalLeading + tm.tmExternalLeading;

	return line_height * line_count + 1;
}


void InfoDisplay::SetInfo(const String& info)
{
	data_ = info;

	CString text= info.c_str();
	// count no of lines
	int line_count= text.Replace(_T('\n'), _T('x'));

	SetItemCount(line_count);
	Invalidate();
} */


BOOL InfoDisplay::OnGetDispInfo(NMHDR* nmhdr, LRESULT* result)
{
	LV_DISPINFO* disp_info= reinterpret_cast<LV_DISPINFO*>(nmhdr);
	LV_DISPINFO& di= *reinterpret_cast<LV_DISPINFO*>(nmhdr);
	*result = 0;

	if ((di.item.mask & LVIF_TEXT) == 0)
		return false;

	di.item.pszText[0] = _T('\0');

	if (get_text_)
		get_text_(di.item.iItem, di.item.iSubItem, di.item.pszText, di.item.cchTextMax);

	return true;
}


void InfoDisplay::SetInfo(int item_count)
{
	if (item_count == 0)
	{
		EnsureVisible(0, false);
		SetItemCountEx(0, 0);
	}
	else
	{
		int count= GetItemCount();
		int new_count= item_count;
		if (new_count != count)
		{
			int top= GetTopIndex();
			int page= GetCountPerPage();

			if (new_count <= page)
				EnsureVisible(0, false);	// "unscroll", or else redrawing will fail...

			SetItemCountEx(new_count, 0);//LVSICF_NOSCROLL);

			if (new_count > page)
			{
				int bottom= top + page - 1;
				if (bottom > 0)
					EnsureVisible(std::min(bottom, new_count - 1), false);
			}
		}
	}

	Invalidate(false);
}


void InfoDisplay::OnDestroy()
{
	if (CHeaderCtrl* ctrl= &header_)// GetHeaderCtrl())
	{
		int col_count= ctrl->GetItemCount();

/*		vector<INT> col_order;
		col_order.resize(col_count);
		GetColumnOrderArray(&col_order.front(), col_order.size());
		WriteProfileVector(REG_INFO_PANE, REG_COL_WIDTH, col_order); */

		std::vector<int16> width_array;
		width_array.resize(col_count);
		for (int i= 0; i < col_count; ++i)
		{
			width_array[i] = GetColumnWidth(i);
			if (width_array[i] <= 0)
			{
				ASSERT(false);
				width_array[i] = 2;
			}
		}
		WriteProfileVector(REG_INFO_PANE, REG_COL_WIDTH, width_array);
	}

	CListCtrl::OnDestroy();
}


void InfoDisplay::OnCustomDraw(NMHDR* nm_hdr, LRESULT* result)
{
	NMLVCUSTOMDRAW* NM_custom_draw= reinterpret_cast<NMLVCUSTOMDRAW*>(nm_hdr);
	*result = CDRF_DODEFAULT;

	if (NM_custom_draw->nmcd.dwDrawStage == CDDS_PREPAINT)
	{
		*result = CDRF_NOTIFYITEMDRAW;
	}
	else if (NM_custom_draw->nmcd.dwDrawStage == CDDS_ITEMPREPAINT)
	{
		if (NM_custom_draw->nmcd.dwItemSpec & 1)	// odd item?
			NM_custom_draw->clrTextBk = rgb_background_dark_;

		*result = CDRF_NOTIFYSUBITEMDRAW;
	}
	else if (NM_custom_draw->nmcd.dwDrawStage == (CDDS_ITEMPREPAINT | CDDS_SUBITEM))
	{
		// first two columns darker
		if (NM_custom_draw->iSubItem < 2)
			NM_custom_draw->clrText = dim_text_color_;
		else
			NM_custom_draw->clrText = text_color_;
	}
}


void InfoDisplay::OnChar(UINT chr, UINT rep_cnt, UINT flags)
{
	if (chr == VK_ESCAPE)
	{
		// cheesy: main wnd wants to handle Esc
		if (CWnd* wnd= AfxGetMainWnd())
			wnd->SendMessage(WM_COMMAND, ID_ESCAPE);
	}
	else
		CListCtrl::OnChar(chr, rep_cnt, flags);
}


void ResizeLastColumn(CListCtrl& list_wnd)
{
	if (list_wnd.m_hWnd == 0)
		return;

	static bool update= false;

	if (update)
		return;

	Block block(update);

	CHeaderCtrl* header= list_wnd.GetHeaderCtrl();
	if (header == 0)
		return;

	int count= header->GetItemCount();
	if (count <= 0)
		return;

	int last= count - 1;
	CRect rect(0,0,0,0);
	header->GetItemRect(last, rect);

	CRect cl_rect;
	list_wnd.GetClientRect(cl_rect);
	/*if ((list_wnd.GetStyle() & WS_VSCROLL) == 0)
		cl_rect.right -= ::GetSystemMetrics(SM_CXVSCROLL);
	int right= cl_rect.right - 2;

	if (rect.right != right)
	{
		rect.right = right;
		if (rect.Width() > 20)
		{
			list_wnd.SetColumnWidth(last, rect.Width());
			list_wnd.SetColumnWidth(last, rect.Width() + 1);	// stupid, but works; forces horz scrollbar to disappear
		}
	}*/

	rect.right = cl_rect.right;
	if (rect.Width() > 10)
	{
		//list_wnd.SetColumnWidth(last, rect.Width());
		list_wnd.SetColumnWidth(last, rect.Width() - 1);	// stupid, but works; forces horz scrollbar to disappear
		list_wnd.SetColumnWidth(last, rect.Width());
	}
}


void InfoDisplay::OnSize(UINT type, int cx, int cy)
{
	CListCtrl::OnSize(type, cx, cy);

	if (cx > 0 && cy > 0)
		ResizeLastColumn(*this);
}


void InfoDisplay::OnEndTrack(NMHDR* /*nmhdr*/, LRESULT* result)
{
	*result = 0;
	ResizeLastColumn(*this);
}


void InfoDisplay::SetColors(COLORREF backgnd, COLORREF dark_backgnd, COLORREF text, COLORREF dim_text)
{
	rgb_background_dark_ = dark_backgnd;
	text_color_ = text;
	dim_text_color_ = dim_text;
	SetBkColor(backgnd);
	SetTextBkColor(backgnd);
	SetTextColor(text);

	header_.SetColors(::GetSysColor(COLOR_3DFACE), dim_text);//backgnd, dim_text);

	Invalidate();
}

////////////////////////////////////////

void CustomHeaderCtrl::OnCustomDraw(NMHDR* pNMHDR, LRESULT* result)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	*result = CDRF_DODEFAULT;

	if (pNMCD->dwDrawStage == CDDS_PREPAINT)
	{
		CDC* pDC = CDC::FromHandle(pNMCD->hdc);
		CRect rect(0, 0, 0, 0);
		GetClientRect(&rect);
		pDC->FillSolidRect(&rect, background_);

		*result = CDRF_NOTIFYITEMDRAW;
	}
	else if (pNMCD->dwDrawStage == CDDS_ITEMPREPAINT)
	{
		HDITEM hditem;
		TCHAR buffer[MAX_PATH] = { 0 };
		SecureZeroMemory(&hditem, sizeof(HDITEM));
		hditem.mask = HDI_TEXT;
		hditem.pszText = buffer;
		hditem.cchTextMax = MAX_PATH;
		GetItem(static_cast<int>(pNMCD->dwItemSpec), &hditem);
		CDC* pDC = CDC::FromHandle(pNMCD->hdc);
		pDC->SetTextColor(text_color_);
		pDC->SetBkColor(background_);
		CString str(buffer);
		pDC->DrawText(str, CRect(pNMCD->rc), DT_VCENTER | DT_LEFT);
		*result = CDRF_SKIPDEFAULT;
	}
}

void CustomHeaderCtrl::SetColors(COLORREF backgnd, COLORREF text)
{
	background_ = backgnd;
	text_color_ = text;
	Invalidate();
}

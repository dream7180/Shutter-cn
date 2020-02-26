/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// PrintEngine.cpp: implementation of the PrintEngine class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PrintEngine.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

PrintEngine::PrintEngine()
{
	margins_rect_.SetRectEmpty();
	printable_area_rect_.SetRectEmpty();
	page_size_ = CSize(0, 0);
	columns_ = 0;
	show_margins_ = true;
	show_prinatable_area_ = true;
	show_image_space_ = false;
	cur_page_ = 0;
	print_page_number_ = true;
	image_copies_ = 1;
	printing_options_ = None;
	zoom_ = 1.2;
}


PrintEngine::~PrintEngine()
{}


void PrintEngine::SetPageSize(CSize size, const CRect& margins_rect)
{
	page_size_ = size;
	margins_rect_ = margins_rect;
}


void PrintEngine::PrepareDC(CDC& dc, const CRect& device_rect)
{
	if (dc.IsPrinting())
	{
		dc.SetMapMode(MM_LOMETRIC);
		CSize we= dc.GetWindowExt();
		CSize ve= dc.GetViewportExt();

		dc.SetMapMode(MM_ISOTROPIC);
		dc.SetWindowExt(we);
		ve.cy = -ve.cy;
		dc.SetViewportExt(ve);

		// counter physical printer offset
		CPoint phys_offset(dc.GetDeviceCaps(PHYSICALOFFSETX), dc.GetDeviceCaps(PHYSICALOFFSETY));
		dc.SetViewportOrg(-phys_offset);
	}
	else
	{
		dc.SetMapMode(MM_ISOTROPIC);
		dc.SetWindowExt(page_size_);
		dc.SetViewportExt(device_rect.Size());
	}
}


void PrintEngine::SetCurPage(int page)
{
//	if (page < 0 || page >= GetPageCount())
//	{
//		ASSERT(false);
//		cur_page_ = 0;
//	}
//	else
		cur_page_ = page;
}


void PrintEngine::Print(CDC& dc, const CRect& area_rect, VectPhotoInfo& photos, int page)
{
	if (page_size_.cx == 0 || page_size_.cy == 0)
		return;

	if (photos.empty() || columns_ < 1)
		return;

	PrepareDC(dc, area_rect);

	CRect rect= area_rect;

	if (!dc.IsPrinting())
		dc.DPtoLP(rect);

	CPoint zero((rect.Width() - page_size_.cx) / 2, (rect.Height() - page_size_.cy) / 2);
	CRect physical_page_rect(zero, page_size_);

	CRect page_rect= physical_page_rect;

	// check margins
	{
		CRect rect= page_rect;
		rect.DeflateRect(margins_rect_);
		if (rect.Width() <= 0 || rect.Height() <= 0)
			return; // no space left
	}

	if (!PrintFn(dc, page_rect, photos, page))
		return;

	page_rect.DeflateRect(margins_rect_);

	COLORREF rgb_white= RGB(255,255,255);

/*
	int cols= columns_;
	int rows, item_size, offset, line_h;
	if (!CalcDrawingParams(rows, item_size, offset, line_h))
		return;

	int items_per_pages= rows * cols;
	int start= page * items_per_pages;
	if (start >= photos.size())
		return;

	CRect item_rect(CPoint(page_rect.left, page_rect.top), CSize(item_size, item_size));

	CFont fnt;
	fnt.CreateFont(line_h, 0, 0, 0, FW_NORMAL, false, false, false, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS, dc.IsPrinting() ? NONANTIALIASED_QUALITY : ANTIALIASED_QUALITY, DEFAULT_PITCH, _T("Arial"));

	CFont* font= dc.SelectObject(&fnt);
	COLORREF rgb_white= RGB(255,255,255);
	COLORREF rgb_black= RGB(0,0,0);
	dc.SetBkColor(rgb_white);
	dc.SetTextColor(rgb_black);
	dc.SetBkMode(OPAQUE);

	VectPhotoInfo::iterator it= photos.begin() + start;
	VectPhotoInfo::iterator end= photos.end();

	for (int y= 0; y < rows; ++y)
	{
		for (int x= 0; x < cols; ++x)
		{
			(*it)->Draw(&dc, item_rect, 0, dc.IsPrinting() ? PhotoInfo::DRAW_HALFTONE : PhotoInfo::DRAW_FAST);

			CRect text_rect= item_rect;
			text_rect.OffsetRect(0, item_rect.Height());

			dc.DrawText((*it)->name_.c_str(), (*it)->name_.size(), text_rect, DT_CENTER | DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS);

			if (++it == end)
				break;

			item_rect.OffsetRect(offset, 0);
		}

		if (it == end)
			break;

		item_rect.OffsetRect(-cols * offset, offset + line_h);
	}

	if (print_page_number_ || !footer_.empty())
	{
		CRect text_rect= page_rect;
		text_rect.top = page_rect.bottom;
		text_rect.bottom = text_rect.top + 100;

		CFont page_fnt;
		page_fnt.CreateFont(42, 0, 0, 0, FW_NORMAL, false, false, false, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
			CLIP_DEFAULT_PRECIS, dc.IsPrinting() ? NONANTIALIASED_QUALITY : ANTIALIASED_QUALITY, DEFAULT_PITCH, _T("Arial"));
		dc.SelectObject(&page_fnt);

		CString page;
		int pages= GetPageCount(photos.size());
		page.Format(_T("Page %d of %d"), page + 1, pages);

		if (!footer_.empty())
		{
			CRect rect= text_rect;
			if (print_page_number_)
			{
				int w= dc.GetTextExtent(page).cx;
				rect.right -= w + 10;
			}

			if (rect.Width() > 0)
				dc.DrawText(footer_.c_str(), footer_.size(), rect, DT_LEFT | DT_SINGLELINE | DT_NOPREFIX | DT_TOP | DT_PATH_ELLIPSIS);
		}

		if (print_page_number_)
			dc.DrawText(page, text_rect, DT_RIGHT | DT_SINGLELINE | DT_NOPREFIX | DT_TOP | DT_END_ELLIPSIS);
	}

	dc.SelectObject(font);
*/
	if (!dc.IsPrinting() && (show_margins_ || show_prinatable_area_))
	{
		CRect prn_area_rect= printable_area_rect_;
		prn_area_rect.OffsetRect(zero);
		dc.LPtoDP(prn_area_rect);
		dc.LPtoDP(page_rect);
		dc.LPtoDP(physical_page_rect);
		dc.SetMapMode(MM_TEXT);
		dc.SetBkColor(rgb_white);
		dc.SelectStockObject(NULL_BRUSH);

		if (show_prinatable_area_ && !printable_area_rect_.IsRectEmpty())
		{
			CPen pen(PS_DOT, 0, RGB(255,160,160));
			CPen* old_pen= dc.SelectObject(&pen);
			dc.Rectangle(prn_area_rect);
			dc.SelectObject(old_pen);
		}
		if (show_margins_)
		{
			CPen pen(PS_DOT, 0, RGB(192,192,192));
			CPen* old_pen= dc.SelectObject(&pen);
			dc.MoveTo(physical_page_rect.left, page_rect.top); dc.LineTo(physical_page_rect.right, page_rect.top);
			dc.MoveTo(physical_page_rect.left, page_rect.bottom); dc.LineTo(physical_page_rect.right, page_rect.bottom);
			dc.MoveTo(page_rect.left, physical_page_rect.top); dc.LineTo(page_rect.left, physical_page_rect.bottom);
			dc.MoveTo(page_rect.right, physical_page_rect.top); dc.LineTo(page_rect.right, physical_page_rect.bottom);
			dc.SelectObject(old_pen);
		}
	}
}


void PrintEngine::GetDefaultFont(LOGFONT& lf) const
{}

void PrintEngine::SetDefaultFont(const LOGFONT& lf)
{}


void PrintEngine::SetPrintingOptions(PrintingOptions opt)
{
	printing_options_ = opt;
}


void PrintEngine::SetZoom(double zoom)
{
	zoom_ = zoom;
}

double PrintEngine::GetZoom() const
{
	return zoom_;
}

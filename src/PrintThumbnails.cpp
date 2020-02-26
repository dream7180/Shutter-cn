/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// PrintThumbnails.cpp: implementation of the PrintThumbnails class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PrintThumbnails.h"
#include "ExifTags.h"	// date/time formatting
#include "DateTimeUtils.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

enum { INITIAL_FONT_SIZE= -16 };


PrintThumbnails::PrintThumbnails()
{
	// default font
	memset(&font_, 0, sizeof(font_));

	font_.lfHeight = INITIAL_FONT_SIZE;
	font_.lfWidth = 0;
	font_.lfEscapement = 0;
	font_.lfOrientation = 0;
	font_.lfWeight = FW_NORMAL;
	font_.lfItalic = false;
	font_.lfUnderline = false;
	font_.lfStrikeOut = false;
	font_.lfCharSet = DEFAULT_CHARSET;
	font_.lfOutPrecision = OUT_DEFAULT_PRECIS;
	font_.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	font_.lfQuality = ANTIALIASED_QUALITY;
	font_.lfPitchAndFamily = DEFAULT_PITCH;
	_tcscpy(font_.lfFaceName, _T("Arial"));

	printing_options_ = PrintFileNames;
}


PrintThumbnails::~PrintThumbnails()
{}


bool PrintThumbnails::PrintFn(CDC& dc, CRect page_rect, VectPhotoInfo& photos, int page)
{
/*
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
*/
	page_rect.DeflateRect(margins_rect_);

	int cols= columns_;
	int rows, item_size, offset, lineH;
	if (!CalcDrawingParams(rows, item_size, offset, lineH))
		return false;

	int items_per_pages= rows * cols;
	int start= page * items_per_pages;
	if (start >= photos.size())
		return false;

	CRect item_rect(CPoint(page_rect.left, page_rect.top), CSize(item_size, item_size));

	LOGFONT lf= font_;
	lf.lfQuality = dc.IsPrinting() ? NONANTIALIASED_QUALITY : ANTIALIASED_QUALITY;
	lf.lfHeight = lineH * abs(lf.lfHeight) / -INITIAL_FONT_SIZE;	// keeping positive value
	CFont fnt;
	fnt.CreateFontIndirect(&lf);
//	fnt.CreateFont(line_h, 0, 0, 0, FW_NORMAL, false, false, false, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
//		CLIP_DEFAULT_PRECIS, dc.IsPrinting() ? NONANTIALIASED_QUALITY : ANTIALIASED_QUALITY, DEFAULT_PITCH, _T("Arial"));

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
			(*it)->Draw(&dc, CRect(item_rect), 0, 0, 0, 0, 0, dc.IsPrinting() ? PhotoInfo::DRAW_HALFTONE : PhotoInfo::DRAW_FAST);

			CRect text_rect= item_rect;
			text_rect.OffsetRect(0, item_rect.Height());

			if (printing_options_ != None)
			{
				String str;
				if (printing_options_ == PrintDateTime)
					str = ::DateTimeFmt((*it)->GetDateTime(), _T("\n"));
				else
					str = (*it)->GetName();

				dc.DrawText(str.c_str(), str.size(), text_rect, DT_CENTER | DT_NOPREFIX | DT_END_ELLIPSIS);
			}

			if (++it == end)
				break;

			item_rect.OffsetRect(offset, 0);
		}

		if (it == end)
			break;

		item_rect.OffsetRect(-cols * offset, offset + lineH);
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

		CString page_str;
		int pages= GetPageCount(photos.size());
		page_str.Format(_T("Page %d of %d"), page + 1, pages);

		if (!footer_.empty())
		{
			CRect rect= text_rect;
			if (print_page_number_)
			{
				int w= dc.GetTextExtent(page_str).cx;
				rect.right -= w + 10;
			}

			if (rect.Width() > 0)
				dc.DrawText(footer_.c_str(), footer_.size(), rect, DT_LEFT | DT_SINGLELINE | DT_NOPREFIX | DT_TOP | DT_PATH_ELLIPSIS);
		}

		if (print_page_number_)
			dc.DrawText(page_str, text_rect, DT_RIGHT | DT_SINGLELINE | DT_NOPREFIX | DT_TOP | DT_END_ELLIPSIS);
	}

	dc.SelectObject(font);

	return true;

/*
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
			CPen* pen= dc.SelectObject(&pen);
			dc.Rectangle(prn_area_rect);
			dc.SelectObject(pen);
		}
		if (show_margins_)
		{
			CPen pen(PS_DOT, 0, RGB(192,192,192));
			CPen* pen= dc.SelectObject(&pen);
			dc.MoveTo(physical_page_rect.left, page_rect.top); dc.LineTo(physical_page_rect.right, page_rect.top);
			dc.MoveTo(physical_page_rect.left, page_rect.bottom); dc.LineTo(physical_page_rect.right, page_rect.bottom);
			dc.MoveTo(page_rect.left, physical_page_rect.top); dc.LineTo(page_rect.left, physical_page_rect.bottom);
			dc.MoveTo(page_rect.right, physical_page_rect.top); dc.LineTo(page_rect.right, physical_page_rect.bottom);
			dc.SelectObject(pen);
		}
	}
*/
}


int PrintThumbnails::GetPageCount(int items_count) const
{
	ASSERT(items_count >= 0);
	if (items_count == 0)
		return 0;

	int rows, item_size, offset, line_h;
	if (!CalcDrawingParams(rows, item_size, offset, line_h))
		return 0;

	int items_per_pages= rows * columns_;

	if (items_per_pages == 0)
		return 0;

	return (items_count + items_per_pages - 1) / items_per_pages;
}


CSize PrintThumbnails::GetImageSize() const
{
	int rows, item_size, offset, line_h;
	if (!CalcDrawingParams(rows, item_size, offset, line_h))
		return CSize(0, 0);

	return CSize(item_size, item_size);
}


bool PrintThumbnails::CalcDrawingParams(int& rows, int& item_size, int& offset, int& line_h) const
{
	CRect page_rect(CPoint(0, 0), page_size_);
	page_rect.DeflateRect(margins_rect_);

	int cols= columns_;
	int w= page_rect.Width() / (cols + 1);
	if (w < 1)
		return false;

	offset = w + w / (cols - 1);
	line_h = w / 6;
	rows = page_rect.Height() / (offset + line_h);
	if (rows < 1)
		return false;

	item_size = w;

	return true;
}


void PrintThumbnails::GetDefaultFont(LOGFONT& lf) const
{
	lf = font_;
}

void PrintThumbnails::SetDefaultFont(const LOGFONT& lf)
{
	font_ = lf;
}

/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "ImageDraw.h"
#include "resource.h"
#include "UIElements.h"
#include "Color.h"
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
using namespace boost::algorithm;

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


namespace {
	const int SHADOW= 5;
}

Dib ImageDraw::shadow_template_bmp_;
uint8 ImageDraw::shades_of_gray_[28];


void ImageDraw::Draw(const Dib* bmp, CDC* dc, CRect& destination_rect,
					 COLORREF rgb_back, COLORREF rgb_selection, COLORREF rgb_outline,
					 COLORREF rgb_text, COLORREF rgb_text_bk, UINT flags, const String* label)
{
	CRect dest_rect= destination_rect;

	// extra space for selection or drop shadow
	if (flags & (DRAW_SELECTION | DRAW_SHADOW))
		dest_rect.DeflateRect(SHADOW, SHADOW);

	CSize dest_size= dest_rect.Size();
	CRect bmp_rect= dest_rect;
	//bool simple_rescaling= true;
	double scale= 1.0;

	if ((flags & FLIP_SIZE_IF_NEEDED) != 0 && bmp != 0 && bmp->IsValid())
	{
		CSize size_bmp= bmp->GetSize();
		if (dest_size.cx > 0 && dest_size.cy > 0 && size_bmp.cx > 0 && size_bmp.cy > 0)
		{
			if (size_bmp.cx > size_bmp.cy && dest_size.cx < dest_size.cy ||
				size_bmp.cx < size_bmp.cy && dest_size.cx > dest_size.cy)
			{
				// correct it
				flags &= ~NO_RECT_RESCALING;
			}
		}
	}

	if (bmp == 0 || !bmp->IsValid())
	{
		if (flags & DRAW_BACKGND)
			dc->FillSolidRect(dest_rect, rgb_back);
//		return;
	}
	else if ((flags & NO_RECT_RESCALING) == 0)
	{
		CSize bmp_size= bmp->GetSize();
		if (bmp_size.cx < 1 || bmp_size.cy < 1)
		{
			ASSERT(false);
			return;
		}

		if (dest_size.cx <= 0 || dest_size.cy <= 0)
		{
			ASSERT(false);
			return;
		}

		double bmp_ratio= double(bmp_size.cx) / double(bmp_size.cy);
		double dest_ratio= double(dest_size.cx) / double(dest_size.cy);
		double epsillon= 0.01;

		// compare bmp ratio and destination ratio; if same bmp will be simply scalled
		//simple_rescaling = fabs(bmp_ratio - dest_ratio) < epsillon;

//		if (!simple_rescaling)
		{
			// calc how to rescale bmp to fit into dest rect
			double scale_w= double(dest_size.cx) / double(bmp_size.cx);
			double scale_h= double(dest_size.cy) / double(bmp_size.cy);

			/*double*/ scale= MIN(scale_w, scale_h);

			// rescale bmp
			bmp_size.cx = static_cast<LONG>(bmp_size.cx * scale);
			bmp_size.cy = static_cast<LONG>(bmp_size.cy * scale);

			ASSERT(bmp_size.cx <= dest_size.cx);
			ASSERT(bmp_size.cy <= dest_size.cy);
			CSize diff_size= dest_size - bmp_size;

			// center rescalled bitmap in destination rect
			CPoint pos(diff_size.cx / 2, diff_size.cy / 2);

			if (flags & DRAW_BACKGND)
			{
				CSize dest_size= destination_rect.Size();

				// fill areas not covered by bitmap
				if (diff_size.cx > diff_size.cy)
				{
					dc->FillSolidRect(destination_rect.left, destination_rect.top, pos.x, dest_size.cy, rgb_back);
					dc->FillSolidRect(destination_rect.left + pos.x + bmp_size.cx, destination_rect.top, dest_size.cx - pos.x - bmp_size.cx, dest_size.cy, rgb_back);
				}
				else
				{
					dc->FillSolidRect(destination_rect.left, destination_rect.top, dest_size.cx, pos.y, rgb_back);
					dc->FillSolidRect(destination_rect.left, destination_rect.top + pos.y + bmp_size.cy, dest_size.cx, dest_size.cy - pos.y - bmp_size.cy, rgb_back);
				}
			}

			// bitmap destination rect
			bmp_rect = CRect(pos, bmp_size);
			bmp_rect.OffsetRect(dest_rect.TopLeft());
		}
	}

	// draw outline/shadow/selection frame
	if (flags & DRAW_SELECTION)
		DrawOutline(dc, bmp_rect, SELECTION_DISTANCE, SELECTION_THICKNESS, rgb_selection);
	else if (flags & DRAW_SHADOW && bmp != 0)
		DrawShadow(dc, bmp_rect, rgb_back);

	if (flags & DRAW_OUTLINE)
		DrawOutline(dc, bmp_rect, SHADOW, OUTLINE_THICKNESS, rgb_outline);

	if (label)
	{
		CRect text_rect;
		text_rect.left = destination_rect.left;
		text_rect.top = bmp_rect.bottom + SHADOW;
		text_rect.right = destination_rect.right;
		text_rect.bottom = text_rect.top;

		std::vector<String> lines;
		split(lines, *label, is_any_of(L"\n"));

		CSize text_size= dc->GetTextExtent(L"X", 1);//label->c_str(), static_cast<int>(label->size()));

		//int lines= 1;
		//for (size_t i= 0; i < label->size(); ++i)
		//	if ((*label)[i] == L'\n')
		//		lines++;
		text_rect.bottom += text_size.cy;// * static_cast<int>(lines.size());

		UINT DT_flags= DT_CENTER | DT_TOP | DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS;
		int max_width= 0;
		int first_line_width= 0;

		for (size_t i= 0; i < lines.size(); ++i)
		{
			CSize size= dc->GetTextExtent(lines[i].c_str(), static_cast<int>(lines[i].size()));
			if (size.cx > max_width)
				max_width = size.cx;
			if (i == 0)
				first_line_width = size.cx;
		}

		CRect first_line= text_rect;
		if (first_line_width < text_rect.Width())
		{
			int dx = (text_rect.Width() - first_line_width) / 2;
			first_line.OffsetRect(dx, 0);	// center horizontally
			first_line.right = first_line.left + first_line_width;
		}

		CRect calc_rect= text_rect;
		int dx= 0;
		if (max_width < text_rect.Width())
		{
			dx = (text_rect.Width() - max_width) / 2;
			calc_rect.OffsetRect(dx, 0);	// center horizontally
			calc_rect.right = calc_rect.left + max_width;
		}

		int height= text_size.cy * static_cast<int>(lines.size());

//		dc->DrawText(label->c_str(), static_cast<int>(label->length()), calc_rect, DT_flags | DT_CALCRECT);

		//if (text_size.cx < text_rect.Width())
		//{
		//	text_rect.left += (text_rect.Width() - text_size.cx) / 2;
		//	text_rect.right = text_rect.left + text_size.cx;
		//}
		//else
		//	DT_flags |= DT_END_ELLIPSIS;

		//int dx= text_rect.Width() - calc_rect.Width();
		//if (dx <= 0)
		//	calc_rect = text_rect;
		//else
		//	calc_rect.OffsetRect(dx / 2, 0);	// center horizontally

		if (flags & DRAW_SELECTION)
		{
			Gdiplus::Graphics g(*dc);

			g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
			g.TranslateTransform(-0.5f, -0.5f);

			int radius= text_size.cy / 2;	// make sure radius is an integer (for proper placement)
			Gdiplus::RectF area= CRectToRectF(calc_rect.left - radius / 2, calc_rect.top, calc_rect.Width() + radius, height + 1);
			if (dx == 0)	// no space?
			{
				// make round rect tighter
				area.X++;
				area.Width -= 2;
			}
			{
				Gdiplus::GraphicsPath frame;
				RoundRect(frame, area, static_cast<float>(radius));
				Gdiplus::SolidBrush brush(c2c(rgb_text_bk));
				g.FillPath(&brush, &frame);
			}
		}
		else
			dc->FillSolidRect(first_line.left - 1, first_line.top, first_line.Width() + 3, first_line.Height(), rgb_text_bk);

		dc->SetTextColor(rgb_text);
		dc->SetBkColor(rgb_text_bk);
		dc->SetBkMode(TRANSPARENT);

		for (size_t i= 0; i < lines.size(); ++i)
		{
			dc->DrawText(lines[i].c_str(), static_cast<int>(lines[i].length()), text_rect, DT_flags);
			text_rect.OffsetRect(0, text_size.cy);
		}

//		dc->DrawText(label->c_str(), static_cast<int>(label->length()), calc_rect, DT_flags);

		// draw two lines on the sides of text label extending outline
		if ((flags & DRAW_OUTLINE) && !(flags & DRAW_SELECTION))
		{
			int left= first_line.left - OUTLINE_THICKNESS - 1;
			int right= first_line.right + OUTLINE_THICKNESS;

			if (left >= bmp_rect.left - SHADOW)
			{
				dc->FillSolidRect(left, first_line.top, OUTLINE_THICKNESS, first_line.Height() * 2 / 4, rgb_outline);
				dc->FillSolidRect(right, first_line.top, OUTLINE_THICKNESS, first_line.Height() * 2 / 4, rgb_outline);
			}
		}
	}

	if (bmp != 0 && bmp->IsValid())
	{
		// draw an image
		// Note: DrawDibDraw happens to hang some computers; it probably shouldn't be used at all
//TODO: revise this code
//		if (flags & DRAW_DIBDRAW)
//			bmp->DibDraw(dc, bmp_rect, 0);
//		else
			bmp->Draw(dc, bmp_rect, 0, !!(flags & DRAW_HALFTONE), !!(flags & DRAW_WITH_ICM));
//TRACE(L"Bmp rect: %d x %d\n", bmp_rect.Width(), bmp_rect.Height());
	}

	destination_rect = bmp_rect;
}


// return size of image when drawing with given set of flags
//
CRect ImageDraw::GetImageSize(Dib* bmp, const CRect& destination_rect, UINT flags)
{
	CRect dest_rect= destination_rect;

	// extra space for selection or drop shadow
	if (flags & (DRAW_SELECTION | DRAW_SHADOW))
		dest_rect.DeflateRect(SHADOW, SHADOW);

	CSize dest_size= dest_rect.Size();
	CRect bmp_rect= dest_rect;

	double scale= 1.0;

	if (bmp == 0 || !bmp->IsValid())
	{
		return bmp_rect;
	}
	else if ((flags & NO_RECT_RESCALING) == 0)
	{
		CSize bmp_size= bmp->GetSize();
		if (bmp_size.cx < 1 || bmp_size.cy < 1)
		{
			ASSERT(false);
			return bmp_rect;
		}

		if (dest_size.cx <= 0 || dest_size.cy <= 0)
		{
			ASSERT(false);
			return bmp_rect;
		}

		double bmp_ratio= double(bmp_size.cx) / double(bmp_size.cy);
		double dest_ratio= double(dest_size.cx) / double(dest_size.cy);
		double epsillon= 0.01;

		{
			// calc how to rescale bmp to fit into dest rect
			double scale_w= double(dest_size.cx) / double(bmp_size.cx);
			double scale_h= double(dest_size.cy) / double(bmp_size.cy);

			/*double*/ scale= MIN(scale_w, scale_h);

			// rescale bmp
			bmp_size.cx = std::max(static_cast<LONG>(bmp_size.cx * scale), 1L);
			bmp_size.cy = std::max(static_cast<LONG>(bmp_size.cy * scale), 1L);

			ASSERT(bmp_size.cx <= dest_size.cx);
			ASSERT(bmp_size.cy <= dest_size.cy);
			CSize diff_size= dest_size - bmp_size;

			// center rescalled bitmap in destination rect
			CPoint pos(diff_size.cx / 2, diff_size.cy / 2);

			// bitmap destination rect
			bmp_rect = CRect(pos, bmp_size);
			bmp_rect.OffsetRect(dest_rect.TopLeft());
		}
	}

	return bmp_rect;
}



// draw drop shadow
//
void ImageDraw::DrawShadow(CDC* dc, const CRect& rect, COLORREF rgb_back)
{
	if (rect.Width() < SHADOW || rect.Height() < SHADOW)
		return;	// drop shadow needs objects bigger than current rect

	extern RGBQUAD CalcShade(RGBQUAD rgbqColor, float shade);

	if (shadow_template_bmp_.GetBuffer() == 0)
	{
		shadow_template_bmp_.Load(IDB_SHADOW);

		for (int i= 0; i < array_count(shades_of_gray_); ++i)
			shades_of_gray_[i] = shadow_template_bmp_.GetColorTable()[i].rgbRed;
	}

	RGBQUAD back;
	back.rgbRed = GetRValue(rgb_back);
	back.rgbGreen = GetGValue(rgb_back);
	back.rgbBlue = GetBValue(rgb_back);
	back.rgbReserved = 0;

	RGBQUAD* colors= shadow_template_bmp_.GetColorTable();

	for (int i= 0; i < array_count(shades_of_gray_); ++i)
		colors[i] = CalcShade(back, -0.3f * (255 - shades_of_gray_[i]));

	CRect dest_rect= rect;
	dest_rect.InflateRect(1, 1, SHADOW, SHADOW);

	CRect corner_rect= dest_rect;
	corner_rect.left = corner_rect.right - SHADOW;
	corner_rect.top = corner_rect.bottom - SHADOW;

	CSize src_size= shadow_template_bmp_.GetSize();

	CRect src_rect(CPoint(0, 0), src_size);
	src_rect.left = src_rect.right - SHADOW;
	src_rect.top = src_rect.bottom - SHADOW;
	shadow_template_bmp_.Draw(dc, corner_rect, &src_rect, false);

	// sides

	CRect right_rect= dest_rect;
	right_rect.left = right_rect.right - SHADOW;
	right_rect.top += SHADOW;
	right_rect.bottom -= SHADOW;
	shadow_template_bmp_.Draw(dc, right_rect, &CRect(src_size.cx - SHADOW, 8, src_size.cx, 16), false);

	CRect bottom_rect= dest_rect;
	bottom_rect.left += SHADOW;
	bottom_rect.right -= SHADOW;
	bottom_rect.top = bottom_rect.bottom - SHADOW;
	shadow_template_bmp_.Draw(dc, bottom_rect, &CRect(8, src_size.cy - SHADOW, 16, src_size.cy), false);

	CRect left_rect= dest_rect;
	left_rect.right = left_rect.left + SHADOW;
	left_rect.top += SHADOW;
	left_rect.bottom -= SHADOW;
	shadow_template_bmp_.Draw(dc, left_rect, &CRect(0, 8, SHADOW, 16), false);

	CRect top_rect= dest_rect;
	top_rect.left += SHADOW;
	top_rect.right -= SHADOW;
	top_rect.bottom = top_rect.top + SHADOW;
	shadow_template_bmp_.Draw(dc, top_rect, &CRect(8, 0, 16, SHADOW), false);

	// corners

	corner_rect = dest_rect;
	corner_rect.left = corner_rect.right - SHADOW;
	corner_rect.bottom = corner_rect.top + SHADOW;
	shadow_template_bmp_.Draw(dc, corner_rect, &CRect(src_size.cx - SHADOW, 0, src_size.cx, SHADOW), false);

	corner_rect = dest_rect;
	corner_rect.right = corner_rect.left + SHADOW;
	corner_rect.top = corner_rect.bottom - SHADOW;
	shadow_template_bmp_.Draw(dc, corner_rect, &CRect(0, src_size.cy - SHADOW, SHADOW, src_size.cy), false);

	corner_rect = dest_rect;
	corner_rect.right = corner_rect.left + SHADOW;
	corner_rect.bottom = corner_rect.top + SHADOW;
	shadow_template_bmp_.Draw(dc, corner_rect, &CRect(0, 0, SHADOW, SHADOW), false);
}


// draw selection outline
//
void ImageDraw::DrawOutline(CDC* dc, const CRect& rect, int distance, int thickness, COLORREF rgb_selection)
{
	CRect frm= rect;
	const int THICK= thickness;		// frame thickness
	frm.InflateRect(THICK + distance, THICK + distance);

	dc->FillSolidRect(frm.left, frm.top, frm.Width(), THICK, rgb_selection);	// top
	dc->FillSolidRect(frm.left, frm.bottom - THICK, frm.Width(), THICK, rgb_selection);	// bottom
	dc->FillSolidRect(frm.left, frm.top + THICK, THICK, frm.Height() - 2 * THICK, rgb_selection);	// left
	dc->FillSolidRect(frm.right - THICK, frm.top + THICK, THICK, frm.Height() - 2 * THICK, rgb_selection);	// right
}

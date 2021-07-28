/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
#include "Dib.h"


struct ImageDraw
{
	// draw image preserving aspect ratio
	enum { DRAW_FAST= 0, DRAW_HALFTONE= 1, DRAW_DIBDRAW= 2, DRAW_BACKGND= 4, DRAW_SHADOW= 8,
		DRAW_SELECTION= 0x10, DRAW_OUTLINE= 0x20, DRAW_WITH_ICM= 0x40, NO_RECT_RESCALING= 0x80, FLIP_SIZE_IF_NEEDED= 0x100 };

	static void Draw(const Dib* bmp, CDC* dc, CRect& dest_rect, COLORREF rgb_back, COLORREF rgb_selection, COLORREF rgb_outline,
		COLORREF rgb_text, COLORREF rgb_text_bk, UINT flags, const String* pstrLabel);

	//static void DrawShadow(CDC* dc, const CRect& rect, COLORREF rgb_back);

	//static void DrawOutline(CDC* dc, const CRect& rect, int distance, int thickness, COLORREF rgb_selection);

	//enum { SELECTION_DISTANCE= 1, SELECTION_THICKNESS= 3, OUTLINE_THICKNESS= 2 };

	static CRect GetImageSize(Dib* bmp, const CRect& dest_rect, UINT flags);

private:
	static Dib shadow_template_bmp_;
	static uint8 shades_of_gray_[];
};

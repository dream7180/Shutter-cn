/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once

// draw horizontal separator line at the top of UI element; return height
int DrawTopSeparator(CDC& dc, CRect rect, const COLORREF* base_color= 0);
// draw vertical separator line at the right of UI element; return width
int DrawRightSeparator(CDC& dc, CRect rect, const COLORREF* base_color= 0);

// draw horizontal separator line
void DrawLineSeparator(CDC& dc, CRect rect, const COLORREF* base_color= 0);
// draw vertical separator line
void DrawVertLineSeparator(CDC& dc, CRect rect, const COLORREF* base_color= 0);

// draw background of the horizontal panel, with lighter top edge, and darker bottom edge
void DrawPanelBackground(CDC& dc, const CRect& rect, const COLORREF* base_color= 0);
void DrawPanelBackground(CDC& dc, const CRect& rect, bool faint_botom_edge, const COLORREF* base_color);

// draw panel's selected caption
void DrawPanelCaption(CDC& dc, const CRect& rect, const COLORREF* base_color= 0);

// draw sunken round rect
void Draw3DWell(CDC& dc, const CRect& rect);

// draw (typically blue) rounded shape for a photo tag overlay
void DrawTagBackground(CDC& dc, const CRect& rect, COLORREF base, int alpha);

// draw selected item (background only, no text)
void DrawItemSelection(CDC& dc, const CRect& rect, COLORREF base_color, bool disabled, bool hot);

// draw a tick at the edge of 3D well
void DrawTick(CDC& dc, const CRect& well_rect, CSize tick_size, int x_position, bool pointing_down);

// helper functions
extern Gdiplus::RectF CRectToRectF(int x, int y, int w, int h);
extern Gdiplus::RectF CRectToRectF(const CRect& rect);
extern Gdiplus::Color c2c(COLORREF color);
extern double PixelsDbl(double logical_pixels);
extern int Pixels(double logical_pixels);
extern int Pixels(int logical_pixels);
extern int Pixels(CDC& dc, double logical_pixels);
extern CSize Pixels(CDC& dc, CSize logical_pixels);
// resolution in pixels per meter
extern Gdiplus::SizeF GetResolution();
// resolution in dpi
extern Gdiplus::SizeF GetResolutionDpi(CDC* dc = nullptr);

extern void FillRoundRect(CDC& dc, const CRect& rect, float radius, COLORREF fill);
extern void RoundRect(Gdiplus::GraphicsPath& rr, const Gdiplus::RectF& area, float radius);

// draw non-client frame around a control; for use in OnNcPaint
void DrawNCBorder(HDC hdc, const CRect& rect);

// draw resizing grid (dimples)
void DrawResizingGrip(HDC dc, const CRect& rect, COLORREF base, bool is_active);

void FillSolidRect(HDC dc, const CRect& rect, COLORREF clr);
void FillSolidRect(HDC dc, int x, int y, int cx, int cy, COLORREF clr);

/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "UIElements.h"
#include "Color.h"
#include "CtrlDraw.h"
#include "AppColors.h"

extern COLORREF CalcColor(COLORREF rgb_color1, COLORREF rgb_color2, float bias);


COLORREF GetBaseColor(const COLORREF* base_color)
{
	return base_color != nullptr ? *base_color : GetAppColors()[AppColors::Background];
}


bool RectIsEmpty(const CRect& rect)
{
	return rect.Height() <= 0 || rect.Width() <= 0;
}


int DrawTopSeparator(CDC& dc, CRect rect, const COLORREF* base_color)
{
	if (RectIsEmpty(rect))
		return 0;

	auto color = GetAppColors()[AppColors::SecondarySeparator];
//	int height = Pixels(dc, 3);

//	rect.bottom = rect.top + height;
	dc.FillSolidRect(rect, color);

	return rect.Height();
/*
	COLORREF base= GetBaseColor(base_color);

	rect.bottom = rect.top + 1;

	// darker line first
	dc.FillSolidRect(rect, CalcShade(base, -7.0f));
	rect.OffsetRect(0, 1);
	// lighter edge follows
	dc.FillSolidRect(rect, CalcShade(base, 80.0f));
	rect.OffsetRect(0, 1);
	dc.FillSolidRect(rect, CalcShade(base, 40.0f));

	return 3;	// three lines drawn */
}


int DrawRightSeparator(CDC& dc, CRect rect, const COLORREF* base_color)
{
	if (RectIsEmpty(rect))
		return 0;

	COLORREF base= GetBaseColor(base_color);

	rect.right = rect.left + 1;

	// darker line first
	dc.FillSolidRect(rect, CalcShade(base, -7.0f));
	rect.OffsetRect(1, 0);
	// lighter edge follows
	dc.FillSolidRect(rect, CalcShade(base, 80.0f));
	rect.OffsetRect(1, 0);
	dc.FillSolidRect(rect, CalcShade(base, 40.0f));

	return 3;	// three lines drawn
}


void DrawPanelCaption(CDC& dc, const CRect& rect, const COLORREF* base_color)
{
	if (RectIsEmpty(rect))
		return;

	COLORREF base= GetBaseColor(base_color);

	dc.FillSolidRect(rect, base);
/*
	int h= rect.Height();

	CRect line= rect;
	line.bottom = line.top + 1;

	// first two steps nonlinear to enhance 3D effect
	dc.FillSolidRect(line, CalcShade(base, -2.0f));
	line.OffsetRect(0, 1);
	dc.FillSolidRect(line, CalcShade(base, -4.0f));
	line.OffsetRect(0, 1);

	float shadeFrom= -6.0f, shadeTo= -14.0f;
	h -=  2 + 3;

	for (int i= h; i >= 0; --i)
	{
		dc.FillSolidRect(line, CalcShade(base, shadeTo + (shadeFrom - shadeTo) * i / h));
		line.OffsetRect(0, 1);
	}

	// last steps nonlinear to enhance 3D effect
	dc.FillSolidRect(line, CalcShade(base, -16.0f));
	line.OffsetRect(0, 1);
	dc.FillSolidRect(line, CalcShade(base, -25.0f));
	line.OffsetRect(0, 1);
	dc.FillSolidRect(line, CalcShade(base, -36.0f)); */
}


void DrawPanelBackground(CDC& dc, const CRect& rect, const COLORREF* base_color)
{
	DrawPanelBackground(dc, rect, false, base_color);
}


void DrawPanelBackground(CDC& dc, const CRect& rect, bool faint_botom_edge, const COLORREF* base_color)
{
	if (RectIsEmpty(rect))
		return;

	COLORREF base= GetBaseColor(base_color);
	dc.FillSolidRect(rect, base);
/*
	int h= rect.Height();

	CRect line= rect;
	line.bottom = line.top + 1;

	// first two steps nonlinear to enhance 3D effect
	dc.FillSolidRect(line, CalcShade(base, 50.0f));
	line.OffsetRect(0, 1);
	dc.FillSolidRect(line, CalcShade(base, 20.0f));
	line.OffsetRect(0, 1);
	dc.FillSolidRect(line, CalcShade(base, 10.0f));
	line.OffsetRect(0, 1);

	int shade_steps= faint_botom_edge ? 2 : 6;
//	float shadeFrom= 0.0f, shadeTo= 0.0f;
	h -= 3 + shade_steps;
	if (h > 0)
	{
		dc.FillSolidRect(line.left, line.top, line.Width(), h, base);
		line.OffsetRect(0, h);
	}
/*	for (int i= h; i >= 0; --i)
	{
		dc.FillSolidRect(line, CalcShade(base, shadeTo + (shadeFrom - shadeTo) * i / h));
		line.OffsetRect(0, 1);
	} * /

	// last steps nonlinear to enhance 3D effect
	if (faint_botom_edge)
	{
		dc.FillSolidRect(line, CalcShade(base, -1.0f));
		line.OffsetRect(0, 1);
		dc.FillSolidRect(line, CalcShade(base, -3.0f));
	}
	else
	{
		dc.FillSolidRect(line, CalcShade(base, -0.3f));
		line.OffsetRect(0, 1);
		dc.FillSolidRect(line, CalcShade(base, -0.6f));
		line.OffsetRect(0, 1);
		dc.FillSolidRect(line, CalcShade(base, -0.9f));
		line.OffsetRect(0, 1);
		dc.FillSolidRect(line, CalcShade(base, -2.0f));
		line.OffsetRect(0, 1);
		dc.FillSolidRect(line, CalcShade(base, -6.0f));
		line.OffsetRect(0, 1);
		dc.FillSolidRect(line, CalcShade(base, -14.0f));
	}*/
}


void DrawLineSeparator(CDC& dc, CRect rect, const COLORREF* base_color)
{
	if (RectIsEmpty(rect))
		return;

	int h= rect.Height() - 4;

	rect.bottom = rect.top + 1;

	COLORREF base= GetBaseColor(base_color);

	// darker lines first
	dc.FillSolidRect(rect, CalcShade(base, -1.5f));
	rect.OffsetRect(0, 1);
	dc.FillSolidRect(rect, CalcShade(base, -6.0f));
	rect.OffsetRect(0, 1);
	// lighter edge follows
	dc.FillSolidRect(rect, CalcShade(base, 50.0f));
	rect.OffsetRect(0, 1);
	dc.FillSolidRect(rect, CalcShade(base, 35.0f));
	rect.OffsetRect(0, 1);

	if (h > 0)
		dc.FillSolidRect(rect.left, rect.top, rect.Width(), h, base);
}


void DrawVertLineSeparator(CDC& dc, CRect rect, const COLORREF* base_color)
{
	if (RectIsEmpty(rect))
		return;

	int w= rect.Width() - 4;

	rect.right = rect.left + 1;

	COLORREF base= GetBaseColor(base_color);

	// darker lines first
	dc.FillSolidRect(rect, CalcShade(base, -1.5f));
	rect.OffsetRect(1, 0);
	dc.FillSolidRect(rect, CalcShade(base, -6.0f));
	rect.OffsetRect(1, 0);
	// lighter edge follows
	dc.FillSolidRect(rect, CalcShade(base, 50.0f));
	rect.OffsetRect(1, 0);
	dc.FillSolidRect(rect, CalcShade(base, 35.0f));
	rect.OffsetRect(1, 0);

	if (w > 0)
		dc.FillSolidRect(rect.left, rect.top, w, rect.Height(), base);
}


extern void RoundRect(Gdiplus::GraphicsPath& rr, const Gdiplus::RectF& area, float radius)
{
	float dbl= 2.0f * radius;
	rr.StartFigure();

	rr.AddArc(area.GetLeft(), area.GetTop(), dbl, dbl, 180.0f, 90.0f);
	rr.AddArc(area.GetRight() - dbl, area.GetTop(), dbl, dbl, 270.0f, 90.0f);
	rr.AddArc(area.GetRight() - dbl, area.GetBottom() - dbl, dbl, dbl, 0.0f, 90.0f);
	rr.AddArc(area.GetLeft(), area.GetBottom() - dbl, dbl, dbl, 90.0f, 90.0f);

	rr.CloseFigure();
}


extern double PixelsDbl(CDC& dc, double logical_pixels)
{
	auto dpi = GetResolutionDpi(&dc);
	auto physical_pixels = static_cast<double>(logical_pixels * dpi.Width / 96);
	return physical_pixels;
}

extern int Pixels(CDC& dc, double logical_pixels)
{
	return static_cast<int>(std::round(PixelsDbl(dc, logical_pixels)));
}


extern CSize Pixels(CDC& dc, CSize logical_pixels)
{
	auto dpi = GetResolutionDpi(&dc);
	auto physical_pixels = CSize(static_cast<int>(logical_pixels.cx * dpi.Width / 96), static_cast<int>(logical_pixels.cy * dpi.Height / 96));
	return physical_pixels;
}

extern int Pixels(double logical_pixels)
{
	CDC dc;
	dc.CreateIC(_T("DISPLAY"), 0, 0, 0);
	return Pixels(dc, logical_pixels);
}

extern int Pixels(int logical_pixels)
{
	return Pixels(static_cast<double>(logical_pixels));
}


extern Gdiplus::SizeF GetResolution()
{
	auto dpi = GetResolutionDpi();
	// dpi to pix/m
	return Gdiplus::SizeF(100.0f * dpi.Width / 2.54f, 100.0f * dpi.Height / 2.54f);
}

extern Gdiplus::SizeF GetResolutionDpi(CDC* pdc)
{
	CDC dc;
	if (pdc == nullptr)
	{
		dc.CreateIC(_T("DISPLAY"), 0, 0, 0);
		pdc = &dc;
	}
	int log_inch_x = pdc->GetDeviceCaps(LOGPIXELSX);
	int log_inch_y = pdc->GetDeviceCaps(LOGPIXELSY);
	return Gdiplus::SizeF(static_cast<Gdiplus::REAL>(log_inch_x), static_cast<Gdiplus::REAL>(log_inch_y));
}

extern Gdiplus::RectF CRectToRectF(const CRect& rect)
{
	return Gdiplus::RectF(static_cast<float>(rect.left), static_cast<float>(rect.top), static_cast<float>(rect.Width()), static_cast<float>(rect.Height()));
}


extern Gdiplus::RectF CRectToRectF(int x, int y, int w, int h)
{
	return Gdiplus::RectF(static_cast<float>(x), static_cast<float>(y), static_cast<float>(w), static_cast<float>(h));
}


Gdiplus::Color c2c(COLORREF color)
{
	return Gdiplus::Color(GetRValue(color), GetGValue(color), GetBValue(color));
}

void SetAlpha(Gdiplus::Color& c, int alpha)
{
	Gdiplus::ARGB argb= c.GetValue();
	argb &= 0xffffff;
	argb |= alpha << Gdiplus::Color::AlphaShift;
	c.SetValue(argb);
}


COLORREF TopEdge(COLORREF base)		{ return CalcShade(base, -21.0f); }	// darker at the top
COLORREF BottomEdge(COLORREF base)	{ return CalcShade(base, -14.0f); }	// lighter at the bottom

void FillRoundRect(CDC& dc, const CRect& rect, float radius, COLORREF fill)
{
	Gdiplus::Graphics g(dc);

	g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
	g.TranslateTransform(-0.5f, -0.5f);

	Gdiplus::RectF area = CRectToRectF(rect);
	float pix_radius = static_cast<float>(PixelsDbl(dc, radius));

	Gdiplus::GraphicsPath frame;
	RoundRect(frame, area, pix_radius);

	Gdiplus::SolidBrush brush(c2c(fill));
	g.FillPath(&brush, &frame);
}


void Draw3DWell(CDC& dc, const CRect& rect)
{
	//auto background = GetAppColors()[AppColors::Background];
	auto fill = GetAppColors()[AppColors::EditBox];
	FillRoundRect(dc, rect, 3.0f, fill);
/*
	Gdiplus::Graphics g(dc);

	auto background = g_Settings.AppColors()[AppColors::Background];
	auto well_color = g_Settings.AppColors()[AppColors::EditBox];

	g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
	g.TranslateTransform(-0.5f, -0.5f);

	Gdiplus::RectF area= CRectToRectF(rect);
	float radius= Pixels(dc, 3);

	{
		Gdiplus::GraphicsPath frame;
		RoundRect(frame, area, radius);

		COLORREF base = background;// ::GetSysColor(COLOR_3DFACE);
*/
/*
		// light up bottom edge
		Gdiplus::SolidBrush light(c2c(CalcShade(base, 60.0f)));
		g.TranslateTransform(0.0f, 1.0f);
		g.FillPath(&light, &frame);
		g.TranslateTransform(0.0f, -1.0f);

		// draw what become a frame
		Gdiplus::Color top= c2c(TopEdge(base));	// darker at the top
		Gdiplus::Color bottom= c2c(BottomEdge(base));
		Gdiplus::LinearGradientBrush brush(area, top, bottom, Gdiplus::LinearGradientModeVertical);
		*/
		//Gdiplus::SolidBrush brush(c2c(well_color));
		//g.FillPath(&brush, &frame);
//	}
/*
	area.Inflate(-1.0f, -1.0f);
	{
		// draw a fill with shadow at the top
		Gdiplus::GraphicsPath fill;
		RoundRect(fill, area, radius - 1.0f);

		COLORREF base = well_color;// ::GetSysColor(COLOR_WINDOW);
		Gdiplus::Color c(base);
		Gdiplus::LinearGradientBrush brush(area, c, c, Gdiplus::LinearGradientModeVertical);
		Gdiplus::Color colors[]= { c2c(CalcShade(base, -8.0f)), c2c(base), c2c(base) };
		float positions[]= { 0.0f, 0.15f, 1.0f };
		brush.SetInterpolationColors(colors, positions, array_count(colors));

		g.FillPath(&brush, &fill);
	}*/
}

// draw V-shaped triangle pointing down, open at the top
void DrawTick(CDC& dc, const CRect& well_rect, CSize tick_size, int x_position, bool pointing_down)
{
	Gdiplus::Graphics g(dc);

	g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
	g.TranslateTransform(0.0f, -0.5f);

	Gdiplus::RectF area= CRectToRectF(well_rect);

	float x= area.X + x_position;
	float y= area.Y;

	if (!pointing_down)
	{
		g.TranslateTransform(0.0f, area.GetBottom());
		y = 0.0f;
		g.ScaleTransform(1.0f, -1.0f);
	}

	{
		int shadow= 2;
		Gdiplus::PointF shadow_points[]=
		{ Gdiplus::PointF(x - tick_size.cx / 2 - shadow, y), Gdiplus::PointF(x, y + tick_size.cy + shadow), Gdiplus::PointF(x + tick_size.cx / 2 + shadow, y) };

		// draw shadow
		Gdiplus::Color black(20, 0, 0, 0);
		Gdiplus::LinearGradientBrush shade(Gdiplus::PointF(x, y), Gdiplus::PointF(x, y + tick_size.cy + shadow), black, Gdiplus::Color(0, 0, 0, 0));
		g.FillPolygon(&shade, shadow_points, array_count(shadow_points));
	}

	// triangle -> down pointing V-shape
	Gdiplus::PointF points[]=
	{ Gdiplus::PointF(x - tick_size.cx / 2, y), Gdiplus::PointF(x, y + tick_size.cy), Gdiplus::PointF(x + tick_size.cx / 2, y) };

	COLORREF base = GetAppColors()[AppColors::Selection];// ::GetSysColor(COLOR_3DFACE);

	Gdiplus::Color dark= c2c(pointing_down ? TopEdge(base) : BottomEdge(base));
	Gdiplus::SolidBrush outline(dark);
	Gdiplus::SolidBrush fill(c2c(base));

	g.FillPolygon(&outline, points, array_count(points));

	// mask top edge
	g.TranslateTransform(0.0f, -1.0f);
	g.FillPolygon(&fill, points, array_count(points));
}


void DrawTagBackground(CDC& dc, const CRect& rect, COLORREF base, int alpha)
{
	Gdiplus::Graphics g(dc);

	g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
	g.TranslateTransform(-0.5f, -0.5f);

	Gdiplus::RectF area= CRectToRectF(rect);

	float round_corner = static_cast<float>(PixelsDbl(dc, 5));

	Gdiplus::GraphicsPath shape;

	shape.StartFigure();

	// half a circle at left
	shape.AddArc(area.GetLeft(), area.GetTop(), area.Height, area.Height, 90.0f, 180.0f);

	// slight rounding at right
	shape.AddArc(area.GetRight() - round_corner, area.GetTop(), round_corner, round_corner, 270.0f, 90.0f);
	shape.AddArc(area.GetRight() - round_corner, area.GetBottom() - round_corner, round_corner, round_corner, 0.0f, 90.0f);

	shape.CloseFigure();

	Gdiplus::Color top= c2c(CalcShade(base, 5.0f));	// lighter at the top
	Gdiplus::Color bottom= c2c(CalcShade(base, -8.0f));	// darker at the bottom
	SetAlpha(top, alpha);
	SetAlpha(bottom, alpha);
	Gdiplus::LinearGradientBrush brush(area, top, bottom, Gdiplus::LinearGradientModeVertical);
	g.FillPath(&brush, &shape);
}


void DrawItemSelection(CDC& dc, const CRect& rect, COLORREF base_color, bool disabled, bool hot)
{
	if (disabled)
		base_color = CalcNewColorDelta(base_color, -1.0f, 0.0f);	// desaturate selection color for disabled items

	COLORREF base= base_color;
	float top_light= 20.0f;
	float bottom_light= 0.0f;

	if (disabled)
	{
		top_light = 80.0f;		// lighter color for disabled, gray items
		bottom_light = 60.0f;
		base = CalcShade(base, 20.0f);
	}

	if (hot)
		base_color = CalcShade(base_color, 50.0f);		// lighter color for hot items

	Gdiplus::Graphics g(dc);

	g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
	g.TranslateTransform(-0.5f, -0.5f);

	Gdiplus::RectF area= CRectToRectF(rect);
	float radius = static_cast<float>(PixelsDbl(dc, 3));

	// draw outline
	{
//		Gdiplus::GraphicsPath frame;
//		RoundRect(frame, area, radius);

		// draw what become a frame
		Gdiplus::Color top= c2c(CalcShade(base, 10.0f));	// lighter at the top
		Gdiplus::Color bottom= c2c(CalcShade(base, -3.0f));
		Gdiplus::LinearGradientBrush brush(area, top, bottom, Gdiplus::LinearGradientModeVertical);

//		g.FillPath(&brush, &frame);
		g.FillRectangle(&brush, area);
	}
	area.Inflate(-1.0f, -1.0f);
	{
		// draw a fill
		//Gdiplus::GraphicsPath fill;
		//RoundRect(fill, area, radius - 1.0f);

		Gdiplus::Color top= c2c(CalcShade(base_color, top_light));	// lighter at the top
		Gdiplus::Color bottom= c2c(CalcShade(base_color, bottom_light));
		Gdiplus::LinearGradientBrush brush(area, top, bottom, Gdiplus::LinearGradientModeVertical);

//		g.FillPath(&brush, &fill);
		g.FillRectangle(&brush, area);
	}
}


//static void FillSolidRect(HDC hdc, int x, int y, int width, int height, COLORREF c)
//{
//	::SetBkColor(hdc, c);
//	RECT rect;
//	rect.left = x;
//	rect.top = y;
//	rect.right = x + width;
//	rect.bottom = y + height;
//
//	::ExtTextOut(hdc, rect.left, rect.top, ETO_OPAQUE, &rect, 0, 0, 0);
//}


static void DrawRectangle(HDC hdc, RECT rect, COLORREF color)
{
	FillSolidRect(hdc, rect.left, rect.top, 1, rect.bottom - rect.top, color);
	FillSolidRect(hdc, rect.left, rect.top, rect.right - rect.left, 1, color);
	FillSolidRect(hdc, rect.left, rect.bottom - 1, rect.right - rect.left, 1, color);
	FillSolidRect(hdc, rect.right - 1, rect.top, 1, rect.bottom - rect.top, color);
}


void DrawNCBorder(HDC hdc, const CRect& rect)
{
	if (!hdc)
		return;

	HWND hwnd= ::WindowFromDC(hdc);

	// draw scrollbar
//	::DefWindowProc(hwnd, iMessage, wParam, lParam);

	//RECT rect;
	//GetWindowRect(hwnd, &rect);
	//ScreenToClient(hwnd, (POINT*)&rect);
	//ScreenToClient(hwnd, (POINT*)&rect + 1);
	//::OffsetRect(&rect, -rect.left, -rect.top);

	int state= EPSN_NORMAL;
	if (!::IsWindowEnabled(hwnd))
		state = EPSN_DISABLED;
	else if (::GetFocus() == hwnd)
		state = EPSN_FOCUSED;

	if (!CtrlDraw::DrawThemedElement(hdc, rect, L"EDIT", EP_EDITBORDER_NOSCROLL, state, 2))
	{
		COLORREF frame= CalcColor(::GetSysColor(COLOR_3DFACE), ::GetSysColor(COLOR_BTNSHADOW), 0.3f);
		DrawRectangle(hdc, rect, frame);
		CRect r= rect;
		r.DeflateRect(1, 1);
		DrawRectangle(hdc, r, ::GetSysColor(COLOR_WINDOW));
	}

//	::ReleaseDC(MainHWND(), hdc);

/*
	Gdiplus::Graphics g(hdc);

	COLORREF frame_clr= ::GetSysColor(COLOR_BTNSHADOW);
//::FillSolidRect(hdc, rect.left, rect.top, 1, rect.Height(), frame);
//::FillSolidRect(hdc, rect.left, rect.top, rect.Width(), 1, frame);
//::FillSolidRect(hdc, rect.left, rect.bottom - 1, rect.Width(), 1, frame);
//::FillSolidRect(hdc, rect.right - 1, rect.top, 1, rect.Height(), frame);

	COLORREF corner= CalcColor(frame_clr, ::GetSysColor(COLOR_3DFACE), 0.5);

//		::FillSolidRect(hdc, rect.left, rect.top, 1, 1, corner);
	g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);

	Gdiplus::RectF area= CRectToRectF(rect);
	Gdiplus::RectF frame_rect= area;
	frame_rect.Width--;
	frame_rect.Height--;
	float radius= 1.1f;

	Gdiplus::GraphicsPath frame;
	RoundRect(frame, frame_rect, radius);

	Gdiplus::Color top= c2c(frame_clr);	// darker at the top
	Gdiplus::Color bottom= c2c(corner);
	Gdiplus::LinearGradientBrush brush(area, top, bottom, Gdiplus::LinearGradientModeVertical);
	Gdiplus::Color colors[]= { top, bottom, bottom };
	float positions[]= { 0.0f, 0.05f, 1.0f };
	brush.SetInterpolationColors(colors, positions, array_count(colors));
	Gdiplus::Pen pen(&brush);

	g.DrawPath(&pen, &frame);
*/
}


void FillSolidRect(HDC dc, int x, int y, int cx, int cy, COLORREF clr)
{
	::SetBkColor(dc, clr);
	CRect rect(x, y, x + cx, y + cy);
	::ExtTextOut(dc, 0, 0, ETO_OPAQUE, rect, 0, 0, 0);
}


void FillSolidRect(HDC dc, const CRect& rect, COLORREF clr)
{
	FillSolidRect(dc, rect.left, rect.top, rect.Width(), rect.Height(), clr);
}


void DrawDimple(HDC dc, const CRect& rect, COLORREF rgb_light, COLORREF rgb_dark)
{
	CRect grip= rect;
	FillSolidRect(dc, grip, rgb_light);
	grip.OffsetRect(-1, -1);
	FillSolidRect(dc, grip, rgb_dark);
}


void DrawResizingGrip(HDC dc, const CRect& rect, COLORREF base, bool is_active)
{
	const int size= 3;
	CRect grip(0, 0, size - 1, size - 1);
	grip.OffsetRect(rect.left, rect.top);
	COLORREF rgb_light= CalcShade(base, is_active ? 40.0f : 80.0f);
	COLORREF rgb_dark= CalcShade(base, is_active ? -20.0f : -12.0f);

	int dimples= rect.Height() / size;

	// use odd number of dimples
	if ((dimples & 1) == 0)
		dimples--;

	if (dimples <= 0)
		return;

	// center vertically
	grip.OffsetRect(0, (rect.Height() - dimples * size + 1) / 2);

	for (int i= 0; i < dimples; ++i)
	{
		if (grip.bottom + grip.Height() >= rect.bottom)
			break;

		DrawDimple(dc, grip, rgb_light, rgb_dark);
		grip.OffsetRect(size, size);
		DrawDimple(dc, grip, rgb_light, rgb_dark);
		grip.OffsetRect(-size, size);
	}
	DrawDimple(dc, grip, rgb_light, rgb_dark);
}

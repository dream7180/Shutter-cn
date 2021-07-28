/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// Histogram.cpp: implementation of the Histogram class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "Histogram.h"
#include "Dib.h"
#include "CtrlDraw.h"
#include "Color.h"
#include <math.h>
#include "GetDefaultGuiFont.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
//#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Histogram::Histogram()
{
	rgb_fore_ = rgb_back_ = RGB(255,255,255);
	min_edge_ = max_edge_ = -1;
	histogram_rect_.SetRectEmpty();
	logarithmic_ = false;
	using_luminosity_and_rgb_ = true;
}

Histogram::~Histogram()
{}


static double Log(double x)
{
	return log(x + 1.0);
}


std::vector<uint32>* Histogram::GetChannelData(int channel)
{
	switch (channel)
	{
	case 0:		return &lum_;
	case 1:		return &red_;
	case 2:		return &green_;
	case 3:		return &blue_;
	case 4:		return &rgb_;
	default:
		ASSERT(false);
		return 0;
	}
}


void Histogram::DrawLines(CDC* dc, CPoint start, int width, int height, int channel)
{
	--start.y;
	dc->MoveTo(start);

	CPoint pos(start);

	const std::vector<uint32>* v= GetChannelData(channel);

	if (v == 0)
		return;

	--width;
	--height;

	double max_val= max_val_;
	if (logarithmic_)
		max_val = Log(max_val);
	double scale= height / max_val;

	const int size= static_cast<int>(v->size());

	// draw histogram lines
	for (int i= 0; i < size; ++i)
	{
		pos.x = start.x + width * (i + 1) / size;

		double val= (*v)[i];
		if (logarithmic_)
			val = Log(val);
		uint32 h= static_cast<uint32>(val * scale);
		if (h > height)
			h = height;

		dc->LineTo(pos.x, pos.y - h);
	}
}


void Histogram::DrawBars(CDC* dc, CPoint start, int width, int height, int channel, COLORREF rgb_bar, bool stretch_bar)
{
	dc->MoveTo(start);

	CPoint pos(start);

	const std::vector<uint32>* v= GetChannelData(channel);

	if (v == 0)
		return;

	ASSERT(!v->empty());

	uint32 max= stretch_bar ? max_vals_[channel] : max_val_;

	double max_val= max;
	if (logarithmic_)
		max_val = Log(max_val);
	double scale= height / max_val;

	int size= static_cast<int>(v->size());

	// draw histogram bars
	for (int i= 0; i < size; ++i)
	{
		pos.x = start.x + width * i / size;
		int copy= start.x + width * (i + 1) / size;
		int bar_width= copy - pos.x;
		if (bar_width > 0)
		{
			double val= (*v)[i];
			if (logarithmic_)
				val = Log(val);
			uint32 h= static_cast<uint32>(val * scale);
			if (h > height)
				h = height;
			dc->FillSolidRect(pos.x, pos.y - h, bar_width, h, rgb_bar);
		}
	}
}

#ifdef USE_GDI_PLUS		//====================================================================================

static Gdiplus::Matrix* CreateMapping(const Gdiplus::Rect& from, const Gdiplus::Rect& to)
{
	Gdiplus::Point parallelogram[3]=
	{
		Gdiplus::Point(to.GetLeft(), to.GetBottom()),
		Gdiplus::Point(to.GetRight(), to.GetBottom()),
		Gdiplus::Point(to.GetLeft(), to.GetTop())
	};

	return new Gdiplus::Matrix(from, parallelogram);
}


static float VertPixels(Gdiplus::Matrix* transform, float pixels)
{
	Gdiplus::PointF p[]= { Gdiplus::PointF(0.0f, 0.0f), Gdiplus::PointF(0.0f, pixels) };

	if (Gdiplus::Matrix* m= transform->Clone())
	{
		m->Invert();
		m->TransformPoints(p, array_count(p));
		delete m;
	}

	return p[1].Y - p[0].Y;
}


static void DrawHistogram(Gdiplus::Graphics* g, Gdiplus::Rect range, const std::vector<uint32>& histogram, bool logarithmic, Gdiplus::Rect area, Gdiplus::Color base_color, int alpha_top, int alpha_btm, Gdiplus::Matrix* transform)
{
//	Gdiplus::Rect range= histogram.Range;

	const int count= range.Width + 3;
	std::vector<Gdiplus::PointF> points(count);
	std::vector<BYTE> types(count);

	int i= 1;
//	float val= 0.0f;
	for (int x= range.GetLeft(); x <= range.GetRight(); ++x)
	{
		float val= static_cast<float>(histogram[x]);
		if (logarithmic)
			val = static_cast<float>(Log(val));
		points[i] = Gdiplus::PointF(static_cast<float>(x), val);
		types[i] = static_cast<BYTE>(Gdiplus::PathPointTypeLine);
		i++;
	}
	// starting and ending points on the floor level to make fill look good
	points[0] = Gdiplus::PointF(range.GetLeft(), 0.0f);
	types[0] = (byte)Gdiplus::PathPointTypeStart;
	points[i] = Gdiplus::PointF(range.GetRight(), 0.0f);
	types[i] = (byte)Gdiplus::PathPointTypeLine;

	Gdiplus::Region clip;
	g->GetClip(&clip);
	g->SetClip(area); //Gdiplus::Region(area);
	g->SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);

	Gdiplus::GraphicsPath path(&points.front(), &types.front(), count);
	{
		Gdiplus::Matrix m;
		g->GetTransform(&m);

		float angle= 270.0f;	// linear gradient brush angle

		g->SetTransform(transform);

		float pixels= VertPixels(transform, 0.5f);

		BYTE red= base_color.GetRed();
		BYTE green= base_color.GetGreen();
		BYTE blue= base_color.GetBlue();

		// half a pixel shift for interior fill to align it to the pixel grid
		g->TranslateTransform(0.0f, pixels);

		// fill interior
		{
			Gdiplus::LinearGradientBrush br(range, Gdiplus::Color(alpha_top, red,green,blue), Gdiplus::Color(alpha_btm, red,green,blue), angle);
			g->FillPath(&br, &path);
		}

		// move lower still to draw relief outline
		g->TranslateTransform(0.0f, pixels);

		{
			COLORREF white= RGB(255,255,255);
			COLORREF base= base_color.ToCOLORREF();
			COLORREF from= CalcNewColor(base, white, 0.90f);
			COLORREF to= CalcNewColor(base, white, 0.50f);

			Gdiplus::LinearGradientBrush br(range, Gdiplus::Color(GetRValue(from), GetGValue(from), GetBValue(from)),
				Gdiplus::Color(GetRValue(to), GetGValue(to), GetBValue(to)), angle);
			Gdiplus::Pen pen(&br, 0.0f);
			g->DrawPath(&pen, &path);
		}

		// restore original location
		g->TranslateTransform(0, -2.0f * pixels);

		// and draw histogram outline
		{
			Gdiplus::LinearGradientBrush br(range, Gdiplus::Color(120, red,green,blue), Gdiplus::Color(red,green,blue), angle);
			Gdiplus::Pen pen(&br, 0.0f);
			g->DrawPath(&pen, &path);
		}

		g->SetTransform(&m);
	}

	g->SetSmoothingMode(Gdiplus::SmoothingModeDefault);
	g->SetClip(&clip);
}

#endif	//=================================================================================================


void Histogram::DrawOverlaidBars(CDC* dc, CPoint start, int width, int bar_height)
{
	dc->MoveTo(start);

	CPoint pos(start);

	uint32 max= /*stretch_bar ? max_vals_[channel] :*/ max_val_;

	if (red_.empty() || red_.size() != green_.size() || green_.size() != blue_.size())
	{
		ASSERT(false);
		return;
	}

	int size= static_cast<int>(red_.size());

	double height= bar_height;
	double max_val= max;
	if (logarithmic_)
		max_val = Log(max_val);
	double scale= height / max_val;

	// draw histogram bars
	for (int index= 0; index < size; ++index)
	{
		pos.x = start.x + width * index / size;
		int copy= start.x + width * (index + 1) / size;

		int bar_width= copy - pos.x;
		if (bar_width <= 0)
			continue;

		double vals[3]=	{ red_[index], green_[index], blue_[index] };

		if (logarithmic_)
		{
			vals[0] = Log(vals[0]);
			vals[1] = Log(vals[1]);
			vals[2] = Log(vals[2]);
		}

		vals[0] *= scale;
		vals[1] *= scale;
		vals[2] *= scale;

		if (vals[0] > height)	vals[0] = height;
		if (vals[1] > height)	vals[1] = height;
		if (vals[2] > height)	vals[2] = height;

		COLORREF rgb[3]= { RGB(255,0,0), RGB(0,255,0), RGB(0,0,255) };

		// sort
		if (vals[1] < vals[0])		{ std::swap(vals[0], vals[1]);	std::swap(rgb[0], rgb[1]); }
		if (vals[2] < vals[1])		{ std::swap(vals[1], vals[2]);	std::swap(rgb[1], rgb[2]); }
		if (vals[2] < vals[0])		{ std::swap(vals[0], vals[2]);	std::swap(rgb[0], rgb[2]); }
		if (vals[1] < vals[0])		{ std::swap(vals[0], vals[1]);	std::swap(rgb[0], rgb[1]); }

		uint32 y_from= 0;
		for (int i= 0; i < array_count(vals); ++i)
		{
			uint32 y= static_cast<uint32>(vals[i]);
			if (y == 0)
				continue;

			COLORREF clr= 0;

			if (i == 0)	// all three together
				clr = RGB(222,222,222);
			else		// overlay bars
				for (int j= i; j < 3; ++j)
					clr |= rgb[j];

			uint32 h= y - y_from;

			if (h > 0)
				dc->FillSolidRect(pos.x, pos.y - y, bar_width, h, clr);

			y_from = y;
		}
	}
}


static void DrawSliderArrow(CDC* dc, int x, int y, COLORREF color, int height, bool point_up)
{
	if (!point_up)
		y--;

	int width= 1;
	for (int i= 0; i < height; ++i)
	{
		dc->FillSolidRect(x, y, width, 1, color);
		y += point_up ? 1 : -1;
		x--;
		width += 2;
	}
}



void Histogram::Draw(CDC* dc, CRect rect, ChannelSel channels, UINT flags)
{
	if ((flags & NO_ERASE_BACKGND) == 0)
		dc->FillSolidRect(rect, rgb_back_);
	/*
	LOGFONT lf;
	HFONT hfont = static_cast<HFONT>(::GetStockObject(DEFAULT_GUI_FONT));
	::GetObject(hfont, sizeof(lf), &lf);
	//lf.lfQuality = ANTIALIASED_QUALITY;
	//lf.lfHeight += 1;
	_tcscpy(lf.lfFaceName, _T("Tahoma"));
	CFont _font;
	_font.CreateFontIndirect(&lf);
	dc->SelectObject(&_font);
	*/
	dc->SelectObject(&GetDefaultGuiFont());
	//dc->SelectStockObject(DEFAULT_GUI_FONT);

	CSize text_size= dc->GetTextExtent(_T("0.00%"), 5);
	bool draw_arrows= (flags & DRAW_SLIDER_ARROWS) != 0;
	const int ARROW= draw_arrows ? 5 : 0;
	const int LEFT= flags & DRAW_PERCENTAGE ? text_size.cx + 5 : 1 + (draw_arrows ? 1 : 0); //text_size.cx / 4;
	const int TOP= ARROW + (flags & NO_TOP_SPACE ? 1 : text_size.cy / 2 + 1);
	const int BOTTOM= flags & NO_BOTTOM_SPACE ? 1/*text_size.cy*/ : text_size.cy * 3 / 2;

	int height= rect.Height() - TOP - BOTTOM - 2 * ARROW;

	if (flags & DRAW_LABELS)
		height -= text_size.cy;

	// TODO: messed up
	if (flags & DRAW_GRADIENT)
		height -= 12;

	int width= rect.Width() - 2 * LEFT;

	if (height <= 0 || width <= 0)
		return;

	CPoint start= CPoint(LEFT, height + TOP) + rect.TopLeft();

	bool lines= true;
	bool overlayed= false;
	bool lum_rgb= channels == LumRGB;
	int channel= 0;
	COLORREF rgb_bar= RGB(0,0,0);

	switch (channels)
	{
	case LumRGB:		// Lum+RGB
	case RGBLines:		// RGB
		break;
	case RGB:			// RGB (avg)
		channel = 4;
		lines = false;
		break;
	case LUM:			// Luminance (bars)
		lines = false;
		rgb_bar = RGB(96, 96, 96);
		break;
	case RED:			// R
		lines = false;
		channel = 1;
		rgb_bar = RGB(128, 0, 0);
		break;
	case GREEN:			// G
		lines = false;
		channel = 2;
		rgb_bar = RGB(0, 76, 0);
		break;
	case BLUE:			// B
		lines = false;
		channel = 3;
		rgb_bar = RGB(0, 0, 128);
		break;
	case RGBOverlaid:	// RGB overlaid bars
		lines = false;
		overlayed = true;
		break;
	default:
		ASSERT(false);
		return;
	}

	COLORREF color_table[]= { RGB(255,255,255), RGB(255,0,0), RGB(0,255,0), RGB(0,0,255), RGB(255,255,255) };
	int labelY_pos= rect.bottom;

	if (flags & DRAW_GRADIENT)
	{
		// color stripe
		rect.left = start.x;
		rect.top = start.y + 5;
		if (draw_arrows)
			rect.top += 1;
		rect.right = start.x + width;
		rect.bottom = rect.top + 12;

		if (!lines && channels != RGBOverlaid)
		{
			rect.bottom = rect.top + 6;
			PaintStripe(dc, rect, color_table[channel]);
		}
		else
		{
			int step= 3;//lum_rgb ? 3 : 4;
			for (int i= /*lum_rgb ? 0 :*/ 1; i < 4; ++i)
			{
				rect.bottom = rect.top + step;
				PaintStripe(dc, rect, color_table[i]);
				rect.top += step;
			}
		}

		labelY_pos = rect.bottom + 2;
	}

	// field
	dc->FillSolidRect(start.x, start.y - height, width, height, rgb_fore_);

	histogram_rect_ = CRect(CPoint(start.x, start.y - height), CSize(width, height));

	// draw gray frame
	if ((flags & NO_FRAME) == 0)
	{
		CRect frame_rect(CPoint(start.x - 1, start.y - height - 1), CSize(width + 2, height + 2));
		CtrlDraw::DrawBorder(*dc, frame_rect);
	}

	COLORREF lines_color= RGB(192,192,192);

	// draw X axis under the histogram plot
	if (flags & DRAW_BASE_AXIS)
	{
		CPen pen(PS_DOT, 0, CalcShade(rgb_fore_, -10.0f));
		CPen* old= dc->SelectObject(&pen);
		dc->SetBkColor(rgb_fore_);
		dc->MoveTo(start.x, start.y);
		dc->LineTo(start.x + width, start.y);// - height);
		dc->SelectObject(old);
	}

	// draw labels and vert lines (zones)
	dc->SetTextColor(RGB(0,0,0));
	CPoint text(start.x, labelY_pos);
	// TODO: this 'x' positioning is messed up
	CRect text_rect(text.x - text_size.cx / 2 + width / 5, text.y, text.x + text_size.cx / 2, text.y + text_size.cy);
	UINT txt_flags= DT_CENTER | DT_TOP | DT_SINGLELINE | DT_NOCLIP;
	CPen penGrayDots(PS_DOT, 0, lines_color);
	CPen* old= dc->SelectObject(&penGrayDots);
	int zone= 1;
	for (int label= 0; label <= 255; label += 51)
	{
		int label_pos= label == 256 ? 255 : label;
		int x_pos= start.x + label_pos * (width - 1) / 255;

		if (label > 0 && (flags & DRAW_LABELS))
		{
			oStringstream ost;
			ost << _T("zone ") << zone++;
			dc->SetBkColor(rgb_back_);
			String str= ost.str();
			dc->SetBkMode(TRANSPARENT);

			dc->DrawText(str.c_str(), static_cast<int>(str.size()), text_rect, txt_flags);
			text_rect += CSize(width / 5, 0);
		}

		if (label != 0 && label != 255)
		{
			dc->SetBkColor(rgb_fore_);
			dc->MoveTo(x_pos, start.y - 1);
			dc->LineTo(x_pos, start.y - 1 - height);
		}
	}

	if (flags & DRAW_PERCENTAGE)
	{
		double top= double(max_val_) * 100.0 / double(total_pixels_);
		UINT txt_flags = DT_RIGHT | DT_VCENTER | DT_SINGLELINE | DT_NOCLIP;
		double vals[]= { top, top / 2.0, 0.0 };
		int heights[]= { height, height / 2, 0 };
		for (int i = 0; i < 3; ++i)
		{
			oStringstream ost;
			ost.precision(vals[i] >= 1.0 ? 3 : 2);
			ost << vals[i] << _T("%");
			CPoint text(start + CPoint(0, -heights[i] - text_size.cy / 2));
			CRect text_rect(text.x - 36, text.y, text.x - 4, text.y + text_size.cy);
			dc->DrawText(ost.str().c_str(), static_cast<int>(ost.str().size()), text_rect, txt_flags);
		}
	}

	if (red_.empty() || green_.empty() || blue_.empty() || lum_.empty() || rgb_.empty())
	{
		//
	}
	else
	{
		if (lines)
		{
			int first= lum_rgb ? 0 : 1;
			for (int chnl= 3; chnl >= first; --chnl)
			{
				CPen penLine(PS_SOLID, 0, chnl == 0 ? RGB(0,0,0) : color_table[chnl]);
				CPen* old= dc->SelectObject(&penLine);
				Histogram::DrawLines(dc, start, width, height, chnl);
				dc->SelectObject(old);
			}
		}
		else
		{
			bool streatch_bar= !!(flags & STRETCH_EACH_BAR);
			if (overlayed)
			{
#ifdef USE_GDI_PLUS
for (int chnl= 1; chnl <= 3; ++chnl)
{
	const std::vector<uint32>* histogram= GetChannelData(chnl);
	if (histogram == 0 || histogram->empty())
		continue;

	uint32 max= max_val_;
	if (logarithmic_)
		max = Log(max);

	Gdiplus::Rect range(0, 0, 255, max + 1);

	Gdiplus::Rect area(start.x, start.y - height, width, height);

	Gdiplus::Graphics g(*dc);

	std::auto_ptr<Gdiplus::Matrix> transform(CreateMapping(range, area));

	COLORREF color= color_table[chnl];

	DrawHistogram(&g, range, *histogram, logarithmic_, area, Gdiplus::Color(GetRValue(color),GetGValue(color),GetBValue(color)), 150, 10, transform.get());
}
#else
				DrawOverlaidBars(dc, start, width, height);
#endif
			}
			else
			{
#ifdef USE_GDI_PLUS
if (const std::vector<uint32>* histogram= GetChannelData(channel))
{
//	uint32 max= std::accumulate(histogram->begin(), histogram->end(), 0);

	uint32 max= max_vals_[channel];
	if (logarithmic_)
		max = Log(max);

	Gdiplus::Rect range(0, 0, 255, max + 1);

	Gdiplus::Rect area(start.x, start.y - height, width, height);

	Gdiplus::Graphics g(*dc);

	std::auto_ptr<Gdiplus::Matrix> transform(CreateMapping(range, area));

	DrawHistogram(&g, range, *histogram, logarithmic_, area, Gdiplus::Color(GetRValue(rgb_bar),GetGValue(rgb_bar),GetBValue(rgb_bar)), 100, 200, transform.get());

//	g; detach?
}
//else
#else
				DrawBars(dc, start, width, height, channel, rgb_bar, streatch_bar);
#endif
			}
		}
	}

	if (min_edge_ >= 0 && min_edge_ < 256)
	{
		COLORREF rgbSLIDER= RGB(96,96,96);
//		CPen penGray(PS_DOT, 0, rgbSLIDER);
//		dc->SelectObject(&penGray);

		int x_pos= start.x + min_edge_ * (width - 1) / 255;
		dc->FillSolidRect(x_pos, start.y - height, 1, height, rgbSLIDER);
//		dc->SetBkColor(rgb_fore_);
//		dc->SetBkMode(OPAQUE);
//		dc->MoveTo(x_pos, start.y);
//		dc->LineTo(x_pos, start.y - height);

		if (draw_arrows)
		{
			DrawSliderArrow(dc, x_pos, start.y, rgbSLIDER, ARROW, true);
			DrawSliderArrow(dc, x_pos, start.y - height, rgbSLIDER, ARROW, false);
		}

		if (max_edge_ >= 0 && max_edge_ < 256)
		{
			x_pos = start.x + max_edge_ * (width - 1) / 255;
			dc->FillSolidRect(x_pos, start.y - height, 1, height, rgbSLIDER);
//			dc->MoveTo(x_pos, start.y);
//			dc->LineTo(x_pos, start.y - height);

			if (draw_arrows)
			{
				DrawSliderArrow(dc, x_pos, start.y, rgbSLIDER, ARROW, true);
				DrawSliderArrow(dc, x_pos, start.y - height, rgbSLIDER, ARROW, false);
			}
		}
	}

	dc->SelectObject(old);
}


void Histogram::BuildHistogram(const Dib* bmp, const CRect* prectSelection/*= 0*/)
{
	const size_t COUNT= 256;
	red_.resize(COUNT, 0);
	green_.resize(COUNT, 0);
	blue_.resize(COUNT, 0);
	lum_.resize(COUNT, 0);
	rgb_.resize(COUNT, 0);

	// erase previous values
	for (size_t i= 0; i < COUNT; ++i)
		red_[i] = green_[i] = blue_[i] = lum_[i] = rgb_[i] = 0;

	max_val_ = 0;

	if (bmp == 0 || (bmp->GetBitsPerPixel() != 24 && bmp->GetBitsPerPixel() != 8))
	{
//		TRACE(_T("Histogram not supported for this dib\n"));
		return;
	}
	bool grayscale= bmp->GetBitsPerPixel() == 8;
	int step= grayscale ? 1 : 3;

	CRect selection_rect;
	if (prectSelection == 0)
	{
		selection_rect.left = selection_rect.top = 0;
		selection_rect.right = bmp->GetWidth() - 1;
		selection_rect.bottom = bmp->GetHeight() - 1;
	}
	else
		selection_rect = *prectSelection;

	int line_size= bmp->GetBytesPerLine() / step;
	pixels_count_ = bmp->GetWidth() * bmp->GetHeight();

	int top= std::max<int>(0, selection_rect.top);
	if (top > bmp->GetHeight())
		top = bmp->GetHeight();

	int bottom= std::min<int>(selection_rect.bottom + 1, bmp->GetHeight());

	int left= std::max<int>(selection_rect.left, 0);
	if (left > bmp->GetWidth())
		left = bmp->GetWidth();
	int right= std::min<int>(selection_rect.right + 1, bmp->GetWidth());

	for (int line= top; line < bottom; ++line)
	{
		const uint8* data= bmp->LinePixelBuffer(line, left);

		if (grayscale)
			for (int n= left; n < right; ++n)
			{
				uint8 gray= *data++;

				++red_[gray];
				++green_[gray];
				++blue_[gray];
				++lum_[gray];
				++rgb_[gray];
			}
		else
			if (using_luminosity_and_rgb_)
				for (int n= left; n < right; ++n)
				{
					// dibs are BGR
					uint8 blue= *data++;
					uint8 green= *data++;
					uint8 red= *data++;

					// this is taken from luminocity fn in YIQ color system (std NTSC)
					uint8 luminosity= (76 * red + 150 * green + 29 * blue) / 255;

					++red_[red];
					++green_[green];
					++blue_[blue];
					++lum_[luminosity];

					++rgb_[red];
					++rgb_[green];
					++rgb_[blue];
				}
			else
				for (int n= left; n < right; ++n)
				{
					// dibs are BGR
					uint8 blue= *data++;
					uint8 green= *data++;
					uint8 red= *data++;

					++red_[red];
					++green_[green];
					++blue_[blue];
				}
	}
/*
	for (int line= 0; line < bmp->GetHeight(); ++line)
	{
		if (line < selection_rect.top || line > selection_rect.bottom)
			continue;

		const uint8* data= bmp->LineBuffer(line);

		if (grayscale)
		{
			for (int n= 0; n < line_size; ++n)
			{
				uint8 gray= *data++;

				if (n < selection_rect.left || n > selection_rect.right)
					continue;

				++red_[gray];
				++green_[gray];
				++blue_[gray];
				++lum_[gray];
				++rgb_[gray];
			}
		}
		else
		{
			for (int n= 0; n < line_size; ++n)
			{
				// dibs are BGR
				uint8 blue= *data++;
				uint8 green= *data++;
				uint8 red= *data++;

				if (n < selection_rect.left || n > selection_rect.right)
					continue;

				++red_[red];
				++green_[green];
				++blue_[blue];
				// this is taken from luminocity fn in YIQ color system (std NTSC)
				uint8 luminosity= (76 * int(red) + 150 * int(green) + 29 * int(blue)) / 255;
				++lum_[luminosity];

				++rgb_[red];
				++rgb_[green];
				++rgb_[blue];
			}
		}
	}
*/
	bool skip_max= selection_rect.left != 0 || selection_rect.top != 0 ||
		selection_rect.right != bmp->GetWidth() - 1 || selection_rect.bottom != bmp->GetHeight() - 1;

	// find max number of pixels; eliminate false extreme values
	{
		uint32 max= 0;
		uint32 prev= 0;
		for (int i= 1; i < COUNT - 1; ++i)
			if (red_[i] > max)
				prev = max, max = red_[i];
			else if (red_[i] > prev)
				prev = red_[i];

		if (!skip_max && max > 0 && double(max - prev) / max > 0.1)
			max = prev;

		max_vals_[1] = max;
		max_val_ = max;
	}
	{
		uint32 max= 0;
		uint32 prev= 0;
		for (int i= 1; i < COUNT - 1; ++i)
			if (green_[i] > max)
				prev = max, max = green_[i];
			else if (green_[i] > prev)
				prev = green_[i];

		if (!skip_max && max > 0 && double(max - prev) / max > 0.1)
			max = prev;

		max_vals_[2] = max;
		if (max_val_ < max)
			max_val_ = max;
	}
	{
		uint32 max= 0;
		uint32 prev= 0;
		for (int i= 1; i < COUNT - 1; ++i)
			if (blue_[i] > max)
				prev = max, max = blue_[i];
			else if (blue_[i] > prev)
				prev = blue_[i];

		if (!skip_max && max > 0 && double(max - prev) / max > 0.1)
			max = prev;

		max_vals_[3] = max;
		if (max_val_ < max)
			max_val_ = max;
	}
	{
		uint32 max= 0;
		uint32 prev= 0;
		for (int i= 1; i < COUNT - 1; ++i)
			if (lum_[i] > max)
				prev = max, max = lum_[i];
			else if (lum_[i] > prev)
				prev = lum_[i];

		if (!skip_max && max > 0 && double(max - prev) / max > 0.1)
			max = prev;

		max_vals_[0] = max;
		if (max_val_ < max)
			max_val_ = max;
	}
	{
		uint32 max= 0;
		uint32 prev= 0;
		for (int i= 1; i < COUNT - 1; ++i)
			if (rgb_[i] > max)
				prev = max, max = rgb_[i];
			else if (rgb_[i] > prev)
				prev = rgb_[i];

		if (!skip_max && max > 0 && double(max - prev) / max > 0.1)
			max = prev;

		max_vals_[4] = max;
		//if (max_val_ < max)
		//	max_val_ = max;
	}

	total_pixels_ = (selection_rect.Width() + 1) * (selection_rect.Height() + 1);
}


void Histogram::PaintRect(CDC* dc, int x, int y, int w, int h, COLORREF rgb_color)
{
	CBrush brush(rgb_color);
	CBrush* old_brush= dc->SelectObject(&brush);
	dc->PatBlt(x, y, w, h, PATCOPY);
	dc->SelectObject(old_brush);
}


void Histogram::PaintStripe(CDC* dc, CRect rect, COLORREF rgb_color)
{
	const int STEP= 4;
	const int SIZE= rect.Width();
	int w= rect.Width() / (SIZE / STEP);
	for (int i= 0; i < SIZE; i += STEP)
	{
		COLORREF rgb_shade= RGB(GetRValue(rgb_color) * i / SIZE, GetGValue(rgb_color) * i / SIZE, GetBValue(rgb_color) * i / SIZE);
		PaintRect(dc, rect.left + i, rect.top, w, rect.Height(), rgb_shade);
	}
}

/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "../Color.h"
#include "../Dib.h"
#include "../Pixels.h"


extern void ModifyHueSaturation(Dib& dib, float saturation, float lightness)
{
	ASSERT(lightness <= 0.0f);

	float shade= 1.0f + lightness;

	if (!dib.IsValid() || dib.GetBitsPerPixel() != 32)
		return;

	for (int i= 0; i < dib.GetHeight(); ++i)
	{
		BYTE* p= dib.LineBuffer(i);
		int width= dib.GetWidth();

		for (int x= 0; x < width; ++x)
		{
			COLORREF rgb= CalcNewColorDelta(RGB(p[2], p[1], p[0]), saturation, 0.0f);

			p[0] = static_cast<BYTE>(GetBValue(rgb) * shade);
			p[1] = static_cast<BYTE>(GetGValue(rgb) * shade);
			p[2] = static_cast<BYTE>(GetRValue(rgb) * shade);
			p += 4;
		}
	}
}

//extern COLORREF CalcNewColorDeltaCtr(COLORREF rgb_color, float saturation_delta, float lightness_delta, float contrast);

extern void ModifyHueSaturation(Dib& dib, const CRect& rect, float saturation, float lightness, float contrast)
{
	CRect bmp(0, 0, dib.GetWidth(), dib.GetHeight());

	CRect dest= bmp & rect;

	if (dest.Width() <= 0 || dest.Height() <= 0)
		return;

	for (int y= dest.top; y < dest.bottom; ++y)
	{
		BYTE* p= dib.LinePixelBuffer(y, dest.left);
//		int width= dib.GetWidth();

		for (int x= dest.left; x < dest.right; ++x)
		{
			//COLORREF rgb= CalcNewColorDeltaCtr(RGB(p[2], p[1], p[0]), saturation, lightness, contrast);

			p[0] = 15;//static_cast<BYTE>(GetBValue(rgb));
			p[1] = 15;//static_cast<BYTE>(GetGValue(rgb));
			p[2] = 20;//static_cast<BYTE>(GetRValue(rgb));
			p += 4;
		}
	}
}


extern void DrawGlow(Dib& dib, CRect rect, const Dib& glow, COLORREF  color)
{
	int x_bmp= rect.left < 0 ? -rect.left : 0;
	int y_bmp= rect.top < 0 ? -rect.top : 0;

	int x_from= std::max(0L, rect.left);
	int x_to= std::min<int>(dib.GetWidth(), rect.right);

	int y_from= std::max(0L, rect.top);
	int y_to= std::min<int>(dib.GetHeight(), rect.bottom);

	int width= x_to - x_from;
	int height= y_to - y_from;

	if (glow.GetHeight() < height || glow.GetWidth() < width ||
		glow.GetHeight() < y_bmp || glow.GetWidth() < x_bmp)
	{
		ASSERT(false);
		return;
	}

	int red= GetRValue(color);
	int green= GetGValue(color);
	int blue= GetBValue(color);

	for (int y= 0; y < height; ++y)
		for (int x= 0; x < width; ++x)
		{
			Pixel4* p= (Pixel4*)dib.LinePixelBuffer(y + y_from, x + x_from);
			//BYTE pix= (255 - OPQ) + *glow.LinePixelBuffer(y_bmp + y, x_bmp + x) * OPQ / 255;
			BYTE pix= 255 - *glow.LinePixelBuffer(y_bmp + y, x_bmp + x);

			// glow effect (apply in screen blending mode)
			p->r = static_cast<BYTE>(pix * (red * (255 - p->r)) / (255*255) + p->r);
			p->g = static_cast<BYTE>(pix * (green * (255 - p->g)) / (255*255) + p->g);
			p->b = static_cast<BYTE>(pix * (blue * (255 - p->b)) / (255*255) + p->b);
			p->a; // not used
		}
}


extern void CreateVertAlphaGradient(Dib& dib, CRect rect)
{
	CRect dest(0, 0, dib.GetWidth(), dib.GetHeight());
	rect &= dest;

	if (rect.IsRectEmpty())
		return;

	int height= rect.Height() - 1;
	if (height < 1)
		height = 1;

	for (int y= rect.top; y < rect.bottom; ++y)
	{
		int shadow= (y - rect.top) * 255 / height;
		memset(dib.LineBuffer(y), shadow, dib.GetBytesPerLine());
	}
}


extern void DrawShadow(Dib& dib, CRect rect, const Dib& shadow, const int OPQ)
{
	int x_bmp= rect.left < 0 ? -rect.left : 0;
	int y_bmp= rect.top < 0 ? -rect.top : 0;

	int x_from= std::max(0L, rect.left);
	int x_to= std::min<int>(dib.GetWidth(), rect.right);

	int y_from= std::max(0L, rect.top);
	int y_to= std::min<int>(dib.GetHeight(), rect.bottom);

	int width= x_to - x_from;
	int height= y_to - y_from;

	if (shadow.GetHeight() < height || shadow.GetWidth() < width ||
		shadow.GetHeight() < y_bmp || shadow.GetWidth() < x_bmp)
	{
		ASSERT(false);
		return;
	}

	for (int y= 0; y < height; ++y)
		for (int x= 0; x < width; ++x)
		{
			Pixel4* p= (Pixel4*)dib.LinePixelBuffer(y + y_from, x + x_from);
			int val= *shadow.LinePixelBuffer(y_bmp + y, x_bmp + x) * OPQ;
			BYTE pix= (255 - OPQ) + ((val + (val << 8) + 255) >> 16);
//			BYTE pix= (255 - OPQ) + val / 255;

			// black shadow (src is black -> 0, so alpha blanding is simplified)
			p->r = static_cast<BYTE>(pix * p->r >> 8);
			p->g = static_cast<BYTE>(pix * p->g >> 8);
			p->b = static_cast<BYTE>(pix * p->b >> 8);
			p->a; // not used
		}
}


// shadow creation
/*					CSize s= frm.Size() + CSize(EXTRA, EXTRA);
					if (s != shadow.GetSize())
					{
						shadow.Create(s.cx, s.cy, 8);

						// full white
						memset(shadow.GetBuffer(), 0xff, shadow.GetBufferSize());

						int offset= EXTRA / 2;

						// shifted black rect
						for (int line= offset; line < s.cy - offset; ++line)
						{
							BYTE* from= shadow.LinePixelBuffer(line, offset);
							BYTE* to= shadow.LinePixelBuffer(line, s.cx - offset);
							memset(from, 0, to - from);
						}

						// blur edges to create smooth shadow areas
						BlurAlphaChnl(shadow, 0, shadow.GetWidth(), blur, 0xff);
					}

					// draw shadow
					CRect shd= frm;
					shd.right += EXTRA;
					shd.bottom += EXTRA;
					DrawShadow(bmp, shd, blur, 180); */

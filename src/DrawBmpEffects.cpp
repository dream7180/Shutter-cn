/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "Dib.h"
#include "Pixels.h"
#include "DrawBmpEffects.h"



void DrawBlackGradient(Dib& dib, CRect rect, float opacity, int direction)
{
	CRect clip(0, 0, dib.GetWidth(), dib.GetHeight());

	if ((rect & clip).IsRectEmpty())
		return;

	// sanity check
	if (rect.Width() > 0xffff || rect.Height() > 0xffff)
		return;

	switch (direction)
	{
	case 1:
	case -1:
		for (int x= rect.left; x < rect.right; ++x)
		{
			int mul= direction > 0 ? 255 * (rect.right - x) / rect.Width() : 255 * (x - rect.left + 1) / rect.Width();
			int not_black= 255 - static_cast<int>(opacity * mul + 0.49f);

			for (int y= rect.top; y < rect.bottom; ++y)
			{
				if (!clip.PtInRect(CPoint(x, y)))
					continue;

				Pixel4* p= reinterpret_cast<Pixel4*>(dib.LinePixelBuffer(y, x));

				p->r = static_cast<BYTE>(p->r * not_black / 255);
				p->g = static_cast<BYTE>(p->g * not_black / 255);
				p->b = static_cast<BYTE>(p->b * not_black / 255);
			}
		}
		break;

	case 2:
	case -2:
		for (int y= rect.top; y < rect.bottom; ++y)
		{
			int mul= direction > 0 ? 255 * (rect.bottom - y) / rect.Height() : 255 * (y - rect.top + 1) / rect.Height();
			int not_black= 255 - static_cast<int>(opacity * mul + 0.49f);

			for (int x= rect.left; x < rect.right; ++x)
			{
				if (!clip.PtInRect(CPoint(x, y)))
					continue;

				Pixel4* p= reinterpret_cast<Pixel4*>(dib.LinePixelBuffer(y, x));

				p->r = static_cast<BYTE>(p->r * not_black / 255);
				p->g = static_cast<BYTE>(p->g * not_black / 255);
				p->b = static_cast<BYTE>(p->b * not_black / 255);
			}
		}
		break;
	}
}


void DrawBlackCorner(Dib& dib, CRect rect, float opacity, int direction)
{
	CRect clip(0, 0, dib.GetWidth(), dib.GetHeight());

	if ((rect & clip).IsRectEmpty())
		return;

	for (int x= rect.left; x < rect.right; ++x)
	{
		for (int y= rect.top; y < rect.bottom; ++y)
		{
			if (!clip.PtInRect(CPoint(x, y)))
				continue;

			int mul1= direction > 0 ? 255 * (rect.right - x) / rect.Width() : 255 * (x - rect.left + 1) / rect.Width();
			int mul2= abs(direction) > 1 ? 255 * (rect.bottom - y) / rect.Height() : 255 * (y - rect.top + 1) / rect.Height();

			int not_black= 255 - static_cast<int>(opacity * mul1 * mul2 / 255 + 0.49f);

			Pixel4* p= reinterpret_cast<Pixel4*>(dib.LinePixelBuffer(y, x));

			p->r = static_cast<BYTE>(p->r * not_black / 255);
			p->g = static_cast<BYTE>(p->g * not_black / 255);
			p->b = static_cast<BYTE>(p->b * not_black / 255);
		}
	}
}


void DrawBlackShadow(Dib& dib, CRect rect, float opacity, CSize offset, int size)
{
	CRect shadow= rect;
	shadow.OffsetRect(offset);

	CRect core= shadow;
	core.DeflateRect(size, size);

	if (core.Width() < 0)
		core.left = core.right = shadow.CenterPoint().x;
	if (core.Height() < 0)
		core.top = core.bottom = shadow.CenterPoint().y;

	DrawBlackGradient(dib, CRect(shadow.left, core.top, core.left, core.bottom), opacity, -1);
	DrawBlackGradient(dib, CRect(core.right, core.top, shadow.right, core.bottom), opacity, 1);

	DrawBlackGradient(dib, CRect(core.left, shadow.top, core.right, core.top), opacity, -2);
	DrawBlackGradient(dib, CRect(core.left, core.bottom, core.right, shadow.bottom), opacity, 2);

	DrawBlackCorner(dib, CRect(shadow.left, shadow.top, core.left, core.top), opacity, -1);
	DrawBlackCorner(dib, CRect(core.right, shadow.top, shadow.right, core.top), opacity, 1);

	DrawBlackCorner(dib, CRect(shadow.left, core.bottom, core.left, shadow.bottom), opacity, -2);
	DrawBlackCorner(dib, CRect(core.right, core.bottom, shadow.right, shadow.bottom), opacity, 2);
}



void DrawGlowGradient(Dib& dib, CRect rect, COLORREF color, float opacity, int direction, bool screenMode)
{
	CRect clip(0, 0, dib.GetWidth(), dib.GetHeight());

	if ((rect & clip).IsRectEmpty())
		return;

	int red= GetRValue(color);
	int green= GetGValue(color);
	int blue= GetBValue(color);

	switch (direction)
	{
	case 1:
	case -1:
		for (int x= rect.left; x < rect.right; ++x)
		{
			int mul= direction > 0 ? 255 * (rect.right - x) / rect.Width() : 255 * (x - rect.left + 1) / rect.Width();
			int pix= static_cast<int>(opacity * mul + 0.49f);

			for (int y= rect.top; y < rect.bottom; ++y)
			{
				if (!clip.PtInRect(CPoint(x, y)))
					continue;

				Pixel4* p= reinterpret_cast<Pixel4*>(dib.LinePixelBuffer(y, x));

				if (screenMode)
				{
					// glow effect (apply in screen blending mode)
					p->r = static_cast<BYTE>(pix * (red * (255 - p->r)) / (255*255) + p->r);
					p->g = static_cast<BYTE>(pix * (green * (255 - p->g)) / (255*255) + p->g);
					p->b = static_cast<BYTE>(pix * (blue * (255 - p->b)) / (255*255) + p->b);
					p->a; // not used
				}
				else
				{
					// normal blending mode
					p->r = static_cast<BYTE>(p->r + pix * (red - p->r) / 255);
					p->g = static_cast<BYTE>(p->g + pix * (green - p->g) / 255);
					p->b = static_cast<BYTE>(p->b + pix * (blue - p->b) / 255);
					p->a; // not used
				}
			}
		}
		break;

	case 2:
	case -2:
		for (int y= rect.top; y < rect.bottom; ++y)
		{
			int mul= direction > 0 ? 255 * (rect.bottom - y) / rect.Height() : 255 * (y - rect.top + 1) / rect.Height();
			int pix= static_cast<int>(opacity * mul + 0.49f);

			for (int x= rect.left; x < rect.right; ++x)
			{
				if (!clip.PtInRect(CPoint(x, y)))
					continue;

				Pixel4* p= reinterpret_cast<Pixel4*>(dib.LinePixelBuffer(y, x));

				if (screenMode)
				{
					// glow effect (apply in screen blending mode)
					p->r = static_cast<BYTE>(pix * (red * (255 - p->r)) / (255*255) + p->r);
					p->g = static_cast<BYTE>(pix * (green * (255 - p->g)) / (255*255) + p->g);
					p->b = static_cast<BYTE>(pix * (blue * (255 - p->b)) / (255*255) + p->b);
					p->a; // not used
				}
				else
				{
					// normal blending mode
					p->r = static_cast<BYTE>(p->r + pix * (red - p->r) / 255);
					p->g = static_cast<BYTE>(p->g + pix * (green - p->g) / 255);
					p->b = static_cast<BYTE>(p->b + pix * (blue - p->b) / 255);
					p->a; // not used
				}
			}
		}
		break;
	}
}


void DrawGlowCorner(Dib& dib, CRect rect, COLORREF color, float opacity, int direction, bool screenMode)
{
	CRect clip(0, 0, dib.GetWidth(), dib.GetHeight());

	if ((rect & clip).IsRectEmpty())
		return;

	int red= GetRValue(color);
	int green= GetGValue(color);
	int blue= GetBValue(color);

	for (int x= rect.left; x < rect.right; ++x)
	{
		for (int y= rect.top; y < rect.bottom; ++y)
		{
			if (!clip.PtInRect(CPoint(x, y)))
				continue;

			int mul1= direction > 0 ? 255 * (rect.right - x) / rect.Width() : 255 * (x - rect.left + 1) / rect.Width();
			int mul2= abs(direction) > 1 ? 255 * (rect.bottom - y) / rect.Height() : 255 * (y - rect.top + 1) / rect.Height();

			int pix= static_cast<int>(opacity * mul1 * mul2 / 255 + 0.49f);

			Pixel4* p= reinterpret_cast<Pixel4*>(dib.LinePixelBuffer(y, x));

			if (screenMode)
			{
				// glow effect (apply in screen blending mode)
				p->r = static_cast<BYTE>(pix * (red * (255 - p->r)) / (255*255) + p->r);
				p->g = static_cast<BYTE>(pix * (green * (255 - p->g)) / (255*255) + p->g);
				p->b = static_cast<BYTE>(pix * (blue * (255 - p->b)) / (255*255) + p->b);
				p->a; // not used
			}
			else
			{
				// normal blending mode
				p->r = static_cast<BYTE>(p->r + pix * (red - p->r) / 255);
				p->g = static_cast<BYTE>(p->g + pix * (green - p->g) / 255);
				p->b = static_cast<BYTE>(p->b + pix * (blue - p->b) / 255);
				p->a; // not used
			}
		}
	}
}


void DrawGlowEffect(Dib& dib, CRect rect, COLORREF color, float opacity, int size, bool screenMode)
{
	CRect glow= rect;
	glow.InflateRect(size, size);

	CRect core= rect;

	DrawGlowGradient(dib, CRect(glow.left, core.top, core.left, core.bottom), color, opacity, -1, screenMode);
	DrawGlowGradient(dib, CRect(core.right, core.top, glow.right, core.bottom), color, opacity, 1, screenMode);

	DrawGlowGradient(dib, CRect(core.left, glow.top, core.right, core.top), color, opacity, -2, screenMode);
	DrawGlowGradient(dib, CRect(core.left, core.bottom, core.right, glow.bottom), color, opacity, 2, screenMode);

	DrawGlowCorner(dib, CRect(glow.left, glow.top, core.left, core.top), color, opacity, -1, screenMode);
	DrawGlowCorner(dib, CRect(core.right, glow.top, glow.right, core.top), color, opacity, 1, screenMode);

	DrawGlowCorner(dib, CRect(glow.left, core.bottom, core.left, glow.bottom), color, opacity, -2, screenMode);
	DrawGlowCorner(dib, CRect(core.right, core.bottom, glow.right, glow.bottom), color, opacity, 2, screenMode);
}



/* // bmp-based glow effect
					const int EXTRA= 16;
					CSize s= frm.Size() + CSize(EXTRA, EXTRA);

					Dib sel(s.cx, s.cy, 8);

					// full white
					memset(sel.GetBuffer(), 0xff, sel.GetBufferSize());

					// very thickness of selection
					int offset= EXTRA / 2 - min<int>(3, (max(s.cx, s.cy) + 49) / 50);

					// black round rect
					CDC dcb;
					dcb.CreateCompatibleDC(0);
					dcb.SelectObject(sel.GetBmp());
					dcb.SelectStockObject(NULL_PEN);
					dcb.SelectStockObject(BLACK_BRUSH);
					dcb.RoundRect(CRect(offset, offset, s.cx - offset + 1, s.cy - offset + 1), CPoint(8, 8));
					dcb.DeleteDC();

					// blur edges to create smooth selection glow
					Dib blur;
					BlurAlphaChnl2(sel, 0, sel.GetWidth(), blur, 0xff);

					// draw glow
					CRect srect= frm;
					srect.OffsetRect(-EXTRA / 2, -EXTRA / 2);
					srect.right = srect.left + blur.GetWidth();
					srect.bottom = srect.top + blur.GetHeight();

					DrawGlow(bmp, srect, blur, ::GetSysColor(COLOR_HIGHLIGHT));
*/

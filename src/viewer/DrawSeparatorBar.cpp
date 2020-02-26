/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "../Dib.h"
#include "../Pixels.h"

extern void DrawSeparatorBar(Dib& dest, int x, int y, int height, float black_opacity, float white_opacity)
{
	ASSERT(black_opacity >= 0.0f && black_opacity <= 1.0f);
	ASSERT(white_opacity >= 0.0f && white_opacity <= 1.0f);

	if (x < 0 || x + 2 >= dest.GetWidth())
	{
		ASSERT(false);
		return;
	}

	if (y < 0 || y + height > dest.GetHeight())
	{
		ASSERT(false);
		return;
	}

	int not_black= static_cast<int>((1.0 - black_opacity) * 255 + 0.49f);
	int white= static_cast<int>(white_opacity * 255 + 0.49f);
	int not_white= 255 - white;
	int bright= 255 * white;

	for (int line= 0; line < height; ++line)
	{
		Pixel4* p= reinterpret_cast<Pixel4*>(dest.LinePixelBuffer(line + y, x));

		p->r = static_cast<BYTE>(int(p->r) * not_black / 255);
		p->g = static_cast<BYTE>(int(p->g) * not_black / 255);
		p->b = static_cast<BYTE>(int(p->b) * not_black / 255);

		p++;

		p->r = static_cast<BYTE>((bright + not_white * p->r) / 255);
		p->g = static_cast<BYTE>((bright + not_white * p->g) / 255);
		p->b = static_cast<BYTE>((bright + not_white * p->b) / 255);
	}
}

/*
extern void DrawHorzSeparatorBar(Dib& dest, int x, int y, int width, float black_opacity, float white_opacity)
{
	ASSERT(black_opacity >= 0.0f && black_opacity <= 1.0f);
	ASSERT(white_opacity >= 0.0f && white_opacity <= 1.0f);

	if (x < 0 || x + width > dest.GetWidth())
	{
		ASSERT(false);
		return;
	}

	if (y < 0 || y + 2 >= dest.GetHeight())
	{
		ASSERT(false);
		return;
	}

	int not_black= static_cast<int>((1.0 - black_opacity) * 255 + 0.49f);
	int white= static_cast<int>(white_opacity * 255 + 0.49f);
	int not_white= 255 - white;
	int bright= 255 * white;

	Pixel4* p= reinterpret_cast<Pixel4*>(dest.LinePixelBuffer(y, x));
	Pixel4* q= reinterpret_cast<Pixel4*>(dest.LinePixelBuffer(y + 1, x));

	for (int col= 0; col < width; ++col)
	{
		p->r = static_cast<BYTE>(int(p->r) * not_black / 255);
		p->g = static_cast<BYTE>(int(p->g) * not_black / 255);
		p->b = static_cast<BYTE>(int(p->b) * not_black / 255);

		++p;

		q->r = static_cast<BYTE>((bright + not_white * q->r) / 255);
		q->g = static_cast<BYTE>((bright + not_white * q->g) / 255);
		q->b = static_cast<BYTE>((bright + not_white * q->b) / 255);

		++q;
	}
}
*/
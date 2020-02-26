/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "ImgManipulation.h"
#include "Dib.h"
#include <math.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


ColorBalance::ColorBalance()
{
	static bool ready= false;

	if (!ready)
	{
		for (size_t i= 0; i < MAX; ++i)
		{
			transfer_[0][i] = transfer_[1][MAX - 1 - i] = 1.075 - 1.0 / (i / 16.0 + 1.0);
			double x= (i - 127.0) / 127.0;
			transfer_[2][i] = 0.667 * (1.0 - x * x);
		}

		ready = true;
	}

	preserve_luminosity_ = true;
}


double ColorBalance::transfer_[3][MAX];

inline int Limit255(int n)	{ return n > 255 ? 255 : (n < 0 ? 0 : n); }


void ColorBalance::CalcLookupTables(const ColorBalanceParams& params)
{
	double* cyan_red_shadows=			params.shadows.cyan_red > 0.0 ? transfer_[2] : transfer_[1];
	double* magenta_green_shadows=		params.shadows.magenta_green > 0.0 ? transfer_[2] : transfer_[1];
	double* yellow_blue_shadows=		params.shadows.yellow_blue > 0.0 ? transfer_[2] : transfer_[1];

	double* cyan_red_midtones=			transfer_[0];
	double* magenta_green_midtones=		transfer_[0];
	double* yellow_blue_midtones=		transfer_[0];

	double* cyan_red_highlights=		params.highlights.cyan_red > 0.0 ? transfer_[0] : transfer_[2];
	double* magenta_green_highlights=	params.highlights.magenta_green > 0.0 ? transfer_[0] : transfer_[2];
	double* yellow_blue_highlights=		params.highlights.yellow_blue > 0.0 ? transfer_[0] : transfer_[2];

	for (size_t i= 0; i < MAX; ++i)
	{
		int red= i;

		red += static_cast<int>(params.shadows.cyan_red * cyan_red_shadows[red]);
		red = Limit255(red);
		red += static_cast<int>(params.midtones.cyan_red * cyan_red_midtones[red]);
		red = Limit255(red);
		red += static_cast<int>(params.highlights.cyan_red * cyan_red_highlights[red]);
		red = Limit255(red);

		int green= i;

		green += static_cast<int>(params.shadows.magenta_green * magenta_green_shadows[green]);
		green = Limit255(green);
		green += static_cast<int>(params.midtones.magenta_green * magenta_green_midtones[green]);
		green = Limit255(green);
		green += static_cast<int>(params.highlights.magenta_green * magenta_green_highlights[green]);
		green = Limit255(green);

		int blue= i;

		blue += static_cast<int>(params.shadows.yellow_blue * yellow_blue_shadows[blue]);
		blue = Limit255(blue);
		blue += static_cast<int>(params.midtones.yellow_blue * yellow_blue_midtones[blue]);
		blue = Limit255(blue);
		blue += static_cast<int>(params.highlights.yellow_blue * yellow_blue_highlights[blue]);
		blue = Limit255(blue);

		red_[i] = red;
		green_[i] = green;
		blue_[i] = blue;
	}
}


extern void RGB2HSL(int r, int g, int b, int& hue, int& sat, int& lum)
{
	int min, max;

	if (r > g)
	{
		max = MAX(r, b);
		min = MIN(g, b);
	}
	else
	{
		max = MAX(g, b);
		min = MIN(r, b);
	}

	int l= (max + min);// / 2;		// lightness

	int h, s;

	if (max == min)
	{
		// achromatic case
		s = 0;
		h = 0;
	}
	else
	{
		// chromatic case
		int delta= max - min;

		if (l < 256)
			s = (255 * delta + (max + min) / 2) / (max + min);
		else
		{
			int d= 511 - max - min;
			s = (255 * delta + d / 2) / d;
		}

		if (r == max)
			h =    0 + (600 * (g - b) / delta);
		else if (g == max)
			h = 1200 + (600 * (b - r) / delta);
		else
			h = 2400 + (600 * (r - g) / delta);

		if (h < 0)
			h += 3600;

		ASSERT(h <= 3600);
	}

	hue = h;
	sat = s;
	lum = l;
}


int RGB2Lum(int r, int g, int b)
{
	int min, max;

	if (r > g)
	{
		max = MAX(r, b);
		min = MIN(g, b);
	}
	else
	{
		max = MAX(g, b);
		min = MIN(r, b);
	}

	return (max + min); // / 2;
}


int HSLValue(int n1, int n2, int hue)
{
	int value;

	if (hue > 3600)
		hue -= 3600;
	else if (hue < 0)
		hue += 3600;

	if (hue < 600)
		value = n1 + ((n2 - n1) * hue + 300) / 600;
	else if (hue < 1800)
		value = n2;
	else if (hue < 2400)
		value = n1 + ((n2 - n1) * (2400 - hue) + 300) / 600;
	else
		value = n1;

	return value;
}


extern void HSL2RGB(int h, int s, int l, int& r, int& g, int& b)
{
	if (s == 0)
	{
		// achromatic case
		r = g = b = l / 2;
	}
	else
	{
		int m2;

		if (l < 256)
			m2 = (l * (255 + s) + 255) / (2 * 255);
		else
			m2 = (l + 2 * s - (l * s) / 255) / 2;

		int m1= /*2 * */ l - m2;

		// chromatic case
		r = HSLValue(m1, m2, h + 1200);
		g = HSLValue(m1, m2, h);
		b = HSLValue(m1, m2, h - 1200);
	}
}


void ColorBalance::Transform(Dib& dib)
{
	size_t width= dib.GetWidth();
	size_t height= dib.GetHeight();

	int bpp= dib.GetBitsPerPixel();
	if (bpp != 24)
	{
		ASSERT(false);
		return;
	}

	for (size_t line= 0; line < height; ++line)
	{
		size_t len= dib.GetBytesPerLine() / 3;

		BYTE* src= dib.LineBuffer(line);

		if (preserve_luminosity_)
		{
			for (size_t x= 0; x < len; ++x, src += 3)
			{
				int r= src[2];
				int g= src[1];
				int b= src[0];

				int h, s, l;
				RGB2HSL(red_[r], green_[g], blue_[b], h, s, l);
				l = RGB2Lum(r, g, b);
				HSL2RGB(h, s, l, r, g, b);

				src[2] = r;
				src[1] = g;
				src[0] = b;
			}
		}
		else
		{
			for (size_t x= 0; x < len; ++x, src += 3)
			{
				src[2] = red_[src[2]];
				src[1] = green_[src[1]];
				src[0] = blue_[src[0]];
			}
		}
	}

}








#if 0

int test()
{
	int h, s, l;
	int r, g, b;

	RGB2HSL(2, 1, 1, h, s, l);
	HSL2RGB(h, s, l, r, g, b);

	RGB2HSL(254, 255, 255, h, s, l);
	HSL2RGB(h, s, l, r, g, b);

	RGB2HSL(127, 128, 127, h, s, l);
	HSL2RGB(h, s, l, r, g, b);

	RGB2HSL(128, 128, 127, h, s, l);
	HSL2RGB(h, s, l, r, g, b);

//	l = RGB2Lum(r, g, b);
	HSL2RGB(h, s, l, r, g, b);

	return 0;
}

int a= test();
#endif


///////////////////////////////////////////////////////////////////////////////////////////////////


Levels::Levels()
{
}


void Levels::CalcLookupTables(const LevelParams& params)
{
	uint8* tables[]= { red_, red_, green_, blue_ };

	size_t from= 1;
	size_t to= 3;

	if (params.channels_ == LevelParams::RGB)
	{
		from = 0;
		to = 0;
	}

	for (size_t channel= from; channel <= to; ++channel)
	{
		uint8* table= tables[channel];

		uint8 min= params.levels_[channel].min;
		uint8 max= params.levels_[channel].max;

		double gamma= params.levels_[channel].gamma;
		if (gamma < 0.1)
			gamma = 0.1;
		else if (gamma > 10.0)
			gamma = 10.0;
		double inv_gamma= 1.0 / gamma;

		if (min > max)
			std::swap(min, max);

		int delta= std::max(1, static_cast<int>(max) - min);

		size_t i= 0;
		for ( ; i < min; ++i)
			table[i] = 0;

		for ( ; i < max; ++i)
		{
			uint8 val;

			if (gamma == 1.0)
				val = static_cast<uint8>((i - min) * (MAX - 1) / delta);
			else
				val = static_cast<uint8>((MAX - 1) * pow(double(i - min) / delta, inv_gamma));

			table[i] = val;
		}

		for ( ; i < MAX; ++i)
			table[i] = MAX - 1;
	}

	if (params.channels_ == LevelParams::RGB)
	{
		memcpy(green_, red_, MAX * sizeof(uint8));
		memcpy(blue_, red_, MAX * sizeof(uint8));
	}
}


void Levels::Transform(Dib& dib)
{
	size_t width= dib.GetWidth();
	size_t height= dib.GetHeight();

	int bpp= dib.GetBitsPerPixel();
	if (bpp != 24 && bpp != 8)
	{
		ASSERT(false);
		return;
	}

	for (size_t line= 0; line < height; ++line)
	{
		if (bpp == 8)
		{
			// Grayscale
			size_t len= dib.GetBytesPerLine();

			BYTE* src= dib.LineBuffer(line);

			for (size_t x= 0; x < len; ++x, ++src)
				*src = red_[*src];
		}
		else
		{
			// RGB
			size_t len= dib.GetBytesPerLine() / 3;

			BYTE* src= dib.LineBuffer(line);

			for (size_t x= 0; x < len; ++x, src += 3)
			{
				src[2] = red_[src[2]];
				src[1] = green_[src[1]];
				src[0] = blue_[src[0]];
			}
		}
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////


BrightnessContrast::BrightnessContrast()
{
}


inline int ScaleAndLimit255(double d)	{ int n= static_cast<int>(d * 255.0 + 0.5); return n > 255 ? 255 : (n < 0 ? 0 : n); }


void BrightnessContrast::CalcLookupTables(const BrightnessContrastParams& params)
{
	double contrast;
	if (params.val[1] >= 0)
		contrast = 1.0 / (1.0 - (params.val[1] < 1.0 ? params.val[1] : 0.999));
	else
		contrast = 1.0 + params.val[1];

	double brightness= params.val[0] / 2.0;

	for (size_t i= 0; i < MAX; ++i)
	{
		double d= i / 255.0;

		// brightness

		if (brightness >= 0.0)
			d = d * (1.0 - brightness) + brightness;
		else
			d = d * (1.0 + brightness);

		// contrast

		double x= d <= 0.5 ? d : 1.0 - d;
		if (x < 0.0)
			x = 0.0;

		x = 0.5 * pow(2.0 * x, contrast);

		d = d <= 0.5 ? x : 1.0 - x;
		lut_[i] = static_cast<uint8>(ScaleAndLimit255(d));
	}
}


void BrightnessContrast::Transform(Dib& dib)
{
	size_t width= dib.GetWidth();
	size_t height= dib.GetHeight();

	int bpp= dib.GetBitsPerPixel();
	if (bpp != 24 && bpp != 8)
	{
		ASSERT(false);
		return;
	}

	for (size_t line= 0; line < height; ++line)
	{
		if (bpp == 8)
		{
			// Grayscale
			size_t len= dib.GetBytesPerLine();

			BYTE* src= dib.LineBuffer(line);

			for (size_t x= 0; x < len; ++x, ++src)
				*src = lut_[*src];
		}
		else
		{
			// RGB
			size_t len= dib.GetBytesPerLine() / 3;

			BYTE* src= dib.LineBuffer(line);

			for (size_t x= 0; x < len; ++x, src += 3)
			{
				src[2] = lut_[src[2]];
				src[1] = lut_[src[1]];
				src[0] = lut_[src[0]];
			}
		}
	}
}

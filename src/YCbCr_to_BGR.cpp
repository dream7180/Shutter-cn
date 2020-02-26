/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "YCbCr_to_BGR.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


YCbCr_conversion_tables::YCbCr_conversion_tables() : range_limit(limit + 2 * MAXJSAMPLE)
{
	// initialize conversion tables

	const int SCALEBITS=	16;	/* speediest right-shift on some machines */
	const int ONE_HALF=	((INT32) 1 << (SCALEBITS-1));
#define FIX(x)		((INT32) ((x) * (1L<<SCALEBITS) + 0.5))
#define RIGHT_SHIFT(x, n)	((x) >> (n))

	for (int i= 0, x= -CENTERJSAMPLE; i <= MAXJSAMPLE; ++i, ++x)
	{
		/* i is the actual input pixel value, in the range 0..MAXJSAMPLE */
		/* The Cb or Cr value we are thinking of is x = i - CENTERJSAMPLE */
		/* Cr=>R value is nearest int to 1.40200 * x */
		Crrtab[i] = (int)	RIGHT_SHIFT(FIX(1.40200) * x + ONE_HALF, SCALEBITS);
		/* Cb=>B value is nearest int to 1.77200 * x */
		Cbbtab[i] = (int)	RIGHT_SHIFT(FIX(1.77200) * x + ONE_HALF, SCALEBITS);
		/* Cr=>G value is scaled-up -0.71414 * x */
		Crgtab[i] = (- FIX(0.71414)) * x;
		/* Cb=>G value is scaled-up -0.34414 * x */
		/* We also add in ONE_HALF so that need not do it in inner loop */
		Cbgtab[i] = (- FIX(0.34414)) * x + ONE_HALF;
	}
#undef RIGHT_SHIFT
#undef FIX

	std::fill(limit, limit + MAXJSAMPLE * 2, 0);
	for (size_t i= 0; i < MAXJSAMPLE; ++i)
		limit[MAXJSAMPLE * 2 + i] = i;
	std::fill(limit + MAXJSAMPLE * 3, limit + MAXJSAMPLE * 5, 0xff);
}


color_conversion::~color_conversion()
{}

void color_conversion::operator () (const uint8* in, uint8* out, uint32 width)
{}


void YCbCr2_to_BGR::operator () (const uint8* in, uint8* out, uint32 width)
{
	ASSERT((width & 1) == 0);	// cannot be odd

	width /= 2;
	for (uint32 n= 0; n < width; ++n)
	{
		uint8 y1= in[0];
		uint8 y2= in[1];
		uint8 cb= in[2];
		uint8 cr= in[3];

		out[2] = t_.range_limit[y1 + t_.Crrtab[cr]];
		out[1] = t_.range_limit[y1 + ((t_.Cbgtab[cb] + t_.Crgtab[cr]) >> 16)];
		out[0] = t_.range_limit[y1 + t_.Cbbtab[cb]];

		out[5] = t_.range_limit[y2 + t_.Crrtab[cr]];
		out[4] = t_.range_limit[y2 + ((t_.Cbgtab[cb] + t_.Crgtab[cr]) >> 16)];
		out[3] = t_.range_limit[y2 + t_.Cbbtab[cb]];

		in += 4;
		out += 6;
	}
}


void YCbCr_to_BGR::operator () (const uint8* in, uint8* out, uint32 width)
{
	for (uint32 n= 0; n < width; ++n)
	{
		uint8 y= in[0];
		uint8 cb= in[1];
		uint8 cr= in[2];

		out[2] = t_.range_limit[y + t_.Crrtab[cr]];
		out[1] = t_.range_limit[y + ((t_.Cbgtab[cb] + t_.Crgtab[cr]) >> 16)];
		out[0] = t_.range_limit[y + t_.Cbbtab[cb]];

		in += 3;
		out += 3;
	}
}


void CMYK_to_RGB(const uint8* in, uint8* out, uint32 width)
{
	for (uint32 n= 0; n < width; ++n)
	{
		uint8 c= in[0];
		uint8 m= in[1];
		uint8 y= in[2];
		uint8 k= in[3];

		out[2] = static_cast<uint8>(255 - c - k + c * k / 255);
		out[1] = static_cast<uint8>(255 - m - k + m * k / 255);
		out[0] = static_cast<uint8>(255 - y - k + y * k / 255);

		in += 4;
		out += 3;
	}
/*
	BYTE r, g, b;
	COLORREF rgb;

	double R, G, B;
	double C, M, Y, K;

	C = (double) c;
	M = (double) m;
	Y = (double) y;
	K = (double) k;

	C = C / 255.0;
	M = M / 255.0;
	Y = Y / 255.0;
	K = K / 255.0;

	R = C * (1.0 - K) + K;
	G = M * (1.0 - K) + K;
	B = Y * (1.0 - K) + K;

	R = (1.0 - R) * 255.0 + 0.5;
	G = (1.0 - G) * 255.0 + 0.5;
	B = (1.0 - B) * 255.0 + 0.5;

	r = (BYTE) R;
	g = (BYTE) G;
	b = (BYTE) B;

	rgb = RGB(r,g,b);

	return rgb;*/
}

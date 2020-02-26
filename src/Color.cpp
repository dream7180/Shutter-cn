/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include <math.h>

void rgb_to_hsl(float r, float g, float b, float* h, float* s, float* l);
void hsl_to_rgb(float h, float s, float l, float* r, float* g, float* b);

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// calculate new brighter (or darker if percent_brighter < 0) color using rgb_color
//
extern COLORREF CalcNewColor(COLORREF rgb_color, float percent_brighter)
{
	float h, s, l;
	rgb_to_hsl(GetRValue(rgb_color) / 255.0f, GetGValue(rgb_color) / 255.0f, GetBValue(rgb_color) / 255.0f, &h, &s, &l);

	l += percent_brighter / 100.0f;
	if (l < 0.0f)
		l = 0.0f;
	else if (l > 1.0f)
		l = 1.0f;

	float r, g, b;
	hsl_to_rgb(h, s, l, &r, &g, &b);

	return RGB(uint8(r * 255), uint8(g * 255), uint8(b * 255));
}


extern COLORREF CalcNewColorDelta(COLORREF rgb_color, float saturation_delta, float lightness_delta, float new_hue)
{
	float h, s, l;
	rgb_to_hsl(GetRValue(rgb_color) / 255.0f, GetGValue(rgb_color) / 255.0f, GetBValue(rgb_color) / 255.0f, &h, &s, &l);

	l += lightness_delta;
	if (l < 0.0f)
		l = 0.0f;
	else if (l > 1.0f)
		l = 1.0f;

	s += saturation_delta;
	if (s < 0.0f)
		s = 0.0f;
	else if (s > 1.0f)
		s = 1.0f;

	float r, g, b;
	hsl_to_rgb(new_hue, s, l, &r, &g, &b);

	return RGB(uint8(r * 255), uint8(g * 255), uint8(b * 255));
}


extern COLORREF CalcNewColorDelta(COLORREF rgb_color, float saturation_delta, float lightness_delta)
{
	float h, s, l;
	rgb_to_hsl(GetRValue(rgb_color) / 255.0f, GetGValue(rgb_color) / 255.0f, GetBValue(rgb_color) / 255.0f, &h, &s, &l);

	l += lightness_delta;
	if (l < 0.0f)
		l = 0.0f;
	else if (l > 1.0f)
		l = 1.0f;

	s += saturation_delta;
	if (s < 0.0f)
		s = 0.0f;
	else if (s > 1.0f)
		s = 1.0f;

	float r, g, b;
	hsl_to_rgb(h, s, l, &r, &g, &b);

	return RGB(uint8(r * 255), uint8(g * 255), uint8(b * 255));
}


extern COLORREF CalcNewColor(COLORREF rgb_color, float saturation, float lightness)
{
	float h, s, l;
	rgb_to_hsl(GetRValue(rgb_color) / 255.0f, GetGValue(rgb_color) / 255.0f, GetBValue(rgb_color) / 255.0f, &h, &s, &l);

	l = lightness;
	if (l < 0.0f)
		l = 0.0f;
	else if (l > 1.0f)
		l = 1.0f;

	s = saturation;
	if (s < 0.0f)
		s = 0.0f;
	else if (s > 1.0f)
		s = 1.0f;

	float r, g, b;
	hsl_to_rgb(h, s, l, &r, &g, &b);

	return RGB(uint8(r * 255), uint8(g * 255), uint8(b * 255));
}

/*
extern COLORREF CalcNewColorDeltaCtr(COLORREF rgb_color, float saturation_delta, float lightness_delta, float contrast)
{
	float h, s, l;
	rgb_to_hsl(GetRValue(rgb_color) / 255.0f, GetGValue(rgb_color) / 255.0f, GetBValue(rgb_color) / 255.0f, &h, &s, &l);

	//l += lightness_delta;

	l = l * contrast * 0.5 + lightness_delta;//l * contrast - (contrast / 2.0f - 0.5f);

	if (l < 0.0f)
		l = 0.0f;
	else if (l > 1.0f)
		l = 1.0f;

	s += saturation_delta;
	if (s < 0.0f)
		s = 0.0f;
	else if (s > 1.0f)
		s = 1.0f;

	float r, g, b;
	hsl_to_rgb(h, s, l, &r, &g, &b);

	return RGB(uint8(r * 255), uint8(g * 255), uint8(b * 255));
	
	return RGB(0, 0, 0);
}
*/

void rgb_to_hsl(float r, float g, float b, float* h, float* s, float* l)
{
	float Min= r < g ? r : g;
	Min = b < Min ? b : Min;

	float Max= r > g ? r : g;
	Max = b > Max ? b : Max;

	*l = (Min + Max) / 2.0f;

	float Dist= Max - Min;
	if (Dist > 0)
		*s = *l <= 0.5f ? Dist / (Min + Max) : Dist / (2.0f - Min - Max);
	else
	{
		*s = 0.0f;
		*h = 0.0f;
		return;
	}

	if (r == Max)
		*h = g == Min ? 5.0f + (Max - b) / Dist : 1.0f - (Max - g) / Dist;
	else if (g == Max)
		*h = b == Min ? 1.0f + (Max - r) / Dist : 3.0f - (Max - b) / Dist;
	else
		*h = r == Min ? 3.0f + (Max - g) / Dist : 5.0f - (Max - r) / Dist;
}


void hsl_to_rgb(float h, float s, float l, float* r, float* g, float* b)
{
	float v= l <= 0.5f ? l * (1.0f + s) : l + s - l * s;
	if (v <= 0.0f)
	{
		*r = *g = *b = 0.0f;
	}
	else
	{
		float m= l + l - v;
		float sv= (v - m) / v;
//		h *= 6.0;
		int ns= int(h);
		float fract= h - ns;
		float vsf= v * sv * fract;
		float mid1= m + vsf;
		float mid2= v - vsf;
		switch (ns)
		{
		case 6:
		case 0: *r = v; *g = mid1; *b = m; break;
		case 1: *r = mid2; *g = v; *b = m; break;
		case 2: *r = m; *g = v; *b = mid1; break;
		case 3: *r = m; *g = mid2; *b = v; break;
		case 4: *r = mid1; *g = m; *b = v; break;
		case 5: *r = v; *g = m; *b = mid2; break;
		}
	}
}


extern COLORREF CalcNewColor(COLORREF color1, COLORREF color2, float alpha)
{
	ASSERT(alpha >= 0.0f && alpha <= 1.0f);

	int red1= GetRValue(color1);
	int green1= GetGValue(color1);
	int blue1= GetBValue(color1);

	int red2= GetRValue(color2);
	int green2= GetGValue(color2);
	int blue2= GetBValue(color2);

	return RGB(
		red1 + alpha * (red2 - red1),
		green1 + alpha * (green2 - green1),
		blue1 + alpha * (blue2 - blue1)
		);
}


COLORREF CalcNewColor(COLORREF color, double gamma)
{
	gamma = 1.0 / gamma;
	
	return RGB(
		static_cast<BYTE>(pow(GetRValue(color) / 255.0, gamma) * 255),
		static_cast<BYTE>(pow(GetGValue(color) / 255.0, gamma) * 255),
		static_cast<BYTE>(pow(GetBValue(color) / 255.0, gamma) * 255)
		);
}


extern COLORREF CalcShade(COLORREF rgb_color, float shade)
{
	shade /= 100.0f;

	int red= GetRValue(rgb_color);
	int green= GetGValue(rgb_color);
	int blue= GetBValue(rgb_color);

	if (shade > 0.0f)	// lighter
	{
		return RGB(red + shade * (0xff - red), green + shade * (0xff - green), blue + shade * (0xff - blue));
	}
	else if (shade < 0.0f)	// darker
	{
		shade = 1.0f + shade;

		return RGB(red * shade, green * shade, blue * shade);
	}

	return rgb_color;
}


extern float CalcColorBrightness(COLORREF rgb_color)
{
	float brightness = GetRValue(rgb_color) * 0.30f + GetGValue(rgb_color) * 0.59f + GetBValue(rgb_color) * 0.11f;
	return brightness;
}

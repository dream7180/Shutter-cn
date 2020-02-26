/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once

extern void ModifyHueSaturation(Dib& dib, float saturation, float lightness);
extern void ModifyHueSaturation(Dib& dib, const CRect& rect, float saturation, float lightness, float contrast);
extern void DrawGlow(Dib& dib, CRect rect, const Dib& glow, COLORREF  color);
extern void CreateVertAlphaGradient(Dib& dib, CRect rect);
extern void DrawShadow(Dib& dib, CRect rect, const Dib& shadow, const int OPQ);

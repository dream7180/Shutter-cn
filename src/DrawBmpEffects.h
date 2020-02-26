/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once

class Dib;

extern void DrawBlackGradient(Dib& dib, CRect rect, float opacity, int direction);
extern void DrawBlackShadow(Dib& dib, CRect rect, float opacity, CSize offset, int size);
extern void DrawGlowGradient(Dib& dib, CRect rect, COLORREF color, float opacity, int direction, bool screenMode);
extern void DrawGlowEffect(Dib& dib, CRect rect, COLORREF color, float opacity, int size, bool screenMode);
extern void DrawBlackCorner(Dib& dib, CRect rect, float opacity, int direction);

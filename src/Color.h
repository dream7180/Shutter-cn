/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

extern COLORREF CalcNewColor(COLORREF rgb_color, float percent_brighter);

extern COLORREF CalcNewColor(COLORREF rgb_color, float saturation, float lightness);

extern COLORREF CalcNewColorDelta(COLORREF rgb_color, float saturation_delta, float lightness_delta);

extern COLORREF CalcNewColorDelta(COLORREF rgb_color, float saturation_delta, float lightness_delta, float new_hue);

extern COLORREF CalcNewColor(COLORREF rgb_color1, COLORREF rgb_color2, float alphaBlend);

extern COLORREF CalcNewColor(COLORREF color, double gamma);

extern COLORREF CalcShade(COLORREF rgb_color, float shade);

extern float CalcColorBrightness(COLORREF rgb_color);

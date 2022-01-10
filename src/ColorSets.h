#pragma once

class ColorSets
{
public:
	ColorSets();
	~ColorSets();
	COLORREF color_gui;
	COLORREF color_viewer_bg;
	COLORREF color_text;
	COLORREF color_text_disabled;
	COLORREF color_sepline;
	COLORREF color_sepline_light;
	COLORREF color_previewband_bg;
	COLORREF color_viewer_cap;
	COLORREF color_viewer_label;
//protected:
//private:
};

extern ColorSets g_Colorsets;
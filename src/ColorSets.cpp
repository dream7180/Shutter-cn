#include "stdafx.h"
#include "ColorSets.h"

ColorSets g_Colorsets;

ColorSets::ColorSets()
{
	color_gui = ::GetSysColor(COLOR_3DFACE);
	color_text = ::GetSysColor(COLOR_WINDOWTEXT);
	color_text_disabled = ::GetSysColor(COLOR_GRAYTEXT);
	color_sepline = RGB(165, 165, 165);//CalcShade(bkgnd, -31.0f);
	color_sepline_light = RGB(200, 200, 200);//CalcShade(bkgnd, -21.0f);
	color_previewband_bg = RGB(80, 80, 80);
	color_viewer_cap = RGB(180, 180, 180);
	color_viewer_label = RGB(130,130,130);
}

ColorSets::~ColorSets()
{}
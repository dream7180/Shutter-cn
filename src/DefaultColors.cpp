/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "DefaultColors.h"
#include "Color.h"
#include "PhotoCtrl.h"


std::vector<COLORREF> GetPhotoCtrlDefaultColors(const ApplicationColors& app_colors)
{
	std::vector<COLORREF> colors(PhotoCtrl::C_MAX_COLORS);

	auto background = app_colors[AppColors::Background];
	colors[PhotoCtrl::C_BACKGND] = RGB(205, 205, 205);//::GetSysColor(COLOR_WINDOW);//background;
	COLORREF text = app_colors[AppColors::Text]; // ::GetSysColor(COLOR_WINDOWTEXT);
	colors[PhotoCtrl::C_TEXT] = text;
	colors[PhotoCtrl::C_SELECTION] = app_colors[AppColors::Selection];// ::GetSysColor(COLOR_HIGHLIGHT);
	colors[PhotoCtrl::C_SEL_TEXT] = app_colors[AppColors::SelectedText];// ::GetSysColor(COLOR_HIGHLIGHTTEXT);
	colors[PhotoCtrl::C_DISABLED_TEXT] = ::GetSysColor(COLOR_GRAYTEXT);
	colors[PhotoCtrl::C_TAG_BACKGND] = RGB(247, 123, 0);//app_colors[AppColors::Selection];
	colors[PhotoCtrl::C_TAG_TEXT] = RGB(255, 255, 255);
	colors[PhotoCtrl::C_SORT] = RGB(220, 220, 220);
	//{	// calculate sorting indicator color
	//	COLORREF rgb_wnd = background;// ::GetSysColor(COLOR_WINDOW);
	//	int r= GetRValue(rgb_wnd);
	//	int g= GetGValue(rgb_wnd);
	//	int b= GetBValue(rgb_wnd);
	//	if (r > 0xf0 && g > 0xf0 && b > 0xf0)
	//		colors[PhotoCtrl::C_SORT] = RGB(r - 8, g - 8, b - 8);
	//	else
	//		colors[PhotoCtrl::C_SORT] = RGB(std::min(r + 12, 0xff), std::min(g + 12, 0xff), std::min(b + 12, 0xff));
	//}
//	colors[PhotoCtrl::C_FRAME] = ::GetSysColor(COLOR_3DFACE);
	colors[PhotoCtrl::C_SEPARATOR] = app_colors[AppColors::Separator];//app_colors[AppColors::SecondarySeparator]; CalcNewColor(app_colors[AppColors::Separator], background, 0.5f);// ::GetSysColor(COLOR_HIGHLIGHT);
	colors[PhotoCtrl::C_DIM_TEXT] = app_colors[AppColors::DimText];// CalcNewColor(text, 50.0f);
	colors[PhotoCtrl::C_EDIT_BACKGND] = ::GetSysColor(COLOR_WINDOW);//app_colors[AppColors::EditBox];// 
	colors[PhotoCtrl::C_ACTIVEBG] = app_colors[AppColors::Activebg];

	return colors;
}


std::vector<COLORREF> GetViewerWndDefaultColors()
{
	std::vector<COLORREF> colors(6);

	colors[0] = RGB(110,110,110);		// background
	colors[1] = RGB(255, 138, 22);
	// [2]
	colors[3] = RGB(247, 123, 0);//::GetSysColor(COLOR_HIGHLIGHT);	// preview selection color
	colors[4] = colors[3];//RGB(247, 123, 0);	// tag bkgnd
	colors[5] = RGB(255,255,255);	// tag text color

	return colors;
}

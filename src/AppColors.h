/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
#include "ColorConfiguration.h"


enum class AppColors : int
{
	// dominating UI color
	Background = 'bgnd',
	// color of normal text
	Text = 'text',
	// secondary/dim text
	DimText = 'dim',
	// inactive/disabled text
	DisabledText = 'dist',
	// active/bright text
	ActiveText = 'actt',
	// selected text
	SelectedText = 'selt',
	// separator between panels
	Separator = 'sep',
	// secondary separator between UI elements (subtler than separator)
	SecondarySeparator = 'sep2',
	// text input
	EditBox = 'edit',
	// color of selection
	Selection = 'sel',
	Activebg = 'act',
	// slightly highlighted background
	AccentBackground = 'acbg'
};


class ApplicationColors
{
public:
	ApplicationColors()
	{}

	ApplicationColors(const std::vector<std::pair<AppColors, ColorCfg>>& colors)
	{
		SetColors(colors);
	}

	COLORREF operator [] (AppColors key) const
	{
		return map_[static_cast<int>(key)].SelectedColor();
	}

	const ColorCfg& GetColor(AppColors key) const
	{
		return map_[static_cast<int>(key)];
	}

	void SetColors(const std::vector<std::pair<AppColors, ColorCfg>>& colors)
	{
		map_.SetColors((const std::vector<std::pair<int, ColorCfg>>&)(colors));
	}

private:
	ColorConfigurationMap map_;
};


extern ApplicationColors& GetAppColors();

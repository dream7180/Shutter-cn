/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "ColorConfiguration.h"
//#include "AppColors.h"


COLORREF& ColorConfiguration::CustomColor(size_t i)
{
	return colors_.at(i).color;
}


bool& ColorConfiguration::UseCustomColor(size_t i)
{
	return colors_.at(i).use_custom;
}


size_t ColorConfiguration::size() const
{
	return colors_.size();
}


ColorCfg& ColorConfiguration::operator [] (size_t i)
{
	return colors_.at(i);
}


const ColorCfg& ColorConfiguration::operator [] (size_t i) const
{
	return colors_.at(i);
}


//ColorCfg& ColorConfiguration::operator [] (int key)
//{
//	return color_map_.at(key);
//}
//
//const ColorCfg& ColorConfiguration::operator [] (int key) const
//{
//	return color_map_.at(key);
//}

bool ColorConfiguration::operator != (const ColorConfiguration& cc) const
{
	return !(cc == *this);
}


bool ColorConfiguration::operator == (const ColorConfiguration& cc) const
{
	return cc.colors_ == colors_;
}


void ColorConfiguration::Create(const std::vector<COLORREF>& custom_colors,
		const std::vector<char>& use_custom,
		const std::vector<COLORREF>& default_colors)
{
	const size_t count= default_colors.size();

	colors_.clear();
	colors_.resize(count);

	for (size_t i= 0; i < count; ++i)
	{
		ColorCfg& cc= colors_[i];
		cc.color = cc.default_color = default_colors[i];
		cc.use_custom = false;

		if (i < custom_colors.size())
			cc.color = custom_colors[i];

		if (i < use_custom.size())
			cc.use_custom = use_custom[i] != 0;
	}
}

// custom colors only
std::vector<COLORREF> ColorConfiguration::CustomColors() const
{
	const size_t count= colors_.size();
	std::vector<COLORREF> c(count, 0);

	for (size_t i= 0; i < count; ++i)
		c[i] = colors_[i].color;

	return c;
}

// configured colors (default and/or custom depending on the flags)
std::vector<COLORREF> ColorConfiguration::Colors() const
{
	const size_t count= colors_.size();
	std::vector<COLORREF> c(count, 0);

	for (size_t i= 0; i < count; ++i)
		c[i] = colors_[i].SelectedColor();

	return c;
}

// vector of true/false flags
std::vector<char> ColorConfiguration::UseCustomFlags() const
{
	const size_t count= colors_.size();
	std::vector<char> f(count, 0);

	for (size_t i= 0; i < count; ++i)
		f[i] = colors_[i].use_custom;

	return f;
}

/////////////////////////////////////

void ColorConfigurationMap::SetColors(const std::vector<std::pair<int, ColorCfg>>& colors)
{
	color_map_.clear();
	for (auto& c : colors)
		color_map_[c.first] = c.second;
}


const ColorCfg& ColorConfigurationMap::operator [] (int key) const
{
	return color_map_.at(key);
}

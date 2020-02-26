/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once


struct ColorCfg
{
	COLORREF color;
	COLORREF default_color;
	bool use_custom;

	ColorCfg()
	{
		color = 0;
		default_color = 0;
		use_custom = false;
	}

	ColorCfg(COLORREF def) : ColorCfg()
	{
		default_color = def;
	}

	COLORREF SelectedColor() const
	{
		return use_custom ? color : default_color;
	}

	bool operator != (const ColorCfg& cc) const
	{
		return !(cc == *this);
	}

	bool operator == (const ColorCfg& cc) const
	{
		return color == cc.color &&
			default_color == cc.default_color &&
			use_custom == cc.use_custom;
	}
};


class ColorConfiguration
{
public:
	void Create(const std::vector<COLORREF>& custom_colors,
		const std::vector<char>& use_custom,
		const std::vector<COLORREF>& default_colors);

	COLORREF& CustomColor(size_t i);
	bool& UseCustomColor(size_t i);

	ColorCfg& operator [] (size_t i);
	const ColorCfg& operator [] (size_t i) const;

	size_t size() const;

	// custom colors only
	std::vector<COLORREF> CustomColors() const;
	// configured colors (default and/or custom depending on the flags)
	std::vector<COLORREF> Colors() const;
	// vector of true/false flags
	std::vector<char> UseCustomFlags() const;

	bool operator != (const ColorConfiguration& cc) const;
	bool operator == (const ColorConfiguration& cc) const;

private:
	std::vector<ColorCfg> colors_;
};


class ColorConfigurationMap
{
public:
	void SetColors(const std::vector<std::pair<int, ColorCfg>>& colors);

	const ColorCfg& operator [] (int key) const;

private:
	std::unordered_map<int, ColorCfg> color_map_;
};

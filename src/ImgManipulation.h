/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
class Dib;


struct ColorBalanceParams
{
	struct Shift
	{
		double cyan_red;
		double magenta_green;
		double yellow_blue;
	};
	Shift shadows;
	Shift midtones;
	Shift highlights;
};


class ColorBalance
{
public:
	ColorBalance();

	void CalcLookupTables(const ColorBalanceParams& params);

	void Transform(Dib& dib);

	void PreserveLuminosity(bool enable)	{ preserve_luminosity_ = enable; }

private:
	static const int MAX= 256;
	// lookup tables
	uint8 red_[MAX];
	uint8 green_[MAX];
	uint8 blue_[MAX];

	static double transfer_[3][MAX];

	bool preserve_luminosity_;
};


///////////////////////////////////////////////////////////////////////////////


struct LevelParams
{
	enum Channels { RGB, Red, Green, Blue };
	struct Levels
	{
		int min;
		int max;
		double gamma;
	};
	Levels levels_[4];
	Channels channels_;

	LevelParams()
	{
		channels_ = RGB;
		Reset();
	}

	void Reset()
	{
		for (size_t i= RGB; i <= Blue; ++i)
		{
			levels_[i].gamma = 1.0;
			levels_[i].min = 0;
			levels_[i].max = 255;
		}
	}
};


class Levels
{
public:
	Levels();

	void CalcLookupTables(const LevelParams& params);

	void Transform(Dib& dib);

private:
	static const int MAX= 256;
	// lookup tables
	uint8 red_[MAX];
	uint8 green_[MAX];
	uint8 blue_[MAX];
};


///////////////////////////////////////////////////////////////////////////////


struct BrightnessContrastParams
{
	double val[2];		// brightness & contrast: -1..1

	BrightnessContrastParams()
	{
		Reset();
	}

	void Reset()
	{
		val[0] = val[1] = 0.0;
	}
};


class BrightnessContrast
{
public:
	BrightnessContrast();

	void CalcLookupTables(const BrightnessContrastParams& params);

	void Transform(Dib& dib);

private:
	static const int MAX= 256;
	// lookup tables
	uint8 lut_[MAX];
};

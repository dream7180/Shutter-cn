/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "Rational.h"

struct YCbCr_conversion_tables
{
	YCbCr_conversion_tables();

	static const int MAXJSAMPLE= 255;
	static const int CENTERJSAMPLE= 128;
	// not static, so doesn't take up space
	int Crrtab[MAXJSAMPLE + 1];
	int Crgtab[MAXJSAMPLE + 1];
	int Cbgtab[MAXJSAMPLE + 1];
	int Cbbtab[MAXJSAMPLE + 1];
	const uint8* range_limit;

private:
	uint8 limit[5 * MAXJSAMPLE];
};


class color_conversion
{
public:
	virtual void operator () (const uint8* in, uint8* out, uint32 width);

	virtual ~color_conversion() = 0;
};


// this helper class transforms lines of YCbCr with horizontal subsampling == 2 into 24-bit BGR

class YCbCr2_to_BGR : public color_conversion
{
public:
	virtual void operator () (const uint8* in, uint8* out, uint32 width);

private:
	YCbCr_conversion_tables t_;
};


// this helper class transforms lines of YCbCr with no horizontal subsampling into 24-bit BGR

class YCbCr_to_BGR : public color_conversion
{
public:
	virtual void operator () (const uint8* in, uint8* out, uint32 width);

private:
	YCbCr_conversion_tables t_;
};


void CMYK_to_RGB(const uint8* in, uint8* out, uint32 width);

/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// PNGReader.cpp: implementation of the PNGReader class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "PNGReader.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

PNGReader::PNGReader()
{
	png_ = 0;
	info_ = 0;
	number_of_passes_ = 0;
	width_ = height_ = 0;
	bit_depth_ = color_type_ = interlace_type_ = 0;
	scan_line_ = pass_ = 0;
	line_bytes_ = 0u;
}

PNGReader::~PNGReader()
{
	Close();
}


void PNGReader::Close()
{
	if (png_)
	{
		if (void* fp= png_get_io_ptr(png_))
			fclose(reinterpret_cast<FILE*>(fp));
		png_destroy_read_struct(&png_, &info_, nullptr);
	}
}


void PNGAPI PNGReader::ErrorFunc PNGARG((png_structp, png_const_charp))
{
	throw PNGReaderException();
}

void PNGAPI PNGReader::WarnFunc PNGARG((png_structp, png_const_charp))
{}


ImageStat PNGReader::Open(const TCHAR* file_path, double gamma)
{
	FILE* fp= _tfopen(file_path, _T("rb"));

	if (fp == 0)
		return IS_OPEN_ERR;

	try
	{
		png_ = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, &ErrorFunc, &WarnFunc);

		if (png_ == 0)
		{
			fclose(fp);
			return IS_OPEN_ERR;
		}

		// Allocate/initialize the memory for image information
		info_ = png_create_info_struct(png_);
		if (info_ == 0)
		{
			fclose(fp);
			png_destroy_read_struct(&png_, nullptr, nullptr);
			png_ = 0;
			return IS_OPEN_ERR;
		}

		png_init_io(png_, fp);

		png_read_info(png_, info_);

		png_get_IHDR(png_, info_, &width_, &height_, &bit_depth_, &color_type_, &interlace_type_, nullptr, nullptr);

		// tell libpng to strip 16 bit/color files down to 8 bits/color
		png_set_strip_16(png_);

		// strip alpha bytes from the input data without combining with the background
		//png_set_strip_alpha(png_);

		// get rid of transparency, since ExifPro doesn't handle it
		png_color_16 backgnd= { 0, 0xff, 0xff, 0xff, 0xff };
		png_set_background(png_, &backgnd, PNG_BACKGROUND_GAMMA_FILE, 0, 1.0);

		// extract multiple pixels with bit depths of 1, 2, and 4 from a single
		// byte into separate bytes (useful for paletted and grayscale images)
		png_set_packing(png_);

		// expand paletted colors into true RGB triplets
		if (color_type_ == PNG_COLOR_TYPE_PALETTE)
			png_set_palette_to_rgb(png_);
		else if (color_type_ == PNG_COLOR_TYPE_GRAY)
			// Expand grayscale images to the full 8 bits from 1, 2, or 4 bits/pixel
			if (bit_depth_ < 8)
				png_set_expand_gray_1_2_4_to_8(png_);

		if (gamma != 0.0 && gamma != 1.0)
		{
			int intent= 0;
			if (png_get_sRGB(png_, info_, &intent))
				png_set_gamma(png_, gamma, 0.45455);
			else
			{
				double image_gamma;
				if (png_get_gAMA(png_, info_, &image_gamma))
					png_set_gamma(png_, gamma, image_gamma);
				else
					png_set_gamma(png_, gamma, 0.45455);
			}
		}

		// flip the RGB pixels to BGR (or RGBA to BGRA)
		if (color_type_ & PNG_COLOR_MASK_COLOR)
			png_set_bgr(png_);

		// turn on interlace handling; REQUIRED if png_read_image() is not used
		number_of_passes_ = png_set_interlace_handling(png_);

		/* Optional call to gamma correct and add the background to the palette
		* and update info structure.  REQUIRED if you are expecting libpng to
		* update the palette for you (ie you selected such a transform above). */
		png_read_update_info(png_, info_);

	}
	catch (PNGReaderException&)
	{
		return IS_OPEN_ERR;
	}

	return IS_OK;
}


bool PNGReader::IsGrayscale() const
{
	return color_type_ == PNG_COLOR_TYPE_GRAY;
}


bool PNGReader::IsSupported() const
{
	if (width_ > 0xffff || width_ < 1 ||
		height_ > 0xffff || height_ < 1)
		return false;

	return true;
}


ImageStat PNGReader::PrepareReading(int reduction_factor)
{
	line_bytes_ = png_get_rowbytes(png_, info_);
	return IS_OK;
}


ImageStat PNGReader::ReadNextLine(Dib& dib)
{
	if (scan_line_ >= height_)
		if (pass_ >= number_of_passes_)
			return IS_OK;

	if (BYTE* line= dib.LineBuffer(scan_line_++))
		png_read_rows(png_, &line, nullptr, 1);

//	if (scan_line_ >= height_)
//		pass_++;

	return IS_OK;
}


void PNGReader::NextPass()
{
	scan_line_ = 0;
	pass_++;
}

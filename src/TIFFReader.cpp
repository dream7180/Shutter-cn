/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// TIFFReader.cpp: implementation of the TIFFReader class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "TIFFReader.h"
#include "ImageDecoder.h"
#include "ColorProfile.h"
#include "YCbCr_to_BGR.h"
#include "RGB_to_BGR.h"
#include "tifflib/libtiff/tiff.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////

TIFFReader::TIFFReader()
{
	tiff_ = 0;
	buffer_ = 0;
	bits_per_sample_ = samples_per_pixel_ = 0;
	scan_line_ = 0;

	colors_ = 0;
	color_table_r_ = color_table_g_ = color_table_b_ = 0;

	YCbCrSubSampling_[0] = YCbCrSubSampling_[1] = YCbCrPositioning_ = 0;

	exif_offset_ = 0;
	xmp_offset_ = 0;
	intel_order_ = false;

	TIFFLib::TIFFSetWarningHandler(0);
	TIFFLib::TIFFSetErrorHandler(0);
}

TIFFReader::~TIFFReader()
{
	Close();
}


ImageStat TIFFReader::Open(const TCHAR* file_path)
{
	HANDLE file= ::CreateFile(file_path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (file == INVALID_HANDLE_VALUE)
		return IS_OPEN_ERR;

	tiff_ = TIFFLib::TIFFFdOpen(reinterpret_cast<int>(file), "tif", "r");

	if (tiff_ == 0)
	{
		// tifflib closes file handle when error occurs
		//::CloseHandle(file);
	}
	else
	{
		intel_order_ = !TIFFLib::TIFFIsBigEndian(tiff_);

		TIFFLib::TIFFSetDirectory(tiff_, 0);

		TIFFLib::uint32 width= 0, height= 0;
		TIFFLib::TIFFGetField(tiff_, TIFFTAG_IMAGEWIDTH, &width);
		TIFFLib::TIFFGetField(tiff_, TIFFTAG_IMAGELENGTH, &height);

		uint16 BPS= 0;
		TIFFLib::TIFFGetField(tiff_, TIFFTAG_BITSPERSAMPLE, &BPS);

		uint16 SPP= 0;
		TIFFLib::TIFFGetField(tiff_, TIFFTAG_SAMPLESPERPIXEL, &SPP);

		uint16 photometric= 0xffff;
		TIFFLib::TIFFGetField(tiff_, TIFFTAG_PHOTOMETRIC, &photometric);

		TIFFLib::TIFFGetField(tiff_, TIFFTAG_EXIFIFD, &exif_offset_);
		uint32 xmp_length= 0;
		void* xmp_addr= 0;
		TIFFLib::TIFFGetField(tiff_, TIFFTAG_XMLPACKET, &xmp_length, &xmp_addr);//xmp_offset_);	// DNG metadata (field 700)

		// sub IFDs
		//uint16 length= 0;
		//uint32* addr= 0;
		//TIFFLib::TIFFGetField(tiff_, TIFFTAG_SUBIFD, &length, &addr);

		image_size_ = CSize(width, height);
		bits_per_sample_ = physical_bits_per_sample_ = BPS;
		if (physical_bits_per_sample_ == 16)
			bits_per_sample_ = 8;	// simulate 8-bit image
		samples_per_pixel_ = SPP;
		mode_ = photometric;

		if (mode_ == 6)	// YCbCr - Fuji MX-2900 uses this format for 'raw' images
		{
			//TODO: YCbCrCoefficients (0x211); not present in Fuji's TIFF
/*			if (TIFFLib::TIFFGetField(tiff_, 0x211, YCbCrCoefficients_, YCbCrCoefficients_ + 1, YCbCrCoefficients_ + 2) == 0)
			{
				// CCIR Recommendation 601-1
				YCbCrCoefficients_[0].numerator_ = 299;
				YCbCrCoefficients_[0].denominator_ = 1000;
				YCbCrCoefficients_[1].numerator_ = 587;
				YCbCrCoefficients_[1].denominator_ = 1000;
				YCbCrCoefficients_[2].numerator_ = 114;
				YCbCrCoefficients_[2].denominator_ = 1000;
			} */

			//TODO: ReferenceBlackWhite (0x214); it's not present in Fuji's TIFF

			YCbCrSubSampling_[0] = YCbCrSubSampling_[1] = 0;
			TIFFLib::TIFFGetField(tiff_, TIFFTAG_YCBCRSUBSAMPLING, &YCbCrSubSampling_[0], &YCbCrSubSampling_[1]);

			YCbCrPositioning_ = 0;
			TIFFLib::TIFFGetField(tiff_, TIFFTAG_YCBCRPOSITIONING, &YCbCrPositioning_);
		}

		colors_ = 0;

		// color palette available?
		if (photometric == 3 && bits_per_sample_ <= 8)
		{
			TIFFLib::TIFFGetField(tiff_, TIFFTAG_COLORMAP, &color_table_r_, &color_table_g_, &color_table_b_);
			if (color_table_r_ && color_table_g_ && color_table_b_)
				colors_ = 1 << bits_per_sample_;
		}
	}

	return tiff_ ? IS_OK : IS_OUT_OF_MEM;
}

/*
ImageStat TIFFReader::Read()
{
	if (tiff_ == 0)
		return IS_OUT_OF_MEM;

	TIFFLib::tdata_t buf= TIFFLib::_TIFFmalloc(TIFFLib::TIFFStripSize(tiff_));
	if (buf == 0)
		return IS_OUT_OF_MEM;

	for (TIFFLib::tstrip_t strip= 0; strip < TIFFLib::TIFFNumberOfStrips(tiff_); ++strip)
	{
		TIFFLib::TIFFReadEncodedStrip(tiff_, strip, buf, TIFFLib::tsize_t(-1));
	}

	TIFFLib::_TIFFfree(buf);

	return IS_OK;
}*/


bool TIFFReader::IsSupported() const
{
	if (image_size_.cx < 1 || image_size_.cx > 0xffff &&
		image_size_.cy < 1 || image_size_.cy > 0xffff)
		return false;

	if (physical_bits_per_sample_ != 8)
	{
		// 16-bit RGB mode support
		if (physical_bits_per_sample_ == 16 && samples_per_pixel_ == 3 && mode_ == 2)
			return true;

		return false;
	}

	if (samples_per_pixel_ == 4 && mode_ == 5)	// CMYK
		return true;

	if (samples_per_pixel_ != 3 && samples_per_pixel_ != 1)	// TODO
		return false;

	if (mode_ == 6)	// YCbCr
	{
		// this is Fuji 'raw' specific
		if (YCbCrSubSampling_[0] != 2 || YCbCrSubSampling_[1] != 1)
			return false;

		return true;
	}

	if (mode_ == 2 || mode_ == 1 ||			// RGB & grayscale
		(mode_ == 3 && HasColorPalette()))	// color palette
		return true;

	return false;
}


void TIFFReader::Close()
{
	if (tiff_)
	{
		if (buffer_)
			TIFFLib::_TIFFfree(buffer_);
		buffer_ = 0;

		TIFFLib::TIFFClose(tiff_);
		tiff_ = 0;
	}
}


ImageStat TIFFReader::PrepareReading(int reduction_factor)
{
	if (tiff_ == 0)
		return IS_OUT_OF_MEM;

	if (buffer_)
		return IS_OK;

	scan_line_ = 0;
	strip_index_ = 0;
	buffer_ = TIFFLib::_TIFFmalloc(TIFFLib::TIFFStripSize(tiff_));

	return IS_OK;
}


uint32 TIFFReader::GetLineSize() const
{
	if (physical_bits_per_sample_ == 16 && mode_ == 2)
		return samples_per_pixel_ * image_size_.cx * 2;

	if (mode_ != 6)
		return samples_per_pixel_ * image_size_.cx;

	// YCbCr
	if (YCbCrSubSampling_[0] == 2 && YCbCrSubSampling_[1] == 1)
		return samples_per_pixel_ * image_size_.cx * 2 / 3;

	ASSERT(false);	// not handled today
	return 1;
}


ImageStat TIFFReader::ReadNextLine(Dib& dib, const uint8* gamma_table)
{
	if (tiff_ == 0 || buffer_ == 0)
		return IS_OUT_OF_MEM;

	if (strip_index_ < TIFFLib::TIFFNumberOfStrips(tiff_))
	{
		TIFFLib::tmsize_t bytes= TIFFLib::TIFFReadEncodedStrip(tiff_, strip_index_, buffer_, TIFFLib::tsize_t(-1));
		if (bytes < 0)
			return IS_READ_ERROR;

		uint32 src_line_size= GetLineSize();
		TIFFLib::tmsize_t lines= bytes / src_line_size;

		YCbCr2_to_BGR convert;

		const uint8* src_data= reinterpret_cast<const uint8*>(buffer_);
		for (TIFFLib::tmsize_t i= 0; i < lines; ++i)
		{
			if (BYTE* dest_line= dib.LineBuffer(scan_line_++))
			{
				if (mode_ == 6)	// YCbCr
				{
					convert(src_data, dest_line, dib.GetWidth());
				}
				else if (mode_ == 5) // CMYK
				{
					CMYK_to_RGB(src_data, dest_line, dib.GetWidth());
				}
				else if (HasColorPalette())
				{
					for (uint32 i= 0; i < src_line_size; ++i)
					{
						int c= src_data[i];
						*dest_line++ = color_table_b_[c] >> 8;
						*dest_line++ = color_table_g_[c] >> 8;
						*dest_line++ = color_table_r_[c] >> 8;
					}
				}
				else if (GetSamplesPerPixel() == 3)
				{
					if (physical_bits_per_sample_ == 16)
						RGB16_to_BGR(src_data, src_line_size, dest_line, intel_order_);
					else
						RGB_to_BGR(src_data, src_line_size, dest_line);
				}
				else
//				if (gamma_table)
//					ImageDecoder::ApplyGamma(data, line, line_size / samples_per_pixel_, samples_per_pixel_, gamma_table);
//				else
					memcpy(dest_line, src_data, src_line_size);
			}
			src_data += src_line_size;
		}

		++strip_index_;
	}
	else
		return IS_DECODING_FAILED;

	return IS_OK;
}


int TIFFReader::GetBitsPerPixel() const
{
	if (HasColorPalette())
		// ExifPro doesn't support palettes, so transform it into RGB image
		return 3 * bits_per_sample_ * samples_per_pixel_;
	else if (mode_ == 5) // CMYK
		return bits_per_sample_ * 3;	// return bits per pixels for RGB bitmap, not original CMYK tiff
	else
		return bits_per_sample_ * samples_per_pixel_;
}

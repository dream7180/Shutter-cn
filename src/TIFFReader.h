/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// TIFFReader.h: interface for the TIFFReader class.

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

namespace TIFFLib {

	extern "C" {
		#include "tifflib/libtiff/tiffio.h"
	}

}
#include "Dib.h"
#include "ImageStat.h"


class TIFFReader
{
public:
	TIFFReader();
	~TIFFReader();

	ImageStat Open(const TCHAR* file_path);

	ImageStat PrepareReading(int reduction_factor);

	ImageStat ReadNextLine(Dib& dib, const uint8* gamma_table);

	void Close();

	bool IsSupported() const;

	CSize GetSize() const			{ return image_size_; }
	CSize GetResized() const		{ return image_size_; }
	int GetHeight() const			{ return image_size_.cy; }
	int GetScanLine() const			{ return scan_line_; }
	int GetSamplesPerPixel() const	{ return samples_per_pixel_; }
	int GetBitsPerSample() const	{ return bits_per_sample_; }

	int GetBitsPerPixel() const;

	uint32 GetExifOffset() const	{ return exif_offset_; }
	uint32 GetXmpOffset() const		{ return xmp_offset_; }
	bool IsIntelOrder() const		{ return intel_order_; }

private:
	TIFFLib::TIFF* tiff_;
	TIFFLib::tdata_t buffer_;
	CSize image_size_;
	int bits_per_sample_;
	int physical_bits_per_sample_;
	int samples_per_pixel_;
	int mode_;
	int scan_line_;
	int strip_index_;

	uint16* color_table_r_;
	uint16* color_table_g_;
	uint16* color_table_b_;
	int colors_;
	uint16 YCbCrSubSampling_[2];
	uint16 YCbCrPositioning_;
	uint32 exif_offset_;
	uint32 xmp_offset_;
	bool intel_order_;

	bool HasColorPalette() const	{ return colors_ > 0; }

	uint32 GetLineSize() const;
};

/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
#include "Dib.h"
#include "ImageStat.h"
#include "file.h"

//class FileStream;
typedef uint16 Quad[4];


class NEFReader
{
public:
	NEFReader();
	~NEFReader();

	ImageStat Open(const TCHAR* file_path);
	ImageStat PrepareReading(int reduction_factor);
	ImageStat ReadNextLine(Dib& dib, const uint8* gamma_table);

	void Close();

	CSize GetResized() const			{ return CSize(int(iwidth_), int(iheight_)); }
	int GetBitsPerPixel() const			{ return 24; } // TODO
	int GetHeight() const				{ return int(iheight_); }
	CSize GetSize() const				{ return CSize(int(width_), int(height_)); }

	bool IsHalfSize() const				{ return half_size_; }
	uint32 GetTiffOrientation() const	{ return orientation_; }

private:

	void ParseIFD(FileStream& nef, uint32 ifd_offset);
	void ParseExif(FileStream& nef, uint32 base);
	void ParseMakerNote(FileStream& nef);

	FileStream nef_;
	uint32 width_;
	uint32 raw_width_;
	uint32 height_;
	uint32 iwidth_;
	uint32 iheight_;
	uint32 compression_;
	uint32 data_offset_;
	uint32 orientation_;
	uint32 tiff_samples_;
	uint32 nikon_curve_offset_;
	std::string make_;
	std::string model_;
	uint32 left_margin_;
	uint32 filters_;
	uint32 shrink_;
	uint32 colors_;
	Quad* image_;
	std::vector<uint16> image_buf_;
	bool half_size_;
	std::vector<uint8> contrast_curve_;
	double camera_blue_;
	double camera_red_;
	bool four_color_rgb_;
	bool use_coeff_;
	float pre_mul_[4];
	float coeff_[3][4];
	bool use_camera_wb_;
	bool use_auto_wb_;
	int histogram_[3][0x2000];
	uint32 jpeg_data_offset_;
	uint32 jpeg_data_length_;
	uint32 bits_per_sample_;

	struct decode
	{
		decode() : leaf(0)
		{
			branch[0] = branch[1] = 0;
		}
		decode* branch[2];
		int leaf;
	};

	decode first_decode_[2048];
	decode* second_decode_;
	decode* free_decode_;

	void make_decoder(const uint8* source, int level);
	int leaf_;
	uint32 bitbuf_;
	int vbits_;
	int maximum_;

	int ljpeg_diff(decode* dindex);
	bool is_compressed();
	uint32 getbits(int nbits);
	int LoadCompressedRaw();
	int LoadRaw();
	void ScaleColors(int black, int maximum, uint32 width, uint32 height);
	void ConvertToRGB(int clip_max, uint32 width, uint32 height);

	void dng_coeff(double cc[4][4], double cm[4][3], double xyz[3]);
	void adobe_coeff();
	void vng_interpolate(bool quick_interpolate);

	void eight_bit_load_raw();
};

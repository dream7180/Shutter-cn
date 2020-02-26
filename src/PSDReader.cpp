/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// PSDReader.cpp: implementation of the PSDReader class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PSDReader.h"
#include "File.h"
#include "Dib.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

PSDReader::PSDReader()
{
	channels_ = depth_ = mode_ = 0;
	rows_ = columns_ = 0;
	r = g = b = 0;
	scan_line_ = height_ = 0;
	compressed_ = false;
	reduction_factor_ = 1;
}

/*
PSDReader::~PSDReader()
{

}


bool PSDReader::OpenFile(const TCHAR* file_name, Dib& dib, CSize& img_size)
{
	FileStream ifs;
	if (!ifs.Open(file_name))
		return false;

	if (!OpenHeader(ifs))
		return false;

	if (!IsSupported())
		return false;

	// skip color mode data

	ifs.RPosition(ifs.GetUInt32());

	// skip image resources

	ifs.RPosition(ifs.GetUInt32());

	// skip layer and mask info

	ifs.RPosition(ifs.GetUInt32());

	// image data follows

	uint16 compression= ifs.GetUInt16();

	// read PSD

	// plane size for 8 bps image
	uint32 plane_size= rows_ * columns_;

	Offset start= ifs.RPosition();
	uint32 total_size= ifs.GetLength32();

	if (start + 3 * plane_size > total_size)
		return false;			// truncated image data

	// memory mapped PSD file
	const uint8* base= ifs.GetMapBaseAddr();
	const uint8* r= base + start;
	const uint8* g= r + plane_size;
	const uint8* b= g + plane_size;

	dib.Create(columns_, rows_, 24);

	for (uint32 y= 0; y < rows_; ++y)
	{
		BYTE* line= dib.LineBuffer(y);

		for (uint32 x= 0; x < columns_; ++x)
		{
			line[0] = *b++;
			line[1] = *g++;
			line[2] = *r++;
			line += 3;
		}
	}

	img_size = CSize(columns_, rows_);

	return true;
}
*/

bool PSDReader::OpenHeader(FileStream& ifs)
{
	if (ifs.GetLength32() < 26)	// PSD header's size
		return false;

	ifs.SetByteOrder(true);		// big endian

	if (ifs.GetUInt32() != '8BPS')
		return false;

	if (ifs.GetUInt16() != 0x0001)	// version
		return false;

	if (ifs.GetUInt32() != 0 || ifs.GetUInt16() != 0)	// reserved 6 zero bytes
		return false;

	channels_	= ifs.GetUInt16();
	rows_		= ifs.GetUInt32();
	columns_	= ifs.GetUInt32();
	depth_	= ifs.GetUInt16();
	mode_		= ifs.GetUInt16();

	return true;
}


bool PSDReader::IsSupported() const
{
	if (channels_ != 3)
		return false;

	if (rows_ == 0 || columns_ == 0)
		return false;

	if (depth_ != 8)	// 8 bits per pixel
		return false;

	if (mode_ != 3)	// RGB only
		return false;

	// for safety
	if (columns_ >= 0x10000 || rows_ >= 0x10000 ||
		columns_ < 1 || rows_ < 1)
		return false;

	return true;
}


void PSDReader::ReadNextLine(FileStream& ifs, Dib& dib, const uint8* gamma_table)
{
	if (scan_line_ >= height_)
		return;

	ASSERT(reduction_factor_ >= 1);

	BYTE* line= dib.LineBuffer(GetScanLine());

	if (!compressed_)
	{
		if (reduction_factor_ == 1)
		{
			if (gamma_table)
				for (uint32 x= 0; x < columns_; ++x)
				{
					line[0] = gamma_table[*b++];
					line[1] = gamma_table[*g++];
					line[2] = gamma_table[*r++];
					line += 3;
				}
			else
				for (uint32 x= 0; x < columns_; ++x)
				{
					line[0] = *b++;
					line[1] = *g++;
					line[2] = *r++;
					line += 3;
				}
		}
		else
		{
			size_t delta= scan_line_ * columns_;
			const uint8* pr= r + delta;
			const uint8* pg= g + delta;
			const uint8* pb= b + delta;

			if (gamma_table)
				for (uint32 x= 0; x < columns_; x += reduction_factor_)
				{
					line[0] = gamma_table[*pb]; pb += reduction_factor_;
					line[1] = gamma_table[*pg]; pg += reduction_factor_;
					line[2] = gamma_table[*pr]; pr += reduction_factor_;
					line += 3;
				}
			else
				for (uint32 x= 0; x < columns_; x += reduction_factor_)
				{
					line[0] = *pb; pb += reduction_factor_;
					line[1] = *pg; pg += reduction_factor_;
					line[2] = *pr; pr += reduction_factor_;
					line += 3;
				}
		}
	}
	else	// compressed data (RLE)
	{
		PackBitsDecode(r, &buffer_red_.front(), line_byte_counts_red_[scan_line_], columns_);
		PackBitsDecode(g, &buffer_green_.front(), line_byte_counts_green_[scan_line_], columns_);
		PackBitsDecode(b, &buffer_blue_.front(), line_byte_counts_blue_[scan_line_], columns_);

		for (int i= 0; i < reduction_factor_; ++i)
		{
			r += line_byte_counts_red_[scan_line_ + i];
			g += line_byte_counts_green_[scan_line_ + i];
			b += line_byte_counts_blue_[scan_line_ + i];
		}

		if (gamma_table)
			for (uint32 x= 0; x < columns_; x += reduction_factor_)
			{
				line[0] = gamma_table[buffer_blue_[x]];
				line[1] = gamma_table[buffer_green_[x]];
				line[2] = gamma_table[buffer_red_[x]];
				line += 3;
			}
		else
			for (uint32 x= 0; x < columns_; x += reduction_factor_)
			{
				line[0] = buffer_blue_[x];
				line[1] = buffer_green_[x];
				line[2] = buffer_red_[x];
				line += 3;
			}
	}

	scan_line_ = std::min(scan_line_ + reduction_factor_, height_);
}


void PSDReader::PackBitsDecode(const uint8* rle, uint8* scanline, uint16 rle_data_count, int32 width)
{
	while (rle_data_count > 0)
	{
		int cnt= static_cast<int8>(*rle++);
		--rle_data_count;

		if (cnt < 0)	// repeat next byte -cnt + 1 times
		{
			if (cnt == -128)
				continue;
			cnt = -cnt + 1;

			if (cnt > width)
				cnt = width;

			width -= cnt;

			uint8 val= *rle++;

			--rle_data_count;

			while (cnt-- > 0)
				*scanline++ = val;
		}
		else	// copy next cnt + 1 bytes
		{
			cnt++;

			if (cnt > width)
				cnt = width;

			memcpy(scanline, rle, cnt);
			scanline += cnt;
			rle += cnt;
			width -= cnt;
			rle_data_count -= cnt;
		}
	}
}


bool PSDReader::PrepareReading(FileStream& ifs, int reduction_factor/*= 1*/)
{
	if (!IsSupported())
		return false;

	reduction_factor_ = reduction_factor;

	// skip color mode data

	ifs.RPosition(ifs.GetUInt32());
//TRACE(L"pos: %x\n", ifs.RPosition());
	// skip image resources

	ifs.RPosition(ifs.GetUInt32());
//TRACE(L"pos: %x\n", ifs.RPosition());

	// skip layer and mask info

	ifs.RPosition(ifs.GetUInt32());
//TRACE(L"pos: %x\n", ifs.RPosition());

	// image data follows

	uint16 compression= ifs.GetUInt16();

	// prepare for reading image data

	if (compression == 0)		// no compression
	{
		compressed_ = false;

		// plane size for 8 bps image
		uint32 plane_size= rows_ * columns_;

		Offset start= ifs.RPosition();
		uint32 total_size= ifs.GetLength32();

		if (start + 3 * plane_size > total_size)
			return false;			// truncated image data

		// memory mapped PSD file
		const uint8* base= ifs.GetMapBaseAddr();
		r = base + start;
		g = r + plane_size;
		b = g + plane_size;
	}
	else if (compression == 1)
	{
		compressed_ = true;

		line_byte_counts_red_.resize(rows_);
		line_byte_counts_green_.resize(rows_);
		line_byte_counts_blue_.resize(rows_);

		uint32 red_size= 0;
		{
			for (uint32 y= 0; y < rows_; ++y)
			{
				// line bytes count
				uint16 count= ifs.GetUInt16();
				line_byte_counts_red_[y] = count;
				red_size += count;
			}
		}
		uint32 green_size= 0;
		{
			for (uint32 y= 0; y < rows_; ++y)
			{
				uint16 count= ifs.GetUInt16();
				line_byte_counts_green_[y] = count;
				green_size += count;
			}
		}
		uint32 blue_size= 0;
		{
			for (uint32 y= 0; y < rows_; ++y)
			{
				uint16 count= ifs.GetUInt16();
				line_byte_counts_blue_[y] = count;
				blue_size += count;
			}
		}

		if (channels_ > 3)
		{
			// skip channels byte counts
			ifs.RPosition(rows_ * 2 * (channels_ - 3));
		}

		Offset start= ifs.RPosition();
		uint32 total_size= ifs.GetLength32();

		if (start + red_size + green_size + blue_size > total_size)
			return false;			// truncated image data

		// memory mapped PSD file
		const uint8* base= ifs.GetMapBaseAddr();
		r = base + start;
		g = r + red_size;
		b = g + green_size;

		buffer_red_.resize(columns_);
		buffer_green_.resize(columns_);
		buffer_blue_.resize(columns_);
	}
	else
		return false;	// unsupported compression

	height_ = rows_;
	scan_line_ = 0;

	return true;
}

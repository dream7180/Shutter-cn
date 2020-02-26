/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// PSDReader.h: interface for the PSDReader class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PSDREADER_H__0A7ABCBF_50AF_4DD2_930B_2C968EF7A852__INCLUDED_)
#define AFX_PSDREADER_H__0A7ABCBF_50AF_4DD2_930B_2C968EF7A852__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
class FileStream;
class Dib;


class PSDReader
{
public:
	PSDReader();
//	virtual ~PSDReader();

	bool IsSupported() const;

	bool OpenHeader(FileStream& ifs);

	bool PrepareReading(FileStream& ifs, int reduction_factor= 1);

	CSize GetSize() const			{ return CSize(columns_, rows_); }
	CSize GetResized() const		{ return CSize(columns_ / reduction_factor_, rows_ / reduction_factor_); }

	void ReadNextLine(FileStream& ifs, Dib& bmp, const uint8* gamma_table);

	// current scan line
	int GetScanLine() const			{ return scan_line_ / reduction_factor_; }
	int GetHeight() const			{ return height_ / reduction_factor_; }

private:
	int channels_;
	int depth_;
	int mode_;
	uint32 rows_;
	uint32 columns_;
	int reduction_factor_;
	int height_;
	int scan_line_;	// current scan line

	bool compressed_;
	std::vector<uint16> line_byte_counts_red_;
	std::vector<uint16> line_byte_counts_green_;
	std::vector<uint16> line_byte_counts_blue_;
	const uint8* r;
	const uint8* g;
	const uint8* b;
	std::vector<uint8> buffer_red_;
	std::vector<uint8> buffer_green_;
	std::vector<uint8> buffer_blue_;

	static void PackBitsDecode(const uint8* rle, uint8* scanline, uint16 rle_data_count, int32 width);
};

#endif // !defined(AFX_PSDREADER_H__0A7ABCBF_50AF_4DD2_930B_2C968EF7A852__INCLUDED_)

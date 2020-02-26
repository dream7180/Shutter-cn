/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// PNGReader.h: interface for the PNGReader class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PNGREADER_H__EA6AE4A9_38B6_455A_9D53_08AD4CC131D7__INCLUDED_)
#define AFX_PNGREADER_H__EA6AE4A9_38B6_455A_9D53_08AD4CC131D7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "pnglib/png.h"
#include "ImageStat.h"
#include "Dib.h"


class PNGReaderException
{
};

class PNGReader
{
public:
	PNGReader();
	~PNGReader();

	ImageStat Open(const TCHAR* file_path, double gamma);

	void Close();

	bool IsSupported() const;
	bool IsGrayscale() const;

	ImageStat PrepareReading(int reduction_factor);

	ImageStat ReadNextLine(Dib& dib);

	CSize GetSize() const		{ return CSize(width_, height_); }
	CSize GetResized() const	{ return CSize(width_, height_); }

	int GetScanLine() const		{ return scan_line_; }
	int GetHeight() const		{ return height_; }

	int GetPass() const			{ return pass_; }
	int GetNoOfPasses() const	{ return number_of_passes_; }

	// start next pass
	void NextPass();

private:
	png_structp png_;
	png_infop info_;

	int number_of_passes_;
	png_uint_32 width_;
	png_uint_32 height_;
	int bit_depth_;
	int color_type_;
	int interlace_type_;

	int pass_;
	int scan_line_;
	size_t line_bytes_;

	static void PNGAPI ErrorFunc PNGARG((png_structp, png_const_charp));
	static void PNGAPI WarnFunc PNGARG((png_structp, png_const_charp));
};

#endif // !defined(AFX_PNGREADER_H__EA6AE4A9_38B6_455A_9D53_08AD4CC131D7__INCLUDED_)

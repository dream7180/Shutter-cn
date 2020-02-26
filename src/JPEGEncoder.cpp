/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// JPEGEncoder.cpp: implementation of the JPEGEncoder class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "JPEGEncoder.h"
#include "JPEGDataDestination.h"
#include "Dib.h"
#include "JPEGException.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

METHODDEF(void) jpglib_error_exit(j_common_ptr cinfo)
{
	throw JPEGException(cinfo);
}

JPEGEncoder::JPEGEncoder(int quality/*= 90*/, bool progressive/*= true*/, bool baseline/*= true*/)
{
	ASSERT(quality >= 0 && quality <= 100);
	quality_ = quality;
	progressive_ = progressive;
	baseline_ = baseline;

	err = jpeg_std_error(&jerr_);
	jerr_.error_exit = jpglib_error_exit;
	jpeg_create_compress(this);
}


JPEGEncoder::~JPEGEncoder()
{
	jpeg_destroy_compress(this);
}


bool JPEGEncoder::Encode(JPEGDataDestination& dest, Dib* bmp, const std::vector<uint8>* app_markers/*= 0*/)
{
	jpeg_compress_struct::dest = &dest;

	image_width = bmp->GetWidth();
	image_height = bmp->GetHeight();

	switch (bmp->GetColorComponents())
	{
	case 1:
		input_components = 1;			// number of color channels (samples per pixel)
		in_color_space = JCS_GRAYSCALE;	// colorspace of input image
		break;
	case 3:
		input_components = 3;
		in_color_space = JCS_EXT_BGR; //JCS_RGB;
		break;
	default:
		ASSERT(false);	// unsupported bitmap format
		return false;
	}

	jpeg_set_defaults(this);

	// baseline_ -> if true limit to baseline-JPEG values
	jpeg_set_quality(this, quality_, baseline_);

	// integer DCT implementation--slow and accurate
	dct_method = JDCT_ISLOW;

	if (progressive_)
		jpeg_simple_progression(this);

	// start compress writes JPEG header
	jpeg_start_compress(this, TRUE);

	// now is time to write app markers (if any)
	if (app_markers && !app_markers->empty())
	{
		ASSERT(app_markers->size() > 4);
		dest.Output(*app_markers);
	}

	// write scan lines

	if (!LinesEncoded(0, false))
	{
		dest.Abort();
		jpeg_abort_compress(this);
		return false;
	}

	while (next_scanline < image_height)
	{
		JSAMPROW row= bmp->LineBuffer(next_scanline);
		jpeg_write_scanlines(this, &row, 1);

		// report progress
		if (!LinesEncoded(next_scanline, next_scanline == image_height))
		{
			dest.Abort();
			jpeg_abort_compress(this);
			return false;
		}
	}

	jpeg_finish_compress(this);

	return true;
}


bool JPEGEncoder::LinesEncoded(int lines_ready, bool finished)
{
	if (fn_progress_)
		return fn_progress_(lines_ready, ScanLines(), finished);

	return true;
}

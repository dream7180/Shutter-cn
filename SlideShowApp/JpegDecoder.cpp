// CJPEGDecoder.cpp: implementation of the CJpegDecoder class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "JPEGDecoder.h"
#include "Dib.h"
#include "JPEGException.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

METHODDEF(void) jpglib_error_exit(j_common_ptr cinfo)
{
	throw CJPEGException(cinfo);
/* Example:
  // cinfo->err really points to a my_error_mgr struct, so coerce pointer
  my_error_ptr myerr = (my_error_ptr) cinfo->err;

  // Always display the message.
  // We could postpone this until after returning, if we chose.
  (*cinfo->err->output_message) (cinfo);

  // Return control to the setjmp point
  longjmp(myerr->setjmp_buffer, 1);
*/
}



CJPEGDecoder::CJPEGDecoder(CJPEGDataSource& data)
{
	m_bFast = false;
	m_nDCTMethod = 0;
	m_sizeOriginal = MSize(0, 0);
	jpeg_create_decompress(this);
	err = jpeg_std_error(&m_jerr);
	m_jerr.error_exit = jpglib_error_exit;
	src = &data;
}


CJPEGDecoder::CJPEGDecoder(int nDCTMethod/*= 0*/)
{
	m_bFast = false;
	m_nDCTMethod = nDCTMethod;
	m_sizeOriginal = MSize(0, 0);
	jpeg_create_decompress(this);
	err = jpeg_std_error(&m_jerr);
	m_jerr.error_exit = jpglib_error_exit;
//	src = &data;
}


CJPEGDecoder::~CJPEGDecoder()
{
	jpeg_destroy_decompress(this);
}


bool CJPEGDecoder::Decode(CJPEGDataSource& src, CDib* pBmp, MSize sizeImg/*= MSize(0, 0)*/, COLORREF rgbBack/*= RGB(255,255,255)*/)
{
	jpeg_decompress_struct::src = &src;

	jpeg_read_header(this, true);

	// set decoding params
	SetParams(sizeImg, m_bFast, false);

	jpeg_start_decompress(this);

/*	output_width				image width and height, as scaled
	output_height
	out_color_components		# of color components in out_color_space
	output_components			# of color components returned per pixel
	colormap					the selected colormap, if any
	actual_number_of_colors		number of entries in colormap
*/

//	AutoPtr<CDib> pDib(new CDib(output_width, output_height, output_components));
	pBmp->Create(output_width, output_height, 8 * output_components);

	if (!LinesDecoded(0, false))
	{
		src.Abort();
		jpeg_abort_decompress(this);
		return false;
	}

	while (output_scanline < output_height)
	{
		JSAMPROW row= pBmp->LineBuffer(output_scanline);
		jpeg_read_scanlines(this, &row, 1);

		// report progress
		if (!LinesDecoded(output_scanline, output_scanline == output_height))
		{
			src.Abort();
			jpeg_abort_decompress(this);
			return false;
		}
	}

	jpeg_finish_decompress(this);

	return true;
}


AutoPtr<CDib> CJPEGDecoder::Decode(int nScaleDenominator/*= 1*/)
{
	jpeg_read_header(this, true);

	// params
	scale_num = 1;
	scale_denom = nScaleDenominator;

	jpeg_start_decompress(this);

/*	output_width		image width and height, as scaled
	output_height
	out_color_components	# of color components in out_color_space
	output_components	# of color components returned per pixel
	colormap		the selected colormap, if any
	actual_number_of_colors		number of entries in colormap */

	AutoPtr<CDib> pDib(new CDib(output_width, output_height, output_components * 8));

	while (output_scanline < output_height)
	{
		JSAMPROW row= pDib->LineBuffer(output_scanline); //output_height -  - 1);
		jpeg_read_scanlines(this, &row, 1);
	}

	jpeg_finish_decompress(this);

	return pDib;
}


bool CJPEGDecoder::LinesDecoded(int nLinesReady, bool bFinished)
{
	return true;
}


void CJPEGDecoder::SetParams(MSize& sizeImg, bool bFast, bool bRescaling)
{
	m_sizeOriginal = MSize(image_width, image_height);

	if (bRescaling && sizeImg.cx > 0 && sizeImg.cy > 0)
	{
		// swap width & height if needed (some photos are rotated)
		// we support only uniform rescaling--no distortions
		if (m_sizeOriginal.cx > m_sizeOriginal.cy && sizeImg.cx < sizeImg.cy ||
			m_sizeOriginal.cx < m_sizeOriginal.cy && sizeImg.cx > sizeImg.cy)
		{
			LONG nTmp= sizeImg.cx;
			sizeImg.cx = sizeImg.cy;
			sizeImg.cy = nTmp;
		}
	}

	if (bFast)
	{
		int w= image_width;
		int h= image_height;

		do_fancy_upsampling = FALSE;
		dct_method = JDCT_IFAST;
		two_pass_quantize = FALSE;	// one-pass
		double dScale= min(w / 120.0, h / 80.0);	// thumbnail size
		scale_num = 1;
		if (dScale >= 8.0)
			scale_denom = 8;
		else if (dScale >= 4.0)
			scale_denom = 4;
		else if (dScale >= 2.0)
			scale_denom = 2;
		else
			scale_denom = 1;
		//out_color_space = JCS_GRAYSCALE;
//		w = (image_width + scale_denom - 1) / scale_denom;
//		h = (image_height + scale_denom - 1) / scale_denom;
	}
	else
	{
		// Choose floating point DCT method.
		do_fancy_upsampling = true;
		dct_method = JDCT_ISLOW;
		if (m_nDCTMethod == 1)
		   dct_method = JDCT_IFAST;
		else if (m_nDCTMethod == 2)
		   dct_method = JDCT_FLOAT;

		scale_num = 1;
		scale_denom = 1;

		if (sizeImg.cx > 0 && sizeImg.cy > 0)
		{
			jpeg_calc_output_dimensions(this);
			if (output_width < 1 || output_height < 1)
				return;
			double dRatio= double(output_width) / double(output_height);
			if (double(sizeImg.cx) / double(sizeImg.cy) > dRatio)
				sizeImg.cx = static_cast<int>(sizeImg.cy * dRatio);
			else
				sizeImg.cy = static_cast<int>(sizeImg.cx / dRatio);

			int nTimesX= output_width / sizeImg.cx;
			int nTimesY= output_height / sizeImg.cy;

//			if (nTimesX >= 16 && nTimesY >= 16)
//				scale_denom = 16;
			if (nTimesX >= 8 && nTimesY >= 8)
				scale_denom = 8;
			else if (nTimesX >= 4 && nTimesY >= 4)
				scale_denom = 4;
			else if (nTimesX >= 2 && nTimesY >= 2)
				scale_denom = 2;
		}
	}
}

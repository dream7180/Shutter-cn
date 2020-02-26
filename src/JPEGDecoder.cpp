/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// JPEGDecoder.cpp: implementation of the CJpegDecoder class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "jpeglib.h"
#include "JPEGDataSource.h"
#include "JPEGDecoder.h"
#include "Dib.h"
#include "JPEGException.h"
#include "ThumbnailSize.h"
#include "ColorProfile.h"
#include "ICMProfile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


struct JPEGDecoder::Impl
{
	Impl(AutoPtr<JPEGDataSource> data, JpegDecoderMethod dct_method) : data_(data)
	{
		Init(data_.get(), dct_method);
	}

	Impl(JPEGDataSource& data, JpegDecoderMethod dct_method)
	{
		Init(&data, dct_method);
	}

	~Impl();

	struct jpeg_decompress_struct decoder_;
	void SetParams(CSize& img_size, bool fast, bool rescaling);
	struct jpeg_error_mgr jerr_;
	bool fast_;						// faster decoding, lower quality
	bool thumbnail_decoding_;		// thumbnail img decoding: do not smaller than predefined hardcoded size
	bool enable_size_reduction_;	// faster decoding (skipping lines and columns if img sufficiently big)
	JpegDecoderMethod dct_method_;	// DCT method type
	AutoPtr<JPEGDataSource> data_;

	void Init(JPEGDataSource* data, JpegDecoderMethod dct_method);
	ImageStat DecodeJpeg(JPEGDecoder* img_decoder, Dib& bmp, CSize& img_size, bool resize,
		COLORREF rgb_back, J_COLOR_SPACE color_space= JCS_UNKNOWN);
};


METHODDEF(void) jpglib_error_exit(j_common_ptr cinfo)
{
	throw JPEGException(cinfo);
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


JPEGDecoder::JPEGDecoder(AutoPtr<JPEGDataSource> data, JpegDecoderMethod dct_method)
 : pImpl_(new Impl(data, dct_method))
{}


JPEGDecoder::JPEGDecoder(JPEGDataSource& data, JpegDecoderMethod dct_method/*= 0*/)
 : pImpl_(new Impl(data, dct_method))
{}


void JPEGDecoder::Impl::Init(JPEGDataSource* data, JpegDecoderMethod dct_method)
{
	fast_ = false;
	thumbnail_decoding_ = false;
	enable_size_reduction_ = true;
	dct_method_ = dct_method;

	jpeg_create_decompress(&decoder_);
	decoder_.err = jpeg_std_error(&jerr_);
	jerr_.error_exit = jpglib_error_exit;
	decoder_.src = data;
}



JPEGDecoder::Impl::~Impl()
{
	jpeg_destroy_decompress(&decoder_);
}


JPEGDecoder::~JPEGDecoder()
{
}


//int JPEGDecoder::ScanLines() const
//{
//	return output_height;
//}


ImageStat JPEGDecoder::DecodeImg(Dib& bmp)
{
	return DecodeImg(bmp, CSize(0, 0), false);
}


ImageStat JPEGDecoder::Impl::DecodeJpeg(JPEGDecoder* img_decoder, Dib& bmp, CSize& img_size, bool resize,
										 COLORREF rgb_back, J_COLOR_SPACE color_space)
{
	ASSERT(decoder_.src != 0);
	JPEGDataSource* src= static_cast<JPEGDataSource*>(decoder_.src);

	jpeg_read_header(&decoder_, true);

	if (decoder_.jpeg_color_space == JCS_GRAYSCALE)
		decoder_.out_color_space = JCS_GRAYSCALE;
	else
		decoder_.out_color_space = JCS_EXT_BGR;

	// request color space conversion
	if (color_space != JCS_UNKNOWN)
		decoder_.out_color_space = color_space;

	//HACK: force RGB for CMYK
	if (color_space == JCS_UNKNOWN && decoder_.out_color_space == JCS_CMYK)
		decoder_.out_color_space = JCS_EXT_BGR; //JCS_RGB;

	// set decoding params
	SetParams(img_size, fast_, resize);

	jpeg_start_decompress(&decoder_);


/*	output_width				image width and height, as scaled
	output_height
	out_color_components		# of color components in out_color_space
	output_components			# of color components returned per pixel
	colormap					the selected colormap, if any
	actual_number_of_colors		number of entries in colormap */

	bmp.Create(decoder_.output_width, decoder_.output_height, 8 * decoder_.output_components, 0);

	//SetICCProfiles(trans.in_, trans.out_, trans.rendering_intent_);

	img_decoder->SetTotalLines(decoder_.output_height);

	if (!img_decoder->LinesDecoded(0, false))
	{
		src->Abort();
		jpeg_abort_decompress(&decoder_);
		return IS_DECODING_CANCELLED;
	}

	while (decoder_.output_scanline < decoder_.output_height)
	{
		JSAMPROW row= bmp.LineBuffer(decoder_.output_scanline);
		jpeg_read_scanlines(&decoder_, &row, 1);

		if (img_decoder->IsICCEnabled())
			img_decoder->ApplyICC(row, decoder_.output_width, decoder_.output_components);
			//ApplyGamma(row, output_width, output_components, gamma_);

		// report progress
		bool last_line= decoder_.output_scanline == decoder_.output_height;
		if (!img_decoder->LinesDecoded(decoder_.output_scanline, last_line))
		{
			src->Abort();
			jpeg_abort_decompress(&decoder_);
			return IS_DECODING_CANCELLED;
		}
//::Sleep(1);
	}

	jpeg_finish_decompress(&decoder_);

	return IS_OK;
}


/*bool JPEGDecoder::LinesDecoded(int lines_ready, bool finished)
{
	return true;
}*/


void JPEGDecoder::Impl::SetParams(CSize& img_size, bool fast, bool rescaling)
{
	CSize original_size= CSize(decoder_.image_width, decoder_.image_height);

	if (rescaling && img_size.cx > 0 && img_size.cy > 0)
	{
		// swap width & height if needed (some photos are rotated)
		// we support only uniform rescaling--no distortions
		if (original_size.cx > original_size.cy && img_size.cx < img_size.cy ||
			original_size.cx < original_size.cy && img_size.cx > img_size.cy)
		{
			std::swap(img_size.cx, img_size.cy);
		}
	}

	if (fast)
	{
		int w= decoder_.image_width;
		int h= decoder_.image_height;

		decoder_.do_fancy_upsampling = FALSE;
		decoder_.dct_method = JDCT_IFAST;
		decoder_.two_pass_quantize = FALSE;	// one-pass

		double scale= 0.0;

		if (thumbnail_decoding_)
		{
			// thumbnail img decoding: not smaller than min_width by min_height

			double min_width= img_size.cx > 0 ? img_size.cx /** 3 / 4*/ : 120;
			double min_height= img_size.cy > 0 ? img_size.cy /** 2 / 3*/ : 80;
			scale = std::min(w / min_width, h / min_height);	// thumbnail size
		}
		else
		{
			// not a thumbnail decoding: try to limit img size to the given 'img_size'

			if (img_size.cx == 0)
				img_size.cx = STD_THUMBNAIL_WIDTH;
			if (img_size.cy == 0)
				img_size.cy = STD_THUMBNAIL_HEIGHT;
			if (h > w && img_size.cx > img_size.cy)
				std::swap(img_size.cx, img_size.cy);

			scale = std::min(w / (img_size.cx * 19 / 10), h / (img_size.cy * 19 / 10));
		}

		// set divider for decoder
		decoder_.scale_num = 1;

		// currently jpeg lib doesn't support 16 times reduction
		/*if (scale >= 16.0)
			decoder_.scale_denom = 16;
		else*/ if (scale >= 8.0)
			decoder_.scale_denom = 8;
		else if (scale >= 4.0)
			decoder_.scale_denom = 4;
		else if (scale >= 2.0)
			decoder_.scale_denom = 2;
		else
			decoder_.scale_denom = 1;

		//out_color_space = JCS_GRAYSCALE;
//		w = (image_width + scale_denom - 1) / scale_denom;
//		h = (image_height + scale_denom - 1) / scale_denom;
	}
	else
	{
		// Choose floating point DCT method.
		decoder_.do_fancy_upsampling = true;
		decoder_.dct_method = JDCT_ISLOW;
		if (dct_method_ == JDEC_INTEGER_LOQ)
		   decoder_.dct_method = JDCT_IFAST;
		else if (dct_method_ == JDEC_FLOATING_POINT)
		   decoder_.dct_method = JDCT_FLOAT;

		decoder_.scale_num = 1;
		decoder_.scale_denom = 1;

		if (img_size.cx > 0 && img_size.cy > 0 && enable_size_reduction_)
		{
			jpeg_calc_output_dimensions(&decoder_);
			if (decoder_.output_width < 1 || decoder_.output_height < 1)
				return;
			double ratio= double(decoder_.output_width) / double(decoder_.output_height);
			if (double(img_size.cx) / double(img_size.cy) > ratio)
				img_size.cx = std::max(static_cast<long>(img_size.cy * ratio), 1L);
			else
				img_size.cy = std::max(static_cast<long>(img_size.cx / ratio), 1L);

			int times_x= decoder_.output_width / img_size.cx;
			int times_y= decoder_.output_height / img_size.cy;

//			if (times_x >= 16 && times_y >= 16)
//				scale_denom = 16;
			if (times_x >= 8 && times_y >= 8)
				decoder_.scale_denom = 8;
			else if (times_x >= 4 && times_y >= 4)
				decoder_.scale_denom = 4;
			else if (times_x >= 2 && times_y >= 2)
				decoder_.scale_denom = 2;
		}
	}
}


int JPEGDecoder::ReductionFactor() const
{
	ASSERT(pImpl_->decoder_.scale_num <= 1);
	return pImpl_->decoder_.scale_denom;
}


CSize JPEGDecoder::GetOriginalSize() const
{
	return CSize(pImpl_->decoder_.image_width, pImpl_->decoder_.image_height);
}


#define CATCH_ERRORS \
	catch (JPEGException&) \
	{ \
		throw;	/* those are ok, report them */ \
	} \
	catch (...) \
	{ \
		/* broken or malicious JPEG data can cause access violation in JPEG lib; catch it */ \
		throw JPEGException(_T("Fatal error encountered in JPEG decoder")); \
	}


ImageStat JPEGDecoder::DecodeImgToYCbCr(Dib& bmp, CSize& img_size, bool resize, COLORREF rgb_back/*= RGB(255,255,255)*/)
{
	try
	{
		return pImpl_->DecodeJpeg(this, bmp, img_size, resize, rgb_back, JCS_YCbCr);
	}
	catch (JPEGException& ex)
	{
		if (ex.GetMessageCode() == JERR_CONVERSION_NOTIMPL)
			return IS_OPERATION_NOT_SUPPORTED;
		else
			throw;
	}
	catch (...)
	{
		/* broken or malicious JPEG data can cause access violation in JPEG lib; catch it */
		throw JPEGException(_T("Fatal error encountered in JPEG decoder"));
	}
}


ImageStat JPEGDecoder::DecodeImg(Dib& bmp, CSize& img_size, bool resize, COLORREF rgb_back/*= RGB(255,255,255)*/)
{
//LARGE_INTEGER tm[9];
//::QueryPerformanceCounter(&tm[0]);

	try
	{
		ImageStat stat= pImpl_->DecodeJpeg(this, bmp, img_size, resize, rgb_back);

//::QueryPerformanceCounter(&tm[1]);
//LARGE_INTEGER tt;
//tt.QuadPart = tm[1].QuadPart - tm[0].QuadPart;
//int nn= tt.LowPart;
//
//::QueryPerformanceFrequency(&tm[0]);
//double d= nn;
//d /= tm[0].QuadPart;
//TCHAR buf[128];
//_stprintf(buf, _T("JPEG decoding time: %f sec\n"), d);
//OutputDebugString(buf);

		return stat;
	}
	CATCH_ERRORS
}

#undef CATCH_ERRORS


void JPEGDecoder::SetFast(bool fast_decoding, bool thumbnail_decoding)
{
	pImpl_->fast_ = fast_decoding;
	pImpl_->thumbnail_decoding_ = thumbnail_decoding;
}


void JPEGDecoder::SetQuality(Quality quality)
{
	switch (quality)
	{
	case FAST_LOW_QUALITY:
		pImpl_->fast_ = true;
		pImpl_->enable_size_reduction_ = true;
		break;

	case NORMAL_QUALITY:
		pImpl_->fast_ = false;
		pImpl_->enable_size_reduction_ = true;
		break;

	case SLOW_HIGH_QUALITY:
		pImpl_->fast_ = false;
		pImpl_->enable_size_reduction_ = false;
		break;

	default:
		ASSERT(false);
		break;
	}
}


void JPEGDecoder::SetDCTMethod(JpegDecoderMethod method)
{
	pImpl_->dct_method_ = method;
}


CSize JPEGDecoder::DecodeJpegImageDimensions(JPEGDataSource& src)
{
	std::auto_ptr<Impl> impl(new Impl(src, JDEC_INTEGER_HIQ));

	jpeg_read_header(&impl->decoder_, true);

	return CSize(impl->decoder_.output_width, impl->decoder_.output_height);
}

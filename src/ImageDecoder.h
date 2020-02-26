/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// ImageDecoder.h: interface for the ImageDecoder class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IMAGEDECODER_H__C6E5F06F_0FE4_4E79_A093_2EFA5AD181BB__INCLUDED_)
#define AFX_IMAGEDECODER_H__C6E5F06F_0FE4_4E79_A093_2EFA5AD181BB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "AutoPtr.h"
#include "Dib.h"
#include "DecoderProgress.h"
#include "ImageStat.h"
#include "ColorProfile.h"
#include <boost/function.hpp>


class ImageDecoder : public mik::counter_base
{
public:
	ImageDecoder() : progress_(0) {}
	ImageDecoder(DecoderProgress* progress) : progress_(progress) {}
	virtual ~ImageDecoder();

	// decode image to the bitmap
	virtual ImageStat DecodeImg(Dib& bmp);

	// decode img to the bitmap;
	// if img_size != (0, 0) then img may be reduced in size but cannot be smaller than img_size
	// if resize == true then image will be resized to exact size of img_size preserving aspect original ratio
	virtual ImageStat DecodeImg(Dib& bmp, CSize& img_size, bool resize, COLORREF rgb_back= RGB(255,255,255)) = 0;

	// this is decoding img to the YCbCr format rather than RBG (similarity testing needs that)
	virtual ImageStat DecodeImgToYCbCr(Dib& bmp, CSize& img_size, bool resize, COLORREF rgb_back= RGB(255,255,255));

	// recipient of progress information
	void SetProgressClient(DecoderProgress* progress)	{ progress_ = progress; }

	// original image's size
	virtual CSize GetOriginalSize() const = 0; //		{ return original_size_; }

	// how many times smaller then the original image
	virtual int ReductionFactor() const;

	// apply gamma correction to the buffer of pixels
	//static void ApplyGamma(uint8* line_buffer, uint32 width, uint32 components, const uint8* gamma_table);
	//static void ApplyGamma(const uint8* input, uint8* line_buffer, uint32 width, uint32 components, const uint8* gamma_table);

	enum Quality { FAST_LOW_QUALITY, NORMAL_QUALITY, SLOW_HIGH_QUALITY };
	// set decoding quality (hint for a decoder)
	virtual void SetQuality(Quality quality);

	// set profiles for color correction
	void SetICCProfiles(ColorProfilePtr input, ColorProfilePtr display, int rendering_intent);

	// apply color correction transformation
	void ApplyICC(const uint8* input, uint8* output, uint32 width, uint32 components);
	// ditto (in place)
	void ApplyICC(uint8* line_buffer, uint32 width, uint32 components);

	typedef boost::function<bool (int lines_ready, int img_height, bool finished)> Callback;

	void SetProgressCallback(const Callback& fn)	{ fn_progress_ = fn; }

protected:
	DecoderProgress* progress_;
	//uint8 gamma_[256];			// gamma transformation table (same for R, G and B)
	ColorProfilePtr input_profile_;
	ColorProfilePtr display_profile_;
	ColorTransformPtr rgb_;
	ColorTransformPtr gray_;
	std::vector<BYTE> line_buffer_;

	bool IsICCEnabled() const			{ return rgb_ != 0; }

	void SetTotalLines(int height)
	{
		scan_lines_ = height;
		if (progress_)
			progress_->scan_lines_ = height;
	}

	// progress info
	virtual bool LinesDecoded(int lines_ready, bool finished);

	// calculate possible reduction factor and calc img_size to match the original image aspect ratio
	int CalcReductionFactor(CSize& img_size, bool resize) const;

	// calculate gamma table
//	void PrepareGammaTable(double gamma);
private:
	int scan_lines_;
	Callback fn_progress_;
};


typedef mik::intrusive_ptr<ImageDecoder> CImageDecoderPtr;


#endif // !defined(AFX_IMAGEDECODER_H__C6E5F06F_0FE4_4E79_A093_2EFA5AD181BB__INCLUDED_)

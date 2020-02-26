/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// JPEGDecoder.h: interface for the CJpegDecoder class.

#pragma once
#include "ImageDecoder.h"
#include "JPEGDecoderMethod.h"
class Dib;
struct ICMTransform;
class JPEGDataSource;


class JPEGDecoder : public ImageDecoder
{
public:
	JPEGDecoder(JPEGDataSource& data, JpegDecoderMethod dct_method= JDEC_INTEGER_HIQ);
	JPEGDecoder(AutoPtr<JPEGDataSource> data, JpegDecoderMethod dct_method);
	virtual ~JPEGDecoder();

	virtual ImageStat DecodeImg(Dib& bmp);
	virtual ImageStat DecodeImg(Dib& bmp, CSize& img_size, bool resize, COLORREF rgb_back= RGB(255,255,255));
	virtual ImageStat DecodeImgToYCbCr(Dib& bmp, CSize& img_size, bool resize, COLORREF rgb_back= RGB(255,255,255));

	// set fast decoding (low quality)
	void SetFast(bool fast_decoding, bool thumbnail_decoding);

	// select Discrete Cosine Transform implementation
	void SetDCTMethod(JpegDecoderMethod method);

	virtual int ReductionFactor() const;

	virtual CSize GetOriginalSize() const;

	virtual void SetQuality(Quality quality);

	// scan header to find out real image size
	static CSize DecodeJpegImageDimensions(JPEGDataSource& src);

private:
	struct Impl;
	friend struct Impl;
	std::auto_ptr<Impl> pImpl_;
};

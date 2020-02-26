/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "PhotoInfo.h"
#include "ExifBlock.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

extern int transform_jpeg(int transform, int x, int y, int width, int height,
						  const TCHAR* input_file, const TCHAR* output_file,
						  int* pnOutWidth, int* pnOutHeight);


int Difference(const Dib& d1, const Dib& d2)
{
	if (d1.GetSize() != d2.GetSize() || d1.GetColorComponents() != 3 || d2.GetColorComponents() != 3)
	{
		ASSERT(false);
		return 0xffffff;
	}

	int diff= 0;

	for (int i= 0; i < d1.GetHeight(); ++i)
	{
		const BYTE* p1= d1.LineBuffer(i);
		const BYTE* p2= d2.LineBuffer(i);

		const int width= d1.GetWidth();
		for (int x= 0; x < width; ++x)
		{
			diff += abs(static_cast<int>(p1[0]) - static_cast<int>(p2[0]));
			diff += abs(static_cast<int>(p1[1]) - static_cast<int>(p2[1]));
			diff += abs(static_cast<int>(p1[2]) - static_cast<int>(p2[2]));
			p1 += 3;
			p2 += 3;
		}
	}

	return diff;
}


extern int FixRotatedPhoto(int& transf, PhotoInfo& img, const TCHAR* output_file)
{
	CImageDecoderPtr decoder= img.GetDecoder();

	decoder->SetQuality(ImageDecoder::FAST_LOW_QUALITY);

	const CSize MATRIX(32, 32);

	// load and scale it down (coarse)
	CSize image_size(MATRIX);
	Dib dib;

	if (ImageStat status= decoder->DecodeImg(dib, image_size, false))
		return -1;

	dib.Resize(MATRIX, 0, false);

	String path= img.GetPhysicalPath();
	if (path.empty())
		return 9999;

	ExifBlock exif;
	bool exif_present= img.Scan(path.c_str(), exif, false, nullptr);
	img.bmp_ = 0;

	if (!exif_present)
		return 9999;

	if (img.jpeg_thumb_.IsEmpty())
		return 9999;

	Dib thm;
	if (img.jpeg_thumb_.GetDib(thm))
		return -1;

	thm.Resize(MATRIX, 0, false);

	const double div= 1.0 / (MATRIX.cx * MATRIX.cy);
	double upright= Difference(dib, thm) * div;

	thm.RotateInPlace(true);

	double clkwise= Difference(dib, thm) * div;

	thm.RotateInPlace(false);
	thm.RotateInPlace(false);

	double cntrclkwise= Difference(dib, thm) * div;

	// look for a smallest difference

	if (upright < clkwise && upright < cntrclkwise)
		return 9999;	// nothing to change: thumbnail's and main image orientations match

	double times= 0.0;
	if (clkwise < cntrclkwise)
		times = clkwise > 0.0 ? std::min(cntrclkwise, upright) / clkwise : 0.0;
	else
		times = cntrclkwise > 0.0 ? clkwise / std::min(cntrclkwise, upright) : 0.0;

	if (times < 1.1)	// 10% of confidence at least
		return 9999;	// skip it: too small a difference between the two rotated images

	// copy to output

	if (::CopyFile(path.c_str(), output_file, false) == 0)
		return -1;

	transf = clkwise < cntrclkwise ? ROTATE_90_DEG_CW : ROTATE_90_DEG_COUNTERCW;

	return 0;
}


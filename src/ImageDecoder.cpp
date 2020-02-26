/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// ImageDecoder.cpp: implementation of the ImageDecoder class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ImageDecoder.h"
#include <math.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


ImageDecoder::~ImageDecoder()
{
	scan_lines_ = 0;
}


int ImageDecoder::ReductionFactor() const
{
	return 1;	// no reduction
}


ImageStat ImageDecoder::DecodeImg(Dib& bmp)
{
	return DecodeImg(bmp, CSize(0, 0), false);
}

	
int ImageDecoder::CalcReductionFactor(CSize& img_size, bool resize) const
{
	CSize original_size= GetOriginalSize();
	int reduction_factor= 1;

	if (img_size.cx > 0 && img_size.cy > 0)
	{
		if (resize)
		{
			// swap width & height if needed (some photos are rotated)
			// we support only uniform rescaling--no distortions
			if (original_size.cx > original_size.cy && img_size.cx < img_size.cy ||
				original_size.cx < original_size.cy && img_size.cx > img_size.cy)
			{
				std::swap(img_size.cx, img_size.cy);
			}
		}

		if (original_size.cx < 1 || original_size.cy < 1)
			return 1;

		double ratio= double(original_size.cx) / double(original_size.cy);
		if (double(img_size.cx) / double(img_size.cy) > ratio)
			img_size.cx = std::max<long>(1, long(img_size.cy * ratio + 0.5));
		else
			img_size.cy = std::max<long>(1, long(img_size.cx / ratio + 0.5));

		int times_x= original_size.cx / img_size.cx;
		int times_y= original_size.cy / img_size.cy;

		if (times_x >= 16 && times_y >= 16)
			reduction_factor = 16;
		if (times_x >= 8 && times_y >= 8)
			reduction_factor = 8;
		else if (times_x >= 4 && times_y >= 4)
			reduction_factor = 4;
		else if (times_x >= 2 && times_y >= 2)
			reduction_factor = 2;
	}

	return reduction_factor;
}


//void ImageDecoder::PrepareGammaTable(double gamma)
//{
//	if (gamma <= 0.0 || gamma == 1.0)
//		use_gamma_ = false;
//	else
//	{
//		gamma = 1 / gamma;
//		for (int i= 0; i < 256; ++i)
//			gamma_[i] = static_cast<uint8>(pow(i / 255.0, gamma) * 255);
//
//		use_gamma_ = true;
//	}
//}


//void ImageDecoder::ApplyGamma(const uint8* input, uint8* line_buffer, uint32 width, uint32 components, const uint8* gamma_table)
//{
//	if (components == 3)
//	{
//		__asm
//		{
//			push edi
//			push esi
//			push ecx
//
//			mov  ecx, dword ptr [width]
//			cmp	 ecx, 0
//			jz   exit_loop
//			mov  ebx, gamma_table
//			mov  esi, dword ptr [line_buffer]
//			mov  edi, dword ptr [input]
//		xloop:
//			sub	 ecx, 1
//			xor  eax,eax
//
//			mov  al, byte ptr [edi]
//			mov  dl, byte ptr [ebx + eax]
//			mov  byte ptr [esi],dl
//
//			mov  al, byte ptr [edi+1]
//			mov  dl, byte ptr [ebx + eax]
//			mov  byte ptr [esi+1],dl
//
//			mov  al, byte ptr [edi+2]
//			mov  dl, byte ptr [ebx + eax]
//			mov  byte ptr [esi+2],dl
//				
//			add  esi, 3	  // 24 bpp
//			add  edi, 3	  // 24 bpp
//
//			cmp	 ecx, 0
//			jnz  xloop
//
//		exit_loop:
//			pop ecx
//			pop esi
//			pop edi
//		}
//	}
//	else if (components == 1)
//	{
//		uint8* end= line_buffer + width;
//		for ( ; line_buffer < end; ++line_buffer)
//			*line_buffer = gamma_table[*input++];
//	}
//}
//
//
//void ImageDecoder::ApplyGamma(uint8* line_buffer, uint32 width, uint32 components, const uint8* gamma_table)
//{
//	if (components == 3)
//	{
//		__asm
//		{
//			push edi
//			push esi
//			push ecx
//			mov  edi, dword ptr [width]
//			cmp	 edi, 0
//			jz   exit_loop
//			mov  ecx, gamma_table
//			mov  esi, dword ptr [line_buffer]
//		xloop:
//			sub	 edi, 1
//			xor  eax,eax
//
//			mov  al, byte ptr [esi]
//			mov  dl, byte ptr [ecx + eax]
//			mov  byte ptr [esi],dl
//
//			mov  al, byte ptr [esi+1]
//			mov  dl, byte ptr [ecx + eax]
//			mov  byte ptr [esi+1],dl
//
//			mov  al, byte ptr [esi+2]
//			mov  dl, byte ptr [ecx + eax]
//			mov  byte ptr [esi+2],dl
//				
//			add  esi, 3	  // 24 bpp
//
//			cmp	 edi, 0
//			jnz  xloop
//
//		exit_loop:
//			pop ecx
//			pop esi
//			pop edi
//		}
//	}
//	else if (components == 1)
//	{
//		uint8* end= line_buffer + width;
//		for ( ; line_buffer < end; ++line_buffer)
//			*line_buffer = gamma_table[*line_buffer];
//	}
//}


ImageStat ImageDecoder::DecodeImgToYCbCr(Dib& bmp, CSize& img_size, bool resize, COLORREF rgb_back/*= RGB(255,255,255)*/)
{
	return IS_OPERATION_NOT_SUPPORTED;
}


void ImageDecoder::SetQuality(Quality quality)
{}


void ImageDecoder::SetICCProfiles(ColorProfilePtr input, ColorProfilePtr display, int rendering_intent)
{
	input_profile_ = input;
	display_profile_ = display;

	if (input && display)
	{
		rgb_ = new ColorTransform();
		//gray_ = new ColorTransform();

		rgb_->Create(*input, TYPE_BGR_8, *display, TYPE_BGR_8, rendering_intent);
		//gray_->Create(*input, TYPE_GRAY_8, *display, TYPE_GRAY_8, rendering_intent);
	}
}


// apply color correction transformation
void ImageDecoder::ApplyICC(const uint8* input, uint8* output, uint32 width, uint32 components)
{
	if (components == 3)
	{
		if (rgb_ && *rgb_)
			rgb_->Transform(input, output, width);
		else
			memcpy(output, input, width * components);
	}
	else if (components == 1)
	{
		if (gray_ && *gray_)
			gray_->Transform(input, output, width);
		else
			memcpy(output, input, width);
	}
}


void ImageDecoder::ApplyICC(uint8* line_buffer, uint32 width, uint32 components)
{
	uint32 size= width * components;

	if (size == 0)
		return;

	if (line_buffer_.size() < size)
		line_buffer_.resize(size);
//TODO: revise
// maybe transform form/to same buffer works
	ApplyICC(line_buffer, &line_buffer_[0], width, components);

	memcpy(line_buffer, &line_buffer_[0], size);
}


bool ImageDecoder::LinesDecoded(int lines_ready, bool finished)
{
	if (fn_progress_)
		return fn_progress_(lines_ready, scan_lines_, finished);
	if (progress_)
		return progress_->LinesDecoded(lines_ready, finished);
	return true;
}

/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "BmpFunc.h"
#include <math.h>
#include "ThumbnailSize.h"
#include "IsMMXAvailable.h"

#ifdef _DEBUG
#define new		new(__FILE__, __LINE__)
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


void Rotate(const Dib* bmp, Dib* copy, bool clockwise, int lines_from, int lines_to)
{
	ASSERT(bmp != 0);
	ASSERT(copy != 0);

	int width= bmp->GetWidth();
	int height= bmp->GetHeight();
	int bpp= bmp->GetBitsPerPixel();
	if (bpp != 24)
		return;

	if (clockwise)
	{
		for (int line= lines_from; line < lines_to; ++line)
		{
			int len= bmp->GetWidth(); // this may be too much by one: bmp->GetBytesPerLine() / 3;
			const BYTE* src= bmp->LineBuffer(line);
			int offset= (height - line - 1) * 3;
			BYTE* dest= copy->LineBuffer(0) + offset;
			for (int x= 0; x < len; ++x, src += 3)
			{
				dest[0] = src[0];
				dest[1] = src[1];
				dest[2] = src[2];

				dest -= copy->GetBytesPerLine();
			}
		}
	}
	else
	{
		for (int line= lines_from; line < lines_to; ++line)
		{
			const BYTE* src= bmp->LineBuffer(line);

			int offset= line * 3;
			int len= copy->GetHeight();
			BYTE* dest= copy->LineBuffer(len - 1) + offset;
			for (int x= len - 1; x >= 0; --x, src += 3)
			{
				dest[0] = src[0];
				dest[1] = src[1];
				dest[2] = src[2];
				// dib's upside down, so keep on adding to the dest pointer
				dest += copy->GetBytesPerLine();
			}

		}
	}
}


bool Rotate180(const Dib* bmp, Dib* copy, int lines_from, int lines_to)
{
	ASSERT(bmp != 0);
	ASSERT(copy != 0);

	int width= bmp->GetWidth();
	int height= bmp->GetHeight();
	int bpp= bmp->GetBitsPerPixel();
	if (bpp != 24)
		return false;

	for (int line= lines_from; line < lines_to; ++line)
	{
		const BYTE* src= bmp->LineBuffer(line);
		BYTE* dst= copy->LinePixelBuffer(height - 1 - line, width - 1);

		for (int x= 0; x < width; ++x)
		{
			dst[0] = src[0];
			dst[1] = src[1];
			dst[2] = src[2];

			dst -= 3;
			src += 3;
		}
	}

	return true;
}


struct BITMAPINFO_TBL : BITMAPINFO
{
	BITMAPINFO_TBL()
	{
		for (int i= 0; i < 256; ++i)
		{
			bmiColors[i].rgbBlue = i;		// initialize color palette with shades of gray
			bmiColors[i].rgbGreen = i;
			bmiColors[i].rgbRed = i;
			bmiColors[i].rgbReserved = 0;
		}
	}

	RGBQUAD color_table_[255];	// this vector must follow BITMAPINFO struct
};


int CopyBitmap(Dib* thumb_bmp, CBitmap& dest_bmp, PhotoInfo::ImgOrientation rotation/*= PhotoInfo::ORIENT_NORMAL*/)
{
	BITMAP bm;
	dest_bmp.GetObject(sizeof(bm), &bm);

	CDC dest_dc;
	dest_dc.CreateCompatibleDC(0);
	dest_dc.SelectObject(&dest_bmp);
	dest_dc.PatBlt(0, 0, bm.bmWidth, bm.bmHeight, WHITENESS);
	CRect rect(0, 0, bm.bmWidth, bm.bmHeight);
	CBrush brushFrame(RGB(192,192,192));
	dest_dc.FrameRect(rect, &brushFrame);		// gray frame
	int x= 0;
	int y= 0;
	int dest_w= bm.bmWidth;
	int dest_h= bm.bmHeight;

	Dib* src_bmp= thumb_bmp;
	AutoPtr<Dib> copy;
	bool adjust_size= false;
	bool no_std_thumbnail= false;

	if (rotation == PhotoInfo::ORIENT_90CW)
	{
		copy = thumb_bmp->RotateCopy(false);
		if (copy.get() != 0)
		{
			src_bmp = copy.get();
			adjust_size = true;
		}
	}
	else if (rotation == PhotoInfo::ORIENT_90CCW)
	{
		copy = thumb_bmp->RotateCopy(true);
		if (copy.get() != 0)
		{
			src_bmp = copy.get();
			adjust_size = true;
		}
	}
	else if (src_bmp->GetWidth() != STD_THUMBNAIL_WIDTH || src_bmp->GetHeight() != STD_THUMBNAIL_HEIGHT)
	{
		no_std_thumbnail = true;
	}

	if (adjust_size || no_std_thumbnail)
	{
		double ratio_img= double(src_bmp->GetWidth()) / double(src_bmp->GetHeight());
		if (STD_THUMBNAIL_ASPECT_RATIO > ratio_img)
			dest_w = static_cast<int>(dest_h * ratio_img);
		else
			dest_h = static_cast<int>(dest_w / ratio_img);
		x = (bm.bmWidth - dest_w) / 2;
		y = (bm.bmHeight - dest_h) / 2;
	}

	BITMAPINFO_TBL bi;
	bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
	bi.bmiHeader.biWidth = src_bmp->GetWidth();
	bi.bmiHeader.biHeight = src_bmp->GetHeight();
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biBitCount = src_bmp->GetBitsPerPixel();
	bi.bmiHeader.biCompression = BI_RGB;
	bi.bmiHeader.biSizeImage = 0;
	bi.bmiHeader.biXPelsPerMeter = 0;
	bi.bmiHeader.biYPelsPerMeter = 0;
	bi.bmiHeader.biClrUsed = 0;
	bi.bmiHeader.biClrImportant = 0;

	dest_dc.SetStretchBltMode(HALFTONE);
	::StretchDIBits(dest_dc, x, y, dest_w, dest_h, 0, 0, src_bmp->GetWidth(), src_bmp->GetHeight(), src_bmp->GetBuffer(), &bi, DIB_RGB_COLORS, SRCCOPY);

	return (adjust_size ? 1 : 0) | (no_std_thumbnail ? 2 : 0);
}


AutoPtr<Dib> ApplyGamma(Dib* bmp, double gamma)
{
#ifdef _WIN64
	int width= bmp->GetWidth();
	int height= bmp->GetHeight();
	int bpp= bmp->GetBitsPerPixel();
	if (bpp != 24)
		return 0;
	return new Dib(width, height, bpp);
#else

	/*
DWORD timer= ::GetTickCount();
while (timer == ::GetTickCount())
	;
timer = ::GetTickCount();
while (timer == ::GetTickCount())
	;
*/
	if (bmp == 0)
		return 0;

	int width= bmp->GetWidth();
	int height= bmp->GetHeight();
	int bpp= bmp->GetBitsPerPixel();
	if (bpp != 24)
		return 0;

	BYTE gamma_table[256];
	gamma = 1.0 / gamma;
	for (int i= 0; i < 256; ++i)
		gamma_table[i] = static_cast<BYTE>(pow(i / 255.0, gamma) * 255);

	AutoPtr<Dib> copy= new Dib(width, height, bpp);

	for (int line= 0; line < height; ++line)
	{
		int len= bmp->GetBytesPerLine() / 3;
		BYTE* src= bmp->LineBuffer(line); //GetLineArray()[line];
		BYTE* dest= copy->LineBuffer(line); //GetLineArray()[line];

		__asm
		{
			push edi
			push esi
			mov  edi, dword ptr [len]
			cmp  edi, 0
			jz   exit_loop
			mov  esi,dword ptr [src]
			mov  ecx,dword ptr [dest]
		xloop:
			sub  edi, 1
			xor  eax,eax

			mov  al,byte ptr [esi]
			mov  dl,byte ptr gamma_table[eax]
			mov  byte ptr [ecx],dl

			mov  al,byte ptr [esi+1]
			mov  dl,byte ptr gamma_table[eax]
			mov  byte ptr [ecx+1],dl

			mov  al,byte ptr [esi+2]
			mov  dl,byte ptr gamma_table[eax]
			mov  byte ptr [ecx+2],dl

			add  esi, 3
			add  ecx, 3

			cmp  edi, 0
			jnz  xloop

		exit_loop:
			pop esi
			pop edi
		}

/*------ this is equivalent code:
		for (int x= 0; x < len; ++x)
		{
			dest[0] = gamma_table[src[0]];
			dest[1] = gamma_table[src[1]];
			dest[2] = gamma_table[src[2]];
			src += 3;
			dest += 3;
		}
------------------------------*/
	}
/*
timer = ::GetTickCount() - timer;
char buf[256];
wsprintf(buf, "time: %d", timer);
AfxMessageBox(buf, MB_OK);
*/
	return copy;
#endif
}



void ApplyGammaInPlace(Dib* bmp, double gamma, int line_from, int line_to)
{
#ifdef _WIN64
#pragma message("ApplyGammaInPlace: not implemented")
	return; //TODO
#else
/*
DWORD timer= ::GetTickCount();
while (timer == ::GetTickCount())
	;
timer = ::GetTickCount();
while (timer == ::GetTickCount())
	;
*/
	if (bmp == 0)
		return;

	int width= bmp->GetWidth();
	int height= bmp->GetHeight();
	int bpp= bmp->GetBitsPerPixel();
	if (bpp != 24)
		return;

	BYTE gamma_table[256];
	gamma = 1 / gamma;
	for (int i= 0; i < 256; ++i)
		gamma_table[i] = static_cast<BYTE>(pow(i / 255.0, gamma) * 255);

	if (line_from < 0)
		line_from = 0;
	if (line_to < 0)
		line_to = height;
	ASSERT(line_from <= line_to);

	for (int line= line_from; line < line_to; ++line)
	{
		int len= bmp->GetBytesPerLine() / (bpp / 8);
		BYTE* src= bmp->LineBuffer(line); //GetLineArray()[line];

		__asm
		{
			push edi
			push esi
			mov  edi, dword ptr [len]
			jz   exit_loop
			mov  esi,dword ptr [src]
		xloop:
			sub	 edi, 1
			xor  eax,eax

			mov  al,byte ptr [esi]
			mov  dl,byte ptr gamma_table[eax]
			mov  byte ptr [esi],dl

			mov  al,byte ptr [esi+1]
			mov  dl,byte ptr gamma_table[eax]
			mov  byte ptr [esi+1],dl

			mov  al,byte ptr [esi+2]
			mov  dl,byte ptr gamma_table[eax]
			mov  byte ptr [esi+2],dl
				
			add  esi, 3	  // 24 bpp

			cmp	 edi, 0
			jnz  xloop

		exit_loop:
			pop esi
			pop edi
		}
	}
/*
timer = ::GetTickCount() - timer;
char buf[256];
wsprintf(buf, "time: %d", timer);
AfxMessageBox(buf, MB_OK);
*/
#endif
}


namespace {


struct FilterParam		// zoom-specific filter params
{
	double scale;		// filter scale
	double support;		// scaled filter support radius
	uint32 width;		// filter width

	FilterParam(double img_scale, double filter_support)
	{
		double blur= img_scale >= 1.0 ? 1.0 : 0.8;
		scale = blur * std::max(1.0, 1.0 / img_scale);
		support = std::max(0.5, scale * filter_support);
		width = static_cast<uint32>(ceil(2.0 * support));
	}
};


struct Filter
{
	Filter(double support) : support(support)
	{}

	double support;

	virtual void make_weight_table(double center, const FilterParam& param, int len, int* output) const = 0;
};


static const int WEIGHTBITS= 14;				// # bits in filter coefficients
static const int WEIGHTONE= 1 << WEIGHTBITS;	// filter weight of one


template <class Fn>
struct FilterT : public Filter
{
	FilterT() : Filter(Fn::SUPPORT)
	{}

	virtual void make_weight_table(double center, const FilterParam& param, int len, int* output) const
	{
		int i0= static_cast<int>(center - param.support + 0.5);
		int i1= static_cast<int>(center + param.support + 0.5);

		if (i0 < 0)
			i0 = 0;
		if (i1 > len)
			i1 = len;

		ASSERT(i0 < i1);

		output[0] = i0;
		output[1] = i1;

		int* weight= &output[2];
		int sum= 0;
		for (int i= i0; i < i1; ++i)
		{
			// evaluate the filter function:
			int t= Fn()((i + 0.5 - center) / param.scale);
			*weight++ = t;
			sum += t;
		}

		if (int div= sum)
		{
			int* weight= &output[2];
			sum = 0;
			for (int i= i0; i < i1; ++i)
			{
				int t= *weight * WEIGHTONE / div;
				sum += t;
				*weight++ = t;
			}
		
			if (sum != WEIGHTONE)
				weight[-1] += WEIGHTONE - sum;
		}

	//    if (tr<MINSHORT || tr>MAXSHORT) {
    //        fprintf(stderr, "tr=%g at %d\n", tr, b);
    //        exit(1);
    //    }
    //    t = (int) floor(tr+.5);
    //    if (stillzero && t==0) i0++;    /* find first nonzero */
    //    else {
    //        stillzero = 0;
    //        *wp++ = t;                  /* add weight to table */
    //        sum += t;
    //        if (t!=0) lastnonzero = i;  /* find last nonzero */
    //    }
    //}

		;
	}
};


struct Triangle
{
	static const int SUPPORT= 1;

	int operator () (double x)
	{
		if (x < -1.0)
			return 0;
		if (x < 0.0)
			return WEIGHTONE + static_cast<int>(x * WEIGHTONE);
		if (x < 1.0)
			return WEIGHTONE - static_cast<int>(x * WEIGHTONE);
		return 0;
	}
};


struct Cubic
{
	static const int SUPPORT= 2;

	int operator () (double x)
	{
		if (x < -2.0)
			return 0;
		if (x < -1.0)
		{ double t= x + 2.0; return static_cast<int>(t * t * t / 6.0 * WEIGHTONE); }
		if (x < 0.0)
		{ return static_cast<int>((4.0 + x * x * (-3.0 * x - 6.0)) / 6.0 * WEIGHTONE); }
		if (x < 1.0)
		{ return static_cast<int>((4.0 + x * x * (3.0 * x - 6.0)) / 6.0 * WEIGHTONE); }
		if (x < 2.0)
		{ double t= 2.0 - x; return static_cast<int>(t * t * t / 6.0 * WEIGHTONE); }

		return 0;
	}
};


struct Mitchell
{
	static const int SUPPORT= 2;

	int operator () (double x)
	{
		const double b= 1.0 / 3.0;
		const double c= 1.0 / 3.0;
		const double p0= (  6.0 -  2.0 * b           ) / 6.0;
		const double p2= (-18.0 + 12.0 * b +  6.0 * c) / 6.0;
		const double p3= ( 12.0 -  9.0 * b -  6.0 * c) / 6.0;
		const double q0= (         8.0 * b + 24.0 * c) / 6.0;
		const double q1= (      - 12.0 * b - 48.0 * c) / 6.0;
		const double q2= (         6.0 * b + 30.0 * c) / 6.0;
		const double q3= (      -        b -  6.0 * c) / 6.0;

		if (x < -2.0)
			return 0;
		if (x < -1.0)
			return static_cast<int>((q0 - x * (q1 - x * (q2 - x * q3))) * WEIGHTONE);
		if (x < 0.0)
			return static_cast<int>((p0 + x * x * (p2 - x * p3)) * WEIGHTONE);
		if (x < 1.0)
			return static_cast<int>((p0 + x * x * (p2 + x * p3)) * WEIGHTONE);
		if (x < 2.0)
			return static_cast<int>((q0 + x * (q1 + x * (q2 + x * q3))) * WEIGHTONE);

		return 0;
	}
};

struct CatmullRom	// Catmull-Rom spline
{
	static const int SUPPORT= 2;

	int operator () (double x)
	{
		if (x < -2.0)
			return 0;
		if (x < -1.0)
			return static_cast<int>(0.5 * (4.0 + x * (8.0 + x * (5.0 + x))) * WEIGHTONE);
		if (x < 0.0)
			return static_cast<int>(0.5 * (2.0 + x * x * (-3.0 * x - 5.0)) * WEIGHTONE);
		if (x < 1.0)
			return static_cast<int>(0.5 * (2.0 + x * x * (3.0 * x - 5.0)) * WEIGHTONE);
		if (x < 2.0)
			return static_cast<int>(0.5 * (4.0 + x * (-8.0 + x * (5.0 - x))) * WEIGHTONE);

		return 0;
	}
};


struct Mapping
{
	double sx, sy;	// x and y scales
	double tx, ty;	// x and y translations
	double ux, uy;	// x and y offset used by MAP, private fields
};


template <class T>
struct Scanline
{
	int y;
	std::vector<T> pixels;

	Scanline(size_t size= 0) : pixels(size)
	{
		y = -1;
	}
};

typedef Scanline<uint8> Scanline8;
typedef Scanline<int16> Scanline16;


struct WeightTable
{
	size_t stride;
	std::vector<int> table;
//	vector<int16> weights;	// weights in triplets (for RGB pixels)

	WeightTable(size_t stride, size_t size) : stride(stride), table(stride * size) //, weights((stride - 2) * size * 3)
	{}

	int* begin()	{ return &table.front(); }
	int* weights()	{ return &table[2]; }
};

} // namespace


static void filter_scanline(WeightTable& table, BYTE* src_line, int components, int dest_len, int16* out)
{
	if (components == 3)
	{
		int* table_ptr= table.begin();

		for (int i= 0; i < dest_len; ++i)
		{
			int r= 0x80;
			int g= 0x80;
			int b= 0x80;

			int from= table_ptr[0];
			int to= table_ptr[1];
			table_ptr += 2;

			BYTE* src= src_line + from * 3;

			for (int i= from; i < to; ++i)
			{
				r += *table_ptr * src[2];
				g += *table_ptr * src[1];
				b += *table_ptr * src[0];

				src += 3;
				++table_ptr;
			}

			table_ptr += table.stride - (to - from) - 2;

			out[2] = static_cast<int16>(r >> 8);
			out[1] = static_cast<int16>(g >> 8);
			out[0] = static_cast<int16>(b >> 8);
			out += 3;
		}
	}
	else if (components == 4)
	{
		int* table_ptr = table.begin();

		const int STEP = 4;
		for (int i = 0; i < dest_len; ++i)
		{
			int r = 0x80;
			int g = 0x80;
			int b = 0x80;
			int a = 0x80;

			int from = table_ptr[0];
			int to = table_ptr[1];
			table_ptr += 2;

			BYTE* src = src_line + from * STEP;

			for (int i = from; i < to; ++i)
			{
				a += *table_ptr * src[3];
				r += *table_ptr * src[2];
				g += *table_ptr * src[1];
				b += *table_ptr * src[0];

				src += STEP;
				++table_ptr;
			}

			table_ptr += table.stride - (to - from) - 2;

			out[3] = static_cast<int16>(a >> 8);
			out[2] = static_cast<int16>(r >> 8);
			out[1] = static_cast<int16>(g >> 8);
			out[0] = static_cast<int16>(b >> 8);
			out += STEP;
		}
	}
	else if (components == 1)
	{
		int* table_ptr= table.begin();

		for (int i= 0; i < dest_len; ++i)
		{
			int gray= 0x80;

			int from= table_ptr[0];
			int to= table_ptr[1];
			table_ptr += 2;

			BYTE* src= src_line + from;

			for (int i= from; i < to; ++i)
				gray += *table_ptr++ * *src++;

			table_ptr += table.stride - (to - from) - 2;

			*out++ = static_cast<int16>(gray >> 8);
		}
	}
	else
	{
		ASSERT(false);
	}
}


static void accumulate_scanline(const int16* pixels, int weight, int components, int dest_width, int32* accum)
{
	if (components == 3)
	{
#ifndef _WIN64
		if (IsMMXAvailable())
		{
			dest_width *= 3;	// R, G & B

			uint32 n= dest_width >> 3;
			uint32 rest= dest_width - (n << 3);

			ASSERT(weight <= 0x7fff && weight >= -0x7fff);
			__int64 n64Weight= weight & 0xffff;
			n64Weight |= (n64Weight << 0x10) | (n64Weight << 0x20) | (n64Weight << 0x30);

			__asm
			{
				mov			esi, pixels
				mov			edi, accum

				movq		mm7, n64Weight

				mov			ecx, n
				test		ecx, ecx
				jz			rest_of_line
next_:
				movq		mm0, [esi]		// four pixels (16 bits each)
				movq		mm2, mm0
				pmullw		mm0, mm7
				pmulhw		mm2, mm7

				movq		mm1, [esi + 8]	// next four
				movq		mm3, mm1
				pmullw		mm1, mm7
				pmulhw		mm3, mm7

				add			esi, 0x10

				movq		mm5, [edi]		// two accumulated values (32 bits)
				movq		mm6, [edi + 8]	// and two more

				movq		mm4, mm0
				punpcklwd	mm4, mm2
				paddd		mm5, mm4		// add to accumulator

				movq		mm4, mm0
				punpckhwd	mm4, mm2
				paddd		mm6, mm4		// ditto

				movq		[edi], mm5		// and store results
				movq		[edi + 8], mm6

				movq		mm5, [edi + 0x10]	// repeat for next four values...
				movq		mm6, [edi + 0x18]

				movq		mm4, mm1
				punpcklwd	mm4, mm3
				paddd		mm5, mm4

				movq		mm4, mm1
				punpckhwd	mm4, mm3
				paddd		mm6, mm4

				movq		[edi + 0x10], mm5
				movq		[edi + 0x18], mm6

				add			edi, 0x20

				sub			ecx, 1
				jnz			next_
rest_of_line:
				mov			ecx, rest
				test		ecx, ecx
				jz			finish
next_byte:
				movd		mm0, [esi]	// use one pixel at a time
				movq		mm2, mm0
				pmullw		mm0, mm7
				pmulhw		mm2, mm7

				movd		mm5, [edi]
				movq		mm4, mm0
				punpcklwd	mm4, mm2
				paddd		mm5, mm4	// add to accumulator

				movd		[edi], mm5

				add			esi, 2
				add			edi, 4

				sub			ecx, 1
				jnz			next_byte
finish:
				emms
			}
		}
		else
#endif
			for (int i= 0; i < dest_width; ++i)
			{
				accum[0] += weight * pixels[0];
				accum[1] += weight * pixels[1];
				accum[2] += weight * pixels[2];

				accum += 3;
				pixels += 3;
			}
	}
	else if (components == 4)
	{
		for (int i = 0; i < dest_width; ++i)
		{
			accum[0] += weight * pixels[0];
			accum[1] += weight * pixels[1];
			accum[2] += weight * pixels[2];
			accum[3] += weight * pixels[3];

			accum += 4;
			pixels += 4;
		}
	}
	else if (components == 1)
	{
		for (int i= 0; i < dest_width; ++i)
		{
			*accum++ += weight * *pixels++;
		}
	}
	else
	{
		ASSERT(false);
	}
}


static void copy_scanline(const int32* accum, int components, int dest_width, BYTE* line_buffer)
{
	if (dest_width <= 0)
		return;

#define CLAMP(a)	(a > 255 ? 255 : (a < 0 ? 0 : a))

	if (components == 3)
	{
#ifndef _WIN64
		if (IsMMXAvailable())
		{
			// MMX version of loop below

			dest_width *= 3;	// R, G & B

			uint32 n= dest_width >> 3;	// 8 values at a time
			uint32 rest= dest_width - (n << 3);

			__asm
			{
				mov			esi, accum
				mov			edi, line_buffer

				mov			ecx, n
				test		ecx, ecx
				jz			rest_of_line
next_quad:
				// fetch 8 dwords (total)
				movq		mm0, [esi]
				movq		mm1, [esi + 0x08]
				movq		mm2, [esi + 0x10]
				movq		mm3, [esi + 0x18]

				// shift them
				psrad		mm0, 20
				psrad		mm1, 20
				psrad		mm2, 20
				psrad		mm3, 20

				// and combine
				packssdw	mm0, mm1
				packssdw	mm2, mm3
				packuswb	mm0, mm2	// clamp results to 0..255 range

				add			esi, 0x20

				movq		[edi], mm0
				add			edi, 8

				sub			ecx, 1
				jnz			next_quad
rest_of_line:
				mov			ecx, rest
				test		ecx, ecx
				jz			finish
next_byte:
				movd		mm0, [esi]
				add			esi, 4
				psrad		mm0, 20
				packssdw	mm0, mm0
				packuswb	mm0, mm0
				movd		eax, mm0
				mov			[edi], al
				add			edi, 1
				sub			ecx, 1
				jnz			next_byte
finish:
				emms
			}
		}
		else
#endif
			for (int i= 0; i < dest_width; ++i)
			{
				// some filters need copy with saturation
				int r= accum[0] >> 20;
				int g= accum[1] >> 20;
				int b= accum[2] >> 20;
				line_buffer[0] = CLAMP(r);
				line_buffer[1] = CLAMP(g);
				line_buffer[2] = CLAMP(b);

				accum += 3;
				line_buffer += 3;
			}
	}
	else if (components == 4)
	{
		for (int i = 0; i < dest_width; ++i)
		{
			// some filters need copy with saturation
			int r = accum[0] >> 20;
			int g = accum[1] >> 20;
			int b = accum[2] >> 20;
			int a = accum[3] >> 20;
			line_buffer[0] = CLAMP(r);
			line_buffer[1] = CLAMP(g);
			line_buffer[2] = CLAMP(b);
			line_buffer[3] = CLAMP(a);

			accum += 4;
			line_buffer += 4;
		}
	}
	else if (components == 1)
	{
		for (int i= 0; i < dest_width; ++i)
		{
			int g= *accum++ >> 20;
			*line_buffer++ = CLAMP(g);
		}
	}
	else
	{
		ASSERT(false);
	}
#undef CLAMP
}



bool MagnifyBitmapWorker(Dib& dibSrc, double zoom_x, double zoom_y, Dib& dibDest, Filter* filter)
{
	if (zoom_x <= 0.0 || zoom_y <= 0.0 || filter == 0)
	{
		ASSERT(false);
		return false;
	}

	CSize src_size= dibSrc.GetSize();

	CSize dest_size;
	dest_size.cx = static_cast<int>(zoom_x * src_size.cx + 0.5);
	dest_size.cy = static_cast<int>(zoom_y * src_size.cy + 0.5);

	dibDest.CreateEx(dest_size.cx, dest_size.cy, dibSrc.GetBitsPerPixel(), false);

	FilterParam ax(zoom_x, filter->support);
	FilterParam ay(zoom_y, filter->support);

	Mapping map;
	// scale
	map.sx = zoom_x;
	map.sy = zoom_y;
	// translation
	map.tx = 0 - 0.5 - map.sx * (0 - 0.5);
	map.ty = 0 - 0.5 - map.sy * (0 - 0.5);
	// offset
	map.ux = 0.0 - map.sx * (-0.5) - map.tx;
	map.uy = 0.0 - map.sy * (-0.5) - map.ty;

	// due to the rounding errors filters occasionally span ax.width + 1 pixels rather than ax.width
	const int XSTRIDE= 2 + ax.width + 1;
	WeightTable X_table(XSTRIDE, dest_size.cx);

	const int YSTRIDE= 2 + ay.width + 1;
	WeightTable Y_table(YSTRIDE, 1);

	int* X_table_ptr= X_table.begin();
	int* Y_table_ptr= Y_table.begin();

	// prepare a weight table (a sampled filter for source pixels) for each dest x position

	for (int bx= 0; bx < dest_size.cx; ++bx, X_table_ptr += XSTRIDE)
		filter->make_weight_table((bx + map.ux) / map.sx, ax, src_size.cx, X_table_ptr);

	// TODO: allocator with 8 bytes alignment
	std::vector<Scanline16> linebuf(ay.width);		// circular buffer of active lines

	for (int i= 0; i < linebuf.size(); ++i)
		linebuf[i].pixels.resize(dest_size.cx * dibSrc.GetColorComponents());

	// TODO: allocator with 8 bytes alignment
	std::vector<int32> acc(dest_size.cx * dibSrc.GetColorComponents());

	for (int by= 0; by < dest_size.cy; ++by)
	{
//LARGE_INTEGER tm[9];
//::QueryPerformanceCounter(&tm[0]);

		// prepare a weight table for dest y position
		filter->make_weight_table((by + map.uy) / map.sy, ay, src_size.cy, Y_table_ptr);

		int* weight= Y_table.weights();

		fill(acc.begin(), acc.end(), 0);

//::QueryPerformanceCounter(&tm[1]);

		// loop over source scanlines that influence this dest scanline
		for (int ayf= Y_table_ptr[0]; ayf < Y_table_ptr[1]; ++ayf)
		{
			Scanline16& tbuf= linebuf[ayf % ay.width];
			if (tbuf.y != ayf)
			{
				// scanline needs to be filtered
				filter_scanline(X_table, dibSrc.LineBuffer(ayf), dibSrc.GetColorComponents(), dest_size.cx, &tbuf.pixels[0]);

				tbuf.y = ayf;
			}

			accumulate_scanline(&tbuf.pixels[0], *weight++, dibSrc.GetColorComponents(), dest_size.cx, &acc[0]);
		}
//::QueryPerformanceCounter(&tm[2]);

		copy_scanline(&acc[0], dibSrc.GetColorComponents(), dest_size.cx, dibDest.LineBuffer(by));
//::QueryPerformanceCounter(&tm[3]);


//LARGE_INTEGER tmr[9], frq;
//::QueryPerformanceFrequency(&frq);
//for (int ii= 0; ii < 3; ++ii)
//{
//	tmr[ii].QuadPart = tm[ii + 1].QuadPart - tm[ii].QuadPart;
//	tmr[ii].QuadPart *= 1000000;
//	tmr[ii].QuadPart /= frq.QuadPart;
//}
//int a[10]= { 0 };
//a[0] = int(tmr[0].QuadPart);
//a[1] = int(tmr[1].QuadPart);
//a[2] = int(tmr[2].QuadPart);
//
//TCHAR str[256];
//wsprintf(str, _T("make weight: %d  filer&accum: %d  copy: %d\n"), a[0], a[1], a[2]);
//::OutputDebugString(str);
	}

	return true;
}


//#define MAGNIFY_SPEED_TEST

void MagnifyBitmap(Dib& dibSrc, CSize dest_size, Dib& dibDest, bool cubic)
{
	CSize src_size= dibSrc.GetSize();
	if (dest_size.cx <= 0 || dest_size.cy <= 0 || src_size.cx <= 0 || src_size.cy <= 0)
	{
		ASSERT(false);
		return;
	}

	double zoom_x= double(dest_size.cx) / src_size.cx;
	double zoom_y= double(dest_size.cy) / src_size.cy;

	return MagnifyBitmap(dibSrc, zoom_x, zoom_y, dibDest, cubic);
}


void MagnifyBitmap(Dib& dibSrc, double zoom_x, double zoom_y, Dib& dibDest, bool cubic)
{
#ifdef MAGNIFY_SPEED_TEST
LARGE_INTEGER tm[9];
::QueryPerformanceCounter(&tm[0]);
#endif

//	FilterT<Triangle> filter;
//	FilterT<Cubic> filter;
//	FilterT<Mitchell> filter;
//	FilterT<CatmullRom> filter;

	if (cubic)
		MagnifyBitmapWorker(dibSrc, zoom_x, zoom_y, dibDest, &FilterT<CatmullRom>());
	else
		MagnifyBitmapWorker(dibSrc, zoom_x, zoom_y, dibDest, &FilterT<Triangle>());

#ifdef MAGNIFY_SPEED_TEST
::QueryPerformanceCounter(&tm[1]);
#endif

#ifdef MAGNIFY_SPEED_TEST
::QueryPerformanceCounter(&tm[2]);
LARGE_INTEGER tmr[9], frq;
::QueryPerformanceFrequency(&frq);
for (int ii= 0; ii < 1; ++ii)
{
	tmr[ii].QuadPart = tm[ii + 1].QuadPart - tm[ii].QuadPart;
	tmr[ii].QuadPart *= 1000000;
	tmr[ii].QuadPart /= frq.QuadPart;
}
int a[10]= { 0 };
a[0] = int(tmr[0].QuadPart);
a[1] = int(tmr[1].QuadPart);

TCHAR str[256];
wsprintf(str, _T("magnify: %d\n"), a[0]);
::OutputDebugString(str);
#endif
}


CRect SizeToFill(CSize image, CSize dest, bool magnify_if_needed)
{
	CRect rect(0,0,0,0);

	if (image.cx <= 0 || image.cy <= 0)
		return rect;

	if (dest.cx <= 0 || dest.cy <= 0)
		return rect;

	double dest_w= dest.cx;
	double dest_h= dest.cy;

	double img_ratio= double(image.cx) / double(image.cy);
	double delta= dest_w / dest_h - img_ratio;
	const double epsilon= 0.02;	// 2%


	double x= 0.0, y= 0.0;	// offset

	if (fabs(delta) < epsilon)
	{
		dest_w = dest.cx;
		dest_h = dest.cy;
	}
	else if (delta < 0.0)
	{
		dest_w = std::max(1.0, dest_h * img_ratio);
		x = (dest_w - dest.cx) / 2.0;
	}
	else
	{
		dest_h = std::max(1.0, dest_w / img_ratio);
		y = (dest_h - dest.cy) / 2.0;
	}

	// if resulting image is to be magnified, but magnification is not desired
	// leave original size
	if ((dest_w > image.cx || dest_h > image.cy) && !magnify_if_needed)
	{
		dest_w = image.cx;
		dest_h = image.cy;

		x = y = 0.0;

		// if image still exceeds one desired dimension, calculate offset
		if (image.cx > dest.cx)
			x = (image.cx - dest.cx) / 2;
		else if (image.cy > dest.cy)
			y = (image.cy - dest.cy) / 2;
	}

	if (dest_w > 0xffff || dest_h > 0xffff)	// protection against oversized image; arbitrarily selected size
		return rect;

	CPoint p(static_cast<int>(x + 0.5), static_cast<int>(y + 0.5));
	CSize s(static_cast<int>(dest_w), static_cast<int>(dest_h));
	return CRect(p, s);
}


AutoPtr<Dib> RotateBitmap(Dib& img, int rotation_flag)
{
	rotation_flag &= 3;

	if (rotation_flag == 0)
		return 0;

	AutoPtr<Dib> copy;

	if (rotation_flag == 1)
		copy = img.RotateCopy(true);
	else if (rotation_flag == 3)
		copy = img.RotateCopy(false);
	else if (rotation_flag == 2)
	{
		copy = new Dib(img.GetWidth(), img.GetHeight(), img.GetBitsPerPixel());
		if (!Rotate180(&img, copy.get(), 0, img.GetHeight()))
			return 0;
	}

	return copy;
}


CSize SizeToFit(CSize image, CSize dest, double tolerance)
{
	if (dest.cx <= 0 || dest.cy <= 0 || image.cx <= 0 || image.cy <= 0)
		return CSize(0, 0);

	ASSERT(tolerance >= 0.0);

	int dest_w= dest.cx;
	int dest_h= dest.cy;

	double img_ratio= double(image.cx) / double(image.cy);
	const double delta= double(dest.cx) / double(dest.cy) - img_ratio;

	if (fabs(delta) < tolerance)
		return CSize(dest_w, dest_h);

	// calc remaining size making sure it does not collapse to zero (possible with int numbers)
	if (delta > 0.0)
		dest_w = std::max(1, static_cast<int>(dest_h * img_ratio + 0.5));
	else
		dest_h = std::max(1, static_cast<int>(dest_w / img_ratio + 0.5));

	return CSize(dest_w, dest_h);
}


void FlipBitmap(Dib& bmp, bool horizontally, bool vertically)
{
	int width= bmp.GetWidth();
	int height= bmp.GetHeight();
	int bpp= bmp.GetBitsPerPixel();
	if (bpp != 24 || width == 0 || height == 0)
		return;

	if (vertically)
	{
		const size_t line_size= bmp.GetBytesPerLine();
		// buffer for one line
		std::vector<BYTE> line(line_size, 0);
		BYTE* buffer= &line.front();

		for (int line= 0; line < height / 2; ++line)
		{
			BYTE* first= bmp.LineBuffer(line);
			BYTE* last= bmp.LineBuffer(height - line - 1);
			memcpy(buffer, first, line_size);
			memcpy(first, last, line_size);
			memcpy(last, buffer, line_size);
		}
	}

	if (horizontally)
	{
		for (int line= 0; line < height; ++line)
		{
			// last pixel
			BYTE* right= bmp.LinePixelBuffer(line, width - 1);
			// first pixel
			BYTE* left= bmp.LinePixelBuffer(line, 0);

			int len= bmp.GetWidth() / 2;
			for (int x= 0; x < len; ++x)
			{
				BYTE p[3];

				p[0] = right[0];
				p[1] = right[1];
				p[2] = right[2];

				right[0] = left[0];
				right[1] = left[1];
				right[2] = left[2];

				left[0] = p[0];
				left[1] = p[1];
				left[2] = p[2];

				right -= 3;
				left += 3;
			}
		}
	}
}

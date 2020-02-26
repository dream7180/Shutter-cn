/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "clamp.h"
#include "Dib.h"
#include "Pixels.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


void AlphaBitBlt(const Dib& src, const CRect& src_rect, int src_alpha, Dib& dst, const CPoint& dst_pos)
{
	const int x_limit= std::min<int>(src_rect.Width(), dst.GetWidth() - dst_pos.x);
	if (x_limit <= 0)
		return;

	const int y_limit= std::min<int>(src_rect.Height(), dst.GetHeight() - dst_pos.y);
	if (y_limit <= 0)
		return;

	BYTE alpha[256];
	for (int i= 0; i < array_count(alpha); ++i)
		alpha[i] = i * src_alpha / 255;

	for (int y= 0; y < y_limit; ++y)
	{
		const Pixel4* p= reinterpret_cast<const Pixel4*>(src.LinePixelBuffer(src_rect.top + y, src_rect.left));
		Pixel4* q= reinterpret_cast<Pixel4*>(dst.LinePixelBuffer(dst_pos.y + y, dst_pos.x));

		for (int x= 0; x < x_limit; ++x)
		{
			q->r = static_cast<BYTE>(alpha[p->a] * (int(p->r) - int(q->r)) / 255 + q->r);
			q->g = static_cast<BYTE>(alpha[p->a] * (int(p->g) - int(q->g)) / 255 + q->g);
			q->b = static_cast<BYTE>(alpha[p->a] * (int(p->b) - int(q->b)) / 255 + q->b);
			++p;
			++q;
		}
	}
}


void AlphaBitBlt(const Dib& src, const CRect& src_rect, Dib& dst, const CPoint& dst_pos)
{
	const int x_limit= std::min<int>(src_rect.Width(), dst.GetWidth() - dst_pos.x);
	if (x_limit <= 0)
		return;

	const int y_limit= std::min<int>(src_rect.Height(), dst.GetHeight() - dst_pos.y);
	if (y_limit <= 0)
		return;

	for (int y= 0; y < y_limit; ++y)
	{
		const Pixel4* p= reinterpret_cast<const Pixel4*>(src.LinePixelBuffer(src_rect.top + y, src_rect.left));
		Pixel4* q= reinterpret_cast<Pixel4*>(dst.LinePixelBuffer(dst_pos.y + y, dst_pos.x));

		for (int x= 0; x < x_limit; ++x)
		{
			q->r = static_cast<BYTE>(p->a * (int(p->r) - int(q->r)) / 255 + q->r);
			q->g = static_cast<BYTE>(p->a * (int(p->g) - int(q->g)) / 255 + q->g);
			q->b = static_cast<BYTE>(p->a * (int(p->b) - int(q->b)) / 255 + q->b);
			++p;
			++q;
		}
	}
}


void AlphaBitBlt(const Dib& src1, const Dib& src2, const CRect& src_rect, int src_alpha, Dib& dst, const CPoint& dst_pos)
{
	const int x_limit= std::min<int>(src_rect.Width(), dst.GetWidth() - dst_pos.x);
	if (x_limit <= 0)
		return;

	const int y_limit= std::min<int>(src_rect.Height(), dst.GetHeight() - dst_pos.y);
	if (y_limit <= 0)
		return;

	BYTE alpha[256];
	for (int i= 0; i < array_count(alpha); ++i)
		alpha[i] = i * (255 - src_alpha) / 255;

	BYTE neg_alpha[256];
	for (int i= 0; i < array_count(neg_alpha); ++i)
		neg_alpha[i] = i * src_alpha / 255;

	for (int y= 0; y < y_limit; ++y)
	{
		const Pixel4* p1= reinterpret_cast<const Pixel4*>(src1.LinePixelBuffer(src_rect.top + y, src_rect.left));
		const Pixel4* p2= reinterpret_cast<const Pixel4*>(src2.LinePixelBuffer(src_rect.top + y, src_rect.left));
		Pixel4* q= reinterpret_cast<Pixel4*>(dst.LinePixelBuffer(dst_pos.y + y, dst_pos.x));

		for (int x= 0; x < x_limit; ++x)
		{
			int a= alpha[p1->a] + neg_alpha[p2->a];
			int r= alpha[p1->r] + neg_alpha[p2->r];
			int g= alpha[p1->g] + neg_alpha[p2->g];
			int b= alpha[p1->b] + neg_alpha[p2->b];

			//int r= src_alpha * (p1->r - p2->r) / 255 + p2->r;
			//int g= src_alpha * (p1->g - p2->g) / 255 + p2->g;
			//int b= src_alpha * (p1->b - p2->b) / 255 + p2->b;

			q->r = static_cast<BYTE>(a * (r - q->r) / 255 + q->r);
			q->g = static_cast<BYTE>(a * (g - q->g) / 255 + q->g);
			q->b = static_cast<BYTE>(a * (b - q->b) / 255 + q->b);
			++p1;
			++p2;
			++q;
		}
	}
}


#pragma pack(1)
struct Pixel
{
	BYTE b;
	BYTE g;
	BYTE r;
};
#pragma pack()


void Blur(Dib& dib, Dib& res)
{
	CSize s= dib.GetSize();

	Dib out(s.cx, s.cy, dib.GetBitsPerPixel());

	res.Create(s.cx, s.cy, dib.GetBitsPerPixel());

	const static float kernel[]= { 0.006f, 0.061f, 0.242f, 0.383f, 0.242f, 0.061f, 0.006f };
	const int ksize= array_count(kernel) / 2;

	if (dib.GetColorComponents() == 4)		// RGBA
	{
		for (int y= 0; y < s.cy; ++y)
		{
			const Pixel4* p= (const Pixel4*)dib.LineBuffer(y);
			Pixel4* q= (Pixel4*)out.LineBuffer(y);

			for (int x= 0; x < s.cx; ++x)
			{
				float b= 0.0f;
				float g= 0.0f;
				float r= 0.0f;
				float a= 0.0f;

				for (int i= 0; i < array_count(kernel); ++i)
				{
					int pos= x - ksize + i;
					if (pos >= 0 && pos < s.cx)
					{
						r += p[pos].r * kernel[i];
						g += p[pos].g * kernel[i];
						b += p[pos].b * kernel[i];
						a += p[pos].a * kernel[i];
					}
					else
					{
						r += 255 * kernel[i];
						g += 255 * kernel[i];
						b += 255 * kernel[i];
						a += 255 * kernel[i];
					}
				}

				q->r = (BYTE)r;
				q->g = (BYTE)g;
				q->b = (BYTE)b;
				q->a = (BYTE)a;

				++q;
			}
		}

		//	const int line_bytes= dib.GetBytesPerLine();

		for (int x= 0; x < s.cx; ++x)
		{
			//		const Pixel* p= (const Pixel*)dib.LineBuffer(y);
			//		Pixel* q= (Pixel*)out.LineBuffer(y);

			for (int y= 0; y < s.cy; ++y)
			{
				//			const Pixel* p= (const Pixel*)out.LineBuffer(0) + x;
				Pixel4* q= (Pixel4*)res.LineBuffer(y) + x;

				float b= 0.0f;
				float g= 0.0f;
				float r= 0.0f;
				float a= 0.0f;

				for (int i= 0; i < array_count(kernel); ++i)
				{
					int pos= y - ksize + i;
					if (pos >= 0 && pos < s.cy)
					{
						const Pixel4* p= (const Pixel4*)out.LineBuffer(pos) + x;
						//					const BYTE* pp= (const BYTE*)p + pos * line_bytes;
						//					Pixel* pix= (Pixel*)pp;
						r += p->r * kernel[i];
						g += p->g * kernel[i];
						b += p->b * kernel[i];
						a += p->a * kernel[i];
					}
					else
					{
						r += 255 * kernel[i];
						g += 255 * kernel[i];
						b += 255 * kernel[i];
						a += 255 * kernel[i];
					}
				}

				q->r = (BYTE)r;
				q->g = (BYTE)g;
				q->b = (BYTE)b;
				q->a = (BYTE)a;
			}
		}
	}
	else if (dib.GetColorComponents() == 3)		// RGB
	{
		for (int y= 0; y < s.cy; ++y)
		{
			const Pixel* p= (const Pixel*)dib.LineBuffer(y);
			Pixel* q= (Pixel*)out.LineBuffer(y);

			for (int x= 0; x < s.cx; ++x)
			{
				float b= 0.0f;
				float g= 0.0f;
				float r= 0.0f;

				for (int i= 0; i < array_count(kernel); ++i)
				{
					int pos= x - ksize + i;
					if (pos >= 0 && pos < s.cx)
					{
						r += p[pos].r * kernel[i];
						g += p[pos].g * kernel[i];
						b += p[pos].b * kernel[i];
					}
					else
					{
						r += 255 * kernel[i];
						g += 255 * kernel[i];
						b += 255 * kernel[i];
					}
				}

				q->r = (BYTE)r;
				q->g = (BYTE)g;
				q->b = (BYTE)b;

				++q;
			}
		}

		for (int x= 0; x < s.cx; ++x)
		{
			for (int y= 0; y < s.cy; ++y)
			{
				Pixel* q= (Pixel*)res.LineBuffer(y) + x;

				float b= 0.0f;
				float g= 0.0f;
				float r= 0.0f;

				for (int i= 0; i < array_count(kernel); ++i)
				{
					int pos= y - ksize + i;
					if (pos >= 0 && pos < s.cy)
					{
						const Pixel* p= (const Pixel*)out.LineBuffer(pos) + x;
						r += p->r * kernel[i];
						g += p->g * kernel[i];
						b += p->b * kernel[i];
					}
					else
					{
						r += 255 * kernel[i];
						g += 255 * kernel[i];
						b += 255 * kernel[i];
					}
				}

				q->r = (BYTE)r;
				q->g = (BYTE)g;
				q->b = (BYTE)b;
			}
		}
	}
}


#define SHARPEN(a)	\
		{ \
			int diff= p->a; \
			diff -= q->a; \
			if (diff > threshold || diff < -threshold) \
			{ \
				int v= p->a + static_cast<int>(amount * diff); \
				if (v < 0) \
					p->a = 0; \
				else if (v > 0xff) \
					p->a = 0xff; \
				else \
					p->a = static_cast<BYTE>(v); \
			} \
		}

extern void OutputTimeStr(LARGE_INTEGER start, const TCHAR* str);

bool UnsharpMask(Dib& dib, Dib& sharp, int threshold, float amount)
{
	if (dib.GetColorComponents() != 3)		// RGB
		return false;

//LARGE_INTEGER start,time;
//::QueryPerformanceCounter(&time);
//start.QuadPart = time.QuadPart;

	Dib blur;
	Blur(dib, blur);

//OutputTimeStr(time, _T("blur"));
//::QueryPerformanceCounter(&time);

	CSize s= dib.GetSize();
	sharp.Clone(dib);	// create a copy of dib

	for (int x= 1; x < s.cx - 1; ++x)
	{
		for (int y= 1; y < s.cy - 1; ++y)
		{
			Pixel* p= (Pixel*)sharp.LineBuffer(y) + x;
			Pixel* q= (Pixel*)blur.LineBuffer(y) + x;

			SHARPEN(r)
			SHARPEN(g)
			SHARPEN(b)
		}
	}

//OutputTimeStr(time, _T("sharp"));
//OutputTimeStr(start, _T("unsharp mask"));

	return true;
}

#undef SHARPEN


void BlurAlphaChnl(Dib& dib, int from, int width, Dib& res, const int MIDDLE)
{
	ASSERT(dib.GetBitsPerPixel() == 8);

	CSize s(width, dib.GetHeight());

	Dib out(s.cx, s.cy, dib.GetBitsPerPixel());

	res.Create(s.cx, s.cy, dib.GetBitsPerPixel());

	const static float kernel[]= { 0.006f, 0.061f, 0.242f, 0.383f, 0.242f, 0.061f, 0.006f };
	const int ksize= array_count(kernel) / 2;

	for (int y= 0; y < s.cy; ++y)
	{
		BYTE* p= dib.LineBuffer(y);
		BYTE* q= out.LineBuffer(y);

		for (int x= 0; x < s.cx; ++x)
		{
			float a= 0.0f;

			for (int i= 0; i < array_count(kernel); ++i)
			{
				int pos= x - ksize + i;
				if (pos >= 0 && pos < s.cx)
					a += p[pos + from] * kernel[i];
				else
					a += MIDDLE * kernel[i];
			}

			*q = (BYTE)a;

			++q;
		}
	}

	for (int x= 0; x < s.cx; ++x)
	{
		for (int y= 0; y < s.cy; ++y)
		{
			BYTE* q= res.LinePixelBuffer(y, x);

			float a= 0.0f;

			for (int i= 0; i < array_count(kernel); ++i)
			{
				int pos= y - ksize + i;
				if (pos >= 0 && pos < s.cy)
				{
					const BYTE* p= out.LinePixelBuffer(pos, x + from);
					a += *p * kernel[i];
				}
				else
					a += MIDDLE * kernel[i];
			}

			*q = (BYTE)a;
		}
	}
}


void BlurAlphaChnl2(Dib& dib, int from, int width, Dib& res, const int MIDDLE)
{
	ASSERT(dib.GetBitsPerPixel() == 8);

	CSize s(width, dib.GetHeight());

	Dib out(s.cx, s.cy, dib.GetBitsPerPixel());

	res.Create(s.cx, s.cy, dib.GetBitsPerPixel());

//	const static float kernel[]= { 0.005f, 0.02f, 0.06f, 0.12f, 0.17f, 0.25f, 0.17f, 0.12f, 0.06f, 0.02f, 0.005f };
//	const static float kernel[]= { 0.005f, 0.01f, 0.05f, 0.08f, 0.10f, 0.15f, 0.21f, 0.15f, 0.10f, 0.08f, 0.05f, 0.01f, 0.005f };
//	const static float kernel[]= { 0.005f, 0.01f, 0.02f, 0.03f, 0.05f, 0.08f, 0.10f, 0.13f, 0.15f, 0.13f, 0.10f, 0.08f, 0.05f, 0.03f, 0.02f, 0.01f, 0.005f };
	const static float kernel[]= { 0.005f, 0.01f, 0.02f, 0.05f, 0.08f, 0.10f, 0.13f, 0.21f, 0.13f, 0.10f, 0.08f, 0.05f, 0.02f, 0.01f, 0.005f };
	const int ksize= array_count(kernel) / 2;

	for (int y= 0; y < s.cy; ++y)
	{
		BYTE* p= dib.LineBuffer(y);
		BYTE* q= out.LineBuffer(y);

		for (int x= 0; x < s.cx; ++x)
		{
			float a= 0.0f;

			for (int i= 0; i < array_count(kernel); ++i)
			{
				int pos= x - ksize + i;
				if (pos >= 0 && pos < s.cx)
					a += p[pos + from] * kernel[i];
				else
					a += MIDDLE * kernel[i];
			}

			*q = (BYTE)a;

			++q;
		}
	}

	for (int x= 0; x < s.cx; ++x)
	{
		for (int y= 0; y < s.cy; ++y)
		{
			BYTE* q= res.LinePixelBuffer(y, x);

			float a= 0.0f;

			for (int i= 0; i < array_count(kernel); ++i)
			{
				int pos= y - ksize + i;
				if (pos >= 0 && pos < s.cy)
				{
					const BYTE* p= out.LinePixelBuffer(pos, x + from);
					a += *p * kernel[i];
				}
				else
					a += MIDDLE * kernel[i];
			}

			*q = (BYTE)a;
		}
	}
}



const int MIDDLE= 0x80;

void Bevel(Dib& img, int from, int width, Dib& res)
{
	ASSERT(img.GetBitsPerPixel() == 32);

	CSize s(width, img.GetHeight());

	const int INC= 4;
	Dib tmp(s.cx + INC, s.cy + INC, 8);
	memset(tmp.GetBuffer(), MIDDLE, tmp.GetBufferSize());

	// calculate derivative to detect edges

	for (int y= -1; y < s.cy + 1; ++y)
		for (int x= -1; x < s.cx + 1; ++x)
		{
			int a1= y > 0 && x > 0 ? reinterpret_cast<const Pixel4*>(img.LinePixelBuffer(y - 1, from + x - 1))->a : 0;
			int a2= y < s.cy-1 && x < s.cx-1 ? reinterpret_cast<const Pixel4*>(img.LinePixelBuffer(y + 1, from + x + 1))->a : 0;

			int pix= MIDDLE + (a1 - a2) / 2;

			if (pix < 0)
				pix = 0;
			else if (pix > 255)
				pix = 255;

			BYTE* q= tmp.LinePixelBuffer(y + INC/2, x + INC/2);

			*q = static_cast<BYTE>(pix);
		}

	res.Create(tmp.GetWidth(), tmp.GetHeight(), img.GetBitsPerPixel());

	// blur edges to create smooth highlight and shadow areas
	Dib blur;
	BlurAlphaChnl(tmp, 0, tmp.GetWidth(), blur, MIDDLE);

	// apply blured image to the main image

	for (int y= 0; y < res.GetHeight(); ++y)
		for (int x= 0; x < res.GetWidth(); ++x)
		{
			Pixel4* p= (Pixel4*)res.LinePixelBuffer(y, x);
			BYTE pix= *blur.LinePixelBuffer(y, x);

			//test: p->r = p->g = p->b = pix; p->a = 255;

			if (pix > MIDDLE)
			{
				// white highlight
				p->a = 3 * (pix - MIDDLE) / 2;	// scaled back a bit (1.5)
				p->r = p->g = p->b = 0xff;
			}
			else if (pix < MIDDLE)
			{
				// black shadow
				p->a = (MIDDLE - pix) * 2;	// full intensity range
				p->r = p->g = p->b = 0x0;
			}
			else
			{
				p->a = 0;
			}
		}
}

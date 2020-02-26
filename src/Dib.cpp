/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// Dib.cpp: implementation of the Dib class.

#include "stdafx.h"
#include "Dib.h"
#include <math.h>	// fabs
#include "BmpFunc.h"
#include <vfw.h>	// DrawDib
#include "Exception.h"


Dib::Dib()
{
	width_ = height_ = 0;
	line_width_ = 0;
	bits_per_pixel_ = 0;
	data_buff_ = 0;
	alloc_ = 0;
}


Dib::Dib(int width, int height, int bits_per_pixel, int clear_val/*= 0xff*/)
{
	alloc_ = 0;
	Create(width, height, bits_per_pixel, clear_val);
}


Dib::Dib(int bmp_rsrc_id)
{
	width_ = height_ = 0;
	line_width_ = 0;
	bits_per_pixel_ = 0;
	data_buff_ = 0;
	alloc_ = 0;
	Load(bmp_rsrc_id);
}


void Dib::Create(int width, int height, int bits_per_pixel, int clear_val/*= 0xff*/)
{
	if (width_ == width && height_ == height && bits_per_pixel_ == bits_per_pixel && dib_bmp_.m_hObject != 0)
	{
		// dib is already created and has right dimensions

		// clear it
		memset(data_buff_, clear_val, line_width_ * height_);

		return;
	}

	if (dib_bmp_.m_hObject)
		dib_bmp_.DeleteObject();
	if (alloc_)
		::free(alloc_);

	data_buff_ = 0;

	int color_components= bits_per_pixel / 8;
	ASSERT(color_components == 1 || color_components == 3 || color_components == 4);

	width_ = width; //(width + 3) & ~3;
	height_ = height;
//	line_width_ = width_ * color_components;
	bits_per_pixel_ = bits_per_pixel;
	// create a bitmap
#if USE_BITMAPV5HEADER
	bmp_info_.Init();
	bmp_info_.bmiHeader.bV5Width = width_;
	bmp_info_.bmiHeader.bV5Height = height_;
	bmp_info_.bmiHeader.bV5Planes = 1;
	bmp_info_.bmiHeader.bV5BitCount = bits_per_pixel_;
	bmp_info_.bmiHeader.bV5Compression = BI_RGB;
#else
	bmp_info_.bmiHeader.biSize = sizeof bmp_info_.bmiHeader;
	bmp_info_.bmiHeader.biWidth = width_;
	bmp_info_.bmiHeader.biHeight = height_;
	bmp_info_.bmiHeader.biPlanes = 1;
	bmp_info_.bmiHeader.biBitCount = bits_per_pixel_;
	bmp_info_.bmiHeader.biCompression = BI_RGB;
	bmp_info_.bmiHeader.biSizeImage = 0;
	bmp_info_.bmiHeader.biXPelsPerMeter = 0;
	bmp_info_.bmiHeader.biYPelsPerMeter = 0;
	bmp_info_.bmiHeader.biClrUsed = 0;
	bmp_info_.bmiHeader.biClrImportant = 0;
#endif
	if (color_components == 1)
		for (int i= 0; i < 256; ++i)
		{
			bmp_info_.bmiColors[i].rgbBlue = i;		// initialize color palette with shades of gray
			bmp_info_.bmiColors[i].rgbGreen = i;
			bmp_info_.bmiColors[i].rgbRed = i;
			bmp_info_.bmiColors[i].rgbReserved = 0;
		}

	VOID* data= 0;
	HBITMAP dib= ::CreateDIBSection(0, &bmp_info_.AsBitmapInfo(), DIB_RGB_COLORS, &data, 0, 0);
	if (dib == 0 || data == 0)
		THROW_EXCEPTION(L"Bitmap creation failed", L"Call to CreateDIBSection failed.");

	dib_bmp_.Attach(dib);

	DIBSECTION ds;
	::GetObject(dib, sizeof ds, &ds);

	line_width_ = ds.dsBm.bmWidthBytes;
	// ds.dsBm.bmWidthBytes lies on W2K; on XP it's fine
	if (color_components >= 3)
		line_width_ = (line_width_ + 3) & ~3;
	else
	{
		// for grayscale bmWidthBytes in w2k is also incorrect
		int line_bytes= ds.dsBmih.biSizeImage / ds.dsBm.bmHeight;
		if (line_bytes > line_width_)
			line_width_ = line_bytes;
	}

	// wipe out
	memset(data, clear_val, line_width_ * height_);

	data_buff_ = reinterpret_cast<BYTE*>(data);
}


void Dib::CreateEx(int width, int height, int bits_per_pixel, bool alignment)
{
	if (!alignment && width_ == width && height_ == height && bits_per_pixel_ == bits_per_pixel && data_buff_ != 0)
	{
		// dib is already created and has right dimensions
		return;
	}

	if (dib_bmp_.m_hObject)
		dib_bmp_.DeleteObject();
	if (alloc_)
		::free(alloc_);

	data_buff_ = 0;

	int color_components= bits_per_pixel / 8;
	ASSERT(color_components == 1 || color_components == 3 || color_components == 4);

	width_ = width;
	height_ = height;
	line_width_ = (width_ * color_components + 3) & ~3;
	bits_per_pixel_ = bits_per_pixel;
	// create a bitmap
#if USE_BITMAPV5HEADER
	bmp_info_.Init();
	bmp_info_.bmiHeader.bV5Width = width_;
	bmp_info_.bmiHeader.bV5Height = height_;
	bmp_info_.bmiHeader.bV5Planes = 1;
	bmp_info_.bmiHeader.bV5BitCount = bits_per_pixel_;
	bmp_info_.bmiHeader.bV5Compression = BI_RGB;
#else
	bmp_info_.bmiHeader.biSize = sizeof bmp_info_.bmiHeader;
	bmp_info_.bmiHeader.biWidth = width_;
	bmp_info_.bmiHeader.biHeight = height_;
	bmp_info_.bmiHeader.biPlanes = 1;
	bmp_info_.bmiHeader.biBitCount = bits_per_pixel_;
	bmp_info_.bmiHeader.biCompression = BI_RGB;
	bmp_info_.bmiHeader.biSizeImage = 0;
	bmp_info_.bmiHeader.biXPelsPerMeter = 0;
	bmp_info_.bmiHeader.biYPelsPerMeter = 0;
	bmp_info_.bmiHeader.biClrUsed = 0;
	bmp_info_.bmiHeader.biClrImportant = 0;
#endif
	if (color_components == 1)
		for (int i= 0; i < 256; ++i)
		{
			bmp_info_.bmiColors[i].rgbBlue = i;		// initialize color palette with shades of gray
			bmp_info_.bmiColors[i].rgbGreen = i;
			bmp_info_.bmiColors[i].rgbRed = i;
			bmp_info_.bmiColors[i].rgbReserved = 0;
		}

	size_t size= line_width_ * height_ + (alignment ? 0x10000 : 0);
	alloc_ = ::malloc(size);

	if (alloc_ == 0)
		THROW_EXCEPTION(L"Bitmap creation failed", L"Call to allocate bitmap memory failed.");

	// wipe out
//	memset(alloc_, 0xff, size);

	LONG_PTR p= reinterpret_cast<LONG_PTR>(alloc_);
	if (alignment)
	{
		// the goal is to have an alignment on 32k but not on 64k to avoid P4/Xeon 64k alignment penalty
		// and cache level one heavy trashing; CreateDIBSection allocates data on 64k boundaries already

		p = (p + 0x8000) & ~0x7fff;
		if ((p & 0x8000) == 0)
			p += 0x8000;
	}
	data_buff_ = reinterpret_cast<BYTE*>(p);
}



Dib::~Dib()
{
	if (alloc_)
		::free(alloc_);
}


void Dib::Draw(CDC* dc, CRect rect) const
{
/*	if (g_draw_dib)
	{
		::DrawDibDraw(g_draw_dib, *dc, rect.left, rect.top, rect.Width(), rect.Height(),
			&bi.bmiHeader, data_buff_, 0, 0, width_, height_, DDF_HALFTONE);
	}
	else */
	{
		dc->SetStretchBltMode(HALFTONE);//bits_per_pixel_ > 8 ? HALFTONE : COLORONCOLOR);
		::StretchDIBits(*dc, rect.left, rect.top, rect.Width(), rect.Height(),
			0, 0, width_, height_, data_buff_, &bmp_info_.AsBitmapInfo(), DIB_RGB_COLORS, SRCCOPY);
	}
}


void Dib::DibDraw(CDC* dc, CRect dest_rect, CRect* prectSrc) const
{
	ASSERT(IsValid());

	if (dc == 0)
		return;

	CRect src= prectSrc == 0 ? CRect(0, 0, width_, height_) : *prectSrc;

	if (HDRAWDIB hdd= ::DrawDibOpen())
	{
		::DrawDibDraw(hdd, *dc,
			dest_rect.left, dest_rect.top, dest_rect.Width(), dest_rect.Height(),
			const_cast<BITMAPINFOHEADER*>(bmp_info_.AsBitmapInfoHeader()), data_buff_,
			src.left, src.top, src.Width(), src.Height(), DDF_HALFTONE);

		::DrawDibClose(hdd);
	}
}


void Dib::DibDraw(CDC* dc, CPoint pos) const
{
	CRect rect(pos, GetSize());
	DibDraw(dc, rect, 0);
}



void Dib::Draw(CDC* dc, CRect dest_rect, CRect* prectSrc, bool halftone/*= true*/, bool use_icm/*= false*/) const
{
	ASSERT(IsValid());

	if (dc == 0)
		return;

	int x= 0, y= 0, w= width_, h= height_;

	if (prectSrc != 0)
	{
		x = prectSrc->left;
		y = bmp_info_.bmiHeader.bV5Height - prectSrc->bottom;
		w = prectSrc->Width();
		h = prectSrc->Height();
	}

	dc->SetStretchBltMode(halftone /*&& bits_per_pixel_ > 8*/ ? HALFTONE : COLORONCOLOR);

//	if (use_icm)
//	{
//		::SetICMMode(*dc, ICM_ON);
//*(DWORD*)(&bmp_info_.bmiHeader.V5CS_type) = LCS_sRGB;
//		::StretchDIBits(*dc, dest_rect.left, dest_rect.top, dest_rect.Width(), dest_rect.Height(),
//						x, y, w, h, data_buff_, &bmp_info_.AsBitmapInfo(), DIB_RGB_COLORS, SRCCOPY);
//		::SetICMMode(*dc, ICM_OFF);
//	}
//	else
		::StretchDIBits(*dc, dest_rect.left, dest_rect.top, dest_rect.Width(), dest_rect.Height(),
						x, y, w, h, data_buff_, &bmp_info_.AsBitmapInfo(), DIB_RGB_COLORS, SRCCOPY);
}


void Dib::Draw(CDC* dc, CPoint pos) const
{
	// do not use HALFTONE
	dc->SetStretchBltMode(COLORONCOLOR);

	::StretchDIBits(*dc, pos.x, pos.y, width_, height_,
		0, 0, width_, height_, data_buff_, &bmp_info_.AsBitmapInfo(), DIB_RGB_COLORS, SRCCOPY);
}


// draw image selecting method based on method
void Dib::DrawBitmap(CDC* dc, const CRect& dest_rect, CRect* prectSrc, DibDispMethod method) const
{
	if (method == DIB_DRAW_DIB)	// dib draw?
	{
		//DibDraw(dc, dest_rect, prectSrc);
		Draw(dc, dest_rect, prectSrc, false);
	}
	else
	{
		ASSERT(method == DIB_FAST_DRAW || method == DIB_SMOOTH_DRAW || method == DIB_DIRECT_2D);
		Draw(dc, dest_rect, prectSrc, method != DIB_FAST_DRAW);
	}
}


// Resize dib to requested dimensions; center it and fill not covered areas
// with rgb_back color
//
void Dib::Resize(CSize img_size, COLORREF rgb_back, bool uniform)
{
	if (img_size.cx < 0)		// negative value means percent
		img_size.cx = width_ * (-img_size.cx) / 100;
	if (img_size.cy < 0)
		img_size.cy = height_ * (-img_size.cy) / 100;

	int x= 0;
	int y= 0;
	int dest_w= img_size.cx;
	int dest_h= img_size.cy;

	Dib dibCopy(img_size.cx, img_size.cy, bits_per_pixel_);

	//TODO: revise
	//CDC dc;
	//dc.CreateDC(_T("DISPLAY"), 0, 0, 0);
	ASSERT(dib_bmp_.m_hObject != 0);

	CDC src_dc;
	src_dc.CreateCompatibleDC(0);
	src_dc.SelectObject(&dib_bmp_);

	CDC dst_dc;
	dst_dc.CreateCompatibleDC(0);
	dst_dc.SelectObject(&dibCopy.dib_bmp_);

	dst_dc.FillSolidRect(0, 0, img_size.cx, img_size.cy, rgb_back);

	if (uniform)
	{
		double ratio_img= double(width_) / double(height_);
		if (double(img_size.cx) / double(img_size.cy) > ratio_img)
			dest_w = static_cast<int>(dest_h * ratio_img);
		else
			dest_h = static_cast<int>(dest_w / ratio_img);
		x = (img_size.cx - dest_w) / 2;
		y = (img_size.cy - dest_h) / 2;
	}

//	// HALFTONE doesn't work on grayscale images (they are palette images internally)
// it works just fine
	dst_dc.SetStretchBltMode(HALFTONE);//bits_per_pixel_ > 8 ? HALFTONE : COLORONCOLOR);
	dst_dc.StretchBlt(x, y, dest_w, dest_h, &src_dc, 0, 0, width_, height_, SRCCOPY);

	ASSERT(alloc_ == 0);

	dib_bmp_.DeleteObject();
	dib_bmp_.Attach(dibCopy.dib_bmp_.Detach());
	data_buff_	 = dibCopy.data_buff_;
	width_	 = dibCopy.width_;
	line_width_ = dibCopy.line_width_;
	height_	 = dibCopy.height_;
	bmp_info_	 = dibCopy.bmp_info_;
}


// Create device dependent bmp form dib
//
AutoPtr<CBitmap> Dib::DeviceBitmap()
{
	AutoPtr<CBitmap> bmp(new CBitmap());

	CDC dc;
	dc.CreateIC(_T("DISPLAY"), NULL, NULL, NULL);

	bmp->CreateCompatibleBitmap(&dc, width_, height_);

	CDC dst_dc;
	dst_dc.CreateCompatibleDC(0);
	dst_dc.SelectObject(bmp.get());

	BITMAPINFO bi;
	bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
	bi.bmiHeader.biWidth = width_;
	bi.bmiHeader.biHeight = -height_;
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biBitCount = bits_per_pixel_;
	bi.bmiHeader.biCompression = BI_RGB;
	bi.bmiHeader.biSizeImage = 0;
	bi.bmiHeader.biXPelsPerMeter = 0;
	bi.bmiHeader.biYPelsPerMeter = 0;
	bi.bmiHeader.biClrUsed = 0;
	bi.bmiHeader.biClrImportant = 0;

/*	if (g_draw_dib)
	{
		::DrawDibDraw(g_draw_dib, dst_dc, 0, 0, width_, height_,
			&bi.bmiHeader, data_buff_, 0, 0, width_, height_, DDF_HALFTONE);
	}
	else */
	{
		dst_dc.SetStretchBltMode(HALFTONE);//bits_per_pixel_ > 8 ? HALFTONE : COLORONCOLOR);
		::StretchDIBits(dst_dc, 0, 0, width_, height_, 0, 0, width_, height_, data_buff_, &bi, DIB_RGB_COLORS, SRCCOPY);
	}

	return bmp;
}


void Dib::Save(const TCHAR* output_file)
{
	if (GetColorComponents() != 3 && GetColorComponents() != 1)
		THROW_EXCEPTION(L"Cannot save bitmap file", SF(L"Unsupported source image format.\nDestination: " << output_file));

	CFile out(output_file, CFile::modeWrite | CFile::modeCreate);

	int palette_size= GetPaletteSize();

	// note: sizeof BITMAPINFOHEADER is used to save only portion of BITMAPV5HEADER	bmiHeader
	int bmp_info_header_size= sizeof BITMAPINFOHEADER;
	BITMAPINFOHEADER bih=
#if USE_BITMAPV5HEADER
		*bmp_info_.AsBitmapInfoHeader();
#else
		bmp_info_.bmiHeader;
#endif
	bih.biSize = sizeof bih;
	bih.biClrUsed = GetNumColors();

	BITMAPFILEHEADER bfh;
	bfh.bfType = 'MB';
	bfh.bfReserved1 = 0;
	bfh.bfReserved2 = 0;
	bfh.bfOffBits = sizeof bfh + bmp_info_header_size + palette_size;
	bfh.bfSize = bfh.bfOffBits;
	bfh.bfSize += (((width_ * GetColorComponents()) + 3) & ~3) * height_;

	out.Write(&bfh, sizeof bfh);
	out.Write(&bih, bmp_info_header_size);
	if (palette_size > 0)
		out.Write(bmp_info_.bmiColors, palette_size);

	int line_bytes= width_ * GetColorComponents();
	int line_padding= (4 - (line_bytes & 3)) & 3;

	for (int y= height_ - 1; y >= 0; --y)
	{
		const BYTE* scan_line_data= LineBuffer(y);
		out.Write(scan_line_data, line_bytes);
		if (line_padding > 0)
		{
			static const BYTE padding[4]= { 0, 0, 0, 0 };
			out.Write(padding, line_padding);
		}
	}
}


bool Dib::Load(int bmp_rsrc_id)
{
	if (dib_bmp_.m_hObject)
		dib_bmp_.DeleteObject();

	HINSTANCE inst= AfxFindResourceHandle(MAKEINTRESOURCE(bmp_rsrc_id), RT_BITMAP);
	HANDLE dib= ::LoadImage(inst, MAKEINTRESOURCE(bmp_rsrc_id), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
	if (dib == 0)
		return false;

	dib_bmp_.Attach(dib);
	DIBSECTION ds;
	dib_bmp_.GetObject(sizeof ds, &ds);

#if USE_BITMAPV5HEADER
	bmp_info_ = ds.dsBmih;
#else
	bmp_info_.bmiHeader = ds.dsBmih;
#endif
	width_ = ds.dsBm.bmWidth;
	height_ = ds.dsBm.bmHeight;
	bits_per_pixel_ = ds.dsBm.bmBitsPixel;
	line_width_ = ds.dsBm.bmWidthBytes;
	data_buff_ = reinterpret_cast<BYTE*>(ds.dsBm.bmBits);

	if (bits_per_pixel_ <= 8)
	{
		CDC dc;
		dc.CreateCompatibleDC(0);
		dc.SelectObject(&dib_bmp_);

		::GetDIBColorTable(dc, 0, ColorTableSize(bits_per_pixel_), bmp_info_.bmiColors);
	}

	return true;
}


int Dib::ColorTableSize(int bits_per_pixel)
{
	int colors= 0;
	switch (bits_per_pixel)
	{
	case 8:
		colors = 256;
		break;
	case 4:
		colors = 16;
		break;
	case 1:
		colors = 2;
		break;
	}
	return colors;
}


// draw image preserving aspect ratio
//
void Dib::DrawScaled(CDC* dc, const CRect& dest_rect, COLORREF rgb_back) const
{
	ASSERT(dc);

	CSize bmp_size= GetSize();
	if (bmp_size.cx < 1 || bmp_size.cy < 1)
	{
		ASSERT(false);
		return;
	}

	CSize dest_size= dest_rect.Size();
	ASSERT(dest_size.cx > 0 && dest_size.cy > 0);

	double bmp_ratio= double(bmp_size.cx) / double(bmp_size.cy);

	double dest_ratio= double(dest_size.cx) / double(dest_size.cy);

	double epsillon= 0.01;

	// compare bmp ratio and destination ratio; if same bmp will be simply scalled
	if (fabs(bmp_ratio - dest_ratio) < epsillon)
	{
		Draw(dc, dest_rect, 0);
	}
	else
	{
		// calc how to rescale bmp to fit into dest rect
		double scale_w= double(dest_size.cx) / double(bmp_size.cx);
		double scale_h= double(dest_size.cy) / double(bmp_size.cy);

		double scale= MIN(scale_w, scale_h);

		// rescale bmp
		bmp_size.cx = static_cast<LONG>(bmp_size.cx * scale);
		bmp_size.cy = static_cast<LONG>(bmp_size.cy * scale);

		ASSERT(bmp_size.cx <= dest_size.cx);
		ASSERT(bmp_size.cy <= dest_size.cy);
		CSize diff_size= dest_size - bmp_size;

		// center rescaled bitmap in destination rect
		CPoint pos(diff_size.cx / 2, diff_size.cy / 2);

		// fill not covered areas
		if (diff_size.cx > diff_size.cy)
		{
			dc->FillSolidRect(dest_rect.left, dest_rect.top, pos.x, dest_size.cy, rgb_back);
			dc->FillSolidRect(dest_rect.left + pos.x + bmp_size.cx, dest_rect.top, dest_size.cx - pos.x - bmp_size.cx, dest_size.cy, rgb_back);
		}
		else
		{
			dc->FillSolidRect(dest_rect.left, dest_rect.top, dest_size.cx, pos.y, rgb_back);
			dc->FillSolidRect(dest_rect.left, dest_rect.top + pos.y + bmp_size.cy, dest_size.cx, dest_size.cy - pos.y - bmp_size.cy, rgb_back);
		}

		// finally draw the bitmap
		CRect rect(pos, bmp_size);
		rect.OffsetRect(dest_rect.TopLeft());

		Draw(dc, rect, 0);
	}
}


AutoPtr<Dib> Dib::RotateCopy(bool clockwise)
{
	int width= GetWidth();
	int height= GetHeight();
	int bpp= GetBitsPerPixel();
	if (bpp != 24 || width == 0 || height == 0 || data_buff_ == 0)
		return 0;

	AutoPtr<Dib> copy= new Dib(height, width, bpp);

	if (clockwise)
	{
		for (int line= 0; line < height; ++line)
		{
			int len= copy->GetHeight();
			BYTE* src= LineBuffer(height - line - 1);
			for (int x= 0; x < len; ++x, src += 3)
			{
				BYTE* dest= copy->LineBuffer(x) + line * 3;
				dest[0] = src[0];
				dest[1] = src[1];
				dest[2] = src[2];
			}
		}
	}
	else
	{
		for (int line= 0; line < height; ++line)
		{
			int len= copy->GetHeight();
			BYTE* src= LineBuffer(line);
			for (int x= len - 1; x >= 0; --x, src += 3)
			{
				BYTE* dest= copy->LineBuffer(x) + line * 3;
				dest[0] = src[0];
				dest[1] = src[1];
				dest[2] = src[2];
			}
		}
	}

	return copy;
}


void Dib::Swap(Dib& dib)
{
	HGDIOBJ bmp_org= dib_bmp_.Detach();
	HGDIOBJ bmp_copy= dib.dib_bmp_.Detach();
	dib_bmp_.Attach(bmp_copy);
	dib.dib_bmp_.Attach(bmp_org);

	std::swap(width_, dib.width_);
	std::swap(height_, dib.height_);
	std::swap(line_width_, dib.line_width_);
	std::swap(bits_per_pixel_, dib.bits_per_pixel_);
	std::swap(data_buff_, dib.data_buff_);
	std::swap(bmp_info_, dib.bmp_info_);
	std::swap(alloc_, dib.alloc_);

	RGBQUAD v[255];
	std::copy(color_table_, color_table_ + array_count(color_table_), v);
	std::copy(dib.color_table_, dib.color_table_ + array_count(dib.color_table_), color_table_);
	std::copy(v, v + array_count(v), dib.color_table_);
}


void Dib::RotateInPlace(bool clockwise)
{
	AutoPtr<Dib> copy= RotateCopy(clockwise);
	if (copy)
		Swap(*copy);
}


CSize Dib::SizeToFit(CSize dest_size, CSize orig_img_size)
{
	if (dest_size.cx <= 0 || dest_size.cy <= 0 || orig_img_size.cx <= 0 || orig_img_size.cy <= 0)
		return CSize(0, 0);

	int dest_w= dest_size.cx;
	int dest_h= dest_size.cy;

	double ratio_img= double(orig_img_size.cx) / double(orig_img_size.cy);
	const double delta= double(dest_size.cx) / double(dest_size.cy) - ratio_img;
	const double epsilon= 0.02;	// 2%

	if (fabs(delta) < epsilon)
		return CSize(dest_w, dest_h);

	if (delta > 0.0)
		dest_w = std::max(1, static_cast<int>(dest_h * ratio_img + 0.5));
	else
		dest_h = std::max(1, static_cast<int>(dest_w / ratio_img + 0.5));

	return CSize(dest_w, dest_h);
}


bool Dib::ResizeToFit(CSize img_size, ResizeMethod method, Dib& dibCopy)
{
	CSize dest_size= Dib::SizeToFit(img_size, CSize(width_, height_));

	if (dest_size.cx == 0 || dest_size.cy == 0)
	{
		ASSERT(false);
		return false;
	}

	// do not increase in size
	if (dest_size.cx >= width_ && dest_size.cy >= height_)
		return false;

	//if (dest_w == width_ && dest_h == height_)
	//	return;

	dibCopy.Create(dest_size.cx, dest_size.cy, bits_per_pixel_);

	CDC src_dc;
	src_dc.CreateCompatibleDC(0);
	src_dc.SelectObject(&dib_bmp_);

	CDC dst_dc;
	dst_dc.CreateCompatibleDC(0);
	dst_dc.SelectObject(&dibCopy.dib_bmp_);

	//HACK: cubic resizing has trouble handling grayscale images; investigate
	if (method == RESIZE_CUBIC && GetBitsPerPixel() == 8)	// grayscale image?
		method = RESIZE_HALFTONE;

	switch (method)
	{
	case RESIZE_POINT_SAMPLE:
		dst_dc.SetStretchBltMode(COLORONCOLOR);
		dst_dc.StretchBlt(0, 0, dest_size.cx, dest_size.cy, &src_dc, 0, 0, width_, height_, SRCCOPY);
		break;
	case RESIZE_HALFTONE:
		dst_dc.SetStretchBltMode(HALFTONE);
		dst_dc.StretchBlt(0, 0, dest_size.cx, dest_size.cy, &src_dc, 0, 0, width_, height_, SRCCOPY);
		break;
	case RESIZE_CUBIC:
		MagnifyBitmap(*this, dest_size, dibCopy, true);
		break;
	default:
		ASSERT(false);
		break;
	}

	return true;
}


// resize (preserving aspect ratio) so the resized copy fits into 'img_size'
void Dib::ResizeToFit(CSize img_size, ResizeMethod method)
{
	CSize dest_size= Dib::SizeToFit(img_size, CSize(width_, height_));

	if (dest_size.cx == 0 || dest_size.cy == 0)
	{
		ASSERT(false);
		return;
	}

	// do not increase in size
	if (dest_size.cx >= width_ && dest_size.cy >= height_)
		return;

	//if (dest_w == width_ && dest_h == height_)
	//	return;

	Dib dibCopy(dest_size.cx, dest_size.cy, bits_per_pixel_);

	CDC src_dc;
	src_dc.CreateCompatibleDC(0);
	src_dc.SelectObject(&dib_bmp_);

	CDC dst_dc;
	dst_dc.CreateCompatibleDC(0);
	dst_dc.SelectObject(&dibCopy.dib_bmp_);

	//HACK: cubic resizing has trouble handling grayscale images; investigate
	if (method == RESIZE_CUBIC && GetBitsPerPixel() == 8)	// grayscale image?
		method = RESIZE_HALFTONE;

	switch (method)
	{
	case RESIZE_POINT_SAMPLE:
		dst_dc.SetStretchBltMode(COLORONCOLOR);
		dst_dc.StretchBlt(0, 0, dest_size.cx, dest_size.cy, &src_dc, 0, 0, width_, height_, SRCCOPY);
		break;
	case RESIZE_HALFTONE:
		dst_dc.SetStretchBltMode(HALFTONE);
		dst_dc.StretchBlt(0, 0, dest_size.cx, dest_size.cy, &src_dc, 0, 0, width_, height_, SRCCOPY);
		break;
	case RESIZE_CUBIC:
		MagnifyBitmap(*this, dest_size, dibCopy, true);
		break;
	default:
		ASSERT(false);
		break;
	}

//	dst_dc.SetStretchBltMode(HALFTONE);//bits_per_pixel_ > 8 ? HALFTONE : COLORONCOLOR);
//	dst_dc.StretchBlt(0, 0, dest_w, dest_h, &src_dc, 0, 0, width_, height_, SRCCOPY);

	ASSERT(alloc_ == 0);

	dib_bmp_.DeleteObject();
	dib_bmp_.Attach(dibCopy.dib_bmp_.Detach());
	data_buff_	 = dibCopy.data_buff_;
	width_	 = dibCopy.width_;
	line_width_ = dibCopy.line_width_;
	height_	 = dibCopy.height_;
	bmp_info_	 = dibCopy.bmp_info_;
}


// in-place convert from YCbCr space to RGB
void Dib::ConvertYCbCr2RGB()
{
	int width= GetWidth();
	int height= GetHeight();
	int bpp= GetBitsPerPixel();
	if (bpp != 24 || width == 0 || height == 0 || data_buff_ == 0)
		return;

	const int MAXJSAMPLE= 255;
	const int CENTERJSAMPLE= 128;

	int Crrtab[MAXJSAMPLE + 1];
	int Crgtab[MAXJSAMPLE + 1];
	int Cbgtab[MAXJSAMPLE + 1];
	int Cbbtab[MAXJSAMPLE + 1];

	const int SCALEBITS=	16;	/* speediest right-shift on some machines */
	const int ONE_HALF=	((INT32) 1 << (SCALEBITS-1));
#define FIX(x)		((INT32) ((x) * (1L<<SCALEBITS) + 0.5))
#define RIGHT_SHIFT(x, n)	((x) >> (n))

	for (int i= 0, x= -CENTERJSAMPLE; i <= MAXJSAMPLE; ++i, ++x)
	{
		/* i is the actual input pixel value, in the range 0..MAXJSAMPLE */
		/* The Cb or Cr value we are thinking of is x = i - CENTERJSAMPLE */
		/* Cr=>R value is nearest int to 1.40200 * x */
		Crrtab[i] = (int)	RIGHT_SHIFT(FIX(1.40200) * x + ONE_HALF, SCALEBITS);
		/* Cb=>B value is nearest int to 1.77200 * x */
		Cbbtab[i] = (int)	RIGHT_SHIFT(FIX(1.77200) * x + ONE_HALF, SCALEBITS);
		/* Cr=>G value is scaled-up -0.71414 * x */
		Crgtab[i] = (- FIX(0.71414)) * x;
		/* Cb=>G value is scaled-up -0.34414 * x */
		/* We also add in ONE_HALF so that need not do it in inner loop */
		Cbgtab[i] = (- FIX(0.34414)) * x + ONE_HALF;
	}
#undef RIGHT_SHIFT
#undef FIX

	for (int line= 0; line < height; ++line)
	{
		uint8* data= LineBuffer(line);

		for (int n= 0; n < width; ++n)
		{
			uint8 y= data[0];
			uint8 cb= data[1];
			uint8 cr= data[2];

			int r= y + Crrtab[cr];
			int g= y + ((Cbgtab[cb] + Crgtab[cr]) >> 16);
			int b= y + Cbbtab[cb];

			if (r < 0)			r = 0;
			else if (r > 255)	r = 255;
			if (g < 0)			g = 0;
			else if (g > 255)	g = 255;
			if (b < 0)			b = 0;
			else if (b > 255)	b = 255;

			data[0] = uint8(b);
			data[1] = uint8(g);
			data[2] = uint8(r);

			/*
			data[0] = range_limit[y + Crrtab[cr]];
			data[1] = range_limit[y +
			      ((int) RIGHT_SHIFT(Cbgtab[cb] + Crgtab[cr],
						 SCALEBITS))];
			data[2] = range_limit[y + Cbbtab[cb]]; */

			data += 3;
		}
	}


/*
  while (--num_rows >= 0) {
    inptr0 = input_buf[0][input_row];
    inptr1 = input_buf[1][input_row];
    inptr2 = input_buf[2][input_row];
    input_row++;
    outptr = *output_buf++;
    for (col = 0; col < num_cols; col++) {
      y  = GETJSAMPLE(inptr0[col]);
      cb = GETJSAMPLE(inptr1[col]);
      cr = GETJSAMPLE(inptr2[col]);
      // Range-limiting is essential due to noise introduced by DCT losses.
      outptr[RGB_RED] =   range_limit[y + Crrtab[cr]];
      outptr[RGB_GREEN] = range_limit[y +
			      ((int) RIGHT_SHIFT(Cbgtab[cb] + Crgtab[cr],
						 SCALEBITS))];
      outptr[RGB_BLUE] =  range_limit[y + Cbbtab[cb]];
      outptr += RGB_PIXELSIZE;
    }
  }
*/
}


bool Dib::IsValid() const
{
	return /*dib_bmp_.m_hObject != 0 &&*/ data_buff_ != 0;
}


int Dib::GetNumColors() const
{
#if USE_BITMAPV5HEADER
	if (bmp_info_.bmiHeader.bV5ClrUsed != 0)
		return bmp_info_.bmiHeader.bV5ClrUsed;

	switch (bmp_info_.bmiHeader.bV5BitCount)
#else
	if (bmp_info_.bmiHeader.biClrUsed != 0)
		return bmp_info_.bmiHeader.biClrUsed;

	switch (bmiHeader.biBitCount.biBitCount)
#endif
	{
	case 1:		return 2;
	case 4:		return 16;
	case 8:		return 256;
	default:	return 0;
	}
}


DWORD Dib::GetPaletteSize() const
{
	return GetNumColors() * sizeof color_table_[0];
}


void Dib::Clone(const Dib& src)
{
	if (width_ == src.GetWidth() && height_ == src.GetHeight() &&
		bits_per_pixel_ == src.GetBitsPerPixel() && dib_bmp_.m_hObject != 0)
	{
		//
	}
	else
		Create(src.GetWidth(), src.GetHeight(), src.GetBitsPerPixel());

	if (data_buff_ && src.data_buff_)
	{
		memcpy(GetColorTable(), src.GetColorTable(), 256 * (sizeof RGBQUAD));
		memcpy(data_buff_, src.data_buff_, line_width_ * height_);
	}
}


CRect Dib::GetDisplayRect(CRect dest_rect)
{
	CRect bmp_rect(0,0,0,0);

	CSize bmp_size= GetSize();
	if (bmp_size.cx < 1 || bmp_size.cy < 1)
		return bmp_rect;

	CSize dest_size= dest_rect.Size();
	if (dest_size.cx <= 0 || dest_size.cy <= 0)
		return bmp_rect;

	double bmp_ratio= double(bmp_size.cx) / double(bmp_size.cy);
	double dest_ratio= double(dest_size.cx) / double(dest_size.cy);
	double epsillon= 0.01;

	// compare bmp ratio and destination ratio; if same bmp will be simply scalled
	bool simple_rescaling= fabs(bmp_ratio - dest_ratio) < epsillon;

	// calc how to rescale bmp to fit into dest rect
	double scale_w= double(dest_size.cx) / double(bmp_size.cx);
	double scale_h= double(dest_size.cy) / double(bmp_size.cy);

	double scale= MIN(scale_w, scale_h);

	// rescale bmp
	bmp_size.cx = static_cast<LONG>(bmp_size.cx * scale);
	bmp_size.cy = static_cast<LONG>(bmp_size.cy * scale);

	ASSERT(bmp_size.cx <= dest_size.cx);
	ASSERT(bmp_size.cy <= dest_size.cy);
	CSize diff_size= dest_size - bmp_size;

	// center rescalled bitmap in destination rect
	CPoint pos(diff_size.cx / 2, diff_size.cy / 2);

//	if (flags & DRAW_BACKGND)
/*
	{
		CSize dest_size= dest_rect.Size();

		// fill areas not covered by bitmap
		if (diff_size.cx > diff_size.cy)
		{
			dc->FillSolidRect(dest_rect.left, dest_rect.top, pos.x, dest_size.cy, rgb_backgnd);
			dc->FillSolidRect(dest_rect.left + pos.x + bmp_size.cx, dest_rect.top, dest_size.cx - pos.x - bmp_size.cx, dest_size.cy, rgb_backgnd);
		}
		else
		{
			dc->FillSolidRect(dest_rect.left, dest_rect.top, dest_size.cx, pos.y, rgb_backgnd);
			dc->FillSolidRect(dest_rect.left, dest_rect.top + pos.y + bmp_size.cy, dest_size.cx, dest_size.cy - pos.y - bmp_size.cy, rgb_backgnd);
		}
	}
*/
	// finally draw the bitmap
	bmp_rect = CRect(pos, bmp_size);
	bmp_rect.OffsetRect(dest_rect.TopLeft());

//	Draw(dc, bmp_rect, 0, true, icm);
	return bmp_rect;
}


CRect SizeImageToFill(CSize image, CSize dest)
{
	CRect rect(0,0,0,0);

	if (image.cx <= 0 || image.cy <= 0)
		return rect;

	if (dest.cx <= 0 || dest.cy <= 0)
		return rect;

	double dest_w= dest.cx;
	double dest_h= dest.cy;
	double img_w= image.cx;
	double img_h= image.cy;

	double img_ratio= img_w / img_h;
	double dest_ratio= dest_w / dest_h;
	double delta= dest_ratio - img_ratio;
	const double epsilon= 0.02;	// 2%

	double x= 0.0, y= 0.0;	// offset

	if (fabs(delta) < epsilon)
	{
	}
	else if (delta < 0.0)
	{
		img_w = std::max(1.0, img_h * dest_ratio);
		x = (image.cx - img_w) / 2.0;
	}
	else
	{
		img_h = std::max(1.0, img_w / dest_ratio);
		y = (image.cy - img_h) / 2.0;
	}

	CPoint p(static_cast<int>(x + 0.5), static_cast<int>(y + 0.5));
	CSize s(static_cast<int>(img_w), static_cast<int>(img_h));
	return CRect(p, s);
}


bool DrawPreserveAspectRatio(const Dib& dib, CDC* dc, CRect dest, COLORREF backgnd, bool icm, bool resize_to_fill)
{
	CSize img_size= dib.GetSize();
	if (img_size.cx < 1 || img_size.cy < 1)
		return false;

	CSize dest_size= dest.Size();
	if (dest_size.cx <= 0 || dest_size.cy <= 0)
		return false;

	CRect img_rect(0,0,0,0);
	
	if (resize_to_fill)
		img_rect = SizeImageToFill(img_size, dest_size);
	else
		img_rect = CRect(CPoint(0, 0), Dib::SizeToFit(dest_size, img_size));

	if (img_rect.IsRectEmpty())
		return false;

	if (resize_to_fill)
	{
		dib.Draw(dc, dest, &img_rect, true, icm);
	}
	else
	{
		img_rect.OffsetRect(dest.left + (dest_size.cx - img_rect.Width()) / 2, dest.top + (dest_size.cy - img_rect.Height()) / 2);

		// fill areas not covered by bitmap

		if (img_rect.Width() < dest.Width())
		{
			// bars at the left & right
			dc->FillSolidRect(dest.left, dest.top, img_rect.left - dest.left, dest_size.cy, backgnd);
			dc->FillSolidRect(img_rect.right, dest.top, dest.right - img_rect.right, dest_size.cy, backgnd);
		}

		if (img_rect.Height() < dest.Height())
		{
			// bars at the top & bottom
			dc->FillSolidRect(dest.left, dest.top, dest_size.cx, img_rect.top - dest.top, backgnd);
			dc->FillSolidRect(dest.left, img_rect.bottom, dest_size.cx, dest.bottom - img_rect.bottom, backgnd);
		}

		// finally draw the bitmap
		dib.Draw(dc, img_rect, 0, true, icm);
	}

	return true;
}


RGBQUAD Dib::GetPixel(int x, int y) const
{
	RGBQUAD rgb= { 0, 0, 0, 0 };

	if (data_buff_ && static_cast<size_t>(x) < width_ && static_cast<size_t>(y) < height_)
	{
		if (const BYTE* line= LineBuffer(y))
		{
			switch (GetColorComponents())
			{
			case 3:	// RGB
				x *= 3;
				rgb.rgbBlue = line[x];
				rgb.rgbGreen = line[x + 1];
				rgb.rgbRed = line[x + 2];
				break;

			case 1:	// grayscale
				rgb.rgbRed = rgb.rgbGreen = rgb.rgbBlue = line[x];
				break;

			case 4:	// CMYK?
				break;
			}
		}
	}

	return rgb;
}


void Dib::Detach(CBitmap& bmp)
{
	bmp.Attach(static_cast<HBITMAP>(dib_bmp_.Detach()));
}

void Dib::CreateCBitmap()
{
	if (alloc_ != nullptr && dib_bmp_.m_hObject == nullptr)
	{
		VOID* data = 0;
		HBITMAP dib = ::CreateDIBSection(0, &bmp_info_.AsBitmapInfo(), DIB_RGB_COLORS, &data, 0, 0);
		if (dib == 0 || data == 0)
			THROW_EXCEPTION(L"Bitmap creation failed", L"Call to CreateDIBSection failed.");

		dib_bmp_.Attach(dib);

		memcpy(data, data_buff_, GetBufferSize());
	}
}

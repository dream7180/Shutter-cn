/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// PNGImage.cpp: implementation of the PNGImage class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PNGImage.h"
#include "pnglib/png.h"
#include "WhistlerLook.h"
#include "UIElements.h"
#include "Dib.h"
#include "BmpFunc.h"
#include "GlobalAlloc.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
//#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

PNGImage::PNGImage() : data_(0), end_(0), rgb_backgnd_(~0u)
{}

PNGImage::~PNGImage()
{}


namespace {

struct PNGReaderException
{};


void PNGAPI ErrorFunc PNGARG((png_structp, png_const_charp))
{
	throw PNGReaderException();
}


void PNGAPI WarnFunc PNGARG((png_structp, png_const_charp))
{}


void pngtest_read_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
	PNGImage* p= reinterpret_cast<PNGImage*>(png_get_io_ptr(png_ptr));

	if (p->data_ + length > p->end_)
		png_error(png_ptr, "reading past the end of PNG");
	else
	{
		memcpy(data, p->data_, length);
		p->data_ += length;
	}
}


} // namespace


bool PNGImage::Load(int resource_id, CBitmap& bmp, BITMAPINFOHEADER* bih, float saturation, float lightness, float alpha)
{
	const TCHAR* resource_name= MAKEINTRESOURCE(resource_id);
	return Load(resource_name, bmp, bih, saturation, lightness, alpha);
}

bool PNGImage::Load(LPCTSTR resource_id, CBitmap& bmp, BITMAPINFOHEADER* bmpih, float saturation, float lightness, float alpha)
{
	Dib dib;
	bool ret = Load(resource_id, dib, bmpih, saturation, lightness, alpha);
	dib.Detach(bmp);
	return ret;
}

bool PNGImage::Load(int resource_id, Dib& dib, BITMAPINFOHEADER* bih, float saturation, float lightness, float alpha)
{
	const TCHAR* resource_name = MAKEINTRESOURCE(resource_id);
	return Load(resource_name, dib, bih, saturation, lightness, alpha);
}

const int LOW_RES = 96;
// Inkscape outputs 1:1 PNGs at 90 dpi rather than 96; correction factor
const double INKSCAPE_CORRECTION = 96.0 / 90.0;

int GetLowResValue()		// low resolution in pixels/meter
{
	int low_res = int(LOW_RES * 100 / 2.54 + 0.5); // 96 dpi
	return low_res;
}

bool PNGImage::Load(LPCTSTR resource_id, Dib& dib, BITMAPINFOHEADER* bmpih, float saturation, float lightness, float alpha)
{
	const TCHAR* const RES_TYPE_PNG= _T("PNG");

	const TCHAR* resource_name= resource_id; //MAKEINTRESOURCE(rsrc_id);
	HINSTANCE inst= AfxFindResourceHandle(resource_name, RES_TYPE_PNG);
	HRSRC rsrc= ::FindResource(inst, resource_name, RES_TYPE_PNG);
	if (rsrc == NULL)
		return false;

	HGLOBAL global= ::LoadResource(inst, rsrc);
	if (global == NULL)
		return false;

	// PNG image
	end_ = data_ = reinterpret_cast<BYTE*>(::LockResource(global));
	if (data_ == NULL)
		return false;

	end_ += ::SizeofResource(inst, rsrc);

	png_structp png= png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, &ErrorFunc, &WarnFunc);
	png_infop info= 0;

	if (png == 0)
		return false;

	int width = 0;
	int height = 0;
	CSize logical_size(width, height);

	try
	{
		// Allocate/initialize the memory for image information
		info = png_create_info_struct(png);
		if (info == 0)
		{
			png_destroy_read_struct(&png, nullptr, nullptr);
			return false;
		}

		png_set_read_fn(png, this, pngtest_read_data);

		if (rgb_backgnd_ != ~0u)
		{
			png_color_16 background= { 0, GetRValue(rgb_backgnd_), GetGValue(rgb_backgnd_), GetBValue(rgb_backgnd_), 0 };
			png_set_background(png, &background, PNG_BACKGROUND_GAMMA_SCREEN, 0, 1.0);
		}

		png_read_png(png, info, PNG_TRANSFORM_BGR | PNG_TRANSFORM_STRIP_16, nullptr);

		logical_size.cx = width = png_get_image_width(png, info);
		logical_size.cy = height = png_get_image_height(png, info);

		auto xpm = png_get_x_pixels_per_meter(png, info) * INKSCAPE_CORRECTION;
		auto ypm = png_get_y_pixels_per_meter(png, info) * INKSCAPE_CORRECTION;
		const int low_res = GetLowResValue();
		if (xpm > low_res && ypm > low_res) // high res image?
		{
			auto size = CSize(width, height);
			auto scale = GetResolution();
			CSize pix(static_cast<int>(size.cx * scale.Width / xpm + 0.5), static_cast<int>(size.cy * scale.Height / ypm + 0.5));
			// resized?
			if (size != pix)
			{
				logical_size.cx = pix.cx;
				logical_size.cy = pix.cy;
			}
		}

		if (rgb_backgnd_ == ~0u)
		{
			// the only format supported here:
			ASSERT(png_get_color_type(png, info) == PNG_COLOR_TYPE_RGB_ALPHA);
		}
		else
		{
			ASSERT(png_get_color_type(png, info) == PNG_COLOR_TYPE_RGB);
		}
	}
	catch (PNGReaderException&)
	{
	}

	dib.Create(width, height, png_get_channels(png, info) * 8);
/*
	CDC dc;
	dc.CreateDC(_T("DISPLAY"), 0, 0, 0);

	BITMAPV4HEADER bih;
	memset(&bih, 0, sizeof bih);
	bih.bV4Size = sizeof bih;
	bih.bV4Width = png_get_image_width(png, info);
	bih.bV4Height = -static_cast<LONG>(png_get_image_height(png, info));
	bih.bV4Planes = 1;
	bih.bV4BitCount = png_get_channels(png, info) * 8;
	bih.bV4V4Compression = BI_RGB; //BI_BITFIELDS;
	bih.bV4SizeImage = 0;
	bih.bV4XPelsPerMeter = 0;
	bih.bV4YPelsPerMeter = 0;
	bih.bV4ClrUsed = 0;
	bih.bV4ClrImportant = 0;
	bih.bV4RedMask		= 0x0000ff00;
	bih.bV4GreenMask	= 0x00ff0000;
	bih.bV4BlueMask		= 0xff000000;
	bih.bV4AlphaMask	= 0x000000ff;

	VOID* data= 0;
	HBITMAP dib= ::CreateDIBSection(dc, reinterpret_cast<BITMAPINFO*>(&bih), DIB_PAL_COLORS, &data, 0, 0);
*/
	BYTE* data = dib.GetBuffer();
	if (data)
	{
//		bmp.Attach(dib);
		BITMAP bm;
		dib.GetBmp()->GetBitmap(&bm);

		unsigned int img_width= png_get_image_width(png, info);
		unsigned int img_height= png_get_image_height(png, info);

		png_byte** row_pointers= png_get_rows(png, info);

		if (row_pointers != 0 && png_get_valid(png, info, PNG_INFO_IDAT))
		{
			// bm.bmWidthBytes lies on W2K; on XP it's fine
			//int width= bm.bmWidthBytes;
			int chn= png_get_channels(png, info);
			int width= ((chn * img_width) + 3) & ~3;

			BYTE* p= reinterpret_cast<BYTE*>(data);

			if (png_get_color_type(png, info) == PNG_COLOR_TYPE_RGB_ALPHA && (saturation != 0.0f || lightness != 0.0f || alpha != 1.0f))
			{
				extern COLORREF CalcNewColorDelta(COLORREF rgb_color, float saturation, float lightness);

				for (unsigned int i = 0; i < img_height; ++i, p += width)
				{
					BYTE* dst= dib.LineBuffer(i);
					BYTE* src= row_pointers[i];

					for (unsigned int x= 0; x < img_width; ++x)
					{
						COLORREF rgb= CalcNewColorDelta(RGB(src[2], src[1], src[0]), saturation, lightness);
						dst[0] = GetBValue(rgb);
						dst[1] = GetGValue(rgb);
						dst[2] = GetRValue(rgb);
						dst[3] = static_cast<BYTE>(src[3] * alpha);	// alpha
						src += 4;
						dst += 4;
					}
				}
			}
			else
			{
				size_t row_bytes= png_get_rowbytes(png, info);

				for (unsigned int i = 0; i < img_height; ++i, p += width)
				{
					BYTE* dst = dib.LineBuffer(i);
					memcpy(dst, row_pointers[i], row_bytes);
				}
			}
		}
	}

	png_destroy_read_struct(&png, &info, nullptr);

	::UnlockResource(global);
	::FreeResource(global);

	if (width != logical_size.cx || height != logical_size.cy)
	{
		Dib copy;
		MagnifyBitmap(dib, logical_size, copy, true);
		dib.Swap(copy);
		dib.CreateCBitmap();
	}

	if (bmpih)
	{
		const BITMAPINFO& info = dib.AsBitmapInfo();
		*bmpih = info.bmiHeader;
	}

	return true;
}


bool PNGImage::LoadMask(int resource_id, CBitmap& bmp)
{
	const TCHAR* resource_name= MAKEINTRESOURCE(resource_id);
	return LoadMask(resource_name, bmp);
}


bool PNGImage::LoadMask(LPCTSTR resource_id, CBitmap& bmp)
{
	CBitmap image_bmp;
	BITMAPINFOHEADER bih;

	if (!Load(resource_id, image_bmp, &bih))
		return false;

	BITMAP bm;
	memset(&bm, 0, sizeof bm);
	if (image_bmp.GetObject(sizeof bm, &bm) == 0 || bm.bmBits == 0 || bm.bmBitsPixel != 32)
		return false;

	if (!bmp.CreateBitmap(bm.bmWidth, bm.bmHeight, 1, 1, 0))
		return false;

	CDC dc;
	dc.CreateCompatibleDC(0);
	dc.SelectObject(&bmp);

	BYTE* data= reinterpret_cast<BYTE*>(bm.bmBits);

	UINT line= (bm.bmWidth + 3) & ~3;
	// NOTE: bmWidthBytes lies on W2K; on XP it's fine
	UINT size= bm.bmHeight * line; //bm.bmWidthBytes;

	for (UINT i= 0; i < size; ++i)
	{
		data[0] = data[1] = data[2] = data[3] > 0x20 ? 0x00 : 0xff;
		data[3] = 0;
		data += 4;
	}

	CDC src_dc;
	src_dc.CreateCompatibleDC(0);
	src_dc.SelectObject(&image_bmp);

	dc.BitBlt(0, 0, bm.bmWidth, bm.bmHeight, &src_dc, 0, 0, SRCCOPY);

	return true;
}


bool PNGImage::LoadImageList(int bmp_id, int single_bmp_width, COLORREF rgb_backgnd, CImageList& image_list)
{
	return LoadImageList(bmp_id, single_bmp_width, rgb_backgnd, image_list, 0.0f, 0.0f, 1.0f, true);
}


bool PNGImage::LoadImageList(int bmp_id, int single_bmp_width, COLORREF rgb_backgnd, CImageList& image_list,
		float saturation, float lightness, float alpha, bool has_transparency)
{
	ASSERT(image_list.m_hImageList == 0);

	CBitmap mask_bmp;
	CBitmap image_bmp;
	// mask is only necessary on non-XP windows without transparency support
	bool create_mask_image= !WhistlerLook::IsAvailable();

	PNGImage ping;

	if (create_mask_image && has_transparency)
	{
		ping.AlphaToColor(rgb_backgnd);
		PNGImage().LoadMask(bmp_id, mask_bmp);
	}
	else if (!has_transparency)
		ping.AlphaToColor(rgb_backgnd);

	// try to find PNG first
	if (ping.Load(bmp_id, image_bmp, 0, saturation, lightness, alpha) && (mask_bmp.m_hObject != 0 || !create_mask_image))
	{
		UINT flags= ILC_COLOR32;
		if (create_mask_image)
			flags |= ILC_MASK;

		BITMAP bm;
		memset(&bm, 0, sizeof bm);
		image_bmp.GetObject(sizeof bm, &bm);

		int w = 0;
		if (single_bmp_width > 0)
			w = Pixels(single_bmp_width); // TODO: rounding errors
		else
			w = bm.bmWidth / -single_bmp_width;

		HIMAGELIST img_list= ImageList_Create(w, bm.bmHeight, flags, bm.bmWidth / w, 0);
		ASSERT(img_list != 0);
		if (img_list)
		{
			image_list.Attach(img_list);
			image_list.Add(&image_bmp, create_mask_image ? &mask_bmp : static_cast<CBitmap*>(0));
		}
	}
	else
		return false;

	return image_list.m_hImageList != 0;
}


bool PNGImage::LoadImageList(int bmp_id, COLORREF backgnd_color, CImageList& image_list, float saturation, float lightness, bool has_transparency, int img_count)
{
	ASSERT(image_list.m_hImageList == 0 && img_count > 0);

	CBitmap mask;
	CBitmap image;
	// mask is only necessary on non-XP windows without transparency support
	bool createMaskImage= !WhistlerLook::IsAvailable();

	PNGImage ping;

	if (createMaskImage && has_transparency)
	{
		ping.AlphaToColor(backgnd_color);
		PNGImage().LoadMask(bmp_id, mask);
	}
	else if (!has_transparency)
		ping.AlphaToColor(backgnd_color);

	// try to find PNG first
	if (ping.Load(bmp_id, image, 0, saturation, lightness) && (mask.m_hObject != 0 || !createMaskImage))
	{
		UINT flags= ILC_COLOR32;
		if (createMaskImage)
			flags |= ILC_MASK;

		BITMAP bm;
		memset(&bm, 0, sizeof bm);
		image.GetObject(sizeof bm, &bm);

		int single_item_width= bm.bmWidth / img_count;
		HIMAGELIST list_img= ImageList_Create(single_item_width, bm.bmHeight, flags, img_count, 0);
		ASSERT(list_img != 0);
		if (list_img)
		{
			image_list.Attach(list_img);
			image_list.Add(&image, createMaskImage ? &mask : static_cast<CBitmap*>(0));
		}
	}
	else
		return false;

	return image_list.m_hImageList != 0;
}


extern std::auto_ptr<Gdiplus::Bitmap> LoadPng(int rsrc_id, const wchar_t* rsrc_type, HMODULE instance)
{
	std::auto_ptr<Gdiplus::Bitmap> bmp;

	HRSRC resource = ::FindResource(instance, MAKEINTRESOURCE(rsrc_id), rsrc_type);
	if (resource == 0)
		return bmp;

	DWORD png_size = ::SizeofResource(instance, resource);
	if (png_size == 0)
		return bmp;

	const void* png_data = ::LockResource(::LoadResource(instance, resource));
	if (png_data == 0)
		return bmp;

	GlobalMemAlloc<BYTE> buffer_mem(png_size, GMEM_MOVEABLE);
	GlobalMemLock<BYTE> buf(buffer_mem.get());

	if (buf == 0)
		return bmp;

	memcpy(buf, png_data, png_size);

	IStreamPtr stream;
	if (::CreateStreamOnHGlobal(buf, false, &stream) != S_OK)
		return bmp;

	bmp.reset(Gdiplus::Bitmap::FromStream(stream));

	if (bmp.get() != 0 && bmp->GetLastStatus() != Gdiplus::Ok)
		bmp.reset();

	if (bmp.get())
	{
		auto dpi_x = bmp->GetHorizontalResolution() * INKSCAPE_CORRECTION;
		auto dpi_y = bmp->GetVerticalResolution() * INKSCAPE_CORRECTION;
		if (dpi_x > LOW_RES && dpi_y > LOW_RES) // high res image?
		{
			auto size = CSize(bmp->GetWidth(), bmp->GetHeight());
			auto scale = GetResolutionDpi();
			CSize pix(static_cast<int>(size.cx * scale.Width / dpi_x + 0.5), static_cast<int>(size.cy * scale.Height / dpi_y + 0.5));
			if (size != pix)
			{
				std::auto_ptr<Gdiplus::Bitmap> resized(new Gdiplus::Bitmap(pix.cx, pix.cy, bmp->GetPixelFormat()));
				Gdiplus::Graphics graphics(resized.get());
				graphics.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
				graphics.DrawImage(bmp.get(), 0, 0, pix.cx, pix.cy);
				bmp = resized;
			}
		}
	}

	return bmp;
}

/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "pnglib/png.h"
#include "../Dib.h"


namespace {

struct PNGReaderException
{};


void PNGAPI ErrorFunc PNGARG((png_structp, png_const_charp))
{
	throw PNGReaderException();
}


void PNGAPI WarnFunc PNGARG((png_structp, png_const_charp))
{}


struct PingImg
{
	BYTE* data_;
	BYTE* end_;
};


void pngtest_read_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
	PingImg* p= reinterpret_cast<PingImg*>(png_get_io_ptr(png_ptr));

	if (p->data_ + length > p->end_)
		png_error(png_ptr, "reading past the end of PNG");
	else
	{
		memcpy(data, p->data_, length);
		p->data_ += length;
	}
}

} // namespace


bool LoadPingFromRsrc(LPCTSTR resource_id, Dib& bmp)
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
	PingImg img;
	img.end_ = img.data_ = reinterpret_cast<BYTE*>(::LockResource(global));
	if (img.data_ == NULL)
		return false;

	img.end_ += ::SizeofResource(inst, rsrc);

	png_structp png= png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, &ErrorFunc, &WarnFunc);
	png_infop info= 0;

	if (png == 0)
		return false;

	bool ok= true;

	try
	{
		// Allocate/initialize the memory for image information
		info = png_create_info_struct(png);
		if (info == 0)
		{
			png_destroy_read_struct(&png, nullptr, nullptr);
			return false;
		}

		png_set_read_fn(png, &img, pngtest_read_data);

		//if (rgb_backgnd_ != ~0u)
		//{
		//	png_color_16 background= { 0, GetRValue(rgb_backgnd_), GetGValue(rgb_backgnd_), GetBValue(rgb_backgnd_), 0 };
		//	png_set_background(png, &background, PNG_BACKGROUND_GAMMA_SCREEN, 0, 1.0);
		//}

		png_read_png(png, info, PNG_TRANSFORM_BGR | PNG_TRANSFORM_STRIP_16, nullptr);

//		if (rgb_backgnd_ == ~0u)
		{
			// the only formats supported here:
			ASSERT(png_get_color_type(png, info) == PNG_COLOR_TYPE_RGB_ALPHA || png_get_color_type(png, info) == PNG_COLOR_TYPE_RGB);
		}
		//else
		//{
		//	ASSERT(info->color_type == PNG_COLOR_TYPE_RGB);
		//}
	}
	catch (PNGReaderException&)
	{
		ok = false;
	}

	if (ok)
	{
		unsigned int img_width= png_get_image_width(png, info);
		unsigned int img_height= png_get_image_height(png, info);

		bmp.Create(img_width, img_height, png_get_color_type(png, info) == PNG_COLOR_TYPE_RGB_ALPHA ? 32 : 24);

		BYTE* data= bmp.GetBuffer();

		if (data)
		{
			png_byte** row_pointers= png_get_rows(png, info);

			if (row_pointers != 0 && png_get_valid(png, info, PNG_INFO_IDAT))
			{
				size_t row_bytes= png_get_rowbytes(png, info);
				// bm.bmWidthBytes lies on W2K; on XP it's fine
				//int width= bm.bmWidthBytes;
				int chn= png_get_channels(png, info);
				int width= ((chn * img_width) + 3) & ~3;

				BYTE* p= data;

				int height= static_cast<int>(img_height);

				for (int i= height - 1; i >= 0; --i, p+= width)
					memcpy(p, row_pointers[i], row_bytes);
			}
		}
	}

	png_destroy_read_struct(&png, &info, nullptr);

	::UnlockResource(global);
	::FreeResource(global);

	return ok;
}

/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "DirectRender.h"
#include "Dib.h"
#include <d2d1.h>
#include <comdef.h>
#include "Exception.h"
#include <boost/scoped_array.hpp>

_COM_SMARTPTR_TYPEDEF(ID2D1Factory, __uuidof(ID2D1Factory));
_COM_SMARTPTR_TYPEDEF(ID2D1HwndRenderTarget, __uuidof(ID2D1HwndRenderTarget));
_COM_SMARTPTR_TYPEDEF(ID2D1Bitmap, __uuidof(ID2D1Bitmap));
_COM_SMARTPTR_TYPEDEF(ID2D1BitmapRenderTarget, __uuidof(ID2D1BitmapRenderTarget));


struct DirectRender::Impl
{
	Impl(HWND hwnd)
	{
		hwnd_ = hwnd;

		HRESULT hr= D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &d2d_factory_);
		if (FAILED(hr) || d2d_factory_ == 0)
		{
			_com_error err(hr);
			THROW_EXCEPTION(L"Direct2D not supported", SF(L"Cannot create Direct2D interface\nError: " << err.ErrorMessage()));
		}
	}

	HRESULT CreateDeviceResources(HWND hwnd, const Dib& dib, COLORREF backgnd, bool copy_content);

	void DiscardDeviceResources()
	{
		render_target_ = nullptr;
		bitmap_ = nullptr;
		copy_ = nullptr;
	}

	bool IsBitmapReady(bool primary_bitmap) const
	{
		if (render_target_ == nullptr)
			return false;
		if (primary_bitmap && bitmap_ != nullptr)
			return true;
		if (!primary_bitmap && copy_ != nullptr)
			return true;
		return false;
	}

	HRESULT AdjustBitmap(float scale, CSize new_size, float rotate);

	HRESULT Render(ID2D1BitmapPtr bmp, CPoint offset, float image_scale_x, float image_scale_y, float rotation_angle, COLORREF backgnd, float opacity);

	HWND hwnd_;
	ID2D1FactoryPtr d2d_factory_;
	ID2D1HwndRenderTargetPtr render_target_;
	ID2D1BitmapPtr bitmap_;
	ID2D1BitmapPtr copy_;
};


D2D1_PIXEL_FORMAT GetPixelFmt()
{
	return D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE);
}


// rescale the bitmap_ if new_scale is != 0, or just resize it to 'new_size'
HRESULT DirectRender::Impl::AdjustBitmap(float new_scale, CSize new_size, float rotate)
{
	if (bitmap_ == 0 || render_target_ == 0)
		return S_FALSE;

	D2D1_SIZE_U old_size= bitmap_->GetPixelSize();
	if (old_size.width == 0 || old_size.height == 0)
		return S_FALSE;

	D2D1_SIZE_U size;
	size.width = new_size.cx;
	size.height = new_size.cy;

	UINT32 max_size= render_target_->GetMaximumBitmapSize();

	if (size.width > max_size || size.height > max_size)
		return E_INVALIDARG;

	ID2D1BitmapRenderTargetPtr bmp_render;
	D2D1_PIXEL_FORMAT fmt= GetPixelFmt();
	D2D1_COMPATIBLE_RENDER_TARGET_OPTIONS options= D2D1_COMPATIBLE_RENDER_TARGET_OPTIONS_NONE;
	HRESULT hr= render_target_->CreateCompatibleRenderTarget(nullptr, &size, &fmt, options, &bmp_render);
	if (FAILED(hr) || bmp_render == nullptr)
		return hr;

	bmp_render->BeginDraw();

	D2D1_SIZE_F scale;
	if (new_scale != 0.0f)
		scale.width = scale.height = new_scale;
	else
	{
		scale.width = static_cast<float>(new_size.cx) / old_size.width;
		scale.height = static_cast<float>(new_size.cy) / old_size.height; //static_cast<float>(times);
	}

	D2D1_SIZE_F s= bmp_render->GetSize();
	D2D1_POINT_2F center;
	center.x = s.width / 2;
	center.y = s.height / 2;

	D2D1_SIZE_F r_size= bitmap_->GetSize();
	D2D1_POINT_2F center2;
	center2.x = r_size.width / 2;
	center2.y = r_size.height / 2;

	// shift to (0, 0), scale, and rotate, then move back to the center of destination rect (view)
	D2D1::Matrix3x2F transform= D2D1::Matrix3x2F::Translation(-center2.x, -center2.y) * D2D1::Matrix3x2F::Scale(scale) * D2D1::Matrix3x2F::Rotation(rotate) * D2D1::Matrix3x2F::Translation(center.x, center.y);

	bmp_render->SetTransform(transform);

	bmp_render->DrawBitmap(bitmap_);

	hr = bmp_render->EndDraw();
	if (FAILED(hr))
		return hr;

	ID2D1BitmapPtr bmp;
	hr = bmp_render->GetBitmap(&bmp);
	if (FAILED(hr))
		return hr;

	bitmap_ = bmp;
	return S_OK;
}


bool DirectRender::RescaleBitmap(CSize new_size)
{
	HRESULT hr= impl_.AdjustBitmap(0.0f, new_size, 0.0f);
	return hr == S_OK;
}

//bool DirectRender::RescaleBitmap(int times)
//{
//	HRESULT hr= impl_.RescaleBitmap(times);
//	return hr == S_OK;
//}


HRESULT CopyIntoBitmap(const Dib& dib, CRect src_rect, ID2D1BitmapPtr bitmap)
{
	if (bitmap == 0)
		return S_FALSE;

	const int w= dib.GetWidth();
	const int h= dib.GetHeight();

    D2D1_SIZE_U size= bitmap->GetPixelSize();

	if (size.width != w || size.height != h)
	{
		ASSERT(false);
		return E_INVALIDARG;
	}

	int sw= src_rect.Width();
	int sh= src_rect.Height();

	if (sw > w || sh > h || sw < 0 || sh < 0)
	{
		ASSERT(false);
		return E_INVALIDARG;
	}

	if (sw == 0 || sh == 0)
		return S_OK;

	D2D1_RECT_U dest_rect;
	dest_rect.left = src_rect.left;
	dest_rect.top = src_rect.top;
	dest_rect.right = src_rect.right;
	dest_rect.bottom = src_rect.bottom;

//LARGE_INTEGER time, tm[5], frq;
//::QueryPerformanceCounter(&time);

	// BGR to BGRX
	int pitch= sw * 4;

	boost::scoped_array<BYTE> buffer(new BYTE[pitch * sh]);
	BYTE* dest= buffer.get();

	for (int y= src_rect.top; y < src_rect.bottom; ++y)
	{
		const BYTE* p= dib.LinePixelBuffer(y, src_rect.left);
		for (int x= src_rect.left; x < src_rect.right; ++x)
		{
			dest[0] = p[0];
			dest[1] = p[1];
			dest[2] = p[2];
			dest[3] = 0;
			dest += 4;
			p += 3;
		}
	}

//::QueryPerformanceCounter(&tm[0]);

	HRESULT hr= bitmap->CopyFromMemory(&dest_rect, buffer.get(), pitch);
//::QueryPerformanceCounter(&tm[1]);

//::QueryPerformanceCounter(&tm[2]);
//::QueryPerformanceFrequency(&frq);
//
//tm[2].QuadPart -= tm[1].QuadPart;
//tm[1].QuadPart -= tm[0].QuadPart;
//tm[0].QuadPart -= time.QuadPart;

//wchar_t buf[50];
//::OutputDebugString(_itow(tm[0].QuadPart * 1000 / frq.QuadPart, buf, 10));
//::OutputDebugString(L"\n");
//
//::OutputDebugString(_itow(tm[1].QuadPart * 1000 / frq.QuadPart, buf, 10));
//::OutputDebugString(L"\n");
//
//::OutputDebugString(_itow(tm[2].QuadPart * 1000 / frq.QuadPart, buf, 10));
//::OutputDebugString(L"\n");
	return hr;
}


HRESULT DirectRender::Impl::CreateDeviceResources(HWND hwnd, const Dib& dib, COLORREF backgnd, bool copy_content)
{
	HRESULT hr= S_OK;

	CRect rect(0,0,0,0);
	GetClientRect(hwnd, &rect);

	if (render_target_ == 0)
	{
		D2D1_SIZE_U size= D2D1::SizeU(rect.Width(), rect.Height());

		// Create a Direct2D render target
		hr = d2d_factory_->CreateHwndRenderTarget(
			D2D1::RenderTargetProperties(), D2D1::HwndRenderTargetProperties(hwnd, size), &render_target_);

		if (FAILED(hr))
			return hr;
	}

	UINT32 max_size= render_target_->GetMaximumBitmapSize();

	D2D1_SIZE_U img_size= D2D1::SizeU(dib.GetWidth(), dib.GetHeight());
	if (img_size.width > max_size || img_size.height > max_size)
		return E_INVALIDARG;	// TODO: better code?

	D2D1_BITMAP_PROPERTIES prop;
	prop.dpiX = prop.dpiY = 96.0f;
	prop.pixelFormat = GetPixelFmt();

	boost::scoped_array<uint32> buffer;

	const int w= dib.GetWidth();
	const int h= dib.GetHeight();
	const int pitch= w * 4;

	if (w == 0 || h == 0)
	{
		ASSERT(false);
		return S_FALSE;
	}

	if (copy_content)
	{
		// convert BGR to BGRX

//LARGE_INTEGER time, tm[5], frq;
//::QueryPerformanceCounter(&time);

		buffer.reset(new uint32[pitch * h / sizeof(uint32)]);
		uint32* dest= buffer.get();

//::QueryPerformanceCounter(&tm[0]);

		for (int y= 0; y < h; ++y)
		{
			const BYTE* p= dib.LineBuffer(y);
			for (int x= 0; x < w; ++x)
			{
				// even though lines are DWORD padded, last pixel read may cause illegal mem access;
				// this is 20% improvement over byte by byte copy
//				*dest++ = *reinterpret_cast<const uint32*>(p);	// p may be misaligned for 4B read
//				p += 3;

				BYTE* d= reinterpret_cast<BYTE*>(dest);

				d[0] = p[0];
				d[1] = p[1];
				d[2] = p[2];

				dest += 1;
				p += 3;
			}
		}
	}
//::QueryPerformanceCounter(&tm[1]);

	if (bitmap_ != 0)
	{
		D2D1_SIZE_U size= bitmap_->GetPixelSize();
		if (size.width != img_size.width || size.height != img_size.height)
			bitmap_ = 0;	// recreate
		else if (buffer.get() != 0)
			hr = bitmap_->CopyFromMemory(nullptr, buffer.get(), pitch);
	}

	if (bitmap_ == 0 || FAILED(hr))
		hr = render_target_->CreateBitmap(img_size, buffer.get(), pitch, prop, &bitmap_);

	if (hr == S_OK && bitmap_ != 0 && !copy_content)
	{
		// clear the bitmap using background color
		backgnd;

		const size_t count= pitch / sizeof(uint32);
		boost::scoped_array<uint32> back(new uint32[count]);

		uint32 c= backgnd;

		for (size_t i= 0; i < count; ++i)
			back[i] = c;

		D2D1_RECT_U dest_rect;
		dest_rect.left = 0;
		dest_rect.right = img_size.width;
		dest_rect.top = dest_rect.bottom = 0;

		for (UINT32 y= 0; y < img_size.height; ++y)
		{
			dest_rect.top = y;
			dest_rect.bottom = y + 1;
			bitmap_->CopyFromMemory(&dest_rect, back.get(), pitch);
		}
	}

//::QueryPerformanceCounter(&tm[2]);
//::QueryPerformanceFrequency(&frq);
//
//tm[2].QuadPart -= tm[1].QuadPart;
//tm[1].QuadPart -= tm[0].QuadPart;
//tm[0].QuadPart -= time.QuadPart;
//
//wchar_t buf[50];
//::OutputDebugString(_itow(tm[0].QuadPart * 1000 / frq.QuadPart, buf, 10));
//::OutputDebugString(L"\n");
//
//::OutputDebugString(_itow(tm[1].QuadPart * 1000 / frq.QuadPart, buf, 10));
//::OutputDebugString(L"\n");
//
//::OutputDebugString(_itow(tm[2].QuadPart * 1000 / frq.QuadPart, buf, 10));
//::OutputDebugString(L"\n");

/*
	IWICBitmapPtr bmp;
	hr = wic_factory_->CreateBitmapFromHBITMAP((HBITMAP)dib.GetBmp()->m_hObject, 0, WICBitmapIgnoreAlpha, &bmp);

	if (hr == S_OK)
	{
		IWICFormatConverterPtr converter;
		hr = wic_factory_->CreateFormatConverter(&converter);
		if (SUCCEEDED(hr))
		{
			hr = converter->Initialize(bmp, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone,
				NULL, 0.f, WICBitmapPaletteTypeMedianCut);

			if (SUCCEEDED(hr))
			{
				// Create a Direct2D bitmap from the WIC bitmap.
				hr = render_target_->CreateBitmapFromWicBitmap(converter, 0, &bitmap_);
			}
		}
		//hr = render_target_->CreateBitmap(img_size, dib.GetBuffer(), dib.GetLineBytes(), prop, &bitmap_);
	}
*/
	return hr;
}


DirectRender::DirectRender(HWND hwnd) : impl_(*new Impl(hwnd))
{
}


DirectRender::~DirectRender()
{
	delete &impl_;
}


void DrawImage(ID2D1HwndRenderTarget* target, ID2D1Bitmap* bitmap, D2D1_SIZE_F scale, float rotate, float opacity, D2D1_SIZE_F img_offset)
{
	//D2D1_SIZE_F render_size= target->GetSize();
	D2D1_SIZE_F size= bitmap->GetSize();

	// center bmp
	//D2D1_SIZE_F offset;
	//D2D1_SIZE_F scaled_img;
	//scaled_img.width = scale.width * size.width;
	//scaled_img.height = scale.height * size.height;

	//offset.width = (render_size.width - scaled_img.width) / 2.0f;
	//offset.height = (render_size.height - scaled_img.height) / 2.0f;

	//if (offset.width < 0.0f)
	//	offset.width = 0.0f;
	//if (offset.height < 0.0f)
	//	offset.height = 0.0f;

	//if (scaled_img.width > render_size.width)
	//	offset.width = -std::min(std::max(0.0f, img_offset.width), scaled_img.width - render_size.width);
	//if (scaled_img.height > render_size.height)
	//	offset.height = -std::min(std::max(0.0f, img_offset.height), scaled_img.height - render_size.height);
//	img_offset.height = -img_offset.height;
//	img_offset.width = -img_offset.width;

//	D2D1::Matrix3x2F transform= D2D1::Matrix3x2F::Scale(scale) * D2D1::Matrix3x2F::Translation(img_offset);

	D2D1_SIZE_F s= target->GetSize();
	D2D1_POINT_2F center;
	center.x = s.width / 2;
	center.y = s.height / 2;

	D2D1_SIZE_F r_size= bitmap->GetSize();
	D2D1_POINT_2F center2;
	center2.x = r_size.width / 2;
	center2.y = r_size.height / 2;

	// shift to (0, 0), scale, and rotate, then move back to the center of destination rect (view)
	D2D1::Matrix3x2F transform= rotate != 0.0f ?
		D2D1::Matrix3x2F::Translation(-center2.x, -center2.y) * D2D1::Matrix3x2F::Scale(scale) * D2D1::Matrix3x2F::Rotation(rotate) * D2D1::Matrix3x2F::Translation(center.x, center.y)
		: D2D1::Matrix3x2F::Scale(scale) * D2D1::Matrix3x2F::Translation(img_offset);

	target->SetTransform(transform);

	target->DrawBitmap(bitmap, nullptr /* D2D1::RectF(0.0f, 0.0f, size.width, size.height)*/, opacity);
}


HRESULT DirectRender::Render(CPoint offset, float image_scale_x, float image_scale_y, COLORREF backgnd, float opacity)
{
	if (!IsBitmapReady())
		return S_FALSE;

	return impl_.Render(impl_.bitmap_, offset, image_scale_x, image_scale_y, 0.0f, backgnd, opacity);
}


HRESULT DirectRender::Impl::Render(ID2D1BitmapPtr bmp, CPoint offset, float image_scale_x, float image_scale_y, float rotation_angle, COLORREF backgnd, float opacity)
{
	if (render_target_->CheckWindowState() & D2D1_WINDOW_STATE_OCCLUDED)
		return S_FALSE;

	render_target_->BeginDraw();

	render_target_->SetTransform(D2D1::Matrix3x2F::Identity());

	D2D1::ColorF back(GetRValue(backgnd) / 255.0f, GetGValue(backgnd) / 255.0f, GetBValue(backgnd) / 255.0f);

	render_target_->Clear(back);

	D2D1_SIZE_F scale;
	scale.width = image_scale_x;
	scale.height = image_scale_y;

	D2D1_SIZE_F img_offset= { static_cast<float>(offset.x), static_cast<float>(offset.y) };

	DrawImage(render_target_, bmp, scale, rotation_angle, opacity, img_offset);

	HRESULT hr= render_target_->EndDraw();

	if (hr == D2DERR_RECREATE_TARGET)
	{
		hr = S_FALSE;
		DiscardDeviceResources();
	}

	return hr;
}


HRESULT DirectRender::RenderCopy(CPoint offset, float image_scale_x, float image_scale_y, float rotation_angle, COLORREF backgnd, float opacity)
{
	if (!IsBitmapCopyReady())
		return S_FALSE;

	return impl_.Render(impl_.copy_, offset, image_scale_x, image_scale_y, rotation_angle, backgnd, opacity);
}


void DirectRender::Resize(int width, int height)
{
	if (impl_.render_target_ == 0)
		return;

	D2D1_SIZE_U size;
	size.width = width;
	size.height = height;

	// This method can fail, but it's OK to ignore the error here -- it will be repeated on the next call to EndDraw
	impl_.render_target_->Resize(size);
}


bool DirectRender::IsBitmapReady() const
{
	return impl_.IsBitmapReady(true);
}


bool DirectRender::IsBitmapCopyReady() const
{
	return impl_.IsBitmapReady(false);
}


bool DirectRender::PrepareBitmap(const Dib& dib, COLORREF backgnd, bool copy_content)
{
	HRESULT hr= impl_.CreateDeviceResources(impl_.hwnd_, dib, backgnd, copy_content);
	return SUCCEEDED(hr);
}


bool DirectRender::CopyIntoBitmap(const Dib& dib, CRect src_rect)
{
	HRESULT hr= ::CopyIntoBitmap(dib, src_rect, impl_.bitmap_);
	return hr == S_OK;
}


CSize DirectRender::GetBitmapSize() const
{
	if (impl_.bitmap_)
	{
		D2D1_SIZE_U size= impl_.bitmap_->GetPixelSize();
		return CSize(static_cast<long>(size.width), static_cast<long>(size.height));
	}
	else
		return CSize(0, 0);
}


bool DirectRender::RotateBitmap(bool clockwise)
{
	CSize size= GetBitmapSize();
	std::swap(size.cx, size.cy);
	HRESULT hr= impl_.AdjustBitmap(1.0f, size, clockwise ? 90.0f : -90.0f);
	return hr == S_OK;
}


void DirectRender::DeleteBitmap()
{
	impl_.bitmap_ = nullptr;
}


void DirectRender::KeepCopy()
{
	impl_.copy_ = impl_.bitmap_;
}


void DirectRender::ReleaseCopy()
{
	impl_.copy_ = nullptr;
}

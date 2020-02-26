/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"

#ifdef USE_DIRECT2D

#include "Direct2D.h"
#include <boost/ptr_container/ptr_map.hpp>



CComPtr<ID2D1Factory> D2DFactory;


HRESULT CreateDeviceIndependentResources()
{
	HRESULT hr= ::D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &D2DFactory);

	return S_OK;
}


namespace gr
{

const details::RECT_F& Cast(const RectF& rect)
{
	return *reinterpret_cast<const details::RECT_F*>(&rect);
}

const details::COLOR_F& Cast(const Color& color)
{
	return color;
//	return *reinterpret_cast<const details::COLOR_F*>(&color);
}

const details::MATRIX_3X2_F& Cast(const Matrix& m)
{
	return m;
//	return *reinterpret_cast<const details::MATRIX_3X2_F*>(&m);
}


Color SystemColor(SystemColors c)
{
	COLORREF color= ::GetSysColor(static_cast<int>(c));
	return Color(color);
}

// ========================================== brushes ==========================================

void impl::BrushProxy::SetOpacity(float opacity)
{
	brush_->SetOpacity(opacity);
}

float impl::BrushProxy::GetOpacity() const
{
	return brush_->GetOpacity();
}

void impl::BrushProxy::SetTransform(const Matrix& transform)
{
	return brush_->SetTransform(&Cast(transform));
}

Matrix impl::BrushProxy::GetTransform() const
{
	details::MATRIX_3X2_F m;
	brush_->GetTransform(&m);
	return Matrix(m);
}

void impl::SolidBrushProxy::SetColor(const Color& color)
{
	brush_->SetColor(&color);
}

Color impl::SolidBrushProxy::GetColor() const
{
	return Color(brush_->GetColor());
}

// ========================================== graphics ==========================================

bool CheckHR(HRESULT hr)
{
	if (FAILED(hr))
		throw Exception(hr);

	return true;
}

struct impl::GraphicsProxy::Impl
{
	Impl()
	{
		target_resources_valid_ = false;
	}

	details::RenderTargetPtr target_;
//	boost::ptr_map<IUnknown*, Brush> brushes_;
	details::SolidColorBrushPtr solid_color_brush_;
	bool target_resources_valid_;

	details::SolidColorBrush* get_brush(const Color& color)
	{
		if (solid_color_brush_ == 0)
			CheckHR(target_->CreateSolidColorBrush(&Cast(color), 0, &solid_color_brush_));
		else
			solid_color_brush_->SetColor(color);

		return solid_color_brush_;
	}
};


static details::RenderTargetPtr ConstructWndGraphics(HWND hwnd)
{
	RECT rect;
	::SetRectEmpty(&rect);
	::GetClientRect(hwnd, &rect);

	details::SIZE_U size;
	size.width = rect.right - rect.left;
	size.height = rect.bottom - rect.top;

	details::HwndRenderTargetPtr target;

	CheckHR(D2DFactory->CreateHwndRenderTarget(
		D2D1::RenderTargetProperties(),
		D2D1::HwndRenderTargetProperties(hwnd, size),
		&target));

	return details::RenderTargetPtr(target);
}


impl::WndGraphicsProxy::WndGraphicsProxy(HWND hwnd) : GraphicsProxy(ConstructWndGraphics(hwnd))
{
	wnd_target_ = GetTarget();
}


static details::RenderTargetPtr ConstructBitmapGraphics(const SizeF* desired_size, const SizeU* pixel_size, int* pixel_format, impl::BitmapGraphicsProxy::Options* flags)
{
	details::BitmapRenderTargetPtr target;

//    CheckHR(D2DFactory->CreateCompatibleRenderTarget(

	return details::RenderTargetPtr(target);
}

impl::BitmapGraphicsProxy::BitmapGraphicsProxy() : GraphicsProxy(ConstructBitmapGraphics(0, 0, 0, 0))
{
	bmp_target_ = GetTarget();
}

impl::BitmapGraphicsProxy::BitmapGraphicsProxy(const SizeF& desired_size) : GraphicsProxy(ConstructBitmapGraphics(&desired_size, 0, 0, 0))
{
	bmp_target_ = GetTarget();
}

impl::BitmapGraphicsProxy::BitmapGraphicsProxy(const SizeF& desired_size, const SizeU& pixel_size) : GraphicsProxy(ConstructBitmapGraphics(&desired_size, &pixel_size, 0, 0))
{
	bmp_target_ = GetTarget();
}

//impl::BitmapGraphicsProxy::BitmapGraphicsProxy(const SizeF& desired_size, const SizeU& pixel_size, int pixel_format)
//{
//	bmp_target_ = GetTarget();
//}

//impl::BitmapGraphicsProxy::BitmapGraphicsProxy(const SizeF& desired_size, const SizeU& pixel_size, int pixel_format, Options flags)
//{
//	bmp_target_ = GetTarget();
//}

static details::RenderTargetPtr ConstructDCGraphics(HDC hdc, const RECT* rect, bool use_alpha)
{
	D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
		D2D1_RENDER_TARGET_TYPE_DEFAULT,
		D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, use_alpha ? D2D1_ALPHA_MODE_PREMULTIPLIED : D2D1_ALPHA_MODE_IGNORE),
		0, 0, D2D1_RENDER_TARGET_USAGE_NONE, D2D1_FEATURE_LEVEL_DEFAULT);

	details::DCRenderTargetPtr dc_target;
	CheckHR(D2DFactory->CreateDCRenderTarget(&props, &dc_target));

	return details::RenderTargetPtr(dc_target);
}


impl::DCGraphicsProxy::DCGraphicsProxy(bool use_alpha) : GraphicsProxy(ConstructDCGraphics(0, 0, use_alpha))
{
	dc_target_ = GetTarget();
}

void impl::DCGraphicsProxy::BindDC(HDC hdc, const RECT& rect)
{
	ASSERT(hdc);
	dc_target_->BindDC(hdc, &rect);
}

void impl::DCGraphicsProxy::BindDC(HDC hdc)
{
	ASSERT(hdc);
	if (HWND hwnd= ::WindowFromDC(hdc))
	{
		RECT rect;
		::GetClientRect(hwnd, &rect);
		BindDC(hdc, rect);
	}
	else
	{
		ASSERT(false);
	}
}


impl::GraphicsProxy::GraphicsProxy(details::RenderTargetPtr target) : ResourceProxy(details::ResourcePtr(target))
{
}

details::RenderTargetPtr impl::GraphicsProxy::GetTarget() const
{
	return impl_->target_;
}

void impl::GraphicsProxy::DrawRectangle(const RectF& rect, Brush brush)
{
	impl_->target_->DrawRectangle(Cast(rect), brush->handle());

//        CONST D2D1_RECT_F &rect,
//        __in ID2D1Brush *brush,
//        FLOAT strokeWidth = 1.0f,
//        __in_opt ID2D1StrokeStyle *strokeStyle = NULL 
}


void impl::GraphicsProxy::FillRectangle(const RectF& rect, Brush brush)
{
	impl_->target_->FillRectangle(&Cast(rect), brush->handle());
}


void impl::GraphicsProxy::FillRectangle(float x, float y, float width, float height, Brush brush)
{
	RectF rect(x, y, x + width, y + height);
	impl_->target_->FillRectangle(&Cast(rect), brush->handle());
}


void impl::GraphicsProxy::FillRectangle(const RectF& rect, const Color& color)
{
	if (details::SolidColorBrush* brush= impl_->get_brush(color))
		impl_->target_->FillRectangle(&Cast(rect), brush);
}


void impl::GraphicsProxy::FillRoundedRectangle(const RectF& rect, const SizeF& radii, Brush brush)
{
	details::ROUNDED_RECT rr;
    rr.rect = Cast(rect);
    rr.radiusX = radii.cx;
    rr.radiusY = radii.cy;
	impl_->target_->FillRoundedRectangle(&rr, brush->handle());
}


SolidBrush impl::GraphicsProxy::CreateBrush(const Color& color, float opacity, const Matrix& transform)
{
	details::SolidColorBrushPtr brush;
	details::BRUSH_PROPERTIES prop;
	prop.opacity = opacity;
	prop.transform = Cast(transform);

	if (CheckHR(impl_->target_->CreateSolidColorBrush(&Cast(color), &prop, &brush)))
		return SolidBrush(new impl::SolidBrushProxy(brush));

	return 0;
}


SolidBrush impl::GraphicsProxy::CreateBrush(const Color& color)
{
	details::SolidColorBrushPtr brush;
	if (CheckHR(impl_->target_->CreateSolidColorBrush(&Cast(color), 0, &brush)))
	{
		SolidBrush b(new impl::SolidBrushProxy(brush));
//		IUnknown* key= brush;
//		impl_->brushes_.insert(key, b);
		return b;
	}

	return 0;
}


} // namespace gr


#endif

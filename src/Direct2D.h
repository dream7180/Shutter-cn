/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once

#include <d2d1.h>
#include <d2d1Helper.h>

#include "graphics/base-types.h"
#include <boost/scoped_ptr.hpp>
//#include <boost/shared_ptr.hpp>
#include <boost/intrusive_ptr.hpp>

#define ASSERT_STRUCT_SIZE_2(predicate, line) typedef char constraint_violated_on_line_##line[2*((predicate)!=0)-1]
#define ASSERT_STRUCT_SIZE(predicate) ASSERT_STRUCT_SIZE_2(predicate, __LINE__)


namespace gr
{
	namespace details
	{
		typedef ID2D1Resource Resource;
		typedef ID2D1Brush Brush;
		typedef ID2D1SolidColorBrush SolidColorBrush;
		typedef ID2D1LinearGradientBrush LinearGradientBrush;
		typedef D2D1_COLOR_F COLOR_F;
		typedef D2D1_RECT_F RECT_F;
		typedef D2D1_POINT_2F POINT_2F;
		typedef D2D1_SIZE_F SIZE_F;
		typedef D2D1_SIZE_U SIZE_U;
		typedef D2D1_BRUSH_PROPERTIES BRUSH_PROPERTIES;
		typedef D2D1_MATRIX_3X2_F MATRIX_3X2_F;
		typedef D2D1_ROUNDED_RECT ROUNDED_RECT;
		typedef ID2D1Geometry Geometry;
		typedef ID2D1RenderTarget RenderTarget;
		typedef ID2D1HwndRenderTarget HwndRenderTarget;
		typedef ID2D1BitmapRenderTarget BitmapRenderTarget;
		typedef ID2D1DCRenderTarget DCRenderTarget;
		typedef ID2D1Bitmap Bitmap;

		template <class T>
		struct traits
		{
			typedef CComPtr<T> ComPtr;
		};

		typedef details::traits<details::Resource>::ComPtr ResourcePtr;

		typedef CComPtr<Brush> BrushPtr;
		typedef CComPtr<SolidColorBrush> SolidColorBrushPtr;
		typedef CComPtr<LinearGradientBrush> LinearGradientBrushPtr;
		typedef CComPtr<RenderTarget> RenderTargetPtr;
		typedef CComPtr<HwndRenderTarget> HwndRenderTargetPtr;
		typedef CComPtr<DCRenderTarget> DCRenderTargetPtr;
		typedef CComPtr<BitmapRenderTarget> BitmapRenderTargetPtr;
		typedef CComPtr<Geometry> GeometryPtr;
		typedef CComPtr<Bitmap> BitmapPtr;
	}

	namespace helpers = D2D1;	// helpers from d2d1Helper.h

	typedef types::Rect<float> RectF;
	typedef types::Rect<unsigned int> RectU;
	typedef types::Size<float> SizeF;
	typedef types::Size<unsigned int> SizeU;
	typedef types::Point<float> PointF;
	typedef types::Point<unsigned int> PointU;
//	typedef details::MATRIX_3X2_F Matrix;

	class Exception : public std::exception
	{
	public:
		Exception(HRESULT hr) : hr_(hr)
		{
			_itoa_s(hr_, buf_, BUF, 16);
		}

		HRESULT Result() const
		{
			return hr_;
		}

		virtual const char* what() const
		{
			return buf_;
		}

	private:
		enum { BUF= 20 };
		char buf_[BUF];
		HRESULT hr_;
	};


	//struct RoundRectF
	//{
	//};

	struct Matrix : public helpers::Matrix3x2F
	{
		Matrix()
		{
			// identity
			_11 = 1.0f; _12 = 0.0f;
			_21 = 0.0f; _22 = 1.0f;
			_31 = 0.0f; _32 = 0.0f;
		}

		Matrix(const details::MATRIX_3X2_F& m)
		{
			_11 = m._11; _12 = m._12;
			_21 = m._21; _22 = m._22;
			_31 = m._31; _32 = m._32;
		}

		//float _11;
		//float _12;
		//float _21;
		//float _22;
		//float _31;
		//float _32;
	};

	enum SystemColors
	{
		Color_Scrollbar					= COLOR_SCROLLBAR,
		Color_Background				= COLOR_BACKGROUND,
		Color_ActiveCaption				= COLOR_ACTIVECAPTION,
		Color_InactiveCaption			= COLOR_INACTIVECAPTION,
		Color_Menu						= COLOR_MENU,
		Color_Window					= COLOR_WINDOW,
		Color_WindowFrame				= COLOR_WINDOWFRAME,
		Color_Menutext					= COLOR_MENUTEXT,
		Color_WindowText				= COLOR_WINDOWTEXT,
		Color_CaptionText				= COLOR_CAPTIONTEXT,
		Color_ActiveBorder				= COLOR_ACTIVEBORDER,
		Color_InactiveBorder			= COLOR_INACTIVEBORDER,
		Color_AppWorkspace				= COLOR_APPWORKSPACE,
		Color_Highlight					= COLOR_HIGHLIGHT,
		Color_HighlightText				= COLOR_HIGHLIGHTTEXT,
		Color_BtnFace					= COLOR_BTNFACE,
		Color_BtnShadow					= COLOR_BTNSHADOW,
		Color_GrayText					= COLOR_GRAYTEXT,
		Color_BtnText					= COLOR_BTNTEXT,
		Color_InactiveCaptionText		= COLOR_INACTIVECAPTIONTEXT,
		Color_BtnHighlight				= COLOR_BTNHIGHLIGHT,
		Color_3dDkShadow				= COLOR_3DDKSHADOW,
		Color_3dLight					= COLOR_3DLIGHT,
		Color_InfoText					= COLOR_INFOTEXT,
		Color_InfoBk					= COLOR_INFOBK,
		Color_Hotlight					= COLOR_HOTLIGHT,
		Color_GradientActiveCaption		= COLOR_GRADIENTACTIVECAPTION,
		Color_GradientInactiveCaption	= COLOR_GRADIENTINACTIVECAPTION,
		Color_MenuHilight				= COLOR_MENUHILIGHT,
		Color_MenuBar					= COLOR_MENUBAR,
		Color_Desktop					= COLOR_DESKTOP,
		Color_3dFace					= COLOR_3DFACE,
		Color_3dShadow					= COLOR_3DSHADOW,
		Color_3dHighlight				= COLOR_3DHIGHLIGHT,
		Color_3dHilight					= COLOR_3DHILIGHT,
		Color_BtnHilight				= COLOR_BTNHILIGHT
	};

	struct Color : public details::COLOR_F
	{
		//float r;
		//float g;
		//float b;
		//float a;

		Color(const Color& base, float alpha)
		{
			r = base.r;
			g = base.g;
			b = base.b;
			a = alpha;
		}

		Color(float r, float g, float b)
		{
			this->r = r;
			this->g = g;
			this->b = b;
			this->a = 1.0f;
		}

		Color(float r, float g, float b, float a)
		{
			this->r = r;
			this->g = g;
			this->b = b;
			this->a = a;
		}

		Color(const details::COLOR_F& color) : details::COLOR_F(color)
		{}

		Color()
		{ r = g = b = a = 0.0f; }

		explicit Color(COLORREF c)
		{
			Assign(c);
		}

		Color(int alpha, COLORREF c)
		{
			Assign(alpha, c);
		}

		Color& operator = (COLORREF c)	{ Assign(c); return *this; }

		void Assign(COLORREF c)			{ Assign(0xff, c); }

		void Assign(int alpha, COLORREF c)
		{
			r = GetRValue(c) / 255.0f;
			g = GetGValue(c) / 255.0f;
			b = GetBValue(c) / 255.0f;
			a = alpha / 255.0f;
		}
	};

	Color SystemColor(SystemColors c);

	// verify struct sizes
//	ASSERT_STRUCT_SIZE(sizeof(Color) == sizeof(details::COLOR_F));
//	ASSERT_STRUCT_SIZE(sizeof(Matrix) == sizeof(details::MATRIX_3X2_F));
	ASSERT_STRUCT_SIZE(sizeof(RectF) == sizeof(details::RECT_F));
	ASSERT_STRUCT_SIZE(sizeof(PointF) == sizeof(details::POINT_2F));
	ASSERT_STRUCT_SIZE(sizeof(SizeF) == sizeof(details::SIZE_F));

	namespace impl
	{
		class GraphicsProxy;

		struct RefCounter
		{
			RefCounter() : count_(0)
			{}
			virtual ~RefCounter()
			{}

			void AddRef()	{ ++count_; }
			void Release()	{ if (--count_ == 0) delete this; }

			unsigned int count_;
		};

		class ResourceProxy : boost::noncopyable, RefCounter
		{
		public:
			//void, GetFactory)(

		protected:
			ResourceProxy(details::ResourcePtr rsc) : rsc_(rsc)
			{}
			ResourceProxy()
			{}
			void SetResourcePtr(details::ResourcePtr rsc)
			{
				rsc_ = rsc;
			}
		private:
			friend void intrusive_ptr_add_ref(ResourceProxy* b);
			friend void intrusive_ptr_release(ResourceProxy* b);

			details::ResourcePtr rsc_;
		};

		void intrusive_ptr_add_ref(ResourceProxy* b)	{ b->AddRef(); }
		void intrusive_ptr_release(ResourceProxy* b)	{ b->Release(); }

		class BrushProxy : public ResourceProxy
		{
		public:
			void SetOpacity(float opacity);
			float GetOpacity() const;

			void SetTransform(const Matrix& transform);
			Matrix GetTransform() const;

			BrushProxy(details::Brush* brush) : ResourceProxy(brush), brush_(brush)
			{}

		protected:
			virtual ~BrushProxy() {}
		private:
			friend class GraphicsProxy;

			details::BrushPtr brush_;
			unsigned int count_;
			virtual details::Brush* handle()	{ return brush_; }
		};

		class SolidBrushProxy : public BrushProxy
		{
		public:
			void SetColor(const Color& color);
			Color GetColor() const;

		private:
			friend class GraphicsProxy;

			SolidBrushProxy(details::SolidColorBrushPtr br) : BrushProxy(br), brush_(br)
			{}
			virtual details::SolidColorBrush* handle()	{ return brush_; }
			details::SolidColorBrushPtr brush_;
		};

		class GradientBrushProxy : public BrushProxy
		{
		public:

		private:
			friend class GraphicsProxy;

			GradientBrushProxy(details::LinearGradientBrushPtr br) : BrushProxy(br), brush_(br)
			{}
			virtual details::LinearGradientBrush* handle()	{ return brush_; }
			details::LinearGradientBrushPtr brush_;
		};

		class GeometryProxy
		{
		public:

		private:
			details::GeometryPtr geometry_;
		};

		class StrokeStyle
		{
		};

		class BitmapProxy
		{
		public:

		private:
			details::BitmapPtr bitmap_;
		};
	}

	typedef boost::intrusive_ptr<impl::BrushProxy> Brush;
	typedef boost::intrusive_ptr<impl::SolidBrushProxy> SolidBrush;
	typedef boost::intrusive_ptr<impl::GradientBrushProxy> GradientBrush;
	typedef boost::intrusive_ptr<impl::GeometryProxy> Geometry;
	typedef boost::intrusive_ptr<impl::BitmapProxy> Bitmap;

	namespace impl
	{

		class GraphicsProxy : public ResourceProxy
		{
		public:
			GraphicsProxy(HDC hdc);

			SolidBrush CreateBrush(const Color& color);
			SolidBrush CreateBrush(const Color& color, float opacity, const Matrix& transform);

			GradientBrush CreateBrush(const Color& from, const Color& to);

			void DrawRectangle(const RectF& rect, Brush brush);
			void DrawRectangle(const RectF& rect, Brush brush, float stroke_width= 1.0f);
			void DrawRectangle(const RectF& rect, Brush brush, float stroke_width, const StrokeStyle& stroke_style);

			void FillRectangle(const RectF& rect, Brush brush);
			void FillRectangle(float x, float y, float width, float height, Brush brush);
			void FillRectangle(const RectF& rect, const Color& color);

			void FillRoundedRectangle(const RectF& rect, const SizeF& radii, Brush brush);

			void FillGeometry(Geometry geometry, Brush brush);

			void Clear();
			void Clear(const Color& color);

			void BeginDraw();
			HRESULT EndDraw();

		protected:
			//void SetGraphicsPtr(details::GraphicsPtr rsc);
			GraphicsProxy(details::RenderTargetPtr target);
			details::RenderTargetPtr GetTarget() const;
		private:
			struct Impl;
			boost::scoped_ptr<Impl> impl_;

			GraphicsProxy(); // not default-constructable
		};


		class WndGraphicsProxy : public GraphicsProxy
		{
		public:
			WndGraphicsProxy(HWND hwnd);

			// State CheckWindowState() const;

			// Resize(const SizeU& size_in_pixels);

			// HWND GetHwnd()

		private:
			details::HwndRenderTargetPtr wnd_target_;
		};


		class DCGraphicsProxy : public GraphicsProxy
		{
		public:
			explicit DCGraphicsProxy(bool use_alpha);

			void BindDC(HDC hdc, const RECT& rect);
			void BindDC(HDC hdc); // bind DC using window's client rect

		private:
			details::DCRenderTargetPtr dc_target_;
		};


		class BitmapGraphicsProxy : public GraphicsProxy
		{
		public:
			enum Options { None, GdiCompatible };

			BitmapGraphicsProxy();
			BitmapGraphicsProxy(const SizeF& desired_size);
			BitmapGraphicsProxy(const SizeF& desired_size, const SizeU& pixel_size);
			BitmapGraphicsProxy(const SizeF& desired_size, const SizeU& pixel_size, int pixel_format);
			BitmapGraphicsProxy(const SizeF& desired_size, const SizeU& pixel_size, int pixel_format, Options flags);

			Bitmap GetBitmap() const;

		private:
			details::BitmapRenderTargetPtr bmp_target_;
		};

	} // namespace impl

	typedef boost::intrusive_ptr<impl::GraphicsProxy> Graphics;
	typedef boost::intrusive_ptr<impl::WndGraphicsProxy> WndGraphics;
	typedef boost::intrusive_ptr<impl::DCGraphicsProxy> DCGraphics;
	typedef boost::intrusive_ptr<impl::BitmapGraphicsProxy> BitmapGraphics;

	class BeginDraw
	{
	public:
		BeginDraw(Graphics graphics) : graphics_(graphics)
		{
			graphics_->BeginDraw();
		}

		HRESULT End()
		{
			if (graphics_ == 0)
				return S_FALSE;

			HRESULT hr= graphics_->EndDraw();
			graphics_.reset();
			return hr;
		}

		~BeginDraw()
		{
			if (graphics_)
				graphics_->EndDraw();
		}

	private:
		Graphics graphics_;
	};

} // namespace gr


#undef ASSERT_STRUCT_SIZE_2
#undef ASSERT_STRUCT_SIZE

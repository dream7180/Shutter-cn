/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// Dib.h: interface for the Dib class.

#pragma once

#include "AutoPtr.h"
#include "DibDispMethod.h"

#define USE_BITMAPV5HEADER	1


class Dib
{
public:
	Dib();
	Dib(int bmp_rsrc_id);
	Dib(int width, int height, int bits_per_pixel, int clear_val= 0xff);
//	Dib(int width, int height, int color_components);
	virtual ~Dib();

	void Create(int width, int height, int bits_per_pixel, int clear_val= 0xff);

	// special version that doesn't call CreateDIBSection()
	void CreateEx(int width, int height, int bits_per_pixel, bool alignment);

	bool Load(int bmp_rsrc_id);

	int GetWidth() const			{ return width_; }
	int GetHeight() const			{ return height_; }
	int GetLineBytes() const		{ return line_width_; }
	int GetBytesPerLine() const		{ return line_width_; }
	int GetColorComponents() const	{ return bits_per_pixel_ / 8; }
	int GetBitsPerPixel() const		{ return bits_per_pixel_; }
	CSize GetSize() const			{ return CSize(width_, height_); }
	const BYTE* GetBuffer() const	{ return data_buff_; }

	BYTE* GetBuffer()				{ return data_buff_; }
	size_t GetBufferSize() const	{ return line_width_ * height_; }

	BYTE* LineBuffer(int line)
	{ return static_cast<UINT>(line) < static_cast<UINT>(height_) ? data_buff_ + (height_ - line - 1) * line_width_ : 0; }

	const BYTE* LineBuffer(int line) const
	{ return static_cast<UINT>(line) < static_cast<UINT>(height_) ? data_buff_ + (height_ - line - 1) * line_width_ : 0; }

	BYTE* LinePixelBuffer(size_t line, size_t pixel)
	{
		ASSERT(line < size_t(height_));
		ASSERT(pixel <= size_t(width_));
		return data_buff_ + (height_ - line - 1) * line_width_ + pixel * (bits_per_pixel_ >> 3);
	}

	const BYTE* LinePixelBuffer(size_t line, size_t pixel) const
	{ return const_cast<Dib*>(this)->LinePixelBuffer(line, pixel); }

	int GetNumColors() const;
	DWORD GetPaletteSize() const;
	RGBQUAD GetPixel(int x, int y) const;

	void Draw(CDC* dc, CRect rect) const;
	void Draw(CDC* dc, CRect dest_rect, CRect* prectSrc, bool halftone= true, bool use_icm= false) const;
	// quick draw (no halftoning)
	void Draw(CDC* dc, CPoint pos) const;
	// draw image preserving aspect ratio
	void DrawScaled(CDC* dc, const CRect& dest_rect, COLORREF rgb_back) const;
	// draw image using DrawDibDraw function
	void DibDraw(CDC* dc, CRect dest_rect, CRect* prectSrc) const;
	void DibDraw(CDC* dc, CPoint pos) const;
	// draw image selecting method based on method
	void DrawBitmap(CDC* dc, const CRect& dest_rect, CRect* prectSrc, DibDispMethod method) const;
	// returns bmp destination rect for above drawing method
	CRect GetDisplayRect(CRect dest_rect);

	CBitmap* GetBmp()				{ return &dib_bmp_; }

	// in place resize
	void Resize(CSize img_size, COLORREF rgb_back, bool uniform);

	// in place resize (preserving aspect ratio) so the resized copy fits into 'img_size'
	enum ResizeMethod { RESIZE_POINT_SAMPLE, RESIZE_HALFTONE, RESIZE_CUBIC };
	void ResizeToFit(CSize img_size, ResizeMethod method);
	bool ResizeToFit(CSize img_size, ResizeMethod method, Dib& bmp);

	AutoPtr<CBitmap> DeviceBitmap();

	void Save(const TCHAR* output_file);

	static int ColorTableSize(int bits_per_pixel);

	AutoPtr<Dib> RotateCopy(bool clockwise);

	void RotateInPlace(bool clockwise);

	void Swap(Dib& dib);

	// in-place convert from YCbCr space to RGB
	void ConvertYCbCr2RGB();

	RGBQUAD* GetColorTable()				{ return bmp_info_.bmiColors; }
	const RGBQUAD* GetColorTable() const	{ return bmp_info_.bmiColors; }

	bool IsValid() const;

	void Clone(const Dib& src);

	// return new size of bmp to fit into 'dest_size'
	static CSize SizeToFit(CSize dest_size, CSize orig_img_size);

	const BITMAPINFO& AsBitmapInfo() const	{ return bmp_info_.AsBitmapInfo(); }

	void Detach(CBitmap& bmp);
	void CreateCBitmap();

private:
	CBitmap dib_bmp_;
	int width_;
	int height_;
	int line_width_;
	int bits_per_pixel_;
	BYTE* data_buff_;
	void* alloc_;

	struct NewBITMAPINFO_5
	{
		BITMAPV5HEADER	bmiHeader;
		RGBQUAD			bmiColors[1];

		BITMAPINFO& AsBitmapInfo()				{ return *reinterpret_cast<BITMAPINFO*>(this); }
		const BITMAPINFO& AsBitmapInfo() const	{ return *reinterpret_cast<const BITMAPINFO*>(this); }

		BITMAPINFOHEADER* AsBitmapInfoHeader()				{ return reinterpret_cast<BITMAPINFOHEADER*>(&bmiHeader); }
		const BITMAPINFOHEADER* AsBitmapInfoHeader() const	{ return reinterpret_cast<const BITMAPINFOHEADER*>(&bmiHeader); }

		void Init()
		{
			memset(&bmiHeader, 0, sizeof bmiHeader);
			bmiHeader.bV5Size = sizeof bmiHeader;
			bmiHeader.bV5GammaRed = bmiHeader.bV5GammaGreen = bmiHeader.bV5GammaBlue = 0x10000;
		}

		NewBITMAPINFO_5& operator = (const BITMAPINFOHEADER& src)
		{
			Init();
			*AsBitmapInfoHeader() = src;	// assigns only common part
			bmiHeader.bV5Size = sizeof bmiHeader;
			return *this;
		}
	};

#if USE_BITMAPV5HEADER
	NewBITMAPINFO_5 bmp_info_;
#else
	BITMAPINFO bmp_info_;
#endif
	RGBQUAD color_table_[255];	// this vector must follow BITMAPINFO struct

	Dib& operator = (const Dib&);
	Dib(const Dib&);
};


// rescale bmp preserving its aspect ratio; if resize_to_fill is true entire 'rect' will be covered
// with drawn bitmap (and resized bitmap may appear cropped); if resize_to_fill is false,
// bitmap will be resized to fit 'rect'; returns false if bitmap was not drawn
extern bool DrawPreserveAspectRatio(const Dib& dib, CDC* dc, CRect rect, COLORREF backgnd, bool icm, bool resize_to_fill);


typedef boost::shared_ptr<Dib> DibPtr;

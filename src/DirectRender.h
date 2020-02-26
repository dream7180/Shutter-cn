/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
#include <boost/noncopyable.hpp>
class Dib;


class DirectRender : boost::noncopyable
{
public:
	DirectRender(HWND hwnd);
	~DirectRender();

	// returns true if internal target and bitmap are available, and false when either needs to be refreshed
	bool IsBitmapReady() const;
	bool IsBitmapCopyReady() const;

	// if bitmam is cached/ready, return its size, (0, 0) otherwise
	CSize GetBitmapSize() const;

	// call if cached bitmap needs to be refreshed, this will create target and bitmap as needed by renderer
	// if 'copy_content' is true, dib's pixels data will be copied to the cached bitmap
	bool PrepareBitmap(const Dib& dib, COLORREF backgnd, bool copy_content);

	// copy part of dib (src_rect) into corresponding place in the cached bitmap
	bool CopyIntoBitmap(const Dib& dib, CRect src_rect);

	// if internal cached bitmap needs to be made larger, rescale will do that preserving its content
//	bool RescaleBitmap(int times);
	bool RescaleBitmap(CSize new_size);

	// rotate bitmap 90 degrees CW or CCW
	bool RotateBitmap(bool clockwise);

	// render (cached/prepared) bitmap in the window
	HRESULT Render(CPoint offset, float image_scale_x, float image_scale_y, COLORREF backgnd, float opacity);

	HRESULT RenderCopy(CPoint offset, float image_scale_x, float image_scale_y, float rotation_angle, COLORREF backgnd, float opacity);

	// window got resized
	void Resize(int width, int height);

	// free bitmap, so it won't be used any more
	void DeleteBitmap();

	// current bitmap can be kept in a copy
	void KeepCopy();
	void ReleaseCopy();

private:
	struct Impl;
	Impl& impl_;
};

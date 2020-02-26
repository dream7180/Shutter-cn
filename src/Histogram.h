/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// Histogram.h: interface for the Histogram class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HISTOGRAM_H__F773AB2C_93D2_4557_B28B_94D98C9B0B49__INCLUDED_)
#define AFX_HISTOGRAM_H__F773AB2C_93D2_4557_B28B_94D98C9B0B49__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
class Dib;


class Histogram
{
public:
	Histogram();
	virtual ~Histogram();

	// drawing options
	enum { DRAW_GRADIENT= 1, DRAW_LABELS= 2, DRAW_PERCENTAGE= 4, NO_ERASE_BACKGND= 8, NO_TOP_SPACE= 0x10,
		NO_BOTTOM_SPACE= 0x20, STRETCH_EACH_BAR= 0x40, DRAW_SLIDER_ARROWS= 0x80, NO_FRAME= 0x100, DRAW_BASE_AXIS= 0x200 };

	// channels to draw
	enum ChannelSel { LumRGB, RGBLines, LUM, RED, GREEN, BLUE, RGB, RGBOverlaid };

	void Draw(CDC* dc, CRect rect, ChannelSel channels, UINT flags);

	COLORREF rgb_back_;
	COLORREF rgb_fore_;
	int min_edge_;
	int max_edge_;
	bool logarithmic_;
	bool using_luminosity_and_rgb_;

	void BuildHistogram(const Dib* bmp, const CRect* prectSelection= 0);

	CRect GetHistogramRect() const		{ return histogram_rect_; }

private:
	std::vector<uint32> red_;
	std::vector<uint32> green_;
	std::vector<uint32> blue_;
	std::vector<uint32> lum_;
	std::vector<uint32> rgb_;

	std::vector<uint32>* GetChannelData(int channel);

	uint32 max_val_;
	uint32 max_vals_[5];
	uint32 pixels_count_;
	uint32 total_pixels_;

	CRect histogram_rect_;

	void PaintStripe(CDC* dc, CRect rect, COLORREF rgb_color);
	void PaintRect(CDC* dc, int x, int y, int w, int h, COLORREF color);

	void DrawBars(CDC* dc, CPoint start, int width, int height, int channel, COLORREF rgb_bar, bool stretch_bar);
	void DrawLines(CDC* dc, CPoint start, int width, int height, int channel);
	void DrawOverlaidBars(CDC* dc, CPoint start, int width, int height);
};

#endif // !defined(AFX_HISTOGRAM_H__F773AB2C_93D2_4557_B28B_94D98C9B0B49__INCLUDED_)

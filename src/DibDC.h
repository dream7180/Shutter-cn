/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
#include "Dib.h"


class DibDC : public CDC
{
public:
	DibDC(CDC& dc, CWnd* wnd, COLORREF rgb_clr_back= -1);
	DibDC(CDC& dc, const CRect& rect, COLORREF rgb_clr_back= -1);
	virtual ~DibDC();

	void BitBlt();
	void BitBlt(int height);

	void OffsetTo(const CRect& rect);
	void OffsetTo(const CPoint& point);
	void OffsetYTo(int y_pos);

	Dib& GetDib();

//	virtual BOOL RectVisible(LPCRECT rect) const;

private:
	CDC* dc_;
	Dib screen_bmp_;
	CPoint pos_;
	CSize size_;

	void Init(CDC& dc, const CRect& rect);
};

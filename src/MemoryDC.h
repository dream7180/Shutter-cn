/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// MemoryDC.h: interface for the MemoryDC class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MEMORYDC_H__1DFA018A_CA70_45E4_87EE_865C0E9982AC__INCLUDED_)
#define AFX_MEMORYDC_H__1DFA018A_CA70_45E4_87EE_865C0E9982AC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class MemoryDC : public CDC
{
public:
	MemoryDC(CDC& dc, CWnd* wnd, COLORREF rgb_clr_back= -1);
	MemoryDC(CDC& dc, const CRect& rect, COLORREF rgb_clr_back= -1);
	virtual ~MemoryDC();

	void BitBlt();
	void BitBlt(int height);

	void OffsetTo(const CRect& rect);
	void OffsetTo(const CPoint& point);
	void OffsetYTo(int y_pos);

//	virtual BOOL RectVisible(LPCRECT rect) const;

private:
	CDC* dc_;
	CBitmap screen_bmp_;
	CPoint pos_;
	CSize size_;
	CBitmap* old_bmp_;

	void Init(CDC& dc, const CRect& rect);
};

#endif // !defined(AFX_MEMORYDC_H__1DFA018A_CA70_45E4_87EE_865C0E9982AC__INCLUDED_)

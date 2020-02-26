/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#if !defined(AFX_HISTOGRAMIMG_H__127DB1FB_0C96_416B_9CC0_8521193529DB__INCLUDED_)
#define AFX_HISTOGRAMIMG_H__127DB1FB_0C96_416B_9CC0_8521193529DB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// HistogramImg.h : header file
//
class Dib;
class HistogramCtrl;

/////////////////////////////////////////////////////////////////////////////
// HistogramImg window

class HistogramImg : public CButton
{
// Construction
public:
	HistogramImg(HistogramCtrl* hist, CWnd* rect_label_wnd);

// Attributes
public:

// Operations
public:
	void SetImage(Dib* bmp)	{ bmp_ = bmp; }

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(HistogramImg)
public:
	virtual void DrawItem(LPDRAWITEMSTRUCT draw_item_struct);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~HistogramImg();

	// Generated message map functions
protected:
	//{{AFX_MSG(HistogramImg)
	afx_msg BOOL OnEraseBkgnd(CDC* dc);
	afx_msg void OnLButtonDown(UINT flags, CPoint point);
	afx_msg void OnLButtonUp(UINT flags, CPoint point);
	afx_msg void OnMouseMove(UINT flags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* wnd, UINT hit_test, UINT message);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

private:
	Dib* bmp_;
	bool drawing_;
	CRect selection_rect_;
	CRect image_rect_;
	HistogramCtrl* hist_view_;
	CWnd* rect_label_wnd_;

	void PointInside(CPoint& pos);
	void SetCursor();
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HISTOGRAMIMG_H__127DB1FB_0C96_416B_9CC0_8521193529DB__INCLUDED_)

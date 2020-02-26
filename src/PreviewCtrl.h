/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once

// PreviewCtrl.h : header file
//
#include "Dib.h"


class PreviewCtrlNotifications
{
public:
	virtual void MouseMoved(CWnd* preview, int x, int y, bool outside) {}
	virtual void SelectionRectChanged(const CRect& rect) {}
};


/////////////////////////////////////////////////////////////////////////////
// CPreviewCtrl window

class CPreviewCtrl : public CWnd
{
// Construction
public:
	CPreviewCtrl();

// Attributes
public:
//	void SetGamma(double gamma);
	void SetDib(Dib& dib);

	void SetHost(PreviewCtrlNotifications* host)	{ host_ = host; }

	// use ICM
	bool icm_;

	void EnableSelection(bool enable);

	void SetOriginalBmpSize(CSize original_size);

	void EnableMagneticGrid(bool enable);
	bool IsMagneticGridEnabled() const		{ return magnetic_grid_; }

	CRect GetSelectionRect() const;
	void SetSelectionRect(const CRect& rect);

	void SetRatioConstrain(CSize ratio);

// Operations
public:
	//bool Create(CWnd* parent, CRect rect);
	void Clear();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPreviewCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CPreviewCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CPreviewCtrl)
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* dc);
	afx_msg BOOL OnSetCursor(CWnd* wnd, UINT hit_test, UINT message);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	Dib* image_;
	PreviewCtrlNotifications* host_;
	CRect bitmap_rect_;
	bool show_selection_;
	CRect selection_rect_;
	bool drawing_;
	bool moving_;
	bool resizing_;
	CPoint start_;
	UINT side_;
	CRect orig_bitmap_rect_;
	CSize grid_size_;
	bool magnetic_grid_;
	CSize ratio_constraints_;

	void Draw(CDC& dc);
	LRESULT OnMouseLeave(WPARAM, LPARAM);
	void SendMouseNotification(CPoint point);
	void SetCursor();
	void DrawSelectionRect(CDC& dc, const CRect& selection_rect);
	void PointInside(CPoint& pos);
	void OnLButtonDown(UINT flags, CPoint pos);
	void OnLButtonUp(UINT flags, CPoint point);
	void OnMouseMove(UINT flags, CPoint pos);
	enum { TOP= 1, RIGHT= 2, BOTTOM= 4, LEFT= 8 };
	UINT CheckSide(const CRect& rect, CPoint point);
	CRect GetSelection() const;
	enum Constrain { HEIGHT, WIDTH, ANY };
	void ConstrainAspectRatio(CRect& rect, CSize ratio, const CRect& bounds, UINT sides);

	struct DRect
	{
		double x1, y1;
		double x2, y2;

		DRect() : x1(), y1(), x2(), y2()
		{}

		void Set(const CRect& bmp_rect, const CRect& selection_rect);

		CRect Get(const CRect& bmp_rect) const;

		CRect Get(const CRect& bmp_rect, const CRect& original_bmp_rect, CSize grid_size) const;

	private:
		CRect CalcRect(const CRect& bmp_rect) const;

	} selection_;
};

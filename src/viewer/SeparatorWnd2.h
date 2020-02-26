/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once

class ResizeWnd;


/////////////////////////////////////////////////////////////////////////////
// ViewerSeparatorWnd window

class ViewerSeparatorWnd : public CWnd
{
// Construction
public:
	ViewerSeparatorWnd();

	static void DrawSeparatorBar(CDC& dc, CRect rect, bool horizontal);

// Attributes
public:
	int GetHeight() const		{ return 7; }

	void SetOrientation(bool horizontal);

// Operations
public:
	bool Create(CWnd* parent, ResizeWnd* resize_wnd);

	void Show(bool show);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ViewerSeparatorWnd)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~ViewerSeparatorWnd();

	// Generated message map functions
protected:
	//{{AFX_MSG(ViewerSeparatorWnd)
	afx_msg BOOL OnEraseBkgnd(CDC* dc);
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT flags, CPoint point);
	afx_msg void OnLButtonUp(UINT flags, CPoint point);
	afx_msg void OnMouseMove(UINT flags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	CPoint start_;
	int pane_height_;
	bool resizing_;
	ResizeWnd* resize_wnd_;
	bool horizontal_;

	BOOL OnSetCursor(CWnd* wnd, UINT hitTest, UINT message);
	void SetCursor();
};

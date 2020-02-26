/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#if !defined(AFX_SEPARATORWND_H__AF6235B4_669D_476F_9416_EA9969114A52__INCLUDED_)
#define AFX_SEPARATORWND_H__AF6235B4_669D_476F_9416_EA9969114A52__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SeparatorWnd.h : header file
//
class ResizeWnd;


/////////////////////////////////////////////////////////////////////////////
// SeparatorWnd window

class SeparatorWnd : public CWnd
{
// Construction
public:
	SeparatorWnd(bool horizontal= true);

	static void DrawSeparatorBar(CDC& dc, CRect rect, bool horizontal);

// Attributes
public:
	int GetHeight() const		{ return 7; }
	// draw separator bar and rivets (true), or rivets only (false)
	void DrawBar(bool draw);
	// when moving separator bar to the left report growing size of window being resized (or shrinking if 'set' is false)
	void GrowWhenMovingLeft(bool set);

// Operations
public:
	bool Create(CWnd* parent, ResizeWnd* resize_wnd);

	void Show(bool show);

	void SetCallbacks(ResizeWnd* client_interface);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(SeparatorWnd)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~SeparatorWnd();

	// Generated message map functions
protected:
	//{{AFX_MSG(SeparatorWnd)
	afx_msg BOOL OnEraseBkgnd(CDC* dc);
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT flags, CPoint point);
	afx_msg void OnLButtonUp(UINT flags, CPoint point);
	afx_msg void OnMouseMove(UINT flags, CPoint point);
	BOOL OnSetCursor(CWnd* wnd, UINT hit_test, UINT message);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	CPoint start_;
	int pane_height_;
	bool resizing_;
	ResizeWnd* resize_wnd_;
	bool horizontal_;
	bool draw_bar_;
	HCURSOR cursor_;
	bool grow_when_moving_left_;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SEPARATORWND_H__AF6235B4_669D_476F_9416_EA9969114A52__INCLUDED_)

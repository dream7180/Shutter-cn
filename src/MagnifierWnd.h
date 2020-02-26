/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
// MagnifierWnd.h : header file

#include "Profile.h"
#include "Dib.h"


/////////////////////////////////////////////////////////////////////////////
// MagnifierWnd frame

class MagnifierWnd : public CMiniFrameWnd
{
public:
	MagnifierWnd(DibPtr dibImage, const CRect& display_rect, CWnd* display_wnd);

// Attributes
public:
//	CSize window_size_;

// Operations
public:
	static void Close();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(MagnifierWnd)
	//}}AFX_VIRTUAL
	virtual void PostNcDestroy();

// Implementation
protected:
	virtual ~MagnifierWnd();

	// Generated message map functions
	//{{AFX_MSG(MagnifierWnd)
	afx_msg BOOL OnEraseBkgnd(CDC* dc);
	afx_msg void OnMouseMove(UINT flags, CPoint point);
	afx_msg void OnActivate(UINT state, CWnd* wnd_other, BOOL minimized);
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT_PTR id_event);
	afx_msg void OnLButtonDown(UINT flags, CPoint point);
	afx_msg void OnKeyDown(UINT chr, UINT rep_cnt, UINT flags);
	afx_msg BOOL OnMouseWheel(UINT flags, short delta, CPoint pt);
	afx_msg void OnContextMenu(CWnd* wnd, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	void OnMButtonDown(UINT flags, CPoint point);

	CRect bitmap_rect_;
	DibPtr dib_image_;
	UINT_PTR timer_;
	CPoint last_mouse_pos_;
	CPoint offset_;
	int magnification_factor_;
	bool popup_menu_;
	CWnd* display_;
	CRect limit_rect_;	// limit magnifying glass to this area
	enum { SMALL= 400+2, MEDIUM= 600+2, BIG= 800+2 };	// size (including border line)
	Profile<int> profile_window_size_;
	Profile<int> profile_magnification_factor_;

	static MagnifierWnd* magnifier_wnd_;

	void MouseMoved();
	void ChangeMagnification(bool increase);
	void ResizeWindow(CSize wnd_size);
	LRESULT OnFinish(WPARAM, LPARAM);
	void MoveTo(CPoint pos, CSize wnd_size= CSize(0, 0));
	BOOL EraseBkgnd(CDC* dc);
};

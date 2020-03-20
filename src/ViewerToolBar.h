/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once

//#include "ToolBarWnd.h"
#include "viewer/FancyToolBar.h"

// ViewerToolBar.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// ViewerToolBar window

class ViewerToolBar : public FancyToolBar
{
	typedef FancyToolBar Derived;

// Construction
public:
	ViewerToolBar();

// Attributes
//public:
	bool SmallIcons();
	bool LargeIcons();

	bool IsSmallSet() const		{ return small_icons_; }

// Operations
public:
	bool Create(CWnd* parent, UINT id);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ViewerToolBar)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~ViewerToolBar();

	// Generated message map functions
protected:
	//{{AFX_MSG(ViewerToolBar)
	afx_msg void OnDestroy();
	//afx_msg void OnToolbarCustomize();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

	bool small_icons_;
	//int rebar_band_id_;

	//void OnRightClick(NMHDR* notify_struct, LRESULT* result);
	void OnContextMenu(CWnd* wnd, CPoint point);
	void OnResetToolbar(NMHDR* notify_struct, LRESULT* result);
	void OnInitMenuPopup(CMenu* popup_menu, UINT index, BOOL sys_menu);
	void AdjustReBar();
	//void OnRButtonDown(UINT flags, CPoint pos);
};

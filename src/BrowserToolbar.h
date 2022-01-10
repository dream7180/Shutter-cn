/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#if !defined(AFX_BROWSERTOOLBAR_H__3A54E9EF_6036_4AF3_841E_0DA1491702BB__INCLUDED_)
#define AFX_BROWSERTOOLBAR_H__3A54E9EF_6036_4AF3_841E_0DA1491702BB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "ToolBarWnd.h"
// BrowserToolbar.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// BrowserToolbar window

class BrowserToolbar : public CWnd
{
// Construction
public:
	BrowserToolbar();

// Attributes
public:
	CRect GetButtonRect(int cmd);

	void SetBackgroundColor(COLORREF color);

// Operations
public:
	bool Create(CWnd* parent, UINT id1, UINT id2, UINT rebar_band_id);

	virtual void ResetToolBar(bool resize_to_fit);

	void PressButton(int cmd, bool down= true);

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(BrowserToolbar)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~BrowserToolbar();

	// Generated message map functions
protected:
	//{{AFX_MSG(BrowserToolbar)
	//afx_msg void OnToolbarCustomize();
	afx_msg void OnDestroy();
	afx_msg void OnLButtonDown(UINT flags, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

	//bool small_icons_;
	UINT rebar_band_id_;
	ToolBarWnd main_bar_;
	ToolBarWnd panels_;
	COLORREF background_;

	LRESULT OnSizeParent(WPARAM, LPARAM lParam);
	void OnGetInfoTip(NMHDR*, LRESULT*);
	//void OnRightClick(NMHDR* notify_struct, LRESULT* result);
	//void OnContextMenu(CWnd* wnd, CPoint point);
	//void OnUpdateSmallIcons(CCmdUI* cmd_ui);
	//void OnSmallIcons();
	//void OnUpdateLargeIcons(CCmdUI* cmd_ui);
	//void OnLargeIcons();
	//void OnInitMenuPopup(CMenu* popup_menu, UINT index, BOOL sys_menu);
	void AdjustReBar();
	BOOL OnEraseBkgnd(CDC* dc);
	void OnSize(UINT type, int cx, int cy);
	void Resize();
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BROWSERTOOLBAR_H__3A54E9EF_6036_4AF3_841E_0DA1491702BB__INCLUDED_)

/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#if !defined(AFX_PREVIEWBAR_H__4D379886_03BA_4B17_982B_4CD810C2523B__INCLUDED_)
#define AFX_PREVIEWBAR_H__4D379886_03BA_4B17_982B_4CD810C2523B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PreviewBar.h : header file
//
#include "ToolBarWnd.h"
#include "SliderCtrlEx.h"
class PreviewPane;


/////////////////////////////////////////////////////////////////////////////
// PreviewBar window

class PreviewBar : public CWnd
{
// Construction
public:
	PreviewBar();

// Attributes
public:
	std::pair<int, int> GetMinMaxWidth() const;

	void SetBackgroundColor(COLORREF backgnd);

// Operations
public:
	bool Create(PreviewPane* parent, bool big);

	void SetSlider(const uint16* zoom_table, int count, int zoom);

	void SetBitmapSize(bool big);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(PreviewBar)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~PreviewBar();

	// Generated message map functions
protected:
	//{{AFX_MSG(PreviewBar)
	afx_msg void OnSize(UINT type, int cx, int cy);
	afx_msg void OnVScroll(UINT sb_code, UINT pos, CScrollBar* scroll_bar);
	afx_msg void OnHScroll(UINT sb_code, UINT pos, CScrollBar* scroll_bar);
	afx_msg void OnViewerOptions();
	afx_msg void OnUpdateViewerOptions(CCmdUI* cmd_ui);
	afx_msg BOOL OnEraseBkgnd(CDC* dc);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	bool vertical_;
	ToolBarWnd toolbar_top_wnd_;
	SliderCtrlEx zoom_wnd_;
	ToolBarWnd toolbar_btm_wnd_;
	PreviewPane* parent_;

	void Resize();
	void SetSliderPos(int pos);
	void OnScroll(UINT sb_code, UINT pos, CScrollBar* scroll_bar);
	LRESULT OnIdleUpdateCmdUI(WPARAM wParam, LPARAM lParam);
	LRESULT OnTbClicked(WPARAM hwnd, LPARAM code);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PREVIEWBAR_H__4D379886_03BA_4B17_982B_4CD810C2523B__INCLUDED_)

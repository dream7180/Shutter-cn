/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once

//#include "ViewerReBar.h"
#include "viewer/PreviewBandWnd.h"
//#include "ViewerList.h"
#include "ViewPane.h"
//#include "SeparatorWnd.h"
#include "VectPhotoInfo.h"

/////////////////////////////////////////////////////////////////////////////
// ViewerExampleWnd window

class ViewerExampleWnd : public CWnd
{
// Construction
public:
	ViewerExampleWnd(VectPhotoInfo& photos);

// Attributes
public:
	void ResetColors();

// Operations
public:
	bool Create(CWnd* parent, const RECT& rect);

	void SetColors(const std::vector<COLORREF>& colors);

	void SetDescriptionFont(const LOGFONT& lf);

	void SetUIBrightness(double gamma);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ViewerExampleWnd)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~ViewerExampleWnd();

	// Generated message map functions
protected:
	//{{AFX_MSG(ViewerExampleWnd)
	afx_msg int OnCreate(LPCREATESTRUCT create_struct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	void OnSize(UINT type, int cx, int cy);

	PreviewBandWnd	preview_;		// list of thumbnails
//	CViewerReBar	preview_bar_wnd_;	// rebar ctrl containing preview list
//	SeparatorWnd	separator_wnd_;		// separator between preview list and main display (resizing support)
	ViewPane		display_wnd_;		// main display pane
	VectPhotoInfo&	photos_;

	void Resize();
	void DrawItem(CDC& dc, CRect rect, size_t item, AnyPointer key);
	COLORREF rgb_tag_text_, rgb_tag_bkgnd_;
};

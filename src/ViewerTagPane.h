/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// ViewerTagPane.h: interface for the ViewerTagPane class.

#pragma once

#include "DockedPane.h"
#include "PhotoInfo.h"
#include "ToolBarWnd.h"
#include "ColorSets.h"
class PhotoTagsCollection;


class ViewerTagPane : public DockedPane
{
public:
	ViewerTagPane(PhotoTagsCollection& tag_collection);
	virtual ~ViewerTagPane();

// Operations
	bool Create(CWnd* parent, UINT id, int width);

	void SetUIBrightness(double gamma);

	void PhotoSelected(PhotoInfoPtr photo, bool selection, bool still_loading);

	void AssignTag(int index);

	//{{AFX_VIRTUAL(ViewerTagPane)
	//}}AFX_VIRTUAL

	// Generated message map functions
protected:
	//{{AFX_MSG(ViewerTagPane)
	afx_msg void OnSize(UINT type, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	//static const int VIEWER_TAGPANE_H = 60;
	int header_h;
	struct Impl;
	Impl& impl_;

	void Paint(CDC& dc);
	void Resize();
	BOOL OnEraseBkgnd(CDC* dc);
	LRESULT OnPrintClient(WPARAM HDC, LPARAM flags);
};

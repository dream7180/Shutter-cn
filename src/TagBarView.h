/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once

// TagBarView.h : header file

#include "ToolBarWnd.h"
#include "Profile.h"
#include "ListViewCtrl.h"
#include "Pane.h"
#include "TagsBarCommon.h"


class TagBarView : public PaneWnd
{
// Construction
public:
	TagBarView();

// Attributes
public:
	bool HasFocus() const;

// Operations
public:
	bool Create(CWnd* parent);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(TagBarView)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~TagBarView();

	// Generated message map functions
protected:
	//{{AFX_MSG(TagBarView)
	afx_msg BOOL OnEraseBkgnd(CDC* dc);
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT type, int cx, int cy);
	afx_msg void OnTaskBar();
	BOOL OnSetCursor(CWnd* wnd, UINT hit_test, UINT message);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	std::auto_ptr<TagsBarCommon> tags_ctrl_;

	void Resize();
	void SetColors();

	virtual void SelectionHasChanged(VectPhotoInfo& selected);
	virtual void AssignTag(int index);
	//virtual void CaptionHeightChanged(bool big);
	virtual void OptionsChanged(OptionsDlg& dlg);
	virtual void ActivatePane(bool active);
};

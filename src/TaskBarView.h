/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#if !defined(AFX_TASKBARVIEW_H__5E0E60DE_AAA4_4B55_890F_98F27C116935__INCLUDED_)
#define AFX_TASKBARVIEW_H__5E0E60DE_AAA4_4B55_890F_98F27C116935__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TaskBarView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// TaskBarView window
#include "ToolBarWnd.h"
#include "Profile.h"
#include "ListViewCtrl.h"
#include "Pane.h"


class TaskBarView : public PaneWnd
{
// Construction
public:
	TaskBarView();

// Attributes
public:

// Operations
public:
	bool Create(CWnd* parent);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(TaskBarView)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~TaskBarView();

	// Generated message map functions
protected:
	//{{AFX_MSG(TaskBarView)
	afx_msg BOOL OnEraseBkgnd(CDC* dc);
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT type, int cx, int cy);
	afx_msg void OnTaskBar();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	ToolBarWnd toolbar_wnd_;
	CFont toolbar_fnt_;
	ListViewCtrl list_wnd_;
	CImageList image_list_;

	void Resize();
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TASKBARVIEW_H__5E0E60DE_AAA4_4B55_890F_98F27C116935__INCLUDED_)

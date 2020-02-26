/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#if !defined(_LINE_SEPARATOR_CTRL_)
#define _LINE_SEPARATOR_CTRL_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// LineSeparatorCtrl window - draw horizontal separator line


class LineSeparatorCtrl : public CWnd
{
// Construction
public:
	LineSeparatorCtrl();

// Attributes
public:

// Operations
public:
	bool Create(CWnd* parent);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(LineSeparatorCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~LineSeparatorCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(LineSeparatorCtrl)
	afx_msg BOOL OnEraseBkgnd(CDC* dc);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
};

#endif // !defined(_LINE_SEPARATOR_CTRL_)

/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#if !defined(AFX_PRINTPREVIEWWND_H__C13F398D_F18A_4B15_8589_267577B80856__INCLUDED_)
#define AFX_PRINTPREVIEWWND_H__C13F398D_F18A_4B15_8589_267577B80856__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PrintPreviewWnd.h : header file
//
#include "PrintEngine.h"


/////////////////////////////////////////////////////////////////////////////
// PrintPreviewWnd window

class PrintPreviewWnd : public CWnd
{
// Construction
public:
	PrintPreviewWnd(VectPhotoInfo& selected, PrintEngine* print);

// Attributes
public:
	void SetEngine(PrintEngine* print);

// Operations
public:
	bool Create(CWnd* parent, CRect rect);

	void SetPageSize(CSize size, const CRect& margins_rect);

	void SetPrinableArea(const CRect& rect);

	void SetItemsAcross(int count);

	void SetMargins(const CRect& margins_rect);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(PrintPreviewWnd)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~PrintPreviewWnd();

	// Generated message map functions
protected:
	//{{AFX_MSG(PrintPreviewWnd)
	afx_msg BOOL OnEraseBkgnd(CDC* dc);
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	VectPhotoInfo& selected_;
	PrintEngine* print_;

	CRect PrepareDC(CDC& dc);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PRINTPREVIEWWND_H__C13F398D_F18A_4B15_8589_267577B80856__INCLUDED_)

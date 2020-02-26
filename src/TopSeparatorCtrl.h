/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#if !defined(_TOP_SEPARATOR_CTRL_)
#define _TOP_SEPARATOR_CTRL_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// TopSeparatorCtrl window - draw lighter top edge


class TopSeparatorCtrl : public CWnd
{
// Construction
public:
	TopSeparatorCtrl();

// Attributes
public:
	int GetHeight() const		{ return 7; }

// Operations
public:
	bool Create(CWnd* parent);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(TopSeparatorCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~TopSeparatorCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(TopSeparatorCtrl)
	afx_msg BOOL OnEraseBkgnd(CDC* dc);
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT flags, CPoint point);
	afx_msg void OnLButtonUp(UINT flags, CPoint point);
	afx_msg void OnMouseMove(UINT flags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
};

#endif // !defined(_TOP_SEPARATOR_CTRL_)

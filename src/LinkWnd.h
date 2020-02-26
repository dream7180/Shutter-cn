/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#if !defined(AFX_LINKWND_H__CC0E5D49_8E6A_4D0A_A275_0699ED51CD2C__INCLUDED_)
#define AFX_LINKWND_H__CC0E5D49_8E6A_4D0A_A275_0699ED51CD2C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LinkWnd.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CLinkWnd window

class CLinkWnd : public CWnd
{
// Construction
public:
	CLinkWnd();

// Attributes
public:
	COLORREF rgb_text_color_;
//	COLORREF rgb_hot_text_color_;

// Operations
public:
	bool Create(CWnd* parent, CPoint top_left, const TCHAR* display, const TCHAR* URL,
		CFont* font, UINT uId= ~0u);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLinkWnd)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CLinkWnd();

	// Generated message map functions
protected:
	//{{AFX_MSG(CLinkWnd)
	afx_msg BOOL OnEraseBkgnd(CDC* dc);
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT flags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	virtual void PreSubclassWindow();

	CFont underlined_fnt_;
	CString URL_;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LINKWND_H__CC0E5D49_8E6A_4D0A_A275_0699ED51CD2C__INCLUDED_)

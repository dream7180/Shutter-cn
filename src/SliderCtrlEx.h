/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#if !defined(AFX_SLIDERCTRLEX_H__B014911B_1A11_40D9_BF3A_4C1B8ADA9845__INCLUDED_)
#define AFX_SLIDERCTRLEX_H__B014911B_1A11_40D9_BF3A_4C1B8ADA9845__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SliderCtrlEx.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// SliderCtrlEx window

class SliderCtrlEx : public CSliderCtrl
{
// Construction
public:
	SliderCtrlEx();

// Attributes
public:
	void SetBackgroundColor(COLORREF backgnd);

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(SliderCtrlEx)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~SliderCtrlEx();

	// Generated message map functions
protected:
	//{{AFX_MSG(SliderCtrlEx)
	afx_msg BOOL OnEraseBkgnd(CDC* dc);
	afx_msg void OnCustomDraw(NMHDR* nmhdr, LRESULT* result);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

	CDC background_dc_;
	CBitmap background_bmp_;
	COLORREF rgb_backgnd_;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SLIDERCTRLEX_H__B014911B_1A11_40D9_BF3A_4C1B8ADA9845__INCLUDED_)

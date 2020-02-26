/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#if !defined(AFX_COLORBUTTON_H__CC0561D4_78E5_4006_BD14_1E8B9E6C5581__INCLUDED_)
#define AFX_COLORBUTTON_H__CC0561D4_78E5_4006_BD14_1E8B9E6C5581__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ColorButton.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// ColorButton window

class ColorButton : public CButton
{
// Construction
public:
	ColorButton();

// Attributes
public:

// Operations
public:
	void SetColor(COLORREF rgb_color);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ColorButton)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~ColorButton();

	// Generated message map functions
protected:
	//{{AFX_MSG(ColorButton)
	afx_msg void OnPaint();
	afx_msg void OnEnable(BOOL enable);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

private:
	void CreateIcon(COLORREF rgb_color);
//	CBitmap color_bmp_;
//	HICON hicon_color_;
	COLORREF rgb_color_;
	void PaintIt(int offset);
	int dx_;
	int dy_;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COLORBUTTON_H__CC0561D4_78E5_4006_BD14_1E8B9E6C5581__INCLUDED_)

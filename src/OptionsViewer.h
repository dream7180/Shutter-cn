/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#if !defined(AFX_OPTIONSVIEWER_H__C7E84E0A_67E7_4E9E_9C66_A78C72A3282C__INCLUDED_)
#define AFX_OPTIONSVIEWER_H__C7E84E0A_67E7_4E9E_9C66_A78C72A3282C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OptionsViewer.h : header file
//
#include "ColorButton.h"
#include "RPropertyPage.h"
#include "GrayscaleWnd.h"

/////////////////////////////////////////////////////////////////////////////
// COptionsViewer dialog

class COptionsViewer : public RPropertyPage
{
//	DECLARE_DYNCREATE(COptionsViewer)

// Construction
public:
	COptionsViewer();
	~COptionsViewer();

// Dialog Data
	//{{AFX_DATA(COptionsViewer)
	enum { IDD = IDD_OPTIONS_VIEWER };
	CEdit	edit_gamma_;
//	CColorButton btn_color_;
	double	gamma_;
	BOOL	keep_sel_centered_;
	BOOL	preload_;
	//}}AFX_DATA
	COLORREF rgb_selection_;
	CStatusBarCtrl status_wnd_;

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(COptionsViewer)
	protected:
	virtual void DoDataExchange(CDataExchange* DX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(COptionsViewer)
	afx_msg void OnDeltaPosGammaSpin(NMHDR* nmhdr, LRESULT* result);
	virtual BOOL OnInitDialog();
	afx_msg void OnSelColor();
	afx_msg void OnChangeGamma();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	afx_msg void OnStausClick(NMHDR* nmhdr, LRESULT* result);

private:
	CGrayscaleWnd grayscale_wnd_;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OPTIONSVIEWER_H__C7E84E0A_67E7_4E9E_9C66_A78C72A3282C__INCLUDED_)

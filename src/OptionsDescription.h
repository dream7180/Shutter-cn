/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#if !defined(AFX_OPTIONSDESCRIPTION_H__E4C850F1_025F_484A_92A3_2431D0999C82__INCLUDED_)
#define AFX_OPTIONSDESCRIPTION_H__E4C850F1_025F_484A_92A3_2431D0999C82__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OptionsDescription.h : header file
//
#include "RPropertyPage.h"

/////////////////////////////////////////////////////////////////////////////
// COptionsDescription dialog

class COptionsDescription : public RPropertyPage
{
//	DECLARE_DYNCREATE(COptionsDescription)

// Construction
public:
	COptionsDescription();
	~COptionsDescription();

// Dialog Data
	//{{AFX_DATA(COptionsDescription)
	enum { IDD = IDD_OPTIONS_DESCRIPTION };
	CStatic	example_wnd_;
	CStatic	font_name_wnd_;
	//}}AFX_DATA
	LOGFONT lf_desc_;
	COLORREF rgb_text_;
	CBrush br_back_;

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(COptionsDescription)
protected:
	virtual void DoDataExchange(CDataExchange* DX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(COptionsDescription)
	afx_msg void OnSelectFont();
	afx_msg HBRUSH OnCtlColor(CDC* dc, CWnd* wnd, UINT ctl_color);
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	CFont fnd_desc_;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OPTIONSDESCRIPTION_H__E4C850F1_025F_484A_92A3_2431D0999C82__INCLUDED_)

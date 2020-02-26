/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#if !defined(AFX_SLIDESHOWOPTDLG_H__47F8009C_3F7A_4C18_B148_D5B835DC8F46__INCLUDED_)
#define AFX_SLIDESHOWOPTDLG_H__47F8009C_3F7A_4C18_B148_D5B835DC8F46__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SlideShowOptDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSlideShowOptDlg dialog

class CSlideShowOptDlg : public CDialog
{
// Construction
public:
	CSlideShowOptDlg(CWnd* parent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSlideShowOptDlg)
	enum { IDD = IDD_SLIDE_SHOW_OPTIONS };
	CSpinButtonCtrl	spin_wnd_;
	int		delay_;
	BOOL	hide_toolbar_;
	BOOL	repeat_;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSlideShowOptDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* DX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSlideShowOptDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SLIDESHOWOPTDLG_H__47F8009C_3F7A_4C18_B148_D5B835DC8F46__INCLUDED_)

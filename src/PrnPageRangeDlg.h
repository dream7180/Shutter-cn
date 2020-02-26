/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#if !defined(AFX_PRNPAGERANGEDLG_H__C2FAF3A4_EC5C_456F_9AFC_F377240F6AF5__INCLUDED_)
#define AFX_PRNPAGERANGEDLG_H__C2FAF3A4_EC5C_456F_9AFC_F377240F6AF5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PrnPageRangeDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// PrnPageRangeDlg dialog

class PrnPageRangeDlg : public CDialog
{
// Construction
public:
	PrnPageRangeDlg(CWnd* parent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(PrnPageRangeDlg)
	enum { IDD = IDD_PRN_PAGE_RANGE };
	int		page_range_;
	CEdit	edit_page_range_;
	CString	page_range_str_;
	//}}AFX_DATA

	bool Create(CWnd* parent, int placeholder_id);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(PrnPageRangeDlg)
protected:
	virtual void DoDataExchange(CDataExchange* DX);    // DDX/DDV support
	//}}AFX_VIRTUAL
	afx_msg void DisablePagesEditBox();

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(PrnPageRangeDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelPagesClicked();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PRNPAGERANGEDLG_H__C2FAF3A4_EC5C_456F_9AFC_F377240F6AF5__INCLUDED_)

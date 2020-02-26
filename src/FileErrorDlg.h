/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#if !defined(AFX_FILEERRORDLG_H__85B10D95_04E9_4256_A848_F88C6236FF18__INCLUDED_)
#define AFX_FILEERRORDLG_H__85B10D95_04E9_4256_A848_F88C6236FF18__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FileErrorDlg.h : header file
//
#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// FileErrorDlg dialog

class FileErrorDlg : public CDialog
{
// Construction
public:
	FileErrorDlg(CWnd* parent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(FileErrorDlg)
	enum { IDD = IDD_FILE_ERROR };
	CString	backup_;
	CString	original_;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(FileErrorDlg)
protected:
	virtual void DoDataExchange(CDataExchange* DX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(FileErrorDlg)
	virtual BOOL OnInitDialog();
	afx_msg BOOL OnEraseBkgnd(CDC* dc);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	CFont title_fnt_;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FILEERRORDLG_H__85B10D95_04E9_4256_A848_F88C6236FF18__INCLUDED_)

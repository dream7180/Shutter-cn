/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#if !defined(AFX_EXPORTEXIFDLG_H__A32F3E00_D594_471C_AE32_19B441A441AE__INCLUDED_)
#define AFX_EXPORTEXIFDLG_H__A32F3E00_D594_471C_AE32_19B441A441AE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ExportExifDlg.h : header file
//
#include "DialogChild.h"
#include "PathEdit.h"

/////////////////////////////////////////////////////////////////////////////
// CExportExifDlg dialog

class CExportExifDlg : public DialogChild
{
// Construction
public:
	CExportExifDlg(bool all, CWnd* parent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CExportExifDlg)
	enum { IDD = IDD_EXPORT };
	CEdit	separator_wnd_;
	CPathEdit	out_file_wnd_;
	CString	separator_;
	CString	out_file_;
	CString	expl_;
	//}}AFX_DATA
	bool all_;
	BOOL tags_;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CExportExifDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* DX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CExportExifDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelectFile();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	virtual void OnOK();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EXPORTEXIFDLG_H__A32F3E00_D594_471C_AE32_19B441A441AE__INCLUDED_)

/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#if !defined(AFX_COPYTAGGEDDLG_H__C4B95DBD_25BE_4114_B101_64972C793604__INCLUDED_)
#define AFX_COPYTAGGEDDLG_H__C4B95DBD_25BE_4114_B101_64972C793604__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CopyTaggedDlg.h : header file
//
#include "DialogChild.h"
#include "Profile.h"
#include "PathEdit.h"

/////////////////////////////////////////////////////////////////////////////
// CCopyTaggedDlg dialog

class CCopyTaggedDlg : public DialogChild
{
// Construction
public:
	CCopyTaggedDlg(CWnd* parent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CCopyTaggedDlg)
	enum { IDD = IDD_COPY_TAGGED };
	CString	path_;
	int		separate_folders_;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCopyTaggedDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* DX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	Profile<bool> profile_separate_folders_;
	Profile<CString> profile_dest_path_;
	CPathEdit edit_path_;

	// Generated message map functions
	//{{AFX_MSG(CCopyTaggedDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnBrowse();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COPYTAGGEDDLG_H__C4B95DBD_25BE_4114_B101_64972C793604__INCLUDED_)

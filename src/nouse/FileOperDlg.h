/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#if !defined(AFX_FILEOPERDLG_H__A5B7B841_7662_4A23_8F08_7390B4779B87__INCLUDED_)
#define AFX_FILEOPERDLG_H__A5B7B841_7662_4A23_8F08_7390B4779B87__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "DialogChild.h"
#include "PathEdit.h"

// FileOperDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// FileOperDlg dialog

class FileOperDlg : public DialogChild
{
// Construction
public:
	FileOperDlg(bool copy, const TCHAR* dest_path, CWnd* parent= NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(FileOperDlg)
	enum { IDD = IDD_COPYMOVE };
	CButton	btn_ok_;
	CString	label_;
	CString	path_;
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(FileOperDlg)
protected:
	virtual void DoDataExchange(CDataExchange* DX);    // DDX/DDV support
	//}}AFX_VIRTUAL

public:
	const TCHAR* DestPath() const	{ return path_; }

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(FileOperDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnBrowse();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	bool copy_operation_;
	CPathEdit edit_path_;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FILEOPERDLG_H__A5B7B841_7662_4A23_8F08_7390B4779B87__INCLUDED_)

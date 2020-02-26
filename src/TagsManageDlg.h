/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#if !defined(AFX_TAGSMANAGEDLG_H__4A569A0D_1A1F_4420_9EF6_AD9B4ED543D9__INCLUDED_)
#define AFX_TAGSMANAGEDLG_H__4A569A0D_1A1F_4420_9EF6_AD9B4ED543D9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TagsManageDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTagsManageDlg dialog

class CTagsManageDlg : public CDialog
{
// Construction
public:
	CTagsManageDlg(CWnd* parent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CTagsManageDlg)
	enum { IDD = IDD_TAGS };
	CEdit	edit_tags_;
	CString	tags_;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTagsManageDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* DX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CTagsManageDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TAGSMANAGEDLG_H__4A569A0D_1A1F_4420_9EF6_AD9B4ED543D9__INCLUDED_)

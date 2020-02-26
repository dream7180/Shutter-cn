/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#if !defined(AFX_TOOLDLG_H__13781468_7191_4D37_8A4A_16885C9DCACB__INCLUDED_)
#define AFX_TOOLDLG_H__13781468_7191_4D37_8A4A_16885C9DCACB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "RDialog.h"
// ToolDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CToolDlg dialog

class CToolDlg : public RDialog
{
// Construction
public:
	CToolDlg(const TCHAR* name, CWnd* parent= NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CToolDlg)
	enum { IDD = IDD_TOOL };
	CStatic	file_wnd_;
	CProgressCtrl progress_wnd_;
	//}}AFX_DATA

	void SetProgress(int count)		{ progress_wnd_.SetRange32(0, count); progress_wnd_.SetStep(1);  }
	void Step()						{ progress_wnd_.StepIt(); progress_wnd_.UpdateWindow(); }
	void SetFile(const TCHAR* file)	{ file_wnd_.SetWindowText(file); file_wnd_.UpdateWindow(); }

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CToolDlg)
protected:
	virtual void DoDataExchange(CDataExchange* DX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CToolDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	String caption_;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TOOLDLG_H__13781468_7191_4D37_8A4A_16885C9DCACB__INCLUDED_)

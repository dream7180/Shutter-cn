/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#if !defined(AFX_RDIALOG_H__341C7C8A_570F_4CC7_9FE4_9613E2C7488E__INCLUDED_)
#define AFX_RDIALOG_H__341C7C8A_570F_4CC7_9FE4_9613E2C7488E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// RDialog dialog

class RDialog : public CDialog
{
// Construction
public:
	RDialog(int dlg_id, CWnd* parent= 0);

// Dialog Data
	//{{AFX_DATA(RDialog)
//	enum { IDD = _UNKNOWN_RESOURCE_ID_ };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(RDialog)
public:
	virtual INT_PTR DoModal();
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(RDialog)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	int dlg_id_;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RDIALOG_H__341C7C8A_570F_4CC7_9FE4_9613E2C7488E__INCLUDED_)

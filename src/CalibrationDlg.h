/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#if !defined(AFX_CALIBRATIONDLG_H__44AD0A5F_AD57_4663_BA95_238059B52799__INCLUDED_)
#define AFX_CALIBRATIONDLG_H__44AD0A5F_AD57_4663_BA95_238059B52799__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CalibrationDlg.h : header file
//
#include "CalibrationWnd.h"

/////////////////////////////////////////////////////////////////////////////
// CalibrationDlg dialog

class CalibrationDlg : public CDialog
{
// Construction
public:
	CalibrationDlg(CWnd* parent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CalibrationDlg)
	enum { IDD = IDD_CALIBRATION };
	CEdit	editY_res_;
	CEdit	editX_res_;
	double	X_res_;
	double	Y_res_;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CalibrationDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* DX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CalibrationDlg)
	virtual BOOL OnInitDialog();
	afx_msg BOOL OnEraseBkgnd(CDC* dc);
	afx_msg void OnSize(UINT type, int cx, int cy);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* MMI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	int header_bottom_;
	CalibrationWnd circle_wnd_;
	CSize min_wnd_size_size_;

	void Resize();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CALIBRATIONDLG_H__44AD0A5F_AD57_4663_BA95_238059B52799__INCLUDED_)

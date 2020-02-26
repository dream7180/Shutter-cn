/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#if !defined(AFX_CALIBRATIONWND_H__C81C44DA_AB53_4A8B_8D8F_B55C4D116273__INCLUDED_)
#define AFX_CALIBRATIONWND_H__C81C44DA_AB53_4A8B_8D8F_B55C4D116273__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CalibrationWnd.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CalibrationWnd window

class CalibrationWnd : public CWnd
{
// Construction
public:
	CalibrationWnd();

// Attributes
public:
	void GetResolution(double& X_res, double& Y_res) const;
	CSize CalcSize(double X_res, double Y_res) const;

// Operations
public:
	bool Create(CWnd* parent);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CalibrationWnd)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CalibrationWnd();

	// Generated message map functions
protected:
	//{{AFX_MSG(CalibrationWnd)
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* dc);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	enum { MARGIN= 50, BOTTOM= 20 };
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CALIBRATIONWND_H__C81C44DA_AB53_4A8B_8D8F_B55C4D116273__INCLUDED_)

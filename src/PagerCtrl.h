/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#if !defined(AFX_PAGERCTRL_H__32821E95_BBF1_42A9_80A2_2E63A5E89DEE__INCLUDED_)
#define AFX_PAGERCTRL_H__32821E95_BBF1_42A9_80A2_2E63A5E89DEE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PagerCtrl.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// PagerCtrlEx window

class PagerCtrlEx : public CWnd
{
// Construction
public:
	PagerCtrlEx();

// Attributes
public:

// Operations
public:
	bool Create(CWnd* parent, int id);

	void SetSize(CSize child_size);

	void SetChild(HWND child);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(PagerCtrlEx)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~PagerCtrlEx();

	// Generated message map functions
protected:
	//{{AFX_MSG(PagerCtrlEx)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	void OnCalcSize(NMHDR* hdr, LRESULT* result);
	void OnPageScroll(NMHDR* hdr, LRESULT* result);
	CSize child_size_;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PAGERCTRL_H__32821E95_BBF1_42A9_80A2_2E63A5E89DEE__INCLUDED_)

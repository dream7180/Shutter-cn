/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#if !defined(AFX_PRNPHOTOOPTIONSDLG_H__A47DE4D8_8291_4D2E_9A5F_F7EF7DB7B29A__INCLUDED_)
#define AFX_PRNPHOTOOPTIONSDLG_H__A47DE4D8_8291_4D2E_9A5F_F7EF7DB7B29A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PrnPhotoOptionsDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// PrnPhotoOptionsDlg dialog

class PrnPhotoOptionsDlg : public CDialog
{
// Construction
public:
	PrnPhotoOptionsDlg(CWnd* parent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(PrnPhotoOptionsDlg)
	enum { IDD = IDD_PRN_PHOTO_OPTIONS };
	CEdit	edit_copies_;
	CSpinButtonCtrl	spin_wnd_;
	int		copies_;
	CSpinButtonCtrl	spin_wnd_2_;
	CEdit	edit_zoom_;
	int		zoom_;
	//}}AFX_DATA

	bool Create(CWnd* parent, CWnd& placeholder_wnd);

	void SetZoom(int zoom);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(PrnPhotoOptionsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* DX);    // DDX/DDV support
	//}}AFX_VIRTUAL

	enum { MIN_COPIES= 1, MAX_COPIES= 99 };
	enum { MIN_ZOOM= 1, MAX_ZOOM= 999 };

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(PrnPhotoOptionsDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeCopies();
	void OnChangeZoom();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PRNPHOTOOPTIONSDLG_H__A47DE4D8_8291_4D2E_9A5F_F7EF7DB7B29A__INCLUDED_)

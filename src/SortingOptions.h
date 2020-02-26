/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#if !defined(AFX_SORTINGOPTIONS_H__15BCF1D4_FD3C_11D4_8E79_00B0D078DE24__INCLUDED_)
#define AFX_SORTINGOPTIONS_H__15BCF1D4_FD3C_11D4_8E79_00B0D078DE24__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SortingOptions.h : header file
//
#include "CloseBar.h"

/////////////////////////////////////////////////////////////////////////////
// SortingOptions dialog

class CSortingOptionsNotification
{
public:
	virtual void SetColorVsShapeWeight(float weight)= 0;
};


class SortingOptions : public CDialog
{
// Construction
public:
	SortingOptions(CSortingOptionsNotification* ctrl, float weight);

	// Create modal dialog
	bool Create(CWnd* parent);

// Dialog Data
	//{{AFX_DATA(SortingOptions)
	enum { IDD = IDD_SORT_OPT };
	CStatic	image_wnd_;
	CSliderCtrl	slider_wnd_;
	int		thumbnail_size_;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(SortingOptions)
protected:
	virtual void DoDataExchange(CDataExchange* DX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(SortingOptions)
	virtual BOOL OnInitDialog();
	afx_msg void OnKillFocus(CWnd* new_wnd);
	afx_msg void OnActivate(UINT state, CWnd* wnd_other, BOOL minimized);
	virtual void OnCancel();
	virtual void OnOK();
	afx_msg void OnVScroll(UINT sb_code, UINT pos, CScrollBar* scroll_bar);
	afx_msg void OnClose();
	afx_msg BOOL OnEraseBkgnd(CDC* dc);
	afx_msg HBRUSH OnCtlColor(CDC* dc, CWnd* wnd, UINT ctl_color);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
//	CPoint left_top_;
	bool ready_;
	bool exit_;
	CSortingOptionsNotification* ctrl_;
	CloseBar close_wnd_;
	float weight_;
	CBrush br_back_;

	void Finished();
};


class CSortingOptionsPopup : public CMiniFrameWnd
{
// Construction
public:
	CSortingOptionsPopup(CPoint left_top, CWnd* parent, CSortingOptionsNotification* ctrl, float weight);

	virtual ~CSortingOptionsPopup();

	// Create modal dialog
	bool Create();

	SortingOptions dlg_options_;

	static bool IsPopupActive();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSortingOptionsPopup)
	virtual void PostNcDestroy();
	virtual BOOL ContinueModal();
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSortingOptionsPopup)
	afx_msg void OnKillFocus(CWnd* new_wnd);
	afx_msg void OnActivate(UINT state, CWnd* wnd_other, BOOL minimized);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	LRESULT OnFinish(WPARAM, LPARAM);

private:
	CPoint left_top_;
	bool run_;
	static CSortingOptionsPopup* running_instance_;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SORTINGOPTIONS_H__15BCF1D4_FD3C_11D4_8E79_00B0D078DE24__INCLUDED_)

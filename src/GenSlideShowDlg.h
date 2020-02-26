/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#if !defined(AFX_GENSLIDESHOWDLG_H__0C4F921D_9DFA_4185_8136_1DAB5AED5E0D__INCLUDED_)
#define AFX_GENSLIDESHOWDLG_H__0C4F921D_9DFA_4185_8136_1DAB5AED5E0D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GenSlideShowDlg.h : header file
//
#include "DialogChild.h"
#include "PathEdit.h"

/////////////////////////////////////////////////////////////////////////////
// GenSlideShowDlg dialog

class GenSlideShowDlg : public DialogChild
{
// Construction
public:
	GenSlideShowDlg(CWnd* parent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(GenSlideShowDlg)
	enum { IDD = IDD_GEN_SLIDE_SHOW };
	CSpinButtonCtrl	spin_delay_;
	CEdit	edit_delay_;
	CStatic	compr_ratio_wnd_;
	CSpinButtonCtrl	spin_compr_level_;
	CEdit	edit_compr_level_;
	CSliderCtrl	slider_size_;
	CSliderCtrl	slider_quality_;
	CEdit	edit_width_;
	CEdit	edit_height_;
	CPathEdit	edit_dest_path_;
	int		size_percent_;
	int		width_;
	int		height_;
	CString	dest_path_;
	int		size_;
	int		quality_;
	int		compr_level_;
	UINT	delay_;
	BOOL	full_screen_;
	BOOL loopRepeatedly_;
	//}}AFX_DATA

	const TCHAR* GetDestPath() const	{ return dest_path_; }
	int GetJPEGQuality() const			{ return compr_level_; }


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(GenSlideShowDlg)
protected:
	virtual void DoDataExchange(CDataExchange* DX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(GenSlideShowDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSameDir();
	afx_msg void OnSelectDir();
	afx_msg void OnFixedSize();
	afx_msg void OnPercentageSize();
	afx_msg void OnBrowse();
	virtual void OnOK();
	afx_msg void OnChangeHeight();
	afx_msg void OnChangeWidth();
	afx_msg void OnVScroll(UINT sb_code, UINT pos, CScrollBar* scroll_bar);
	afx_msg void OnChangeComprLevel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void EnableCtrl(CWnd* wnd, bool enable, bool hide_disabled= true);
	void ReadOnlyCtrl(CEdit* wnd, bool read_only);
	void UpdateDirs();
	void UpdateDims();
	void UpdateFmt();
	void UpdateQuality();
	void UpdateSize();
	double ratio_;
	bool update_;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GENSLIDESHOWDLG_H__0C4F921D_9DFA_4185_8136_1DAB5AED5E0D__INCLUDED_)

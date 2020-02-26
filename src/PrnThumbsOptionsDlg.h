/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#if !defined(AFX_PRNTHUMBSOPTIONSDLG_H__C1C3CF73_63C8_401F_A974_CEA34DF0BF80__INCLUDED_)
#define AFX_PRNTHUMBSOPTIONSDLG_H__C1C3CF73_63C8_401F_A974_CEA34DF0BF80__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PrnThumbsOptionsDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// PrnThumbsOptionsDlg dialog

class PrnThumbsOptionsDlg : public CDialog
{
// Construction
public:
	PrnThumbsOptionsDlg(CWnd* parent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(PrnThumbsOptionsDlg)
	enum { IDD = IDD_PRN_THUMBS_OPTIONS };
	CSpinButtonCtrl	spin_wnd_;
	CEdit	edit_items_;
	CSliderCtrl	slider_wnd_;
	BOOL	print_footer_;
	BOOL	print_footer_text_;
	int		items_across_;
	//}}AFX_DATA
	CString footer_text_;
	CEdit	edit_footer_text_;
	LOGFONT font_;
	int print_option_;

	bool Create(CWnd* parent, CWnd& placeholder_wnd);

	enum { MIN_ITEMS= 5, MAX_ITEMS= 20 };

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(PrnThumbsOptionsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* DX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(PrnThumbsOptionsDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeItems();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	void OnHScroll(UINT sb_code, UINT pos, CScrollBar* scroll_bar);
	afx_msg void OnFooter();
	void ItemsNumberChanges(int items);
	void SetFontInfo(const LOGFONT& lf);
	void OnFont();
	void OnPrintOptions();

	bool ready_;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PRNTHUMBSOPTIONSDLG_H__C1C3CF73_63C8_401F_A974_CEA34DF0BF80__INCLUDED_)

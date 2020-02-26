/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#if !defined(AFX_TRANSFERDLG_H__179497BB_E579_4D4B_9A36_DBDCDC6C7649__INCLUDED_)
#define AFX_TRANSFERDLG_H__179497BB_E579_4D4B_9A36_DBDCDC6C7649__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TransferDlg.h : header file
//
#include "DialogChild.h"
#include "ToolBarWnd.h"
#include "PathEdit.h"

/////////////////////////////////////////////////////////////////////////////
// CTransferDlg dialog

class CTransferDlg : public CDialogChild
{
// Construction
public:
	CTransferDlg(CWnd* parent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CTransferDlg)
	enum { IDD = IDD_TRANSFER };
	CStatic	label_to_wnd_;
	CButton	btn_to_;
	CPathEdit	edit_to_;
	CPathEdit	edit_from_;
	CPathEdit	edit_pattern_;
	CStatic	example_wnd_;
	CString	pattern_;
	CString	from_;
	CString	to_;
	int		operation_;
	BOOL	rename_;
	BOOL	read_only_;
	//}}AFX_DATA
	CToolBarWnd symbols_wnd_;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTransferDlg)
protected:
	virtual void DoDataExchange(CDataExchange* DX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void OnTbDropDown(NMHDR* nmhdr, LRESULT* result);

	// Generated message map functions
	//{{AFX_MSG(CTransferDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnChangePattern();
	afx_msg void OnFromBtn();
	afx_msg void OnToBtn();
	virtual void OnOK();
	afx_msg void OnRename();
	afx_msg void OnCopy();
	afx_msg void OnMove();
	afx_msg void OnBatch();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void SymbolsPopupMenu();
	void OnBrowse(CWnd& wnd, const TCHAR* caption);
	void UpdatePatternCtrl();
	void OperationChanged();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TRANSFERDLG_H__179497BB_E579_4D4B_9A36_DBDCDC6C7649__INCLUDED_)

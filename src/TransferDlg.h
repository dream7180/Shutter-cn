/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
// TransferDlg.h : header file

#include "DialogChild.h"


class CTransferDlg : public DialogChild
{
// Construction
public:
	CTransferDlg(CWnd* parent, const TCHAR* src_path);
	virtual ~CTransferDlg();

	String GetDestPath() const;
// Dialog Data
	//{{AFX_DATA(CTransferDlg)
	enum { IDD = IDD_TRANSFER };
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTransferDlg)
protected:
	virtual void DoDataExchange(CDataExchange* DX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
//	void OnTbDropDown(NMHDR* nmhdr, LRESULT* result);

	// Generated message map functions
	//{{AFX_MSG(CTransferDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnFromBtn();
	afx_msg void OnToBtn();
	virtual void OnOK();
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
	BOOL InitDlg();
	BOOL OnEraseBkgnd(CDC* dc);
	void OnOptions();
	void OnTbDropDown(NMHDR* nmhdr, LRESULT* result);
	void OnChangePattern();
	void OnRename();
	virtual BOOL ContinueModal();

	struct Impl;
	std::auto_ptr<Impl> pImpl_;
};

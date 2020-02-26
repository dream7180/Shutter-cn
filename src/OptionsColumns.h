/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#if !defined(AFX_OPTIONSCOLUMNS_H__BA03894C_71AF_49C6_AEB0_5CDFC6635D16__INCLUDED_)
#define AFX_OPTIONSCOLUMNS_H__BA03894C_71AF_49C6_AEB0_5CDFC6635D16__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OptionsColumns.h : header file
//
#include "RPropertyPage.h"
#include "ColumnTree.h"
#include <boost/function.hpp>

/////////////////////////////////////////////////////////////////////////////
// OptionsColumns dialog

class OptionsColumns : public RPropertyPage, public ColumnTree
{
// Construction
public:
	OptionsColumns(Columns& columns);   // standard constructor

// Dialog Data
	//{{AFX_DATA(OptionsColumns)
	enum { IDD = IDD_OPTIONS_COLUMNS };
//	CTreeCtrl	tree_wnd_;
	//}}AFX_DATA
//	vector<uint16>* pv_columns_;
//	vector<uint16> selected_;
//	bool changed_;

	boost::function<void (CWnd* parent)> define_columns_;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(OptionsColumns)
protected:
	virtual void DoDataExchange(CDataExchange* DX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(OptionsColumns)
	virtual BOOL OnInitDialog();
	afx_msg void OnReset();
	afx_msg void OnResetCanon();
	afx_msg void OnResetNikon();
	afx_msg void OnResetFuji();
	afx_msg void OnResetOlympus();
	//}}AFX_MSG
	void OnDefineCustomColumns();
	DECLARE_MESSAGE_MAP()

private:
/*	bool ColumnsChanged() const;

	vector<HTREEITEM> leaves_;

	HTREEITEM item_common_;
	HTREEITEM item_canon_;
	HTREEITEM item_nikon_;
	HTREEITEM item_fuji_;
	HTREEITEM item_olympus_;

	void SetCheck(int index, bool check);
	void RemoveCheckBox(HTREEITEM item);
	void Reset(HTREEITEM skip_this); */
void OptionsColumns::OnItemChanged(NMHDR* nmhdr, LRESULT* result);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OPTIONSCOLUMNS_H__BA03894C_71AF_49C6_AEB0_5CDFC6635D16__INCLUDED_)

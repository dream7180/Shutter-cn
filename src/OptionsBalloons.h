/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#if !defined(AFX_OPTIONSBALLOONS_H__D9A78A45_79E9_4CE1_B250_28ACE07CB48D__INCLUDED_)
#define AFX_OPTIONSBALLOONS_H__D9A78A45_79E9_4CE1_B250_28ACE07CB48D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OptionsBalloons.h : header file
//
#include "ColumnTree.h"
#include "LinkWnd.h"
#include "RPropertyPage.h"

/////////////////////////////////////////////////////////////////////////////
// OptionsBalloons dialog

class OptionsBalloons : public RPropertyPage, public ColumnTree
{
// Construction
public:
	OptionsBalloons(Columns& columns);
	~OptionsBalloons();

// Dialog Data
	//{{AFX_DATA(OptionsBalloons)
	enum { IDD = IDD_OPTIONS_BALLOONS };
	CStatic	shape_wnd_;
	//}}AFX_DATA
	CLinkWnd reset_wnd_;

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(OptionsBalloons)
protected:
	virtual void DoDataExchange(CDataExchange* DX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(OptionsBalloons)
	afx_msg BOOL OnEraseBkgnd(CDC* dc);
	virtual BOOL OnInitDialog();
	afx_msg void OnReset();
	//}}AFX_MSG
	void OnSize(UINT type, int cx, int cy);

	DECLARE_MESSAGE_MAP()
};

#endif // !defined(AFX_OPTIONSBALLOONS_H__D9A78A45_79E9_4CE1_B250_28ACE07CB48D__INCLUDED_)

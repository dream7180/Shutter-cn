/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#if !defined(AFX_RPROPERTYPAGE_H__ECF5622F_B457_4F52_AC5F_2E55F763912B__INCLUDED_)
#define AFX_RPROPERTYPAGE_H__ECF5622F_B457_4F52_AC5F_2E55F763912B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RPropertyPage.h : header file
//
#include "DlgAutoResize.h"

/////////////////////////////////////////////////////////////////////////////
// RPropertyPage dialog

class RPropertyPage : public CPropertyPage
{
// Construction
public:
	RPropertyPage(int dlg_id);
	~RPropertyPage();

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(RPropertyPage)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	void OnSize(UINT type, int cx, int cy);

	DlgAutoResize& ResizeMgr()		{ return resize_; }

private:
	int dlg_id_;
	HGLOBAL dlg_;
	DlgAutoResize resize_;
};


#endif // !defined(AFX_RPROPERTYPAGE_H__ECF5622F_B457_4F52_AC5F_2E55F763912B__INCLUDED_)

/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#if !defined(AFX_INFOTOOLBAR_H__E7FF2965_8DEF_46BB_94B8_B69168913392__INCLUDED_)
#define AFX_INFOTOOLBAR_H__E7FF2965_8DEF_46BB_94B8_B69168913392__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// InfoToolbar.h : header file
//
#include "ToolBarWnd.h"
#include "HorzReBar.h"


/////////////////////////////////////////////////////////////////////////////
// CInfoToolbar window

class CInfoToolbar : public CHorzReBar
{
// Construction
public:
	CInfoToolbar();

// Attributes
public:

// Operations
public:
	bool Create(CWnd* parent, UINT id);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CInfoToolbar)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CInfoToolbar();

	// Generated message map functions
protected:
	//{{AFX_MSG(CInfoToolbar)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

	CToolBarWnd tool_bar_wnd_;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_INFOTOOLBAR_H__E7FF2965_8DEF_46BB_94B8_B69168913392__INCLUDED_)

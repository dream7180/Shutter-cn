/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#if !defined(AFX_SLIDERTOOLBAR_H__53CBE74D_9C02_41D6_BED2_0569360DC674__INCLUDED_)
#define AFX_SLIDERTOOLBAR_H__53CBE74D_9C02_41D6_BED2_0569360DC674__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SliderToolBar.h : header file
//
#include "ToolBarWnd.h"

/////////////////////////////////////////////////////////////////////////////
// SliderToolBar window

class SliderToolBar : public ToolBarWnd
{
// Construction
public:
	SliderToolBar();

// Attributes
public:

// Operations
public:
	bool Create(CWnd* parent, UINT id);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(SliderToolBar)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~SliderToolBar();

	// Generated message map functions
protected:
	//{{AFX_MSG(SliderToolBar)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SLIDERTOOLBAR_H__53CBE74D_9C02_41D6_BED2_0569360DC674__INCLUDED_)

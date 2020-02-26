/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#if !defined(AFX_CLOSEBAR_H__1942F408_DB1E_40E2_B7D9_E4DF6CCD0F88__INCLUDED_)
#define AFX_CLOSEBAR_H__1942F408_DB1E_40E2_B7D9_E4DF6CCD0F88__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CloseBar.h : header file
//
#include "ToolBarWnd.h"

/////////////////////////////////////////////////////////////////////////////
// CloseBar panel for a full-screen mode

class CloseBar : public ToolBarWnd
{
public:
	CloseBar() {}

	bool Create(CWnd* parent, bool close_and_restore= true);

	virtual CString GetToolTip(int cmd_id);

	static const int CLOSE_BAR_WIDTH= 46;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CLOSEBAR_H__1942F408_DB1E_40E2_B7D9_E4DF6CCD0F88__INCLUDED_)

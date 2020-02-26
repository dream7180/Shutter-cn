/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#if !defined(AFX_TOOLBARPOPUP_H__87897ECA_A4E7_42E8_9AC0_2363F28D2630__INCLUDED_)
#define AFX_TOOLBARPOPUP_H__87897ECA_A4E7_42E8_9AC0_2363F28D2630__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ToolBarPopup.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CToolBarPopup window

class CToolBarPopup : public CMiniFrameWnd
{
// Construction
public:
	CToolBarPopup(CWnd& tool_bar_wnd, CSize size, CWnd* owner);

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CToolBarPopup)
	protected:
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CToolBarPopup();

	// Generated message map functions
protected:
	//{{AFX_MSG(CToolBarPopup)
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* dc);
	afx_msg void OnKillFocus(CWnd* new_wnd);
	afx_msg void OnParentNotify(UINT message, LPARAM lParam);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	LRESULT OnIdleUpdateCmdUI(WPARAM wParam, LPARAM lParam);
	LRESULT OnTbClicked(WPARAM hwnd, LPARAM code);
	LRESULT OnCloseWnd(WPARAM hwnd, LPARAM code);

private:
	CWnd* tool_bar_;
	CWnd* tb_parent_;
	CWnd* owner_wnd_;
	CRect original_pos_rect_;

	static CToolBarPopup* tool_bar_popup_;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TOOLBARPOPUP_H__87897ECA_A4E7_42E8_9AC0_2363F28D2630__INCLUDED_)

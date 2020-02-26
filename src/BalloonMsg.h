/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#if !defined(AFX_BALLOONMSG_H__F95E055D_D8B9_4385_8ADF_214F2B27DD49__INCLUDED_)
#define AFX_BALLOONMSG_H__F95E055D_D8B9_4385_8ADF_214F2B27DD49__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BalloonMsg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// BalloonMsg window

class BalloonMsg : public CToolTipCtrl
{
// Construction
public:
	enum MsgIcon { INONE, IINFO, IWARNING, IERROR };

	BalloonMsg(CWnd* wnd, const TCHAR* title, const TCHAR* msg, MsgIcon icon, CPoint pos= CPoint(0,0));

// Attributes
public:

// Operations
public:
	void Close();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(BalloonMsg)
	protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~BalloonMsg();

	// Generated message map functions
protected:
	//{{AFX_MSG(BalloonMsg)
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

	void OnGetDispInfo(NMHDR* nm_hdr, LRESULT* result);

private:
	CString msg_;
	CString title_;
	bool closing_;

	afx_msg void OnTimer(UINT_PTR id_event);
	virtual void PostNcDestroy();

	static void Unsubclass();
	void SubclassWnd(HWND wnd);
	void InstallMouseHook();
	void UninstallMouseHook();
	LRESULT OnWmUser(WPARAM, LPARAM);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BALLOONMSG_H__F95E055D_D8B9_4385_8ADF_214F2B27DD49__INCLUDED_)

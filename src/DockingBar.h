/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#if !defined(AFX_DOCKINGBAR_H__1822BF89_6C8F_4A0C_A52D_384E7B799697__INCLUDED_)
#define AFX_DOCKINGBAR_H__1822BF89_6C8F_4A0C_A52D_384E7B799697__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DockingBar.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDockingBar window

class CTab
{
public:
	enum IconType { ICN_FOLDER, ICN_CAMERA, ICN_PHOTO };

	CTab() : icon_(ICN_FOLDER), active_(false), MDI_child_(0)
	{ location_rect_.SetRectEmpty(); }

	CTab(const TCHAR* name, int icon, HWND MDI_child, bool active= false)
		: name_(name), icon_(IconType(icon)), active_(active), MDI_child_(MDI_child)
	{ location_rect_.SetRectEmpty(); }

	CString name_;
	IconType icon_;
	CRect location_rect_;
	bool active_;
	HWND MDI_child_;
};



class CDockingBar : public CWnd
{
// Construction
public:
	CDockingBar();

// Attributes
public:

// Operations
public:
	bool Create(CWnd* parent);

	void Refresh();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDockingBar)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CDockingBar();

	// Generated message map functions
protected:
	//{{AFX_MSG(CDockingBar)
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* dc);
	afx_msg void OnLButtonDown(UINT flags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	static CString wnd_class_;
	CImageList image_list_;

	vector<CTab> tabs_;
	void ColectTabs(vector<CTab>& tabs);
	void CalcWidth(vector<CTab>& tabs, CDC& dc, CRect wnd_rect);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DOCKINGBAR_H__1822BF89_6C8F_4A0C_A52D_384E7B799697__INCLUDED_)

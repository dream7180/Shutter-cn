/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#if !defined(AFX_MDIFRAMEX_H__739F04D8_64D8_11D5_8E8F_00B0D078DE24__INCLUDED_)
#define AFX_MDIFRAMEX_H__739F04D8_64D8_11D5_8E8F_00B0D078DE24__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MDIFrame.h : header file
//
#include "SnapFrame.h"
class CMDIFrame;
struct FramePageCreateContext;


class CFramePages : std::vector<CSnapFrame*>
{
public:
	CFramePages(CMDIFrame* parent);
	~CFramePages();

// Attributes
	// current page (if any)
	CSnapFrame* GetCurrent();

	// no of pages
	int GetCount() const			{ return size(); }

	// given page
	CSnapFrame* GetPage(int index);

	// current page index
	int GetCurrentIndex() const		{ return current_page_index_; }


// Operations
	// create new page
	int CreatePage(CWnd* parent, const TCHAR* title, int icon,
		const PaneLayoutInfoArray& PanesInfo, FramePageCreateContext* context);

	// select (activate & display) given page
	bool SelectPage(int index, bool change_notification);
	bool SelectPage(CSnapFrame* page);

	// destroy all page windows
	void DestroyPages();

	// activate next/previous page
	void NextPage();
	void PrevPage();

	// call InitialUpdateFrame() for each page
	void InitialUpdateFrames(CDocument* doc, bool make_visible);

	// resize all page widows
	void ResizeWindowsAndHide(const CRect& rect);

   CSnapFrame* FindPage(const TCHAR* title);
   void RemovePage(CSnapFrame* page);
 
private:
	int current_page_index_;
	CMDIFrame* parent_;
};


/////////////////////////////////////////////////////////////////////////////
// CMDIFrame frame

class CMDIFrame : public CMDIChildWnd
{
	DECLARE_DYNCREATE(CMDIFrame)
protected:
	CMDIFrame();           // protected constructor used by dynamic creation

// Attributes
public:
	CSnapFrame* GetCurrentPage()		{ return snap_frames_.GetCurrent(); }
	int GetCurrentPageIndex()			{ return snap_frames_.GetCurrentIndex(); }
	void SetCurrentPage(CSnapFrame* tab);

	CSnapFrame* GetPageFrame(int index);

	virtual CDocument* GetActiveDocument();

// Operations
public:
	void InitialUpdateFrames(CDocument* doc, bool make_visible);

	bool RemovePage(const TCHAR* title);

	// add new tab page
	int AddPage(const PaneLayoutInfoArray* panes_info, recipe_type recipe);

	// create all pane windows once pages are added
	void CreatePaneWindows(bool show_docking_bar);

	// notification: current page changed
	virtual void PageSelected(int page_index, const CString& title);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMDIFrame)
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void RecalcLayout(BOOL notify = TRUE);
	virtual BOOL OnCmdMsg(UINT id, int code, void* extra, AFX_CMDHANDLERINFO* handler_info);
	virtual void ActivateFrame(int cmd_show = -1);
protected:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* context);
	virtual void OnMDIActivate(BOOL activate, CWnd *activate_wnd, CWnd *deactivate_wnd);
	//}}AFX_VIRTUAL
	virtual void OnUpdateFrameTitle(BOOL add_to_title);

// Implementation
protected:
	virtual ~CMDIFrame();

// Generated message map functions
	//{{AFX_MSG(CMDIFrame)
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* dc);
	afx_msg void OnTabNext();
	afx_msg void OnTabPrev();
	afx_msg int OnCreate(LPCREATESTRUCT create_struct);
	afx_msg void OnClose();
	afx_msg void OnFileClose();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	CDockingBar tabs_wnd_;
	CFramePages snap_frames_;
	CDocument* document_;
	bool initialized_;	// set to true after OnInitialUpdate is sent

   CCreateContext create_context_;

	CMDIFrame* GetThis()	{ return this; }
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MDIFRAMEX_H__739F04D8_64D8_11D5_8E8F_00B0D078DE24__INCLUDED_)

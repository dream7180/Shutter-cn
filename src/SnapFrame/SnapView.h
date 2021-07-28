/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// SnapView.h : interface of the SnapView class
//
// SnapView is a frame window hosting pane window (typically CFormView).
// It also contains pane caption window.
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_SNAPVIEW_H__403E9469_5D71_4C6D_BE6C_E9DFD01696A8__INCLUDED_)
#define AFX_SNAPVIEW_H__403E9469_5D71_4C6D_BE6C_E9DFD01696A8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class SnapFrame;
#include "CaptionWindow.h"
struct PaneConstruction;
class ColorConfiguration;


class SnapView : public CFrameWnd
{
protected: // create from serialization only
	SnapView();
	DECLARE_DYNCREATE(SnapView)

public:
	enum Insert { INSERT_NONE, INSERT_LEFT, INSERT_RIGHT, INSERT_TOP, INSERT_BOTTOM };

// Attributes
public:
	void ModifyToolbar(bool wnd_maximized);

	Insert FindInsertPos(CPoint pos);

	bool IsMarkerChanged(Insert pos) const;

	CString GetTitle() const;

	SnapFrame* GetFrame() const			{ return frame_; }

	CString GetContextHelpTopic() const;

	// returns true if this is context help window
	bool IsContextHelpWnd() const			{ return is_context_help_wnd_; }

	CWnd* GetChildView() const				{ return child_view_; }

	// is view active?
	bool IsActive() const					{ return active_; }

	bool CanMaximize() const;

	// separator (resizing bar) width/height
	static CSize GetSeparatorThickness();

	bool IsPaneOpen();

	//void ChangeCaptionHeight(bool big);

	// color of separator between pane windows
	void SetSeparatorBaseColor(COLORREF color);

	void SetFaintCaptionEdge(bool faint);

	static CSize GetBarThickness();

// Operations
public:
	void EraseInsertMarker();
	void DrawInsertMarker(Insert pos);
	CRect GetMarkerRect(Insert pos, bool full= false) const;

	virtual void Show(bool visible, bool notification= false);

	bool TabChange();

	bool CreateClient(PaneConstruction* construction, UINT pane_flags);

	void InitialUpdate(const TCHAR* title, const TCHAR* ctx_help_topic);

	void Activate(bool active);

	// set title in caption window
	void SetTitle(const TCHAR* title);

	// display help in the client help window (if it is context help window)
	void DisplayHelp(const TCHAR* ctx_help_topic);

	// add a toolbar to the caption free space
	void AddBand(CWnd* toolbar, CWnd* owner, std::pair<int, int> min_max_width, bool resizable= false);

	// resets band's min/max width
	void ResetBandsWidth(std::pair<int, int> min_max_width);
	void ResetBandsWidth();

	// force repositioning
	void Resize();

	// invalidate caption window
	void InvalidateCaption();

	// color configuration has changed
	void ResetCaption(const ColorConfiguration& colors);

	// draw separator for preview purposes
	//static void DrawHorzSeparator(CDC* dc, COLORREF color, CRect rect);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(SnapView)
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnCmdMsg(UINT id, int code, void* extra, AFX_CMDHANDLERINFO* handler_info);
	//}}AFX_VIRTUAL
	virtual void OnUpdateFrameTitle(BOOL add_to_title);

// Implementation
public:
	virtual ~SnapView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

// Generated message map functions
protected:
	//{{AFX_MSG(SnapView)
	afx_msg BOOL OnEraseBkgnd(CDC* dc);
	afx_msg void OnLButtonDown(UINT flags, CPoint point);
	afx_msg void OnLButtonUp(UINT flags, CPoint point);
	afx_msg void OnMouseMove(UINT flags, CPoint point);
	afx_msg int OnCreate(LPCREATESTRUCT create_struct);
	afx_msg void OnSize(UINT type, int cx, int cy);
	afx_msg void OnPaneClose();
	afx_msg void OnPaneMaximize();
	afx_msg void OnPaneRestore();
	afx_msg BOOL OnSetCursor(CWnd* wnd, UINT hit_test, UINT message);
	afx_msg int OnMouseActivate(CWnd* desktop_wnd, UINT hit_test, UINT message);
	afx_msg void OnActivate(UINT state, CWnd* wnd_other, BOOL minimized);
	afx_msg void OnDestroy();
	afx_msg void OnPaneContextHelp();
	//}}AFX_MSG
	afx_msg BOOL OnHelp(UINT);
	DECLARE_MESSAGE_MAP()
	LRESULT OnXButtonDown(WPARAM wParam, LPARAM lParam);

private:
	COLORREF rgb_color_;

	bool moving_;
	bool resizing_;
	CPoint start_;
	Insert insert_pos_;		// where to insert moved window (relation to selected view)
	SnapView* view_next_to_;	// insert next to this view
	Insert resizing_edge_;		// resizing this edge
	COLORREF separator_base_color_;

	bool FindQuarter(CPoint first, CPoint second, CPoint test);

	// parent frame window
	SnapFrame* frame_;
	// child window contained by this snap view
	CWnd* child_view_;
	// document for a child view window
	CDocument* current_doc_;
	// is this wnd and child view active?
	bool active_;
	// context help topic (file name)
	CString ctx_help_topic_;
	// true if this is context help window
	bool is_context_help_wnd_;
	//
	COLORREF background_color_;

	Insert marker_;

	CaptionWindow caption_wnd_;

	Insert HitTest(CPoint pos) const;

	static HCURSOR resize_vert_;
	static HCURSOR resize_horz_;

	void OnGetInfoTip(NMHDR* nmhdr, LRESULT* result);	// provide tool tip text for toolbar
	CRect GetViewRect();
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SNAPVIEW_H__403E9469_5D71_4C6D_BE6C_E9DFD01696A8__INCLUDED_)

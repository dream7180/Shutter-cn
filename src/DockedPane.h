/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
#include <boost/function.hpp>

// DockedPane resizable pane

class DockedPane : public CWnd
{
public:
	enum PanelAlignment { PANEL_ALIGN_LEFT, PANEL_ALIGN_RIGHT };

	DockedPane(PanelAlignment alignment, int width);

// Attributes
public:
	bool IsVisible() const;

	int GetWidth() const		{ return width_; }

	void SetEraseCallback(const boost::function<void (CDC& dc, const CRect& rect)>& fn)	{ paint_fn_ = fn; }

// Operations
public:
	bool Create(CWnd* parent, UINT uId, int width);

	void ToggleVisibility();

	void Show(bool show);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(DockedPane)
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~DockedPane();

//virtual BOOL OnCmdMsg(UINT id, int code, void* extra, AFX_CMDHANDLERINFO* handler_info);

	// Generated message map functions
protected:
	//{{AFX_MSG(DockedPane)
	afx_msg void OnWindowPosChanging(WINDOWPOS FAR* lpwndpos);
	afx_msg void OnNcCalcSize(BOOL calc_valid_rects, NCCALCSIZE_PARAMS FAR* lpncsp);
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg void OnNcPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* dc);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	static CString wnd_class_;
	afx_msg LRESULT OnSizeParent(WPARAM, LPARAM lParam);
	afx_msg LRESULT OnEnterSizeMove(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnExitSizeMove(WPARAM wParam, LPARAM lParam);

	PanelAlignment alignment_;
	bool resizing_;
	int width_;
	boost::function<void (CDC& dc, const CRect& rect)> paint_fn_;
};

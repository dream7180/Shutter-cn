/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#if !defined(AFX_HORZREBAR_H__4EA78E49_B582_4AB6_9328_5E29C4F30C05__INCLUDED_)
#define AFX_HORZREBAR_H__4EA78E49_B582_4AB6_9328_5E29C4F30C05__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// HorzReBar.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// HorzReBar window

class HorzReBar : public CWnd
{
// Construction
public:
	HorzReBar();

// Attributes
public:
	int GetHeight() const;

	bool IsBandVisible(int band_id) const;

	CReBarCtrl& GetReBar()	{ return rebar_wnd_; }

	void SetBottomMargin(int margin);

	void SetBackgroundColor(COLORREF color);
	void SetTextColor(COLORREF color);

// Operations
public:
	bool Create(CWnd* parent, bool resizing_border, UINT id= AFX_IDW_REBAR);
	bool Create(CWnd* parent, bool resizing_border, DWORD styles, UINT id);

	bool AddBand(CWnd* child, int id= -1, bool break_to_new_line= false);
	bool AddBand(CWnd* child, CSize child_size, const TCHAR* label, int id, int min_width= 50, bool break_to_new_line= false);
	bool AddFixedBand(CWnd* child, int id, const TCHAR* label= 0);

	void MaximizeBand(UINT band, bool ideal_size= false);

	void ShowBand(int band_id, bool show= true);

	// registry operations: storing & restoring bands' layout
	bool StoreLayout(const TCHAR* section, const TCHAR* key);
	bool RestoreLayout(const TCHAR* section, const TCHAR* key);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(HorzReBar)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~HorzReBar();
	enum { HREBAR_CTRL_ID= 1111 };

	// Generated message map functions
protected:
	//{{AFX_MSG(HorzReBar)
	afx_msg void OnSize(UINT type, int cx, int cy);
	afx_msg void OnNcCalcSize(BOOL calc_valid_rects, NCCALCSIZE_PARAMS* lpncsp);
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg void OnNcPaint();
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* mmi);
	afx_msg void OnSizing(UINT fwSide, LPRECT rect);
	afx_msg void OnWindowPosChanging(WINDOWPOS* wnd_pos);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

private:
	static CString wnd_class_;
	LRESULT OnSizeParent(WPARAM, LPARAM lParam);
	BOOL OnReBarHeightChange(UINT id, NMHDR* nmhdr, LRESULT* result);
	CSize GetBarSize() const;
	void RebarCustomDraw(NMHDR* nmhdr, LRESULT* result);

	CReBarCtrl rebar_wnd_;
	bool resizing_border_;
	int bottom_margin_;
	int height_requested_;
	COLORREF background_;
	COLORREF text_color_;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HORZREBAR_H__4EA78E49_B582_4AB6_9328_5E29C4F30C05__INCLUDED_)

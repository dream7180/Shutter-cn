/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once

//#include "viewer/FancyToolBar.h"
#include "ToolBarWnd.h"

/////////////////////////////////////////////////////////////////////////////
// InfoBand window
class Columns;


struct InfoBandNotification
{
	virtual void FieldSelectionChanged() = 0;
	virtual void PopupMenu(bool displayed) = 0;
};


class InfoBand : public CWnd
{
// Construction
public:
	InfoBand(const Columns& columns);

// Attributes
public:
	// vector of selected fields (info to show)
	std::vector<uint16> fields_;
	//
	bool show_zoom_;
	bool show_name_;
	bool show_field_names_;
	COLORREF text_color_;
	COLORREF label_color_;

// Operations
public:
	bool Create(CWnd* parent, InfoBandNotification* recipient);

	static void DrawInfoText(CDC& dc, const CString& text, CRect rect, COLORREF rgb_text, COLORREF rgb_label, COLORREF rgb_default);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(InfoBand)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~InfoBand();

	// Generated message map functions
protected:
	//{{AFX_MSG(InfoBand)
	afx_msg BOOL OnEraseBkgnd(CDC* dc);
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT type, int cx, int cy);
	//afx_msg void OnContextMenu(CWnd* wnd, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	void OnTbDropDown(NMHDR* nmhdr, LRESULT* result);
	void Resize();

//	ToolBarWnd options_wnd_;
	int tool_bar_width_;
	InfoBandNotification* recipient_;
	ToolBarWnd options_;
	const Columns& columns_;

	void OptionsPopup(CPoint pos);
	void OnPopup();

	LRESULT OnPrintClient(WPARAM HDC, LPARAM flags);
	
private:
	CFont _font;
};

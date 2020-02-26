/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once

// InfoDisplay.h : header file
//
#include <boost/function.hpp>

class CustomHeaderCtrl : public CHeaderCtrl
{
public:
	CustomHeaderCtrl() : background_(0), text_color_(0)
	{}

	void SetColors(COLORREF backgnd, COLORREF text);

private:
	void OnCustomDraw(NMHDR* nm_hdr, LRESULT* result);
	DECLARE_MESSAGE_MAP()

	COLORREF background_;
	COLORREF text_color_;
};

/////////////////////////////////////////////////////////////////////////////
// InfoDisplay window

class InfoDisplay : public CListCtrl
{
// Construction
public:
	InfoDisplay();

// Attributes
public:
	void SetColors(COLORREF backgnd, COLORREF dark_backgnd, COLORREF text, COLORREF dim_text);

// Operations
public:
	typedef boost::function<void (int line, int col, TCHAR* text, int max_len)> GetTextFn;
	bool Create(CWnd* parent, const GetTextFn& get_text, COLORREF rgb_back, COLORREF rgb_back_dark);

	void SetInfo(int item_count);

// Implementation
public:
	virtual ~InfoDisplay();

	// Generated message map functions
protected:
	afx_msg void OnDestroy();
	afx_msg BOOL OnGetDispInfo(NMHDR* nmhdr, LRESULT* result);
	void OnChar(UINT chr, UINT rep_cnt, UINT flags);
	DECLARE_MESSAGE_MAP()

private:
	void OnCustomDraw(NMHDR* nm_hdr, LRESULT* result);
	void OnSize(UINT type, int cx, int cy);
	void OnEndTrack(NMHDR* /*nmhdr*/, LRESULT* result);

	COLORREF rgb_background_dark_;
	COLORREF text_color_;
	COLORREF dim_text_color_;
	GetTextFn get_text_;
	CustomHeaderCtrl header_;
};

/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// HistogramCtrl.h : header file
//
#include "Histogram.h"

class CHistogramNotifications
{
public:
	virtual void MouseClicked(int position, bool left) = 0;
};


/////////////////////////////////////////////////////////////////////////////
// HistogramCtrl window

class HistogramCtrl : public CWnd
{
// Construction
public:
	HistogramCtrl();

// Attributes
public:
	void SelectChannel(Histogram::ChannelSel channel);

	void SetHost(CHistogramNotifications* host)	{ host_ = host; }

	void SetDrawFlags(UINT flags);
	UINT GetDrawFlags() const						{ return drawing_flags_; }

	void SetLogarithmic(bool log);
	void SetRGBOverlaidOnly();

	bool IsLogarithmic() const						{ return histogram_.logarithmic_; }

// Operations
public:
	// clear histogram data
	void Clear();

	// build histogram for a dib
	void Build(const Dib& dib);
	void Build(const Dib& dib, const CRect& selection_rect);

	// if set two vertical lines will be drawn on the histogram at min & max position
	void SetEdgeLines(int min, int max);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(HistogramCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~HistogramCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(HistogramCtrl)
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* dc);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	Histogram histogram_;
	Histogram::ChannelSel channel_;
	CHistogramNotifications* host_;
	int last_histX_pos_;
	bool tracing_cursor_;
	UINT drawing_flags_;

	void SendMouseNotification(CPoint point);
	void OnLButtonDown(UINT flags, CPoint point);
	void OnLButtonUp(UINT flags, CPoint point);
	void OnMouseMove(UINT flags, CPoint point);
	void SetCursor();
	BOOL OnSetCursor(CWnd* wnd, UINT hit_test, UINT message);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

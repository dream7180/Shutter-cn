/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#if !defined(AFX_HISTOGRAMPANE_H__B515FC31_9977_4F6B_8AC7_25BE2FB655F9__INCLUDED_)
#define AFX_HISTOGRAMPANE_H__B515FC31_9977_4F6B_8AC7_25BE2FB655F9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// HistogramPane.h : header file
//
#include "Histogram.h"
#include "Pane.h"
#include "Profile.h"
class PhotoInfo;

/////////////////////////////////////////////////////////////////////////////
// HistogramPane window

class HistogramPane : public PaneWnd
{
// Construction
public:
	HistogramPane();

// Attributes
public:

// Operations
public:
	bool Create(CWnd* parent);

	void Update(PhotoInfoPtr photo);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(HistogramPane)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~HistogramPane();

	// Generated message map functions
protected:
	//{{AFX_MSG(HistogramPane)
	afx_msg BOOL OnEraseBkgnd(CDC* dc);
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	Histogram histogram_;
	CComboBox channels_wnd_;
	int channels_;
	Profile<int> profile_channel_;
	COLORREF rgb_backgnd_;

	bool LoadPhoto(const PhotoInfo& photo, Dib& bmp);
	void OnSelChangeChannel();

	void SetColors();

	// notifications
	virtual void CurrentChanged(PhotoInfoPtr photo);
	virtual void OptionsChanged(OptionsDlg& dlg);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HISTOGRAMPANE_H__B515FC31_9977_4F6B_8AC7_25BE2FB655F9__INCLUDED_)

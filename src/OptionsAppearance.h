/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#if !defined(AFX_OPTIONSAPPEARANCE_H__A0F988AC_92F1_4A1F_99E3_5BDC565D2256__INCLUDED_)
#define AFX_OPTIONSAPPEARANCE_H__A0F988AC_92F1_4A1F_99E3_5BDC565D2256__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// OptionsAppearance.h : header file
//
#include "RPropertyPage.h"
#include "ColorConfiguration.h"

/////////////////////////////////////////////////////////////////////////////
// OptionsAppearance property page dialog

class OptionsAppearance : public RPropertyPage
{
// Construction
public:
	OptionsAppearance();
	~OptionsAppearance();

// Dialog Data
	enum { IDD = IDD_OPTIONS_APPEARANCE };
	ColorConfiguration main_wnd_colors_;
	ColorConfiguration viewer_wnd_colors_;
	//ColorConfiguration pane_caption_colors_;
	double ui_gamma_correction_;
	LOGFONT description_font_;
	LOGFONT tag_font_;

// Overrides
protected:
	virtual void DoDataExchange(CDataExchange* DX);    // DDX/DDV support

// Implementation
protected:
	// Generated message map functions
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()

private:
	void UpdateDescFont();
	void UpdateColorsViewer();
	void UpdateColorsWnd();
	//void UpdateCaptionColors();
	void UpdateTagFont();
	void ChangeActiveElement();
	void OnReset();

	struct Impl;
	std::auto_ptr<Impl> impl_;

	void OnSize(UINT type, int cx, int cy);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OPTIONSAPPEARANCE_H__A0F988AC_92F1_4A1F_99E3_5BDC565D2256__INCLUDED_)

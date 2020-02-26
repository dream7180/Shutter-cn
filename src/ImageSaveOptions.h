/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
#include "Profile.h"
#include "HistogramCtrl.h"


// ImageSaveOptions dialog

class ImageSaveOptions : public CDialog
{
public:
	ImageSaveOptions(CWnd* parent = NULL);   // standard constructor
	virtual ~ImageSaveOptions();

	bool Create(CWnd* parent);

	void SaveOptions();

// Dialog Data
	enum { IDD = IDD_IMG_OPTIONS };

	int quality_;
	BOOL progressive_;
	BOOL base_line_;
	BOOL preserve_exif_;
	CString suffix_;
	HistogramCtrl histogram_wnd_;
	CWnd suffix_wnd_;
	CWnd label2_wnd_;

protected:
	virtual void DoDataExchange(CDataExchange* DX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	void OnChangeComprLevel();
	void OnHScroll(UINT sb_code, UINT pos, CScrollBar* scroll_bar);

	DECLARE_MESSAGE_MAP()

	CSliderCtrl	slider_quality_;
	CEdit edit_quality_;
	CSpinButtonCtrl spin_wnd_;
	Profile<int> profile_quality_;
	Profile<bool> profile_progressive_;
	Profile<bool> profile_base_line_;
	Profile<bool> profile_preserve_exif_;
	Profile<CString> profile_suffix_;

	static const int MIN_LEVEL= 0;
	static const int MAX_LEVEL= 100;
};

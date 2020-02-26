/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
#include "ImgPage.h"
#include "ImgManipulation.h"


// ImgBrightnessPage dialog

class ImgBrightnessPage : public ImgPage
{
public:
	ImgBrightnessPage(CWnd* parent = NULL);   // standard constructor
	virtual ~ImgBrightnessPage();

	virtual void Transform(Dib& dib, bool preview);

// Dialog Data
	enum { IDD = IDD_IMG_BRIGHTNESS };

protected:
	virtual void DoDataExchange(CDataExchange* DX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()

	CSliderCtrl slider_wnd_[2];
	CSpinButtonCtrl spin_wnd_[2];

	static const int MIN_VAL= -100;
	static const int MAX_VAL= 100;
	bool in_update_;

	BrightnessContrastParams params_;
	BrightnessContrast br_cont_;

	void OnHScroll(UINT sb_code, UINT pos, CScrollBar* scroll_bar);
	void OnNumChanged(UINT id);
	void OnReset();
	void SetValues();
};

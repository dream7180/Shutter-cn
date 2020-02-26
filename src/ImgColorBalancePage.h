/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
#include "ImgPage.h"
#include "ImgManipulation.h"


// ImgColorBalancePage dialog

class ImgColorBalancePage : public ImgPage
{
public:
	ImgColorBalancePage(CWnd* parent = NULL);   // standard constructor
	virtual ~ImgColorBalancePage();

	virtual void Transform(Dib& dib, bool preview);

// Dialog Data
	enum { IDD = IDD_IMG_COLOR_BALANCE };

	struct Shift
	{
		int val[3];

		Shift()
		{
			std::fill(val, val + array_count(val), 0);
		}
	};
	Shift shift_[3];

protected:
	virtual void DoDataExchange(CDataExchange* DX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()

	CSliderCtrl slider_wnd_[3];
	CSpinButtonCtrl spin_wnd_[3];

	static const int MIN_VAL= -100;
	static const int MAX_VAL= 100;
	int range_;
	bool in_update_;
	int preserve_lum_;

	ColorBalance balance_;

	void OnRangeChanged(UINT id);
	void OnNumChanged(UINT id);
	void SetRangeValues();
	void OnHScroll(UINT sb_code, UINT pos, CScrollBar* scroll_bar);
	void OnReset();
	void OnPreserveLuminosity();
};

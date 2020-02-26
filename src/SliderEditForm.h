/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
#include "resource.h"
#include "DlgAutoResize.h"


// SliderEditForm dialog

class SliderEditForm : public CDialog
{
public:
	SliderEditForm();
	virtual ~SliderEditForm();

// Dialog Data

	void SetRange(double from, double to);
	void SetPrecision(int decimal_places);

	void SetValue(double val);
	double GetValue() const;

protected:
	virtual void DoDataExchange(CDataExchange* DX);    // DDX/DDV support

	enum { IDD = IDD_SLIDER_EDIT };

	DECLARE_MESSAGE_MAP()

private:
	double from_;
	double to_;
	double cur_val_;
	int precision_;
	bool in_update_;
	CSliderCtrl slider_;
	CSpinButtonCtrl spin_;
	DlgAutoResize dlg_auto_resize_;

	afx_msg void OnChangeEdit();
	void OnHScroll(UINT sb_code, UINT pos, CScrollBar* scroll_bar);
	virtual BOOL OnInitDialog();
	void SetValues();
};

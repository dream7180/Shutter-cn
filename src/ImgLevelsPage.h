/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
#include "ImgPage.h"
#include "HistogramCtrl.h"
#include "ImgManipulation.h"


// ImgLevelsPage dialog

class ImgLevelsPage : public ImgPage, CHistogramNotifications
{
public:
	ImgLevelsPage(CWnd* parent = NULL);   // standard constructor
	virtual ~ImgLevelsPage();

	virtual void Transform(Dib& dib, bool preview);

	virtual void Initialize(const Dib& dibOriginal);

// Dialog Data
	enum { IDD = IDD_IMG_LEVELS };

protected:
	virtual void DoDataExchange(CDataExchange* DX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
	afx_msg void OnReset();
	afx_msg void OnSelChangeChannels();
	void OnDeltaPosGammaSpin(NMHDR* nmhdr, LRESULT* result);
	void OnNumChanged(UINT uId);
	void SetGamma(double gamma);
	virtual void MouseClicked(int position, bool left);

	CComboBox channel_wnd_;
	HistogramCtrl histogram_wnd_;
	CSpinButtonCtrl spin_wnd_[3];
	bool in_update_;

	LevelParams params_;
	Levels levels_;

	static const int MIN_VAL= 0;
	static const int MAX_VAL= 255;
};

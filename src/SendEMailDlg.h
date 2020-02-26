/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
#include "VectPhotoInfo.h"
#include "DialogChild.h"


// CSendEMailDlg dialog

class CSendEMailDlg : public DialogChild
{
public:
	CSendEMailDlg(CWnd* parent, VectPhotoInfo& photos, double ratio);   // standard constructor
	virtual ~CSendEMailDlg();

// Dialog Data
	enum { IDD = IDD_EMAIL };
	CStatic	compr_ratio_wnd_;
	CSpinButtonCtrl	spin_compr_level_;
	CEdit	edit_compr_level_;
	CSliderCtrl	slider_size_;
	CSliderCtrl	slider_quality_;
	CEdit	edit_width_;
	CEdit	edit_height_;
	int		size_percent_;
	int		width_;
	int		height_;
	int		size_;
	int		quality_;
	int		compr_level_;

	int GetJPEGQuality() const			{ return compr_level_; }

protected:
	virtual void DoDataExchange(CDataExchange* DX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnOK();

	DECLARE_MESSAGE_MAP()

private:
	VectPhotoInfo& photos_;
	bool update_;
	double ratio_;

	void UpdateDims();
	void UpdateFmt();
	void OnChangeHeight();
	void OnChangeWidth();
	void UpdateSize();
	void UpdateQuality();
	void OnVScroll(UINT sb_code, UINT pos, CScrollBar* scroll_bar);
	void OnChangeComprLevel();
};

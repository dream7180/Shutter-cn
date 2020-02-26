/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
#include "DialogChild.h"
#include "VectPhotoInfo.h"
#include "Profile.h"


// DateAdjustDlg dialog

class DateAdjustDlg : public DialogChild
{
public:
	DateAdjustDlg(CWnd* parent, VectPhotoInfo& photos);
	virtual ~DateAdjustDlg();

// Dialog Data
	enum { IDD = IDD_DATE_TIME };

	bool RelativeChange() const;
	DateTime GetDateTime() const;
	int GetDays() const;
	int GetHours() const;
	int GetMinutes() const;
	int GetSeconds() const;

protected:
	virtual void DoDataExchange(CDataExchange* DX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	virtual BOOL OnInitDialog();
	virtual void OnOK();

private:
	CEdit edit_days_;
	int days_;
	CEdit edit_hours_;
	int hours_;
	CEdit edit_minutes_;
	int minutes_;
	CEdit edit_seconds_;
	int seconds_;
	int adj_mode_;
	CDateTimeCtrl date_edit_;
	CDateTimeCtrl time_edit_;
	bool ready_;
	CSpinButtonCtrl spin_days_;
	CSpinButtonCtrl spin_hours_;
	CSpinButtonCtrl spin_minutes_;
	CSpinButtonCtrl spin_seconds_;
	VectPhotoInfo& photos_;
	DateTime example_time_;
	CButton ok_btn_;
	CListCtrl results_;
	bool valid_;
	DateTime time_set_;
	TimeDuration time_span_;
	Profile<int> profile_days_;
	Profile<int> profile_hours_;
	Profile<int> profile_minutes_;
	Profile<int> profile_seconds_;
	Profile<int> profile_adj_mode_;
	Profile<std::wstring> profile_date_time_;

	void OnAdjustMode();
	void OnSetMode();
	void EnableGroup(bool first);
	afx_msg void OnDateChange(NMHDR* nmhdr, LRESULT* result);
	afx_msg void OnTimeChange(NMHDR* nmhdr, LRESULT* result);
	void UpdateExampleAndOkBtn();
	afx_msg void OnChangeDays();
	afx_msg void OnChangeHours();
	afx_msg void OnChangeMinutes();
	afx_msg void OnChangeSeconds();
	bool GetAdjustedTimeSpan(TimeDuration& span_ret) const;
	bool CheckAdjustment(VectPhotoInfo& photos, PhotoInfoPtr& invalid, TimeDuration& span, int& d, int& h, int& m, int& s);
	bool GetDateTime(DateTime& dt) const;
	BOOL InitDlg();
	BOOL OnGetDispInfo(UINT id, NMHDR* nmhdr, LRESULT* result);
	void OnCustomDraw(NMHDR* nm_hdr, LRESULT* result);
	String FormatAdjustedDT(DateTime date) const;
};

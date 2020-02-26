/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
#include "DialogChild.h"
#include <boost/scoped_ptr.hpp>
#include "VectPhotoInfo.h"
#include "RenameRule.h"


// RenameToolDlg dialog

class RenameToolDlg : public DialogChild
{
public:
	RenameToolDlg(CWnd* parent, VectPhotoInfo& photos);
	virtual ~RenameToolDlg();

// Dialog Data
	enum { IDD = IDD_RENAME };

protected:
	virtual void DoDataExchange(CDataExchange* DX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	struct Impl;
	boost::scoped_ptr<Impl> impl_;

	virtual BOOL OnInitDialog();
	virtual void Resize();
	void OnSelChangeTab(NMHDR* nmhdr, LRESULT* result);
	BOOL OnGetDispInfo(UINT id, NMHDR* nmhdr, LRESULT* result);
	BOOL OnGetToolTipInfo(UINT id, NMHDR* nmhdr, LRESULT* result);
	void OnTbDropDown(NMHDR* nmhdr, LRESULT* result);
	void OnDestroy();
	void OnTimer(UINT_PTR timer_id);
	virtual void OnOK();
	bool Finish();
	void OnAbort();
	LRESULT OnNextPhase(WPARAM, LPARAM);
	LRESULT OnNextPhoto(WPARAM photo_index, LPARAM);
	LRESULT OnRenameError(WPARAM indexFile, LPARAM message);
	virtual void OnCancel();
	LRESULT OnPhotoStatus(WPARAM file_index, LPARAM status);
};


extern std::vector<RenameRule>& GetFileRenamingRules();

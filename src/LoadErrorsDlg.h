/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
#include "DlgAutoResize.h"
class WindowPosition;
class ImgLogger;

// CLoadErrorsDlg dialog: this window shows list of errors encountered during folder scan (loading images)
//


class CLoadErrorsDlg : public CDialog
{

public:
	CLoadErrorsDlg(CWnd* parent, const ImgLogger& logger);   // standard constructor
	virtual ~CLoadErrorsDlg();

// Dialog Data
	enum { IDD = IDD_LOAD_STATUS };

protected:
	virtual void DoDataExchange(CDataExchange* DX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	CListCtrl list_ctrl_;
	DlgAutoResize resize_;
	std::auto_ptr<WindowPosition> wnd_pos_;
	CSize min_size_;
	const ImgLogger& logger_;

	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT type, int cx, int cy);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* MMI);
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg BOOL OnGetDispInfo(UINT id, NMHDR* nmhdr, LRESULT* result);
};

/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// Simple delete confirmation dialog with pretty background bitmap

#pragma once
#include "Dib.h"

// DeleteConfirmationDlg dialog

class DeleteConfirmationDlg : public CDialog
{
public:
	DeleteConfirmationDlg(CWnd* parent, CString msg);
	virtual ~DeleteConfirmationDlg();

// Dialog Data
	enum { IDD = IDD_DELETE_CONFIRMATION };

protected:
	virtual void DoDataExchange(CDataExchange* DX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	//AutoPtr<Dib> background_;
	CString msg_;
	CBrush back_;

	afx_msg BOOL OnEraseBkgnd(CDC* dc);
	virtual BOOL OnInitDialog();
	afx_msg HBRUSH OnCtlColor(CDC* dc, CWnd* wnd, UINT ctl_color);
};

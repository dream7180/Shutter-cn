/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once


// CopyProgressDlg dialog

class CopyProgressDlg : public CDialog
{
public:
	CopyProgressDlg(CWnd* parent = NULL);   // standard constructor
	virtual ~CopyProgressDlg();

// Dialog Data
	enum { IDD = IDD_COPY_PROGRESS };

protected:
	virtual void DoDataExchange(CDataExchange* DX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
private:
	CAnimateCtrl animation_;

	virtual BOOL OnInitDialog();
};

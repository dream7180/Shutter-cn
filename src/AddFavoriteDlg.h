/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once


// AddFavoriteDlg dialog

class AddFavoriteDlg : public CDialog
{
public:
	AddFavoriteDlg(CWnd* parent = NULL);   // standard constructor
	virtual ~AddFavoriteDlg();

	CString path_;
	CString name_;

// Dialog Data
	enum { IDD = IDD_ADD_FAVORITE };

protected:
	virtual void DoDataExchange(CDataExchange* DX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	virtual BOOL OnInitDialog();
	afx_msg void OnChangeName();
	void UpdateOkBtn();

	CEdit edit_name_;
	CStatic path_wnd_;
	CButton btn_ok_;
};

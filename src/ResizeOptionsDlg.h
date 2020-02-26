/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once


// CResizeOptionsDlg dialog

class CResizeOptionsDlg : public CDialog
{
public:
	CResizeOptionsDlg(CWnd* parent = NULL);   // standard constructor
	virtual ~CResizeOptionsDlg();

	bool Create(CWnd* parent, int dlg_id);

	CComboBox resizing_method_wnd_;
	int		resizing_method_;
	BOOL	baseline_jpeg_;
	BOOL	progressive_jpeg_;
	BOOL	preserve_exif_block_;
	BOOL	copyTags_;

// Dialog Data
	enum { IDD = IDD_RESIZE_OPTIONS };

protected:
	virtual void DoDataExchange(CDataExchange* DX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};

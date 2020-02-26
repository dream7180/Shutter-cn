/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
#include "DlgAutoResize.h"
#include "PathEdit.h"


// ImgBatchModeDlg dialog

class ImgBatchModeDlg : public CDialog
{
public:
	ImgBatchModeDlg(CWnd* parent = NULL);   // standard constructor
	virtual ~ImgBatchModeDlg();

	bool Create(CWnd* parent);

	bool Finish();

	int dest_folder_;
	CString	dest_folder_str_;
	CString	suffix_;
	CEdit	edit_suffix_;
	CWnd	example_wnd_;
	CPathEdit	edit_dest_path_;

// Dialog Data
	enum { IDD = IDD_IMG_BATCH_MODE };

protected:
	virtual void DoDataExchange(CDataExchange* DX);    // DDX/DDV support
	void UpdateExample();
	void OnChangeSuffix();
	void OnSameDir();
	void OnSelectDir();
	void UpdateDirs();
	void OnBrowse();

	DECLARE_MESSAGE_MAP()
	void OnSize(UINT type, int cx, int cy);
	virtual BOOL OnInitDialog();

	DlgAutoResize dlg_resize_map_;
};

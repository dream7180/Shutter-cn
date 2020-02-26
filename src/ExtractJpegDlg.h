/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once

#include "DialogChild.h"
#include "ExtractingThread.h"
#include "PathEdit.h"
#include "RecentPaths.h"


// ExtractJpegDlg dialog

class ExtractJpegDlg : public DialogChild
{
public:
	ExtractJpegDlg(CWnd* parent = NULL);   // standard constructor
	virtual ~ExtractJpegDlg();

// Dialog Data
	enum { IDD = IDD_EXTRACT_JPEG };

	int same_dir_;
	CString suffix_;
	Path GetDestPath() const;
	ExtractFormat GetParams() const;

protected:
	virtual void DoDataExchange(CDataExchange* DX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()

private:
	CEdit suffix_edit_;
	CComboBox dest_path_combo_;
	std::vector<String> recent_path_strings_;
	CPathEdit dest_path_editbox_;
	CString	dest_path_;
	CString registry_key_;
	CStatic	example_wnd_;
	PathVector recent_paths_;

	void OnChangeSuffix();
	void UpdateExample();
	void OnSameDir();
	void OnSelectDir();
	void UpdateDirs();
	virtual void OnOK();
	bool Finish();
	void OnBrowse();
};

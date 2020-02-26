/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
#include "PathEdit.h"
#include <boost/function.hpp>


// NewFolderDlg dialog

class NewFolderDlg : public CDialog
{
public:
	NewFolderDlg(CWnd* parent = NULL);   // standard constructor
	virtual ~NewFolderDlg();

	void SetCreateFolderCallback(const boost::function<bool (CWnd* edit, const CString& name)>& createFolder);

// Dialog Data
	enum { IDD = IDD_NEW_FOLDER };

	CString folderName_;

protected:
	virtual void DoDataExchange(CDataExchange* DX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	void OnNameChanged();

	DECLARE_MESSAGE_MAP()

	CPathEdit name_;
	CButton btnOk_;
	boost::function<bool (CWnd* edit, const CString& name)> createFolder_;
};


extern bool CreateFolderHelperFn(CWnd* edit, const CString& name, String* path);

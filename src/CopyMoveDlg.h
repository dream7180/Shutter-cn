/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
#include "DialogChild.h"
#include "FolderPath.h"

/////////////////////////////////////////////////////////////////////////////
// CopyMoveDlg dialog


class CopyMoveDlg : public DialogChild
{
// Construction
public:
	CopyMoveDlg(bool copy, const TCHAR* dest_path, CWnd* parent, FolderPathPtr cur_path);

// Dialog Data
	//{{AFX_DATA(CopyMoveDlg)
	enum { IDD = IDD_COPY_MOVE };
	CButton	btn_ok_;
	CString	label_;
	CString	path_;
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CopyMoveDlg)
protected:
	virtual void DoDataExchange(CDataExchange* DX);    // DDX/DDV support
	//}}AFX_VIRTUAL

public:
	const TCHAR* DestPath() const	{ return path_; }

	~CopyMoveDlg();

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CopyMoveDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnBrowse();
	void OnSize(UINT type, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	virtual void OnOK();

private:
	struct Impl;
	Impl& impl_;

	void OnGoLevelUp();
	void OnListView();
	void OnIconView();
	BOOL OnEraseBkgnd(CDC* dc);
	void OnPathEditChanged();
	BOOL InitDialog();
};

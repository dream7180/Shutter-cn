/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
#include "DialogChild.h"
class CatalogImages;


// CatalogDlg dialog

class CatalogDlg : public DialogChild
{
public:
	CatalogDlg(CWnd* parent, const TCHAR* src_path);
	virtual ~CatalogDlg();

// Dialog Data
	enum { IDD = IDD_CATALOG_TOOL };

protected:
	virtual void DoDataExchange(CDataExchange* DX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();

private:
	struct Impl;
	std::auto_ptr<Impl> pImpl_;

	void OnAbort();
	void OnSaveAs();
	void OnBrowse();
	LRESULT OnProgress(WPARAM count, LPARAM step);
	LRESULT OnBeginEnd(WPARAM count, LPARAM finished);
	void OnOptions();
	void OnDestroy();
	void OnHScroll(UINT sb_code, UINT pos, CScrollBar* scroll_bar);
	afx_msg void OnDriveChanged(NMHDR* nmhdr, LRESULT* result);
	LRESULT OnThreadAborted(WPARAM, LPARAM);
};

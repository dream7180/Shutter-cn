/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
#include <boost/function.hpp>
#include "DlgAutoResize.h"


// CatalogOptionsDlg dialog

class CatalogOptionsDlg : public CDialog
{
public:
	CatalogOptionsDlg(CWnd* parent = NULL);   // standard constructor
	virtual ~CatalogOptionsDlg();

	bool Create(CWnd* parent);

	void GetSelectedTypes(std::vector<bool>& types) const;

	void SaveSettings(const TCHAR* section, const TCHAR* entry) const;
	void LoadSettings(const TCHAR* section, const TCHAR* entry);

	void SetTypeChangeCallback(const boost::function<void ()>& fn);

	void EnableControls(bool enable);

protected:
	virtual void DoDataExchange(CDataExchange* DX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	virtual BOOL OnInitDialog();
	void OnAll();
	void OnNone();
	void OnFileTypeChanged(NMHDR* nmhdr, LRESULT* result);
	void OnSize(UINT type, int cx, int cy);

	CSpinButtonCtrl spin_;
	CListCtrl img_types_;
	CImageList type_list_;
	boost::function<void ()> type_selection_changed_;
	CEdit compr_level_;
	CButton btn_all_;
	CButton btn_none_;
	DlgAutoResize resize_;
};

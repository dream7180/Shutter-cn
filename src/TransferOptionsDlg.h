/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once


// CTransferOptionsDlg dialog

class CTransferOptionsDlg : public CDialog
{
public:
	CTransferOptionsDlg(CWnd* parent = NULL);   // standard constructor
	virtual ~CTransferOptionsDlg();

// Dialog Data
	BOOL makeReadOnly_;
	BOOL clearArchiveAttr_;

	bool Create(CWnd* parent);

protected:
	virtual void DoDataExchange(CDataExchange* DX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	void OnChangePattern();
	void OnTbDropDown(NMHDR* nmhdr, LRESULT* result);
	void SymbolsPopupMenu();
	void OnRename();
	void UpdatePatternCtrl();

	DECLARE_MESSAGE_MAP()
};

/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once


// CatalogHelpDlg dialog

class CatalogHelpDlg : public CDialog
{
public:
	CatalogHelpDlg(CWnd* parent = NULL);   // standard constructor
	virtual ~CatalogHelpDlg();

	bool Create(CWnd* parent);

// Dialog Data
	enum { IDD = IDD_CATALOG_HELP };

protected:
	virtual void DoDataExchange(CDataExchange* DX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	virtual BOOL OnInitDialog();

private:
	CStatic icons_[4];
	CStatic arr_[3];
	CBitmap bmp_[4];
	CBitmap arrow_;
	CFont small_font_;

	void SetFont(int id, CFont& font);
};

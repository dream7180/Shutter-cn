/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// CatalogHelpDlg.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "CatalogHelpDlg.h"
#include "PNGImage.h"
#include "BoldFont.h"


// CatalogHelpDlg dialog

CatalogHelpDlg::CatalogHelpDlg(CWnd* parent /*=NULL*/)
	: CDialog(CatalogHelpDlg::IDD, parent)
{
}

CatalogHelpDlg::~CatalogHelpDlg()
{
}

void CatalogHelpDlg::DoDataExchange(CDataExchange* DX)
{
	CDialog::DoDataExchange(DX);
}


BEGIN_MESSAGE_MAP(CatalogHelpDlg, CDialog)
END_MESSAGE_MAP()


// CatalogHelpDlg message handlers


void CatalogHelpDlg::SetFont(int id, CFont& font)
{
	if (CWnd* ctrl= GetDlgItem(id))
		ctrl->SetFont(&font);
}


BOOL CatalogHelpDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	icons_[0].SubclassDlgItem(IDC_OPER_1, this);
	icons_[1].SubclassDlgItem(IDC_OPER_2, this);
	icons_[2].SubclassDlgItem(IDC_OPER_3, this);
	icons_[3].SubclassDlgItem(IDC_OPER_4, this);
	arr_[0].SubclassDlgItem(IDC_ARROW_1, this);
	arr_[1].SubclassDlgItem(IDC_ARROW_2, this);
	arr_[2].SubclassDlgItem(IDC_ARROW_3, this);

	PNGImage img;
	img.AlphaToColor(::GetSysColor(COLOR_3DFACE));
	static const int bmps[]= { IDR_CATALOG_HELP_1, IDR_CATALOG_HELP_2, IDR_CATALOG_HELP_3, IDR_CATALOG_HELP_4 };
	for (int i= 0; i < array_count(bmps); ++i)
	{
		img.Load(bmps[i], bmp_[i]);
		icons_[i].SetBitmap(bmp_[i]);
	}

	img.Load(IDR_CATALOG_HELP_ARROW, arrow_);
	for (int i= 0; i < array_count(arr_); ++i)
		arr_[i].SetBitmap(arrow_);

	::CreateSmallFont(this, small_font_);

	SetFont(IDC_LABEL_1, small_font_);
	SetFont(IDC_LABEL_2, small_font_);
	SetFont(IDC_LABEL_3, small_font_);
	SetFont(IDC_LABEL_4, small_font_);

	return true;
}


bool CatalogHelpDlg::Create(CWnd* parent)
{
	return !!CDialog::Create(IDD, parent);
}

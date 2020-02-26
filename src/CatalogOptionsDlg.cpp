/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// CatalogOptionsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "CatalogOptionsDlg.h"
#include "FileTypeIndex.h"
#include "Config.h"
#include "PNGImage.h"
#include "ProfileVector.h"

enum { IDD = IDD_CATALOG_OPTIONS };

// CatalogOptionsDlg dialog

CatalogOptionsDlg::CatalogOptionsDlg(CWnd* parent /*=NULL*/)
	: CDialog(IDD, parent)
{
}

CatalogOptionsDlg::~CatalogOptionsDlg()
{
}

void CatalogOptionsDlg::DoDataExchange(CDataExchange* DX)
{
	CDialog::DoDataExchange(DX);
}


BEGIN_MESSAGE_MAP(CatalogOptionsDlg, CDialog)
	ON_BN_CLICKED(IDC_ALL, OnAll)
	ON_BN_CLICKED(IDC_NONE, OnNone)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_IMG_TYPES, OnFileTypeChanged)
	ON_WM_SIZE()
END_MESSAGE_MAP()


// CatalogOptionsDlg message handlers

bool CatalogOptionsDlg::Create(CWnd* parent)
{
	return !!CDialog::Create(IDD, parent);
}


BOOL CatalogOptionsDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	VERIFY(spin_.SubclassDlgItem(IDC_SPIN, this));
	spin_.SetRange(1, 100);

	VERIFY(img_types_.SubclassDlgItem(IDC_IMG_TYPES, this));
	img_types_.SetExtendedStyle(LVS_EX_CHECKBOXES);
	PNGImage().LoadImageList(IDR_FILE_TYPES, 40, ::GetSysColor(COLOR_WINDOW), type_list_);
	img_types_.SetImageList(&type_list_, LVSIL_SMALL);

	const int count= FT_LAST;	// skip last one actually (it's a catalog)
	for (int i= 0; i < count; ++i)
	{
		LVITEM li;
		memset(&li, 0, sizeof li);

		li.mask			= LVIF_TEXT | LVIF_PARAM | LVIF_STATE | LVIF_IMAGE;
		li.iItem		= i;
		li.iSubItem		= 0;
		li.state		= (i == 0 ? LVIS_SELECTED | LVIS_FOCUSED : 0); // doesn't work -> | INDEXTOSTATEIMAGEMASK(scan_[i] ? 2 : 1);
		li.stateMask	= LVIS_SELECTED | LVIS_FOCUSED; // | LVIS_STATEIMAGEMASK;
		ASSERT(i < g_Settings.file_types_.size());
		li.pszText		= const_cast<TCHAR*>(g_Settings.file_types_[i].name.c_str()); //LPSTR_TEXTCALLBACK;
		li.cchTextMax	= 0;
		li.iImage		= i + 1;
		li.lParam		= 0;
		li.iIndent		= 0;

		img_types_.InsertItem(&li);

		//if (scan_types.size() == count)
		img_types_.SetCheck(i, true);
	}

	VERIFY(compr_level_.SubclassDlgItem(IDC_COMPR, this));

	VERIFY(btn_all_.SubclassDlgItem(IDC_ALL, this));
	VERIFY(btn_none_.SubclassDlgItem(IDC_NONE, this));

	resize_.BuildMap(this);
	resize_.SetWndResizing(IDC_IMG_TYPES, DlgAutoResize::RESIZE_H);
	resize_.SetWndResizing(IDC_ALL, DlgAutoResize::MOVE_H);
	resize_.SetWndResizing(IDC_NONE, DlgAutoResize::MOVE_H);

	return true;
}


void CatalogOptionsDlg::OnAll()
{
	const int count= img_types_.GetItemCount();
	for (int i= 0; i < count; ++i)
		img_types_.SetCheck(i, true);
}


void CatalogOptionsDlg::OnNone()	// this is 'only jpeg' button
{
	const int count= img_types_.GetItemCount();
	for (int i= 1; i < count; ++i)
		img_types_.SetCheck(i, false);
	if (count > 0)
		img_types_.SetCheck(0, true);
}


void CatalogOptionsDlg::GetSelectedTypes(std::vector<bool>& types) const
{
	const int count= img_types_.GetItemCount();
	types.resize(count, false);
	for (int i= 0; i < count; ++i)
		types[i] = !!img_types_.GetCheck(i);
}


void CatalogOptionsDlg::SaveSettings(const TCHAR* section, const TCHAR* entry) const
{
	const int count= img_types_.GetItemCount();
	std::vector<bool> scan_types(count, false);
	for (int i= 0; i < count; ++i)
		scan_types[i] = !!img_types_.GetCheck(i);

	WriteProfileVector(section, entry, scan_types);
}


void CatalogOptionsDlg::LoadSettings(const TCHAR* section, const TCHAR* entry)
{
	std::vector<bool> scan_types;
	GetProfileVector(section, entry, scan_types);

	if (scan_types.empty())
		return;

	const int count= img_types_.GetItemCount();
	if (scan_types.size() != count)
	{
		scan_types.clear();
		scan_types.resize(count, true);

		ASSERT(false);
		return;
	}

	for (int i= 0; i < count; ++i)
		img_types_.SetCheck(i, scan_types[i]);
}


void CatalogOptionsDlg::OnFileTypeChanged(NMHDR* nmhdr, LRESULT* result)
{
	if (type_selection_changed_ == 0)
		return;

	NMLISTVIEW* lv= reinterpret_cast<NMLISTVIEW*>(nmhdr);

	if (lv->uChanged & LVIF_STATE)
		if ((lv->uNewState & LVIS_STATEIMAGEMASK) != (lv->uOldState & LVIS_STATEIMAGEMASK))
			type_selection_changed_();
}


void CatalogOptionsDlg::SetTypeChangeCallback(const boost::function<void ()>& fn)
{
	type_selection_changed_ = fn;
}


void CatalogOptionsDlg::EnableControls(bool enable)
{
	img_types_.EnableWindow(enable);
	compr_level_.EnableWindow(enable);
	spin_.Invalidate();
	btn_all_.EnableWindow(enable);
	btn_none_.EnableWindow(enable);
}


void CatalogOptionsDlg::OnSize(UINT type, int cx, int cy)
{
	resize_.Resize();
}

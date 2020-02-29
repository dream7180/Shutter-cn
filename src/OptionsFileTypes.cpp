/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// OptionsFileTypes.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "OptionsFileTypes.h"
#include "Config.h"
#include "PhotoInfoPNG.h"
#include "PhotoInfoPSD.h"
#include "PhotoInfoCRW.h"
#include "PhotoInfoTIFF.h"
#include "PhotoInfoNEF.h"
#include "PhotoInfoORF.h"
#include "PhotoInfoDNG.h"
#include "PhotoInfoRAF.h"
#include "PhotoInfoGIF.h"
#include "PhotoInfoJPEG.h"
#include "PhotoInfoBMP.h"
#include "PhotoInfoPEF.h"
#include "PhotoInfoARW.h"
#include "PhotoInfoRW2.h"
#include "PhotoInfoX3F.h"
#include "PhotoInfoSRW.h"
#include "CatchAll.h"
#include "UIElements.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// OptionsFileTypes property page

IMPLEMENT_DYNCREATE(OptionsFileTypes, CPropertyPage)

OptionsFileTypes::OptionsFileTypes() : RPropertyPage(OptionsFileTypes::IDD)
{
	//{{AFX_DATA_INIT(OptionsFileTypes)
	//}}AFX_DATA_INIT

	img_list_menu_btn_.Create(IDB_MENU_BTN, 8, 0, RGB(255,0,255));

	size_t count= g_Settings.file_types_.size();
	markers_.resize(count);
	no_exif_.resize(count);
	scan_.resize(count);

	for (int i= 0; i < count; ++i)
	{
		markers_[i] = g_Settings.file_types_[i].show_marker;
		no_exif_[i] = g_Settings.file_types_[i].show_no_exif;
		scan_[i] = g_Settings.file_types_[i].scan;
	}
}


OptionsFileTypes::~OptionsFileTypes()
{
}

void OptionsFileTypes::DoDataExchange(CDataExchange* DX)
{
	CPropertyPage::DoDataExchange(DX);
	//{{AFX_DATA_MAP(OptionsFileTypes)
	DDX_Control(DX, IDC_TYPES, types_wnd_);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(OptionsFileTypes, RPropertyPage)
	//{{AFX_MSG_MAP(OptionsFileTypes)
	ON_NOTIFY(LVN_GETDISPINFO, IDC_TYPES, OnGetDispInfo)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_TYPES, OnListCtrlDraw)
	ON_NOTIFY(NM_CLICK, IDC_TYPES, OnClickList)
	ON_COMMAND(ID_SHOW_MARKER, OnShowMarker)
	ON_UPDATE_COMMAND_UI(ID_SHOW_MARKER, OnUpdateShowMarker)
	ON_COMMAND(ID_HIDE_MARKER, OnHideMarker)
	ON_UPDATE_COMMAND_UI(ID_HIDE_MARKER, OnUpdateHideMarker)
	ON_WM_DESTROY()
	ON_COMMAND(ID_SHOW_NO_EXIF, OnShowNoExif)
	ON_UPDATE_COMMAND_UI(ID_SHOW_NO_EXIF, OnUpdateShowNoExif)
	ON_COMMAND(ID_HIDE_NO_EXIF, OnHideNoExif)
	ON_UPDATE_COMMAND_UI(ID_HIDE_NO_EXIF, OnUpdateHideNoExif)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// OptionsFileTypes message handlers

BOOL OptionsFileTypes::OnInitDialog()
{
	try
	{
		InitDialog();

		ResizeMgr().BuildMap(this);
		ResizeMgr().SetWndResizing(IDC_TYPES, DlgAutoResize::RESIZE);
		ResizeMgr().SetWndResizing(IDC_EXAMPLE, DlgAutoResize::MOVE_V_RESIZE_H);
		ResizeMgr().SetWndResizing(IDC_LABEL, DlgAutoResize::MOVE_V);

		return false;
	}
	CATCH_ALL

	EndDialog(IDCANCEL);
	return false;
}


BOOL OptionsFileTypes::InitDialog()
{
	CPropertyPage::OnInitDialog();

	types_wnd_.SetExtendedStyle(LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT);

	types_wnd_.InsertColumn(0, _T("文件类型"), LVCFMT_LEFT, Pixels(190));
	types_wnd_.InsertColumn(1, _T("扩展名"), LVCFMT_LEFT, Pixels(80));
	types_wnd_.InsertColumn(2, _T("厂商"), LVCFMT_LEFT, Pixels(55));
	types_wnd_.InsertColumn(3, _T("无 EXIF"), LVCFMT_LEFT, Pixels(57));

	const auto count= g_Settings.file_types_.size();
	no_marker_entry_ = static_cast<int>(count - 1);	// catalog files have no markers of its own
	for (int i= 0; i < count; ++i)
	{
		LVITEM li;
		memset(&li, 0, sizeof li);

		li.mask			= LVIF_TEXT | LVIF_PARAM | LVIF_STATE | LVIF_IMAGE;
		li.iItem		= i;
		li.iSubItem		= 0;
		li.state		= (i == 0 ? LVIS_SELECTED | LVIS_FOCUSED : 0); // doesn't work -> | INDEXTOSTATEIMAGEMASK(scan_[i] ? 2 : 1);
		li.stateMask	= LVIS_SELECTED | LVIS_FOCUSED; // | LVIS_STATEIMAGEMASK;
		li.pszText		= LPSTR_TEXTCALLBACK;
		li.cchTextMax	= 0;
		li.iImage		= 0; //I_IMAGECALLBACK;
		li.lParam		= 0;
		li.iIndent		= 0;

		types_wnd_.InsertItem(&li);
		types_wnd_.SetCheck(i, scan_[i]);
	}

	if (CWnd* wnd= GetDlgItem(IDC_EXAMPLE))
	{
		WINDOWPLACEMENT wp;
		wnd->GetWindowPlacement(&wp);
		wnd->DestroyWindow();
		CRect rect= wp.rcNormalPosition;

		border_wnd_.CWnd::CreateEx(WS_EX_CLIENTEDGE, WC_LISTVIEW, 0, WS_VISIBLE | WS_CHILD, rect, this, IDC_EXAMPLE);

		example_wnd_.Create(&border_wnd_, this, IDC_EXAMPLE);
		rect.DeflateRect(1, 1);
		rect.OffsetRect(-rect.TopLeft());
		example_wnd_.MoveWindow(rect);
		example_wnd_.EnableWindow(false);

		// prepare fake photo and fill in PhotoCtrl window

		photos_.clear();
		photos_.reserve(20);
		photos_.Append(new PhotoInfoJPEG);
		photos_.Append(new PhotoInfoPSD);
		photos_.Append(new PhotoInfoTIFF);
		photos_.Append(new PhotoInfoPNG);
		photos_.Append(new PhotoInfoCRW);
		photos_.Append(new PhotoInfoNEF);
		photos_.Append(new PhotoInfoORF);
		photos_.Append(new PhotoInfoDNG);
		photos_.Append(new PhotoInfoRAF);
		photos_.Append(new PhotoInfoGIF);
		photos_.Append(new PhotoInfoBMP);
		photos_.Append(new PhotoInfoPEF);
		photos_.Append(new PhotoInfoARW);
		photos_.Append(new PhotoInfoRW2);
		photos_.Append(new PhotoInfoX3F);
		photos_.Append(new PhotoInfoSRW);

		VectPhotoInfo v;
		photos_.Copy(v);

		v[0]->SetPhotoName(_T("DSC00001"));
		v[0]->bmp_ = new Dib(IDB_PHOTO_7);

		v[1]->SetPhotoName(_T("DSC00002"));
		v[1]->bmp_ = new Dib(IDB_PHOTO_6);

		v[2]->SetPhotoName(_T("DSC00003"));
		v[2]->bmp_ = new Dib(IDB_PHOTO_3);

		v[3]->SetPhotoName(_T("DSC00004"));
		v[3]->bmp_ = new Dib(IDB_PHOTO_4);
		v[3]->SetOrientation(6);

		v[4]->SetPhotoName(_T("DSC00005"));
		v[4]->bmp_ = new Dib(IDB_PHOTO_2);

		v[5]->SetPhotoName(_T("DSC00006"));
		v[5]->bmp_ = new Dib(IDB_PHOTO_8);
		v[5]->SetOrientation(6);

		v[6]->SetPhotoName(_T("DSC00007"));
		v[6]->bmp_ = new Dib(IDB_PHOTO_1);

		v[7]->SetPhotoName(_T("DSC00008"));
		v[7]->bmp_ = new Dib(IDB_PHOTO_5);
		v[7]->SetOrientation(6);

		v[8]->SetPhotoName(_T("DSC00009"));
		v[8]->bmp_ = new Dib(IDB_PHOTO_7);

		v[9]->SetPhotoName(_T("DSC00010"));
		v[9]->bmp_ = new Dib(IDB_PHOTO_1);

		v[10]->SetPhotoName(_T("DSC00011"));
		v[10]->bmp_ = new Dib(IDB_PHOTO_4);
		v[10]->SetOrientation(6);

		v[11]->SetPhotoName(_T("DSC00012"));
		v[11]->bmp_ = new Dib(IDB_PHOTO_3);

		v[12]->SetPhotoName(_T("DSC00013"));
		v[12]->bmp_ = new Dib(IDB_PHOTO_2);

		v[13]->SetPhotoName(_T("DSC00014"));
		v[13]->bmp_ = new Dib(IDB_PHOTO_5);
		v[13]->SetOrientation(6);

		v[14]->SetPhotoName(_T("DSC00015"));
		v[14]->bmp_ = new Dib(IDB_PHOTO_8);
		v[14]->SetOrientation(6);

		v[15]->SetPhotoName(_T("DSC00016"));
		v[15]->bmp_ = new Dib(IDB_PHOTO_2);
		v[15]->SetOrientation(6);

		example_wnd_.SetImageSize(Pixels(50));
		example_wnd_.AddItems(v.begin(), v.end(), _T(""), PhotoCtrl::NO_ICON, 1);
		example_wnd_.ShowItemLabel(false);
	}
	else
	{ ASSERT(false); }

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void OptionsFileTypes::OnGetDispInfo(NMHDR* nmhdr, LRESULT* result)
{
	LV_DISPINFO* disp_info= reinterpret_cast<LV_DISPINFO*>(nmhdr);
	*result = 0;

	if (disp_info->item.mask & LVIF_TEXT)
	{
		disp_info->item.pszText[0] = _T('\0');

		switch (disp_info->item.iSubItem)
		{
		case 0:		// file type name
			_tcsncpy(disp_info->item.pszText, g_Settings.file_types_[disp_info->item.iItem].name.c_str(), disp_info->item.cchTextMax);
			break;

		case 1:		// extension
			_tcsncpy(disp_info->item.pszText, g_Settings.file_types_[disp_info->item.iItem].extensions.c_str(), disp_info->item.cchTextMax);
			break;

		case 2:		// type marker
			if (disp_info->item.iItem != no_marker_entry_)
				_tcsncpy(disp_info->item.pszText, markers_[disp_info->item.iItem] ? _T("显示") : _T("隐藏"), disp_info->item.cchTextMax);
			else
				_tcscpy(disp_info->item.pszText, _T("-"));
			break;

		case 3:		// no EXIF indicator
			if (disp_info->item.iItem != no_marker_entry_)
				_tcsncpy(disp_info->item.pszText, no_exif_[disp_info->item.iItem] ? _T("显示") : _T("隐藏"), disp_info->item.cchTextMax);
			else
				_tcscpy(disp_info->item.pszText, _T("-"));
			break;
		}
	}
	if (disp_info->item.mask & LVIF_IMAGE)
	{
		if (disp_info->item.iSubItem == 2)
			disp_info->item.iImage = 0;
	}
}


void OptionsFileTypes::OnListCtrlDraw(NMHDR* nmhdr, LRESULT* result)
{
	NMLVCUSTOMDRAW* NM_custom_draw= reinterpret_cast<NMLVCUSTOMDRAW*>(nmhdr);
	*result = CDRF_DODEFAULT;

	if (NM_custom_draw->nmcd.dwDrawStage == CDDS_PREPAINT)
	{
		*result = CDRF_NOTIFYITEMDRAW | CDRF_NOTIFYPOSTPAINT;
	}
	else if (NM_custom_draw->nmcd.dwDrawStage == CDDS_ITEMPREPAINT)
	{
		*result = CDRF_NOTIFYITEMDRAW | CDRF_NOTIFYPOSTPAINT;
	}
	else if (NM_custom_draw->nmcd.dwDrawStage == CDDS_ITEMPOSTPAINT)
	{
		auto item= static_cast<int>(NM_custom_draw->nmcd.dwItemSpec);

		if (item != no_marker_entry_)
		{
			CDC* dc= CDC::FromHandle(NM_custom_draw->nmcd.hdc);

			for (int sub_item= 2; sub_item < 4; ++sub_item)
			{
				CRect rect;
				if (types_wnd_.GetSubItemRect(item, sub_item, LVIR_BOUNDS, rect))
				{
					if (rect.Width() > 20)
					{
						int img= types_wnd_.GetItemState(item, LVIS_SELECTED) ? 1 : 0;
						int x= rect.right - 10;
						int y= rect.CenterPoint().y - 4;
						img_list_menu_btn_.Draw(dc, img, CPoint(x, y), ILD_TRANSPARENT);
					}
				}
			}

			*result = CDRF_SKIPDEFAULT;
		}
	}
}


void OptionsFileTypes::OnClickList(NMHDR* nmhdr, LRESULT* result)
{
	// ComCtrl 4.71 or higher required
	NMITEMACTIVATE* item= reinterpret_cast<NMITEMACTIVATE*>(nmhdr);
	*result = 0;

	if (item->iItem >= 0 && item->iSubItem >= 2 && item->iItem != no_marker_entry_)	// popup marker menu?
	{
		CMenu menu;
		if (menu.LoadMenu(item->iSubItem == 2 ? IDR_FILE_TYPE_MARKER : IDR_FILE_NO_EXIF))
			if (CMenu* popup= menu.GetSubMenu(0))
			{
				CRect rect;
				if (types_wnd_.GetSubItemRect(item->iItem, item->iSubItem, LVIR_BOUNDS, rect))
				{
					CPoint pos(rect.left, rect.bottom);
					types_wnd_.ClientToScreen(&pos);
					popup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pos.x, pos.y, this);
				}
			}
	}
}


void OptionsFileTypes::OnShowMarker()
{
	ShowMarker(true);
}

void OptionsFileTypes::OnUpdateShowMarker(CCmdUI* cmd_ui)
{
	cmd_ui->Enable();
}


void OptionsFileTypes::OnHideMarker()
{
	ShowMarker(false);
}

void OptionsFileTypes::OnUpdateHideMarker(CCmdUI* cmd_ui)
{
	cmd_ui->Enable();
}


void OptionsFileTypes::ShowMarker(bool show)
{
	if (POSITION pos= types_wnd_.GetFirstSelectedItemPosition())
	{
		int selected= types_wnd_.GetNextSelectedItem(pos);
		int count= static_cast<int>(markers_.size());
		if (selected >= 0 && selected < count)
			if (markers_[selected] != show)
			{
				markers_[selected] = show;
				types_wnd_.RedrawItems(selected, selected);
				example_wnd_.Invalidate();
			}
	}
}


void OptionsFileTypes::OnDestroy()
{
	if (types_wnd_.m_hWnd)
	{
		int count= static_cast<int>(scan_.size());
		for (int i= 0; i < count; ++i)
			scan_[i] = !!types_wnd_.GetCheck(i);
	}

	CPropertyPage::OnDestroy();
}


void OptionsFileTypes::OnShowNoExif()
{
	ShowNoExif(true);
}

void OptionsFileTypes::OnUpdateShowNoExif(CCmdUI* cmd_ui)
{
	cmd_ui->Enable();
}


void OptionsFileTypes::OnHideNoExif()
{
	ShowNoExif(false);
}

void OptionsFileTypes::OnUpdateHideNoExif(CCmdUI* cmd_ui)
{
	cmd_ui->Enable();
}


void OptionsFileTypes::ShowNoExif(bool show)
{
	if (POSITION pos= types_wnd_.GetFirstSelectedItemPosition())
	{
		int selected= types_wnd_.GetNextSelectedItem(pos);
		int count= static_cast<int>(no_exif_.size());
		if (selected >= 0 && selected < count)
			if (no_exif_[selected] != show)
			{
				no_exif_[selected] = show;
				types_wnd_.RedrawItems(selected, selected);
				example_wnd_.Invalidate();
			}
	}
}


bool OptionsFileTypes::ShowPhotoMarker(int file_type)
{
	return file_type < markers_.size() ? markers_[file_type] : false;
}

bool OptionsFileTypes::ShowNoExifIndicator(int file_type)
{
	return file_type < no_exif_.size() ? no_exif_[file_type] : false;
}


Dib* OptionsFileTypes::RequestThumbnail(PhotoInfoPtr photo, CSize image_size, bool return_if_not_available)
{
	return photo ? photo->bmp_.get() : 0;
}


void OptionsFileTypes::OnSize(UINT type, int cx, int cy)
{
	RPropertyPage::OnSize(type, cx, cy);

	if (type != SIZE_MINIMIZED && example_wnd_.m_hWnd)
	{
		CRect rect(0,0,0,0);
		border_wnd_.GetClientRect(rect);
		example_wnd_.MoveWindow(rect);
	}
}

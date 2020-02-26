/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// OptionsGeneral.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "OptionsGeneral.h"
#include "CalibrationDlg.h"
#include "FolderSelect.h"
#include "AdvancedOptions.h"
#include "ColorMngDlg.h"
#include "CatchAll.h"
#include "UIElements.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// OptionsGeneral property page

OptionsGeneral::OptionsGeneral() : RPropertyPage(OptionsGeneral::IDD)
{
	correct_aspect_ratio_ = FALSE;
//	dct_method_ = JDEC_INTEGER_HIQ;
//	display_method_ = DIB_DIRECT_2D;//DIB_SMOOTH_DRAW;
	horz_resolution_ = vert_resolution_ = 0.0f;
//	show_thumb_warning_ = save_tags_ = preload_ = false;
//	db_file_length_limit_mb_ = 1024;
	profiles_changed_ = false;
//	image_blending_ = 0;
//	generate_thumbs_ = GEN_THUMB_ALWAYS;
//	read_thumbs_from_db_ = false;
//	delete_cache_file_ = false;
//	allow_magnifying_above100_ = true;
//	close_app_ = true;
//	sharpening_ = 0;
}

OptionsGeneral::~OptionsGeneral()
{
}

void OptionsGeneral::DoDataExchange(CDataExchange* DX)
{
	CPropertyPage::DoDataExchange(DX);
	DDX_Control(DX, IDC_RES_STRING, resolution_wnd_);
	DDX_Control(DX, IDC_OPEN_PHOTO_APP, edit_path_photo_app_);
	DDX_Control(DX, IDC_OPEN_RAW_PHOTO_APP, edit_path_raw_photo_app_);
	DDX_Check(DX, IDC_CORRECT_ASPECT_RATIO, correct_aspect_ratio_);
	DDX_Text(DX, IDC_OPEN_PHOTO_APP, open_photo_app_);
	DDX_Text(DX, IDC_OPEN_RAW_PHOTO_APP, open_raw_photo_app_);
	DDX_Control(DX, IDC_FoV_CROP, foV_crop_wnd_);
}


BEGIN_MESSAGE_MAP(OptionsGeneral, RPropertyPage)
	ON_BN_CLICKED(IDC_RESOLUTION, OnResolution)
	ON_BN_CLICKED(IDC_OPEN_APP, OnOpenApp)
	ON_BN_CLICKED(IDC_OPEN_APP2, OnOpenRawApp)
	ON_BN_CLICKED(IDC_ICM, OnICMSetup)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// OptionsGeneral message handlers

void OptionsGeneral::OnResolution()
{
	CalibrationDlg dlg;
	dlg.X_res_ = horz_resolution_;
	dlg.Y_res_ = vert_resolution_;

	if (dlg.DoModal() == IDOK)
	{
		horz_resolution_ = static_cast<float>(dlg.X_res_);
		vert_resolution_ = static_cast<float>(dlg.Y_res_);
		UpdateRes();
	}
}


void OptionsGeneral::UpdateRes()
{
	oStringstream ost;
	ost << std::fixed << std::setprecision(1) << horz_resolution_ << _T(" dpi by ") << vert_resolution_ << _T(" dpi");
	resolution_wnd_.SetWindowText(ost.str().c_str());
}


BOOL OptionsGeneral::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	//if (CWnd* wnd= GetDlgItem(IDC_GRAYSCALE))
	//{
	//	WINDOWPLACEMENT wp;
	//	wnd->GetWindowPlacement(&wp);
	//	wnd->DestroyWindow();

	//	grayscale_wnd_.Create(this, wp.rcNormalPosition);
	//	grayscale_wnd_.SetGamma(gamma_);
	//}

	foV_crop_wnd_.SetHost(this);

	foV_crop_wnd_.InsertColumn(0, _T("Camera Model"), LVCFMT_LEFT, Pixels(172));
	foV_crop_wnd_.InsertColumn(1, _T("FoV Crop"), LVCFMT_RIGHT, Pixels(80), 0);

	foV_crop_wnd_.SetItemCount(data_.size());

	//WINDOWPLACEMENT wp;
	//if (CWnd* wnd= GetDlgItem(IDC_ADVANCED))
	//{
	//	wnd->GetWindowPlacement(&wp);
	//	CRect rect= wp.rcNormalPosition;
	//	CString text;
	//	wnd->GetWindowText(text);
	//	wnd->DestroyWindow();
	//	advanced_wnd_.Create(this, rect.TopLeft(), text, _T(""), 0, IDC_ADVANCED);
	//}

	UpdateRes();

	ResizeMgr().BuildMap(this);
	ResizeMgr().SetWndResizing(IDC_FRAME_1, DlgAutoResize::RESIZE_H);
	ResizeMgr().SetWndResizing(IDC_FRAME_2, DlgAutoResize::RESIZE_H);
	ResizeMgr().SetWndResizing(IDC_LABEL_1, DlgAutoResize::MOVE_V, DlgAutoResize::HALF_MOVE_V);
	ResizeMgr().SetWndResizing(IDC_FoV_CROP, DlgAutoResize::RESIZE);
	ResizeMgr().SetWndResizing(IDC_FRAME_3, DlgAutoResize::MOVE_V_RESIZE_H);
	ResizeMgr().SetWndResizing(IDC_LABEL_2, DlgAutoResize::MOVE_V);
	ResizeMgr().SetWndResizing(IDC_OPEN_PHOTO_APP, DlgAutoResize::MOVE_V_RESIZE_H);
	ResizeMgr().SetWndResizing(IDC_OPEN_APP, DlgAutoResize::MOVE);
	ResizeMgr().SetWndResizing(IDC_LABEL_3, DlgAutoResize::MOVE_V);
	ResizeMgr().SetWndResizing(IDC_OPEN_RAW_PHOTO_APP, DlgAutoResize::MOVE_V_RESIZE_H);
	ResizeMgr().SetWndResizing(IDC_OPEN_APP2, DlgAutoResize::MOVE);

	return true;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

/*
void OptionsGeneral::OnDeltaPosGammaSpin(NMHDR* nmhdr, LRESULT* result)
{
	NM_UPDOWN* NM_up_down = (NM_UPDOWN*)nmhdr;

	CString temp;
	GetDlgItemText(IDC_GAMMA, temp);
	double val= _tcstod(temp, 0);

	if (NM_up_down->iDelta > 0)
		val -= 0.1;
	else
		val += 0.1;

	if (val > 9.9)
		val = 9.9;
	else if (val < 0.1)
		val = 0.1;

	temp.Format(_T("%.1f"), val);
	SetDlgItemText(IDC_GAMMA, temp);

	*result = 0;
}
*/


void OptionsGeneral::OnOpenApp()
{
	CFolderSelect fs(this);

	CString file= fs.DoSelectFile(_T("Select Application for Opening JPEG Images"), CSIDL_PROGRAM_FILES, _T("exe"));

	if (!file.IsEmpty())
		SetDlgItemText(IDC_OPEN_PHOTO_APP, file);
}


void OptionsGeneral::OnOpenRawApp()
{
	CFolderSelect fs(this);

	CString file= fs.DoSelectFile(_T("Select Application for Opening Raw Photographs"), CSIDL_PROGRAM_FILES, _T("exe"));

	if (!file.IsEmpty())
		SetDlgItemText(IDC_OPEN_RAW_PHOTO_APP, file);
}

/*
void OptionsGeneral::OnAdvanced()
{
	AdvancedOptions dlg;
//	dlg.dct_method_ = dct_method_ == JDEC_INTEGER_HIQ ? 0 : 1;
	if (display_method_ == DIB_SMOOTH_DRAW)
		dlg.display_method_ = 0;
	else if (display_method_ == DIB_DIRECT_2D)
		dlg.display_method_ = 2;
	else
		dlg.display_method_ = 1;
	dlg.cache_size_ = image_cache_size_;
	dlg.preload_ = preload_;
	dlg.save_tags_ = save_tags_;
	dlg.show_warning_ = show_thumb_warning_;
	dlg.SetDbFileLimit(db_file_length_limit_mb_);
	dlg.image_blending_ = image_blending_;
	dlg.generate_thumbs_ = generate_thumbs_;
	dlg.thumb_access_ = read_thumbs_from_db_ ? 1 : 0;
	dlg.allow_magnifying_above100_ = allow_magnifying_above100_;
	dlg.close_app_ = close_app_;
	dlg.smooth_scroll_ = smooth_scroll_speed_;
	dlg.db_path_ = db_cache_path_;
	dlg.sharpening_ = sharpening_;

	if (dlg.DoModal() == IDOK)
	{
		dct_method_ = dlg.dct_method_ == 0 ? JDEC_INTEGER_HIQ : JDEC_INTEGER_LOQ;
		if (dlg.display_method_ == 0)
			display_method_ = DIB_SMOOTH_DRAW;
		else if (dlg.display_method_ == 2)
			display_method_ = DIB_DIRECT_2D;
		else
			display_method_ = DIB_FAST_DRAW;
		image_cache_size_ = dlg.cache_size_;
		preload_ = !!dlg.preload_;
		save_tags_ = !!dlg.save_tags_;
		show_thumb_warning_ = !!dlg.show_warning_;
		db_file_length_limit_mb_ = dlg.GetDbFileLimit();
		image_blending_ = dlg.image_blending_;
		generate_thumbs_ = static_cast<GenThumbMode>(dlg.generate_thumbs_);
		read_thumbs_from_db_ = dlg.thumb_access_ == 1;
		delete_cache_file_ = dlg.delete_cache_file_;
		allow_magnifying_above100_ = !!dlg.allow_magnifying_above100_;
		close_app_ = !!dlg.close_app_;
		smooth_scroll_speed_ = dlg.smooth_scroll_;
		db_cache_path_ = dlg.db_path_;
		sharpening_ = dlg.sharpening_;
	}
} */


void OptionsGeneral::OnICMSetup()
{
	try
	{
		ColorMngDlg dlg;

		dlg.monitor_viewer_ = new ICMProfile(monitor_viewer_.get());
		dlg.monitor_main_wnd_ = new ICMProfile(monitor_main_wnd_.get());
		if (default_printer_)
			dlg.default_printer_ = new ICMProfile(default_printer_.get());
		dlg.default_image_ = new ICMProfile(default_image_.get());

		if (dlg.DoModal() == IDOK)
			if (dlg.changed_)
			{
				monitor_viewer_ = dlg.monitor_viewer_;
				monitor_main_wnd_ = dlg.monitor_main_wnd_;
				//if (default_printer_)
					default_printer_ = dlg.default_printer_;
				default_image_ = dlg.default_image_;

				profiles_changed_ = true;
			}
	}
	CATCH_ALL
}


void OptionsGeneral::GetCellText(GridCtrl& ctrl, size_t row, size_t col, CString& text)
{
	if (row < data_.size())
	{
		if (col == 0)
			text = data_[row].first.c_str();
		else
			text = data_[row].second.c_str();
	}
	else
		text.Empty();	// new row
}


void OptionsGeneral::CellTextChanged(GridCtrl& ctrl, size_t row, size_t col, const CString& text)
{
	if (row < data_.size())
	{
		if (col == 0)
			data_[row].first = text;
		else
			data_[row].second = text;
	}
	else
	{
		if (!text.IsEmpty())
		{
			// add new row
			data_.resize(data_.size() + 1);
			if (col == 0)
				data_.back().first = text;
			else
				data_.back().second = text;

			foV_crop_wnd_.SetItemCount(data_.size());
		}
	}
}


void OptionsGeneral::Delete(GridCtrl& ctrl, size_t row, size_t col)
{
	if (row < data_.size())
	{
		data_.erase(data_.begin() + row);
		foV_crop_wnd_.SetItemCount(data_.size());
	}
}

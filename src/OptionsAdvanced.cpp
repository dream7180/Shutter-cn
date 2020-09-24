/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// OptionsAdvanced.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "OptionsAdvanced.h"
//#include "Database/ImageDatabase.h"
//#include "FolderSelect.h"
#include "GenThumbMode.h"
#include "CatchAll.h"
#include "WhistlerLook.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// OptionsAdvanced dialog
/*
int OptionsAdvanced::db_sizes_[]=
{
	256, 512, 768, 1024, 1536, 2048, 2048+512, 2048+1024, 2048+1536, 4*1024,
#ifdef _WIN64
	5*1024, 6*1024, 8*1024, 10*1024, 12*1024, 15*1024, 20*1024, 30*1024, 50*1024
#endif
};
*/
enum { GPU_RENDERING_ITEM= 2 };


OptionsAdvanced::OptionsAdvanced() : RPropertyPage(OptionsAdvanced::IDD)
{
	image_blending_ = 1;	// in slide show only

	//cache_size_ = 0;
	save_tags_ = FALSE;
	show_warning_ = FALSE;
	preload_ = FALSE;
	//db_size_ = 0;
	generate_thumbs_int_ = GEN_THUMB_EXCEPT_REMOVABLE_DRV;

	thumb_access_ = 0;
	//delete_cache_file_ = false;
	allow_magnifying_above100_ = false;
	allow_zoom_to_fill_ = false;
	percent_of_image_to_hide_ = 30;
	close_app_ = true;
	smooth_scroll_ = 10;
	sharpening_ = 0;
}


void OptionsAdvanced::OnRestoreDefaults()
{
	display_method_int_ = Direct2D::IsAvailable() ? GPU_RENDERING_ITEM : 0;
	//cache_size_ = 20;
	save_tags_ = true;
	show_warning_ = false;
//#ifdef _WIN64
	// 5 GB
	//db_file_length_limit_mb_ = 5 * 1024;
//#else
	// 3 GB
	//db_file_length_limit_mb_ = 3 * 1024;
//#endif
	image_blending_ = 1;
	generate_thumbs_int_ = GEN_THUMB_EXCEPT_REMOVABLE_DRV;
//	delete_cache_file_ = false;
	smooth_scroll_ = 10;
	smooth_scroll_slider_.SetPos(smooth_scroll_);
	sharpening_ = 30;
	sharpening_slider_.SetPos(sharpening_);

	UpdateData(false);
	//SetRAMLabel();
	//SetDbSizeLabel();
}


void OptionsAdvanced::DoDataExchange(CDataExchange* DX)
{
	RPropertyPage::DoDataExchange(DX);

	if (!DX->m_bSaveAndValidate)
	{
		if (display_method_ == DIB_SMOOTH_DRAW)
			display_method_int_ = 0;
		else if (display_method_ == DIB_DIRECT_2D)
			display_method_int_ = 2;
		else
			display_method_int_ = 1;

		generate_thumbs_int_ = generate_thumbs_;

		//SetDbFileLimit(db_file_length_limit_mb_);
	}

	//{{AFX_DATA_MAP(OptionsAdvanced)
	//DDX_Control(DX, IDC_CACHE_LABEL, db_size_label_wnd_);
	//DDX_Control(DX, IDC_DB_SIZE_LIMIT, db_size_slider_wnd_);
	//DDX_Control(DX, IDC_RAM, ram_wnd_);
	//DDX_Control(DX, IDC_CACHE, cache_slider_wnd_);
	DDX_CBIndex(DX, IDC_DISPLAY, display_method_int_);
	//DDX_Slider(DX, IDC_CACHE, cache_size_);
	DDX_Check(DX, IDC_SAVE_TAGS, save_tags_);
	DDX_Check(DX, IDC_SHOW_MSG, show_warning_);
	DDX_Check(DX, IDC_PRELOAD, preload_);
	//DDX_Slider(DX, IDC_DB_SIZE_LIMIT, db_size_);
	DDX_CBIndex(DX, IDC_BLENDING, image_blending_);
	DDX_CBIndex(DX, IDC_GEN_THUMBS, generate_thumbs_int_);
	//}}AFX_DATA_MAP
	//DDX_Control(DX, IDC_CLEAR_CACHE, btn_clear_);
//	DDX_Control(DX, IDC_HELP_BTN, btn_help_);
	DDX_Radio(DX, IDC_THUMB_ACCESS, thumb_access_);
	DDX_Check(DX, IDC_ALLOW_100, allow_magnifying_above100_);
	DDX_Check(DX, IDC_ALLOW_FILL, allow_zoom_to_fill_);
	DDX_Check(DX, IDC_CLOSE_APP, close_app_);
	DDX_Control(DX, IDC_SCROLL_SPEED, smooth_scroll_slider_);
	DDX_Slider(DX, IDC_SCROLL_SPEED, smooth_scroll_);
	//DDX_Control(DX, IDC_DB_PATH, db_path_ctrl_);
	DDX_Control(DX, IDC_SHARPENING, sharpening_slider_);
	DDX_Slider(DX, IDC_SHARPENING, sharpening_);
	DDX_Text(DX, IDC_IMG_PERCENT, percent_of_image_to_hide_);

	if (DX->m_bSaveAndValidate)
	{
		if (display_method_int_ == 0)
			display_method_ = DIB_SMOOTH_DRAW;
		else if (display_method_int_ == 2)
			display_method_ = DIB_DIRECT_2D;
		else
			display_method_ = DIB_FAST_DRAW;

		//db_file_length_limit_mb_ = GetDbFileLimit();

		generate_thumbs_ = static_cast<GenThumbMode>(generate_thumbs_int_);
	}
}


BEGIN_MESSAGE_MAP(OptionsAdvanced, RPropertyPage)
	//{{AFX_MSG_MAP(OptionsAdvanced)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_RESET, OnRestoreDefaults)
	//ON_BN_CLICKED(IDC_CLEAR_CACHE, OnClearCache)
//	ON_BN_CLICKED(IDC_HELP_BTN, OnHelpBtn)
	//ON_BN_CLICKED(IDC_SET_PATH, OnSetDbPath)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// OptionsAdvanced message handlers

BOOL OptionsAdvanced::OnInitDialog()
{
	RPropertyPage::OnInitDialog();

	//cache_slider_wnd_.SetRange(0, 100);
	//cache_slider_wnd_.SetTicFreq(10);
	//cache_slider_wnd_.SetPos(cache_size_);

	//SetRAMLabel();

	//db_size_slider_wnd_.SetRange(0, array_count(db_sizes_) - 1, true);
	//db_size_slider_wnd_.SetPos(db_size_);

	//SetDbSizeLabel();

	//ImageDatabase dbImages;
	//if (!dbImages.HaveDbFiles())
	//	btn_clear_.EnableWindow(false);
	//db_path_ctrl_.SetWindowText(db_path_.c_str());

	smooth_scroll_slider_.SetRange(1, 30);
	smooth_scroll_slider_.SetTicFreq(2);
	smooth_scroll_slider_.SetPos(smooth_scroll_);

	sharpening_slider_.SetRange(0, 100);
	sharpening_slider_.SetTicFreq(10);
	sharpening_slider_.SetPos(sharpening_);

	if (!Direct2D::IsAvailable())
		if (CComboBox* cb= static_cast<CComboBox*>(GetDlgItem(IDC_DISPLAY)))
			cb->DeleteString(GPU_RENDERING_ITEM);

	if (CSpinButtonCtrl* spin= static_cast<CSpinButtonCtrl*>(GetDlgItem(IDC_IMG_SPIN)))
		spin->SetRange32(1, 99);

	if (CEdit* edit= static_cast<CEdit*>(GetDlgItem(IDC_IMG_PERCENT)))
		edit->SetLimitText(2);

	ResizeMgr().BuildMap(this);
	ResizeMgr().SetWndResizing(IDC_FRAME_1, DlgAutoResize::RESIZE_H);
	//ResizeMgr().SetWndResizing(IDC_FRAME_2, DlgAutoResize::RESIZE_H);
	//ResizeMgr().SetWndResizing(IDC_FRAME_3, DlgAutoResize::RESIZE_H);
	//ResizeMgr().SetWndResizing(IDC_DB_PATH, DlgAutoResize::RESIZE_H);
	//ResizeMgr().SetWndResizing(IDC_SET_PATH, DlgAutoResize::MOVE_H);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

/*
void OptionsAdvanced::OnHScroll(UINT sb_code, UINT pos, CScrollBar* scroll_bar)
{
//	if (scroll_bar == &cache_slider_wnd_)
		SetRAMLabel();
//	else
		SetDbSizeLabel();
}


void OptionsAdvanced::SetRAMLabel()
{
	if (cache_slider_wnd_.m_hWnd == 0)
		return;

	int percent= std::max(0, cache_slider_wnd_.GetPos());

	// check memory conditions
	extern DWORDLONG GetPhysicalMemoryStatus();

	DWORDLONG total_phys_mem= GetPhysicalMemoryStatus();

	// amount of physical memory
	int RAM_MB= percent > 0 ? static_cast<int>(total_phys_mem / 1024 * percent / 100 / 1024) : 0;

	CString ram;
	ram.Format(_T("= %d MB 内存"), RAM_MB);

	SetDlgItemText(IDC_RAM, ram);
}


void OptionsAdvanced::OnClearCache()
{
	// deletion flag: file can only be deleted on exit

	//ImageDatabase dbImages;
	//if (!dbImages.RemoveDbFiles())
	//	MessageBox(_T("Cannot delete cache files. Please make sure image loading is finished."));
	//else
	{
		if (GetFocus() == &btn_clear_)
			NextDlgCtrl();
		btn_clear_.EnableWindow(false);
		delete_cache_file_ = true;
	}
}


void OptionsAdvanced::SetDbSizeLabel()
{
	if (db_size_slider_wnd_.m_hWnd == 0)
		return;

	int db_size= std::max(0, db_size_slider_wnd_.GetPos());

	CString size;
	size.Format(_T("= %g GB"), static_cast<double>(db_sizes_[db_size]) / 1024.0);

	SetDlgItemText(IDC_CACHE_LABEL, size);
}


void OptionsAdvanced::SetDbFileLimit(uint32 db_file_length_limit_mb)
{
	int* size= std::lower_bound(db_sizes_, db_sizes_ + array_count(db_sizes_), static_cast<int>(db_file_length_limit_mb));
	if (size < db_sizes_ + array_count(db_sizes_))
		db_size_ = static_cast<int>(size - db_sizes_);
	else
	{
		ASSERT(false);
		db_size_ = 3;
	}
}


uint32 OptionsAdvanced::GetDbFileLimit() const
{
	if (db_size_ >= 0 && db_size_ < array_count(db_sizes_))
		return db_sizes_[db_size_];

	ASSERT(false);
	return 1024;
}


void OptionsAdvanced::OnHelpBtn()
{
	extern void OpenHelp(const TCHAR* initial_page);

	OpenHelp(_T("OptionsAdvanced.htm"));
}


void OptionsAdvanced::OnSetDbPath()
{
	CFolderSelect fs(this);

	String path= fs.DoSelectPath(_T("选择缓存文件的储存位置"), db_path_.c_str());

	if (!path.empty() && path != db_path_)
	{
		try
		{
			String fname= ImageDatabase::CreateDbFolderIfNeeded(path);

			// check read/write access
			CFile test(fname.c_str(), CFile::modeNoTruncate | CFile::modeReadWrite | CFile::modeCreate);
			test.Close();

			db_path_ = path;
			db_path_ctrl_.SetWindowText(path.c_str());
		}
		CATCH_ALL_W(&set_db_path_)
	}
}
*/
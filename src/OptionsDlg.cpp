/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// OptionsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "OptionsDlg.h"
#include "Config.h"
#include "RString.h"
#include <math.h>
#include <float.h>
#include "ImgDb.h"
#include "clamp.h"

//#ifdef _DEBUG
//#define new DEBUG_NEW
//#undef THIS_FILE
//static char THIS_FILE[] = __FILE__;
//#endif

#include "DibCache.h"
extern AutoPtr<DibCache> global_dib_cache;


/////////////////////////////////////////////////////////////////////////////
// OptionsDlg

extern Config g_Settings;

int OptionsDlg::active_page_index_= 0;


int CALLBACK PropSheetCallback(HWND hWnd, UINT message, LPARAM lParam)
{
	extern int CALLBACK AfxPropSheetCallback(HWND, UINT message, LPARAM lParam);

	int res= AfxPropSheetCallback(hWnd, message, lParam);

//TRACE(L"callbaack msg: %d\n", message);

	switch (message)
	{
	case PSCB_PRECREATE:
		// Set our own window styles
		reinterpret_cast<DLGTEMPLATE*>(lParam)->style |= DS_3DLOOK | DS_SETFONT | WS_THICKFRAME | WS_SYSMENU | WS_POPUP | WS_VISIBLE | WS_CAPTION;
		break;

	case PSCB_INITIALIZED:
		break;
	}

	return res;
}


OptionsDlg::OptionsDlg(CWnd* parent_wnd, std::vector<uint16>& columns_idx, std::vector<uint16>& balloon_fields, const boost::function<void (CWnd* parent)>& define_columns, int select_page, Columns& columns)
 : CPropertySheet(IDS_OPTIONS, parent_wnd, select_page), page_columns_(columns), page_balloons_(columns),
	position_(L"OptionsDlg")
{
	min_size_ = CSize(0, 0);
	m_strCaption = RString(IDS_OPTIONS);
	m_psh.pszCaption = m_strCaption;

	m_psh.dwFlags |= PSH_NOAPPLYNOW | PSH_HASHELP | PSH_USECALLBACK;
	//m_psh.dwFlags &= ~(/*PSH_HASHELP |*/ PSH_USECALLBACK);
	m_psh.pfnCallback = PropSheetCallback;

	AddPage(&page_general_);
	AddPage(&page_columns_);
	AddPage(&page_appearance_);
	AddPage(&page_file_types_);
	AddPage(&page_balloons_);
//#ifdef _DEBUG
//	AddPage(&page_shortcuts_);
//#endif
	AddPage(&page_cache_);
	AddPage(&page_advanced_);

	pages_[0] = &page_general_;
	pages_[1] = &page_columns_;
	pages_[2] = &page_appearance_;
	pages_[3] = &page_file_types_;
	pages_[4] = &page_balloons_;
	//pages_[5] = &page_shortcuts_;
	pages_[5] = &page_cache_;
	pages_[6] = &page_advanced_;

	dx_.Add(page_general_.correct_aspect_ratio_		, g_Settings.correct_CRT_aspect_ratio_);
	dx_.Add(page_general_.horz_resolution_			, g_Settings.horz_resolution_);
	dx_.Add(page_general_.vert_resolution_			, g_Settings.vert_resolution_);
	dx_.Add(page_cache_.open_photo_app_			, g_Settings.open_photo_app_);
	dx_.Add(page_cache_.open_raw_photo_app_		, g_Settings.open_raw_photo_app_);
	dx_.Add(page_general_.monitor_viewer_			, g_Settings.viewer_);
	dx_.Add(page_general_.monitor_main_wnd_			, g_Settings.main_wnd_);
	dx_.Add(page_general_.default_printer_			, g_Settings.default_printer_);
	dx_.Add(page_general_.default_image_			, g_Settings.default_photo_);
	dx_.Add(page_general_.data_						, g_Settings.field_of_view_crop_);

	dx_.Add(page_advanced_.preload_					, g_Settings.preload_photos_);
	dx_.Add(page_advanced_.display_method_			, g_Settings.display_method_);
	dx_.Add(page_cache_.cache_size_				, g_Settings.image_cache_size_);
	dx_.Add(page_advanced_.save_tags_				, g_Settings.save_tags_to_photo_);
	dx_.Add(page_advanced_.show_warning_			, g_Settings.warn_about_broken_thumbnail_img_);
	dx_.Add(page_cache_.db_file_length_limit_mb_	, g_Settings.db_file_length_limit_mb_);
	dx_.Add(page_advanced_.image_blending_			, g_Settings.image_blending_);
	dx_.Add(page_advanced_.generate_thumbs_			, g_Settings.regenerate_thumbnails_);
	dx_.Add(page_advanced_.thumb_access_			, g_Settings.read_thumbs_from_db_);
	dx_.Add(page_advanced_.allow_magnifying_above100_ , g_Settings.allow_magnifying_above100_);
	dx_.Add(page_advanced_.close_app_				, g_Settings.close_app_when_viewer_exits_);
	dx_.Add(page_advanced_.smooth_scroll_			, g_Settings.smooth_scrolling_speed_);
	dx_.Add(page_cache_.db_path_					, g_Settings.img_cache_db_path_);
	dx_.Add(page_advanced_.sharpening_				, g_Settings.thumbnail_sharpening_);
	dx_.Add(page_advanced_.allow_zoom_to_fill_		, g_Settings.allow_zoom_to_fill_);
	dx_.Add(page_advanced_.percent_of_image_to_hide_, g_Settings.percent_of_image_to_hide_);

	page_columns_.columns_ = &columns_idx;
	page_columns_.define_columns_ = define_columns;

//	dx_.Add(page_appearance_.main_default_colors_	, g_Settings.list_ctrl_sys_colors_);
	dx_.Add(page_appearance_.main_wnd_colors_		, g_Settings.main_wnd_colors_);
	//dx_.Add(page_appearance_.pane_caption_colors_	, g_Settings.pane_caption_colors_);
//	dx_.Add(page_appearance_.viewer_default_colors_	, g_Settings.viewer_default_colors_);
	dx_.Add(page_appearance_.ui_gamma_correction_	, g_Settings.viewer_ui_gamma_correction_);
	dx_.Add(page_appearance_.viewer_wnd_colors_		, g_Settings.viewer_wnd_colors_);
	dx_.Add(page_appearance_.description_font_		, g_Settings.description_font_);
	dx_.Add(page_appearance_.tag_font_				, g_Settings.img_tag_font_);

	page_balloons_.columns_ = &balloon_fields;

//	page_file_types_; // TODO

	if (select_page < 0)
		SetActivePage(active_page_index_);
}


OptionsDlg::~OptionsDlg()
{}


BEGIN_MESSAGE_MAP(OptionsDlg, CPropertySheet)
	ON_WM_DESTROY()
	ON_WM_SIZE()
	//ON_COMMAND(ID_HELP, OnHelp)
	ON_MESSAGE(WM_APP, OnResizePages)
	ON_WM_GETMINMAXINFO()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// OptionsDlg message handlers

bool operator != (const LOGFONT& f1, const LOGFONT& f2)
{
	return memcmp(&f1, &f2, sizeof f1) != 0;
}



INT_PTR OptionsDlg::DoModal()
{
	if (CPropertySheet::DoModal() != IDOK)
		return IDCANCEL;

	bool changed= false;
	bool gamma_changed= false;

	if (page_general_.profiles_changed_)
	{
		g_Settings.viewer_ = page_general_.monitor_viewer_;
		g_Settings.main_wnd_ = page_general_.monitor_main_wnd_;
		g_Settings.default_printer_ = page_general_.default_printer_;
		g_Settings.default_photo_ = page_general_.default_image_;
		changed = gamma_changed = true;
	}

	if (page_columns_.changed_)
		changed = true;

	if (page_balloons_.changed_)
		changed = true;

	if (page_file_types_.markers_.size() == g_Settings.file_types_.size() &&
		page_file_types_.scan_.size() == g_Settings.file_types_.size())
	{
		const size_t count= g_Settings.file_types_.size();
		for (size_t i= 0; i < count; ++i)
		{
			if (changed ||
				page_file_types_.markers_[i] != g_Settings.file_types_[i].show_marker ||
				page_file_types_.scan_[i]    != g_Settings.file_types_[i].scan ||
				page_file_types_.no_exif_[i] != g_Settings.file_types_[i].show_no_exif)
			{
				changed = true;
				g_Settings.file_types_[i].scan = page_file_types_.scan_[i];
				g_Settings.file_types_[i].show_marker = page_file_types_.markers_[i];
				g_Settings.file_types_[i].show_no_exif = page_file_types_.no_exif_[i];
			}
		}
	}

	if (page_advanced_.sharpening_ != g_Settings.thumbnail_sharpening_)
	{
		if (global_dib_cache)
			global_dib_cache->RemoveAll();
//		changed = true;
	}

	if (page_cache_.delete_cache_file_)
		::SetDeleteFlagForImageDataBase(true);

	if (dx_.Read())
		changed = true;

	g_Settings.percent_of_image_to_hide_ = clamp(g_Settings.percent_of_image_to_hide_, 0, 99);

	//-------------------------------------------------

	if (gamma_changed)
		g_Settings.NotifyGammaChanged();

	if (changed)
		return IDOK;

	return IDCANCEL;
}


BOOL OptionsDlg::OnInitDialog()
{
	CPropertySheet::OnInitDialog();

	SetIcon(AfxGetApp()->LoadIcon(IDR_MAINFRAME), false);

	resize_.BuildMap(this);

	const int TAB= 0x3020;

	resize_.SetWndResizing(IDOK, DlgAutoResize::MOVE);
	resize_.SetWndResizing(IDCANCEL, DlgAutoResize::MOVE);
	//resize_.SetWndResizing(IDHELP, DlgAutoResize::MOVE);

	resize_.SetWndResizing(TAB, DlgAutoResize::RESIZE);

	CRect rect(0,0,0,0);
	GetWindowRect(rect);
	min_size_ = rect.Size();

	CRect wnd_rect= rect;
	if (position_.IsRegEntryPresent())
		wnd_rect = position_.GetLocation(true);
	else
		wnd_rect.InflateRect(10, 0);		// initially start bigger than at min size
	MoveWindow(wnd_rect);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void OptionsDlg::OnSize(UINT type, int cx, int cy)
{
	if (type != SIZE_MINIMIZED)
	{
		resize_.Resize();
		ResizePages();
	}
	CPropertySheet::OnSize(type, cx, cy);
}


void OptionsDlg::OnGetMinMaxInfo(MINMAXINFO* mmi)
{
	CPropertySheet::OnGetMinMaxInfo(mmi);
	if (min_size_.cx && min_size_.cy)
	{
		mmi->ptMinTrackSize.x = min_size_.cx;
		mmi->ptMinTrackSize.y = min_size_.cy;
	}
}


void OptionsDlg::OnDestroy()
{
	active_page_index_ = GetActiveIndex();

	position_.StoreState(*this);

	CPropertySheet::OnDestroy();
}

/*
void OptionsDlg::OnHelp()
{
	extern void OpenHelp(const TCHAR* initial_page);

	OpenHelp(_T("OptionsDlg.htm"));
}
*/

BOOL OptionsDlg::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* result)
{
	NMHDR* nmh= reinterpret_cast<NMHDR*>(lParam);

	// the sheet resizes the page whenever it is activated
	// so we need to resize it to what we want
	if (nmh->code == TCN_SELCHANGE)
		// user-defined message needs to be posted because page must
		// be resized after TCN_SELCHANGE has been processed
		PostMessage(WM_APP);

	return CPropertySheet::OnNotify(wParam, lParam, result);
}


LRESULT OptionsDlg::OnResizePages(WPARAM, LPARAM)
{
	ResizePages();
	return 0;
}


void OptionsDlg::ResizePages()
{
	if (CTabCtrl* tab= GetTabControl())
	{
		CRect rect(0,0,0,0);
		tab->GetClientRect(rect);
		tab->AdjustRect(false, rect);
		WINDOWPLACEMENT wp;
		if (tab->GetWindowPlacement(&wp))
		{
			rect.OffsetRect(wp.rcNormalPosition.left, wp.rcNormalPosition.top);

			for (int i= 0; i < PAGES; ++i)
				if (pages_[i] && pages_[i]->m_hWnd)
					pages_[i]->MoveWindow(rect);
		}
	}
}

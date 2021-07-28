/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// ResizeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "ResizeDlg.h"
#include "FolderSelect.h"
#include "RString.h"
#include "EnableCtrl.h"
#include "Path.h"
#include "BalloonMsg.h"
#include "ItemIdList.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
extern String ReplaceIllegalChars(const String& text);


static const TCHAR* REGISTRY_ENTRY_RESIZE	= _T("ResizeDlg");
static const TCHAR* REG_WIDTH				= _T("WidthPx");
static const TCHAR* REG_SIZE_PERCENT		= _T("SizePercent");
static const TCHAR* REG_FIXED_SIZE			= _T("FixedSize");
static const TCHAR* REG_SEL_DIR				= _T("SelectedDir");
static const TCHAR* REG_FORMAT				= _T("OutputFmt");
static const TCHAR* REG_SUFFIX				= _T("Suffix");
static const TCHAR* REG_OUTPUT_DIR			= _T("OutputDir");
static const TCHAR* REG_JPEG_QUALITY		= _T("Compression");
static const TCHAR* REG_JPEG_BASELINE		= _T("JPEGBaselineStd");
static const TCHAR* REG_JPEG_PROGRESSIVE	= _T("JPEGProgressive");
static const TCHAR* REG_JPEG_COPY_EXIF		= _T("PreserveEXIF");
static const TCHAR* REG_JPEG_COPY_IPTC		= _T("CopyIPTC");
static const TCHAR* REG_SIZE_SLIDER			= _T("Size");
static const TCHAR* REG_QUALITY				= _T("Quality");
static const TCHAR* REG_METHOD				= _T("Method");
static const TCHAR* REG_RECENT_DEST_PATH	= _T("\\RecentDestPaths");

const int MAX_RECENT_PATHS= 100;

/////////////////////////////////////////////////////////////////////////////
// ResizeDlg dialog


ResizeDlg::ResizeDlg(CWnd* parent, double ratio, OrientationOfImages orientation, UINT dlg_id/*= IDD*/)
	: DialogChild(dlg_id, parent), ratio_(ratio), orientation_(orientation)
{
	//{{AFX_DATA_INIT(ResizeDlg)
	size_percent_ = 100;
	width_ = 640;
	height_ = 480;
	size_selection_ = 0;
	same_dir_ = 0;
	output_format_ = 0;
	suffix_ = _T("");
	dest_path_ = _T("");
	size_ = 0;
	quality_ = 0;
	compr_level_ = -1;
	//}}AFX_DATA_INIT
	image_size_ = CSize(0, 0);
	update_ = false;
	force_jpeg_ = false;

	registry_ = REGISTRY_ENTRY_RESIZE;
	options_dlg_id_ = CResizeOptionsDlg::IDD;
}


void ResizeDlg::DoDataExchange(CDataExchange* DX)
{
	DialogChild::DoDataExchange(DX);
	//{{AFX_DATA_MAP(ResizeDlg)
	DDX_Control(DX, IDC_JPEG_LABEL_2, compr_ratio_wnd_);
	DDX_Control(DX, IDC_SPIN_COMPR, spin_compr_level_);
	DDX_Control(DX, IDC_COMPR_LEVEL, edit_compr_level_);
	DDX_Control(DX, IDC_SIZE_SLD, slider_size_);
	DDX_Control(DX, IDC_QUALITY, slider_quality_);
	DDX_Control(DX, IDC_JPEG, btn_jpeg_);
	DDX_Control(DX, IDC_BMP, btnBMP_);
	DDX_Control(DX, IDC_WIDTH_PX, edit_width_);
	DDX_Control(DX, IDC_HEIGHT_PX, edit_height_);
	DDX_Control(DX, IDC_SIZE_PERCENT, edit_size_percent_);
	DDX_Control(DX, IDC_PERCENTAGE_SIZE, btn_relative_size_);
	DDX_Control(DX, IDC_FIXED_SIZE, btn_fixed_size_);
	DDX_Control(DX, IDC_EXAMPLE, example_wnd_);
	DDX_Control(DX, IDC_SUFFIX, edit_suffix_);
	DDX_Control(DX, IDC_SAME_DIR, btn_same_folder_);
	DDX_Text(DX, IDC_SIZE_PERCENT, size_percent_);
	DDV_MinMaxInt(DX, size_percent_, 1, 500);
	DDX_Text(DX, IDC_WIDTH_PX, width_);
	DDV_MinMaxInt(DX, width_, 1, 9999);
	DDX_Text(DX, IDC_HEIGHT_PX, height_);
	DDV_MinMaxInt(DX, height_, 1, 9999);
	DDX_Radio(DX, IDC_FIXED_SIZE, size_selection_);
	DDX_Radio(DX, IDC_SAME_DIR, same_dir_);
	DDX_Radio(DX, IDC_JPEG, output_format_);
	DDX_Text(DX, IDC_SUFFIX, suffix_);
	DDX_Text(DX, IDC_DEST_PATH, dest_path_);
	DDX_Slider(DX, IDC_SIZE_SLD, size_);
	DDX_Slider(DX, IDC_QUALITY, quality_);
	DDX_Text(DX, IDC_COMPR_LEVEL, compr_level_);
	//}}AFX_DATA_MAP
	if (DX->m_bSaveAndValidate)
	{
		if (size_selection_ == 0 || TranslateSizeSlider(slider_size_.GetPos()) != 0)
		{
			image_size_.cx = width_;
			image_size_.cy = height_;
		}
		else
		{
			image_size_.cx = -size_percent_;
			image_size_.cy = -size_percent_;
		}
	}
}


BEGIN_MESSAGE_MAP(ResizeDlg, DialogChild)
	//{{AFX_MSG_MAP(ResizeDlg)
	ON_BN_CLICKED(IDC_SAME_DIR, OnSameDir)
	ON_BN_CLICKED(IDC_SELECT, OnSelectDir)
	ON_BN_CLICKED(IDC_FIXED_SIZE, OnFixedSize)
	ON_BN_CLICKED(IDC_PERCENTAGE_SIZE, OnPercentageSize)
	ON_BN_CLICKED(IDC_BROWSE, OnBrowse)
	ON_EN_CHANGE(IDC_SUFFIX, OnChangeSuffix)
	ON_BN_CLICKED(IDC_JPEG, OnJpeg)
	ON_BN_CLICKED(IDC_BMP, OnBmp)
	ON_EN_CHANGE(IDC_HEIGHT_PX, OnChangeHeight)
	ON_EN_CHANGE(IDC_WIDTH_PX, OnChangeWidth)
	ON_WM_VSCROLL()
	ON_EN_CHANGE(IDC_COMPR_LEVEL, OnChangeComprLevel)
	ON_BN_CLICKED(IDC_OPTIONS, OnOptions)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// ResizeDlg message handlers
static const int COMPR_RNG= 100;

BOOL ResizeDlg::OnInitDialog()
{
	CWinApp* app= AfxGetApp();
	size_selection_	= app->GetProfileInt(registry_, REG_FIXED_SIZE, size_selection_);
	if (ratio_ <= 0.0)
		size_selection_ = 1;
	else if (size_selection_ < 0)
		size_selection_ = 0;
	same_dir_		= app->GetProfileInt(registry_, REG_SEL_DIR, same_dir_);
	output_format_	= app->GetProfileInt(registry_, REG_FORMAT, output_format_);

	dlg_options_.baseline_jpeg_			= app->GetProfileInt(registry_, REG_JPEG_BASELINE, dlg_options_.baseline_jpeg_);
	dlg_options_.progressive_jpeg_		= app->GetProfileInt(registry_, REG_JPEG_PROGRESSIVE, dlg_options_.progressive_jpeg_);
	dlg_options_.preserve_exif_block_	= app->GetProfileInt(registry_, REG_JPEG_COPY_EXIF, dlg_options_.preserve_exif_block_);
	dlg_options_.resizing_method_		= app->GetProfileInt(registry_, REG_METHOD, dlg_options_.resizing_method_);
	dlg_options_.copyTags_				= app->GetProfileInt(registry_, REG_JPEG_COPY_IPTC, dlg_options_.copyTags_);

	compr_level_	= app->GetProfileInt(registry_, REG_JPEG_QUALITY, COMPR_RNG - 10);
	suffix_			= app->GetProfileString(registry_, REG_SUFFIX, _T("_th"));
	dest_path_		= app->GetProfileString(registry_, REG_OUTPUT_DIR, _T("c:\\"));
	size_			= app->GetProfileInt(registry_, REG_SIZE_SLIDER, size_);
	quality_		= app->GetProfileInt(registry_, REG_QUALITY, quality_);

	dest_path_combo_.SubclassDlgItem(IDC_DEST_PATH, this);
	COMBOBOXINFO ci;
	memset(&ci, 0, sizeof ci);
	ci.cbSize = sizeof ci;
	if (dest_path_combo_.GetComboBoxInfo(&ci))
		dest_path_editbox_.SubclassWindow(ci.hwndItem);
	else
		dest_path_editbox_.SubclassWindow(::GetWindow(dest_path_combo_.m_hWnd, GW_CHILD));

	DialogChild::OnInitDialog();

	if (orientation_ != LANDSCAPE_ORIENTATION)
	{
		CWnd* w= GetDlgItem(IDC_WIDTH_1_LABEL);
		CWnd* h= GetDlgItem(IDC_HEIGHT_1_LABEL);

		if (w && h)
		{
			if (orientation_ == PORTRAIT_ORIENTATION)
			{
				// all images in portrait mode; switch width/height labels

				//CString s1, s2;
				//w->GetWindowText(s1);
				//h->GetWindowText(s2);
				//w->SetWindowText(s2);
				//h->SetWindowText(s1);

				WINDOWPLACEMENT wp1;
				edit_width_.GetWindowPlacement(&wp1);

				WINDOWPLACEMENT wp2;
				edit_height_.GetWindowPlacement(&wp2);

				edit_width_.MoveWindow(&wp2.rcNormalPosition);
				edit_height_.MoveWindow(&wp1.rcNormalPosition);

				edit_width_.SetWindowPos(h, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);
				edit_height_.SetWindowPos(w, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);
			}
			else if (orientation_ == MIXED_ORIENTATION)
			{
				// landscape and portrait images present; modify labels to avoid confusion
				w->SetWindowText(_T("New"));
				h->SetWindowText(_T("Size"));
			}
		}
	}

	//SubclassHelpBtn(_T("ToolResize.htm"));

	if (!dlg_options_.Create(GetParent(), options_dlg_id_))
	{
		EndDialog(IDCANCEL);
		return true;
	}

	SetFooterDlg(&dlg_options_);
	SetDlgItemText(IDC_OPTIONS, _T("显示选项(&O) >>"));

	if (ratio_ <= 0.0)
		btn_fixed_size_.EnableWindow(false);

	SetDlgItemInt(IDC_WIDTH_PX,  app->GetProfileInt(registry_, REG_WIDTH, 640));

	spin_compr_level_.SetRange(0, COMPR_RNG);
	SetDlgItemInt(IDC_SIZE_PERCENT,  app->GetProfileInt(registry_, REG_SIZE_PERCENT, COMPR_RNG));

	slider_size_.SetRange(0, 4, true);
	slider_size_.SetPos(size_);

	slider_quality_.SetRange(0, 4, true);
	slider_quality_.SetPos(quality_);

	edit_suffix_.LimitText(100);

	// read recent paths
	::RecentPaths(recent_paths_, false, registry_ + REG_RECENT_DEST_PATH, MAX_RECENT_PATHS);

	const size_t count= recent_paths_.size();
	for (size_t i= 0; i < count; ++i)
	{
		CString path= recent_paths_[i]->GetPath();
		if (!path.IsEmpty())
			dest_path_combo_.AddString(path);
	}

	/*
	if (auto_complete_.Create())
	{
		const size_t count= recent_paths_.size();
		for (size_t i= 0; i < count; ++i)
		{
			CString path= recent_paths_[i]->GetPath();
			if (!path.IsEmpty())
				recent_path_strings_.push_back(static_cast<const TCHAR*>(path));
		}
recent_path_strings_.push_back(String(L"c:\\bla bla"));
recent_path_strings_.push_back(String(L"c:\\xbla bla"));
recent_path_strings_.push_back(String(L"c:\\fbla bla"));
recent_path_strings_.push_back(String(L"c:\\gbla bla"));
recent_path_strings_.push_back(String(L"c:\\bbla bla"));

		auto_complete_.ControlEditBox(&dest_path_editbox_, &recent_path_strings_);
	}
*/
	UpdateSize();
	UpdateQuality();
	UpdateDirs();
//	UpdateDims();
	OnChangeSuffix();
//	UpdateFmt();

	return true;	// return TRUE unless you set the focus to a control
					// EXCEPTION: OCX Property Pages should return FALSE
}


void ResizeDlg::OnSameDir()
{
	UpdateDirs();
}

void ResizeDlg::OnSelectDir()
{
	UpdateDirs();
}

void ResizeDlg::UpdateDirs()
{
	if (dest_path_combo_.m_hWnd != 0)
	{
		bool same_dir= !!IsDlgButtonChecked(IDC_SAME_DIR);
		dest_path_combo_.EnableWindow(!same_dir);
		if (CWnd* btn= GetDlgItem(IDC_BROWSE))
			btn->EnableWindow(!same_dir);
	}
}


void ResizeDlg::OnFixedSize()
{
	UpdateDims();
}

void ResizeDlg::OnPercentageSize()
{
	UpdateDims();
}


void ResizeDlg::ReadOnlyCtrl(CEdit* wnd, bool read_only)
{
	if (wnd == 0)
		return;

	wnd->SetReadOnly(read_only);

	if (read_only)
		wnd->ModifyStyle(WS_TABSTOP, 0);
	else
		wnd->ModifyStyle(0, WS_TABSTOP);

	// to avoid repainting problems redraw frame
	wnd->SetWindowPos(0, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED);
}


void ResizeDlg::UpdateDims()
{
	if (edit_width_.m_hWnd != 0 && slider_size_.m_hWnd)
	{
		bool abs_size= true;
		bool rel_size= false;
		int pos= TranslateSizeSlider(slider_size_.GetPos());
		bool custom= pos == 0;
		bool original_size= pos == -1;

		if (custom)	// custom settings?
		{
			//rel_size = !!IsDlgButtonChecked(IDC_PERCENTAGE_SIZE);
			//abs_size = !rel_size
			abs_size = !!IsDlgButtonChecked(IDC_FIXED_SIZE);
			rel_size = !abs_size;
		}

		EnableCtrl(&btn_fixed_size_, custom);
		EnableCtrl(&btn_relative_size_, custom);

		ReadOnlyCtrl(&edit_width_, !custom);
		ReadOnlyCtrl(&edit_height_, !custom);
		{
			bool enable= abs_size && !original_size;
			bool hide_disabled= custom || original_size;
			EnableCtrl(&edit_width_, enable, hide_disabled);
			EnableCtrl(&edit_height_, enable, hide_disabled);
			EnableCtrl(GetDlgItem(IDC_WIDTH_1_LABEL), enable, hide_disabled);
			EnableCtrl(GetDlgItem(IDC_HEIGHT_1_LABEL), enable, hide_disabled);
			EnableCtrl(GetDlgItem(IDC_WIDTH_2_LABEL), enable, hide_disabled);
			EnableCtrl(GetDlgItem(IDC_HEIGHT_2_LABEL), enable, hide_disabled);
	}

		EnableCtrl(&edit_size_percent_, rel_size);
		EnableCtrl(GetDlgItem(IDC_SIZE_LABEL), rel_size);
		EnableCtrl(GetDlgItem(IDC_SIZE_LABEL_2), rel_size);
	}
}


void ResizeDlg::OnBrowse()
{
	CString dest_path;
	dest_path_combo_.GetWindowText(dest_path);

	if (dest_path.IsEmpty())
		dest_path = _T("c:\\");

	CFolderSelect fs(this);
	CString path= fs.DoSelectPath(RString(IDS_SELECT_OUTPUT_FOLDER), dest_path);

	if (path.IsEmpty())
		return;

	dest_path_combo_.SetWindowText(path);
}


void ResizeDlg::OnChangeSuffix()
{
	UpdateExample();
}

void ResizeDlg::UpdateExample()
{
	if (edit_suffix_.m_hWnd != 0)
	{
		CString suffix;
		edit_suffix_.GetWindowText(suffix);
		suffix = ReplaceIllegalChars(static_cast<const TCHAR*>(suffix)).c_str();
		example_wnd_.SetWindowText(_T("DSC01234") + suffix + (IsDlgButtonChecked(IDC_JPEG) ? _T(".jpg") : _T(".bmp")));
	}
}


void ResizeDlg::OnJpeg()
{
	UpdateFmt();
	UpdateExample();
}

void ResizeDlg::OnBmp()
{
	UpdateFmt();
	UpdateExample();
}


extern const float g_vfComprRatios[]=		// approximation
{
	3.5, 4, 5, 5.5, 6, 7, 8, 8.5, 9, 9.5, 10, 11, 11, 12, 12, 13, 13,
	14, 14, 15, 15, 16, 16, 17, 17, 18, 18, 19, 19, 19, 20, 20, 21, 21,
	22, 22, 22, 23, 23, 23, 24, 24, 25, 25, 25, 26, 26, 26, 27, 28, 28,
	28, 28, 29, 29, 30, 30, 31, 31, 31, 32, 33, 33, 34, 34, 35, 36, 36,
	37, 38, 39, 39, 40, 41, 42, 43, 44, 46, 47, 48, 50, 51, 53, 55, 57,
	59, 62, 64, 67, 71, 75, 80, 85, 91, 99, 109, 120, 134, 151, 166, 166
};


void ResizeDlg::UpdateFmt()
{
	if (slider_quality_.m_hWnd)
	{
		bool custom= slider_quality_.GetPos() == 0;
		bool jpeg= true;
		if (custom)
		{
			jpeg = !!IsDlgButtonChecked(IDC_JPEG);
		}

		EnableCtrl(&btn_jpeg_, custom);
		EnableCtrl(&btnBMP_, custom);

		BOOL trans= false;
		int level= GetDlgItemInt(IDC_COMPR_LEVEL, &trans, false);
		float denominator= 0.0f;
		if (trans && level <= COMPR_RNG && level >= 0)
		{
			ASSERT(array_count(g_vfComprRatios) == COMPR_RNG + 1);
			denominator = g_vfComprRatios[COMPR_RNG - level];
		}
		CString ratio;
		if (denominator > 0.0f)
			ratio.Format(_T("(大约 1:%0.*f)"),
				static_cast<int>(denominator * 10) % 10 > 0 ? 1 : 0, static_cast<double>(denominator));
		else
			ratio = _T("(大约 1:?)");
		compr_ratio_wnd_.SetWindowText(ratio);

		ReadOnlyCtrl(&edit_compr_level_, !custom);
		edit_compr_level_.EnableWindow(jpeg);
		spin_compr_level_.EnableWindow(jpeg && custom);
		if (CWnd* wnd= GetDlgItem(IDC_JPEG_LABEL_1))
			wnd->EnableWindow(jpeg);
		compr_ratio_wnd_.EnableWindow(jpeg);

		if (CWnd* wnd= dlg_options_.GetDlgItem(IDC_PROGRESSIVE))
			wnd->EnableWindow(jpeg);
		if (CWnd* wnd= dlg_options_.GetDlgItem(IDC_BASELINE))
			wnd->EnableWindow(jpeg);
		if (CWnd* wnd= dlg_options_.GetDlgItem(IDC_EXIF))
			wnd->EnableWindow(jpeg);
		if (CWnd* wnd= dlg_options_.GetDlgItem(IDC_IPTC))
			wnd->EnableWindow(jpeg);
	}
}


void ResizeDlg::OnOK()
{
	if (!Finish())
		return;

	EndDialog(IDOK);
}


bool ResizeDlg::Finish()
{
	if (!UpdateData() || !dlg_options_.UpdateData())
		return false;

	Path path(dest_path_);
	if (!path.empty())
		if (!path.CreateIfDoesntExist(&dest_path_combo_))
			return false;

	if (force_jpeg_)
		output_format_ = 0;

	extern const TCHAR* PathIllegalChars();

	if (!suffix_.IsEmpty())
		if (suffix_.FindOneOf(PathIllegalChars()) >= 0)
		{
			String msg= _T("后缀文本不能包含以下字符: ");
			msg += PathIllegalChars();
			new BalloonMsg(&edit_suffix_, _T("非法字符"), msg.c_str(), BalloonMsg::IERROR);
			return false;
		}

	if (same_dir_ == 0 && suffix_.IsEmpty())
	{
		new BalloonMsg(&edit_suffix_, _T("缺少后缀"),
			_T("请输入后缀文本, 使目标文件名称与源文件不相同."), BalloonMsg::IERROR);
		return false;
	}
	else
	{
		if (path.empty())
		{
			new BalloonMsg(&dest_path_combo_, _T("缺少目标文件夹"),
				_T("请指定被修改图像的储存文件夹."), BalloonMsg::IERROR);
			return false;
		}
	}

	CWinApp* app= AfxGetApp();

	app->WriteProfileInt(registry_, REG_SIZE_SLIDER, size_);
	app->WriteProfileInt(registry_, REG_QUALITY, quality_);

	BOOL trans= false;
	compr_level_ = GetDlgItemInt(IDC_COMPR_LEVEL, &trans, false);
	if (trans && compr_level_ <= COMPR_RNG)
		app->WriteProfileInt(registry_, REG_JPEG_QUALITY, compr_level_);

	app->WriteProfileInt(registry_, REG_FIXED_SIZE, size_selection_);
	app->WriteProfileInt(registry_, REG_SEL_DIR, same_dir_);
	app->WriteProfileInt(registry_, REG_FORMAT, output_format_);

	app->WriteProfileInt(registry_, REG_WIDTH, width_);

	app->WriteProfileInt(registry_, REG_SIZE_PERCENT, size_percent_);

	app->WriteProfileString(registry_, REG_SUFFIX, suffix_);
	app->WriteProfileString(registry_, REG_OUTPUT_DIR, dest_path_);

	app->WriteProfileInt(registry_, REG_JPEG_BASELINE, dlg_options_.baseline_jpeg_);
	app->WriteProfileInt(registry_, REG_JPEG_PROGRESSIVE, dlg_options_.progressive_jpeg_);
	app->WriteProfileInt(registry_, REG_JPEG_COPY_EXIF, dlg_options_.preserve_exif_block_);
	app->WriteProfileInt(registry_, REG_METHOD, dlg_options_.resizing_method_);
	app->WriteProfileInt(registry_, REG_JPEG_COPY_IPTC, dlg_options_.copyTags_);

	if (same_dir_ != 0)
	{
		// if destination path was given, store it in the recently used ones

		// add path to the recent paths
		InsertUniquePath(recent_paths_, ItemIdList(path.c_str()), true);

		// write paths
		RecentPaths(recent_paths_, true, registry_ + REG_RECENT_DEST_PATH, MAX_RECENT_PATHS);
	}

	return true;
}


void ResizeDlg::OnChangeHeight()
{
	if (update_ || ratio_ <= 0.0)
		return;
	update_ = true;
	int height= GetDlgItemInt(IDC_HEIGHT_PX);
	int width= std::max(1, int(height / ratio_ + 0.5));
	SetDlgItemInt(IDC_WIDTH_PX, width);
	update_ = false;
}

void ResizeDlg::OnChangeWidth()
{
	if (update_ || ratio_ <= 0.0)
		return;
	update_ = true;
	int width= GetDlgItemInt(IDC_WIDTH_PX);
	int height= std::max(1, int(width * ratio_ + 0.5));
	SetDlgItemInt(IDC_HEIGHT_PX, height);
	update_ = false;
}


void ResizeDlg::UpdateSize()
{
	int pos= TranslateSizeSlider(slider_size_.GetPos());

	switch (pos)
	{
	case -1:	// original size
		break;

	case 0:		// custom
		break;

	case 1:		// large
		SetDlgItemInt(IDC_WIDTH_PX, 1024);
		break;

	case 2:		// medium
		SetDlgItemInt(IDC_WIDTH_PX, 800);
		break;

	case 3:		// small
		SetDlgItemInt(IDC_WIDTH_PX, 640);
		break;

	case 4:		// tiny
		SetDlgItemInt(IDC_WIDTH_PX, 320);
		break;

	default:
		ASSERT(false);
		break;
	}

	UpdateDims();
}


void ResizeDlg::UpdateQuality()
{
	force_jpeg_ = true;

	switch (slider_quality_.GetPos())
	{
	case 0:		// custom
		force_jpeg_ = false;
		break;

	case 1:		// super fine
		SetDlgItemInt(IDC_COMPR_LEVEL, compr_level_ = COMPR_RNG - 5);
		break;

	case 2:		// fine
		SetDlgItemInt(IDC_COMPR_LEVEL, compr_level_ = COMPR_RNG - 10);
		break;

	case 3:		// normal
		SetDlgItemInt(IDC_COMPR_LEVEL, compr_level_ = COMPR_RNG - 25);
		break;

	case 4:		// coarse
		SetDlgItemInt(IDC_COMPR_LEVEL, compr_level_ = COMPR_RNG / 2);
		break;

	default:
		ASSERT(false);
		break;
	}

	UpdateFmt();
}


void ResizeDlg::OnVScroll(UINT sb_code, UINT pos, CScrollBar* scroll_bar)
{
	if (scroll_bar == 0)
		return;

	if (scroll_bar->m_hWnd == slider_size_.m_hWnd)
	{
		UpdateSize();
	}
	else if (scroll_bar->m_hWnd == slider_quality_.m_hWnd)
	{
		UpdateQuality();
	}

//	DialogChild::OnVScroll(sb_code, pos, scroll_bar);
}


void ResizeDlg::OnChangeComprLevel()
{
	UpdateFmt();
}


int ResizeDlg::TranslateSizeSlider(int pos)
{
	return pos;
}


void ResizeDlg::OnOptions()
{
	if (dlg_options_.m_hWnd == 0)
		return;

	bool show= dlg_options_.GetStyle() & WS_VISIBLE ? false : true;

	ShowFooterDlg(show);

	SetDlgItemText(IDC_OPTIONS, show ? _T("隐藏选项(&O) <<") : _T("显示选项(&O) >>"));
}

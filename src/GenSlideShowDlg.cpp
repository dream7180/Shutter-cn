/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// GenSlideShowDlg.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "GenSlideShowDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

namespace {

const TCHAR* REG_ENTRY_SLDSHOW		= _T("SlideShowGen");
const TCHAR* REG_WIDTH				= _T("WidthPx");
const TCHAR* REG_SIZE_PERCENT		= _T("SizePercent");
const TCHAR* REG_OUTPUT_DIR			= _T("OutputDir");
const TCHAR* REG_JPEG_QUALITY		= _T("Compression");
const TCHAR* REG_SIZE_SLIDER		= _T("Size");
const TCHAR* REG_QUALITY			= _T("Quality");
const TCHAR* REG_DELAY				= _T("Delay");
const TCHAR* REG_FULL_SCREEN		= _T("FullScreen");
const TCHAR* REG_LOOP				= _T("LoopRepeatedly");

}

/////////////////////////////////////////////////////////////////////////////
// GenSlideShowDlg dialog


GenSlideShowDlg::GenSlideShowDlg(CWnd* parent /*=NULL*/)
	: DialogChild(GenSlideShowDlg::IDD, parent)
{
	//{{AFX_DATA_INIT(GenSlideShowDlg)
	size_percent_ = 100;
	width_ = 640;
	height_ = 480;
	dest_path_ = _T("");
	size_ = 3;
	quality_ = 2;
	compr_level_ = -1;
	delay_ = 0;
	full_screen_ = FALSE;
	loopRepeatedly_ = true;
	//}}AFX_DATA_INIT
	update_ = false;
	ratio_ = 0.75;
}


void GenSlideShowDlg::DoDataExchange(CDataExchange* DX)
{
	DialogChild::DoDataExchange(DX);
	//{{AFX_DATA_MAP(GenSlideShowDlg)
	DDX_Control(DX, IDC_SPIN_DELAY, spin_delay_);
	DDX_Control(DX, IDC_DELAY, edit_delay_);
	DDX_Control(DX, IDC_JPEG_LABEL_2, compr_ratio_wnd_);
	DDX_Control(DX, IDC_SPIN_COMPR, spin_compr_level_);
	DDX_Control(DX, IDC_COMPR_LEVEL, edit_compr_level_);
	DDX_Control(DX, IDC_SIZE_SLD, slider_size_);
	DDX_Control(DX, IDC_QUALITY, slider_quality_);
	DDX_Control(DX, IDC_WIDTH_PX, edit_width_);
	DDX_Control(DX, IDC_HEIGHT_PX, edit_height_);
	DDX_Control(DX, IDC_DEST_PATH, edit_dest_path_);
	DDX_Text(DX, IDC_WIDTH_PX, width_);
	DDV_MinMaxInt(DX, width_, 1, 9999);
	DDX_Text(DX, IDC_HEIGHT_PX, height_);
	DDV_MinMaxInt(DX, height_, 1, 9999);
	DDX_Text(DX, IDC_DEST_PATH, dest_path_);
	DDX_Slider(DX, IDC_SIZE_SLD, size_);
	DDX_Slider(DX, IDC_QUALITY, quality_);
	DDX_Text(DX, IDC_COMPR_LEVEL, compr_level_);
	DDX_Text(DX, IDC_DELAY, delay_);
	DDV_MinMaxUInt(DX, delay_, 0, 9999);
	DDX_Check(DX, IDC_FULL_SCREEN, full_screen_);
	DDX_Check(DX, IDC_LOOP, loopRepeatedly_);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(GenSlideShowDlg, DialogChild)
	//{{AFX_MSG_MAP(GenSlideShowDlg)
	ON_BN_CLICKED(IDC_SAME_DIR, OnSameDir)
	ON_BN_CLICKED(IDC_SELECT, OnSelectDir)
	ON_BN_CLICKED(IDC_BROWSE, OnBrowse)
	ON_EN_CHANGE(IDC_HEIGHT_PX, OnChangeHeight)
	ON_EN_CHANGE(IDC_WIDTH_PX, OnChangeWidth)
	ON_WM_VSCROLL()
	ON_EN_CHANGE(IDC_COMPR_LEVEL, OnChangeComprLevel)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// GenSlideShowDlg message handlers
static const int COMPR_RNG= 100;

BOOL GenSlideShowDlg::OnInitDialog()
{
	CWinApp* app= AfxGetApp();

	compr_level_		= app->GetProfileInt(REG_ENTRY_SLDSHOW, REG_JPEG_QUALITY, COMPR_RNG - 10);
	dest_path_		= app->GetProfileString(REG_ENTRY_SLDSHOW, REG_OUTPUT_DIR, _T("c:\\SlideShow.exe"));
	size_				= app->GetProfileInt(REG_ENTRY_SLDSHOW, REG_SIZE_SLIDER, 2);
	quality_			= app->GetProfileInt(REG_ENTRY_SLDSHOW, REG_QUALITY, 3);
	full_screen_		= app->GetProfileInt(REG_ENTRY_SLDSHOW, REG_FULL_SCREEN, 1);
	loopRepeatedly_	= app->GetProfileInt(REG_ENTRY_SLDSHOW, REG_LOOP, 1);

	DialogChild::OnInitDialog();

	//SubclassHelpBtn(_T("ToolSlideShow.htm"));

	SetDlgItemInt(IDC_WIDTH_PX, app->GetProfileInt(REG_ENTRY_SLDSHOW, REG_WIDTH, 640));

	spin_compr_level_.SetRange(0, COMPR_RNG);
	SetDlgItemInt(IDC_SIZE_PERCENT, app->GetProfileInt(REG_ENTRY_SLDSHOW, REG_SIZE_PERCENT, COMPR_RNG));

	spin_delay_.SetRange(0, 9999);
	SetDlgItemInt(IDC_DELAY, app->GetProfileInt(REG_ENTRY_SLDSHOW, REG_DELAY, 8));

	slider_size_.SetRange(0, 4, true);
	slider_size_.SetPos(size_);

	slider_quality_.SetRange(0, 4, true);
	slider_quality_.SetPos(quality_);

	UpdateSize();
	UpdateQuality();
	UpdateDirs();
//	UpdateDims();
	UpdateFmt();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void GenSlideShowDlg::OnSameDir()
{
	UpdateDirs();
}

void GenSlideShowDlg::OnSelectDir()
{
	UpdateDirs();
}

void GenSlideShowDlg::UpdateDirs()
{
	if (edit_dest_path_.m_hWnd != 0)
	{
		bool same_dir= !!IsDlgButtonChecked(IDC_SAME_DIR);
		edit_dest_path_.EnableWindow(!same_dir);
		GetDlgItem(IDC_BROWSE)->EnableWindow(!same_dir);
	}
}


void GenSlideShowDlg::EnableCtrl(CWnd* wnd, bool enable, bool hide_disabled/*= true*/)
{
	if (wnd == 0)
		return;

	if (enable)
	{
		wnd->EnableWindow(enable);
		wnd->ShowWindow(SW_SHOWNA);
	}
	else
	{
		if (hide_disabled)
			wnd->ShowWindow(SW_HIDE);
		wnd->EnableWindow(enable);
		if (!hide_disabled)
			wnd->ShowWindow(SW_SHOWNA);
	}
}


void GenSlideShowDlg::ReadOnlyCtrl(CEdit* wnd, bool read_only)
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


void GenSlideShowDlg::UpdateDims()
{
	if (edit_width_.m_hWnd != 0 && slider_size_.m_hWnd)
	{
		bool abs_size= true;
		bool rel_size= false;
		bool custom= slider_size_.GetPos() == 0;
		if (custom)	// custom settings?
		{
			//abs_size = !!IsDlgButtonChecked(IDC_FIXED_SIZE);
			//rel_size = !abs_size;
		}

		ReadOnlyCtrl(&edit_width_, !custom);
		ReadOnlyCtrl(&edit_height_, !custom);
		EnableCtrl(&edit_width_, abs_size, custom);
		EnableCtrl(&edit_height_, abs_size, custom);
		EnableCtrl(GetDlgItem(IDC_WIDTH_1_LABEL), abs_size, custom);
		EnableCtrl(GetDlgItem(IDC_HEIGHT_1_LABEL), abs_size, custom);
		EnableCtrl(GetDlgItem(IDC_WIDTH_2_LABEL), abs_size, custom);
		EnableCtrl(GetDlgItem(IDC_HEIGHT_2_LABEL), abs_size, custom);
	}
}


void GenSlideShowDlg::OnBrowse()
{
	CString dest_path;
	edit_dest_path_.GetWindowText(dest_path);

	if (dest_path.IsEmpty())
		dest_path = _T("c:\\");

	CFileDialog dlg(false, _T(".exe"), _T("SlideShow.exe"),
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_NOREADONLYRETURN,
		_T("幻灯片应用程序 (*.exe)|*.exe|所有文件 (*.*)|*.*||"));
//	CFolderSelect fs(this);
//	CString path= fs.DoSelectPath(RString(IDS_SELECT_OUTPUT_FOLDER), dest_path);

	if (dlg.DoModal() != IDOK)
		return;

//	if (path.IsEmpty())
//		return;

	edit_dest_path_.SetWindowText(dlg.GetPathName());
}


extern const float g_vfComprRatios[];


void GenSlideShowDlg::UpdateFmt()
{
	if (slider_quality_.m_hWnd)
	{
		bool custom= slider_quality_.GetPos() == 0;
		bool jpeg= true;
		if (custom)
		{
			//jpeg = !!IsDlgButtonChecked(IDC_JPEG);
		}

		BOOL trans= false;
		int level= GetDlgItemInt(IDC_COMPR_LEVEL, &trans, false);
		float denominator= 0.0f;
		if (trans && level <= COMPR_RNG && level >= 0)
		{
			//ASSERT(array_count(g_vfComprRatios) == COMPR_RNG + 1);
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
		GetDlgItem(IDC_JPEG_LABEL_1)->EnableWindow(jpeg);
		compr_ratio_wnd_.EnableWindow(jpeg);
	}
}


void GenSlideShowDlg::OnOK()
{
	if (!UpdateData(TRUE))
		return;

	CWinApp* app= AfxGetApp();

	app->WriteProfileInt(REG_ENTRY_SLDSHOW, REG_SIZE_SLIDER, size_);
	app->WriteProfileInt(REG_ENTRY_SLDSHOW, REG_QUALITY, quality_);
	app->WriteProfileInt(REG_ENTRY_SLDSHOW, REG_FULL_SCREEN, full_screen_);
	app->WriteProfileInt(REG_ENTRY_SLDSHOW, REG_LOOP, loopRepeatedly_);

	app->WriteProfileInt(REG_ENTRY_SLDSHOW, REG_DELAY, delay_);
	app->WriteProfileInt(REG_ENTRY_SLDSHOW, REG_SIZE_PERCENT, GetDlgItemInt(IDC_SIZE_PERCENT));

	BOOL trans= false;
	compr_level_ = GetDlgItemInt(IDC_COMPR_LEVEL, &trans, false);
	if (trans && compr_level_ <= COMPR_RNG)
		app->WriteProfileInt(REG_ENTRY_SLDSHOW, REG_JPEG_QUALITY, compr_level_);

	app->WriteProfileInt(REG_ENTRY_SLDSHOW, REG_WIDTH, width_);
	app->WriteProfileString(REG_ENTRY_SLDSHOW, REG_OUTPUT_DIR, dest_path_);

	EndDialog(IDOK);
}


void GenSlideShowDlg::OnChangeHeight()
{
	if (update_)
		return;
	update_ = true;
	int height= GetDlgItemInt(IDC_HEIGHT_PX);
	SetDlgItemInt(IDC_WIDTH_PX, int(height / ratio_));
	update_ = false;
}

void GenSlideShowDlg::OnChangeWidth()
{
	if (update_)
		return;
	update_ = true;
	int width= GetDlgItemInt(IDC_WIDTH_PX);
	SetDlgItemInt(IDC_HEIGHT_PX, int(width * ratio_));
	update_ = false;
}


void GenSlideShowDlg::UpdateSize()
{
	switch (slider_size_.GetPos())
	{
	case 0:		// custom
		break;

	case 1:		// large
		SetDlgItemInt(IDC_WIDTH_PX, 1280);
		break;

	case 2:		// normal
		SetDlgItemInt(IDC_WIDTH_PX, 1024);
		break;

	case 3:		// medium
		SetDlgItemInt(IDC_WIDTH_PX, 800);
		break;

	case 4:		// small
		SetDlgItemInt(IDC_WIDTH_PX, 640);
		break;

	default:
		ASSERT(false);
		break;
	}

	UpdateDims();
}


void GenSlideShowDlg::UpdateQuality()
{
	switch (slider_quality_.GetPos())
	{
	case 0:		// custom
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


void GenSlideShowDlg::OnVScroll(UINT sb_code, UINT pos, CScrollBar* scroll_bar)
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
}


void GenSlideShowDlg::OnChangeComprLevel()
{
	UpdateFmt();
}

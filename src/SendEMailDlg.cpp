/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// SendEMailDlg.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "SendEMailDlg.h"
#include "FileSend.h"
#include "EnableCtrl.h"


namespace {

const TCHAR* REGISTRY_ENTRY_EMAIL	= _T("SendEMail");
const TCHAR* REG_WIDTH				= _T("WidthPx");
const TCHAR* REG_SIZE_PERCENT		= _T("SizePercent");
const TCHAR* REG_JPEG_QUALITY		= _T("Compression");
const TCHAR* REG_SIZE_SLIDER		= _T("Size");
const TCHAR* REG_QUALITY			= _T("Quality");

}

// CSendEMailDlg dialog

CSendEMailDlg::CSendEMailDlg(CWnd* parent, VectPhotoInfo& photos, double ratio)
	: DialogChild(CSendEMailDlg::IDD, parent), photos_(photos)
{
//	registry_ = REGISTRY_ENTRY_EMAIL;

	size_percent_ = 100;
	width_ = 640;
	height_ = 480;
	size_ = 3;
	quality_ = 2;
	compr_level_ = -1;

	update_ = false;
	ratio_ = ratio;
}

CSendEMailDlg::~CSendEMailDlg()
{
}

void CSendEMailDlg::DoDataExchange(CDataExchange* DX)
{
	DialogChild::DoDataExchange(DX);

	DDX_Control(DX, IDC_JPEG_LABEL_2, compr_ratio_wnd_);
	DDX_Control(DX, IDC_SPIN_COMPR, spin_compr_level_);
	DDX_Control(DX, IDC_COMPR_LEVEL, edit_compr_level_);
	DDX_Control(DX, IDC_SIZE_SLD, slider_size_);
	DDX_Control(DX, IDC_QUALITY, slider_quality_);
	DDX_Control(DX, IDC_WIDTH_PX, edit_width_);
	DDX_Control(DX, IDC_HEIGHT_PX, edit_height_);
	DDX_Text(DX, IDC_WIDTH_PX, width_);
	DDV_MinMaxInt(DX, width_, 1, 9999);
	DDX_Text(DX, IDC_HEIGHT_PX, height_);
	DDV_MinMaxInt(DX, height_, 1, 9999);
	DDX_Slider(DX, IDC_SIZE_SLD, size_);
	DDX_Slider(DX, IDC_QUALITY, quality_);
	DDX_Text(DX, IDC_COMPR_LEVEL, compr_level_);
}


BEGIN_MESSAGE_MAP(CSendEMailDlg, DialogChild)
//	ON_BN_CLICKED(IDC_SAME_DIR, OnSameDir)
//	ON_BN_CLICKED(IDC_SELECT, OnSelectDir)
//	ON_BN_CLICKED(IDC_BROWSE, OnBrowse)
	ON_EN_CHANGE(IDC_HEIGHT_PX, OnChangeHeight)
	ON_EN_CHANGE(IDC_WIDTH_PX, OnChangeWidth)
	ON_WM_VSCROLL()
	ON_EN_CHANGE(IDC_COMPR_LEVEL, OnChangeComprLevel)
END_MESSAGE_MAP()


// CSendEMailDlg message handlers


static const int COMPR_RNG= 100;


BOOL CSendEMailDlg::OnInitDialog()
{
	CWinApp* app= AfxGetApp();

	compr_level_	= app->GetProfileInt(REGISTRY_ENTRY_EMAIL, REG_JPEG_QUALITY, COMPR_RNG - 10);
	size_			= app->GetProfileInt(REGISTRY_ENTRY_EMAIL, REG_SIZE_SLIDER, 3);
	quality_		= app->GetProfileInt(REGISTRY_ENTRY_EMAIL, REG_QUALITY, 3);

	DialogChild::OnInitDialog();

	SubclassHelpBtn(_T("ToolEMail.htm"));

	SetDlgItemInt(IDC_WIDTH_PX, app->GetProfileInt(REGISTRY_ENTRY_EMAIL, REG_WIDTH, 640));

	spin_compr_level_.SetRange(0, COMPR_RNG);
	SetDlgItemInt(IDC_SIZE_PERCENT, app->GetProfileInt(REGISTRY_ENTRY_EMAIL, REG_SIZE_PERCENT, COMPR_RNG));

	slider_size_.SetRange(0, 4, true);
	slider_size_.SetPos(size_);

	slider_quality_.SetRange(0, 4, true);
	slider_quality_.SetPos(quality_);

	UpdateSize();
	UpdateQuality();
	UpdateFmt();

	return true;
}


static void ReadOnlyCtrl(CEdit* wnd, bool read_only)
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


void CSendEMailDlg::UpdateDims()
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


extern const float g_vfComprRatios[];


void CSendEMailDlg::UpdateFmt()
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


void CSendEMailDlg::OnOK()
{
	if (!UpdateData(TRUE))
		return;

	CWinApp* app= AfxGetApp();

	app->WriteProfileInt(REGISTRY_ENTRY_EMAIL, REG_SIZE_SLIDER, size_);
	app->WriteProfileInt(REGISTRY_ENTRY_EMAIL, REG_QUALITY, quality_);
	app->WriteProfileInt(REGISTRY_ENTRY_EMAIL, REG_SIZE_PERCENT, GetDlgItemInt(IDC_SIZE_PERCENT));

	BOOL trans= false;
	compr_level_ = GetDlgItemInt(IDC_COMPR_LEVEL, &trans, false);
	if (trans && compr_level_ <= COMPR_RNG)
		app->WriteProfileInt(REGISTRY_ENTRY_EMAIL, REG_JPEG_QUALITY, compr_level_);

	app->WriteProfileInt(REGISTRY_ENTRY_EMAIL, REG_WIDTH, width_);
//	app->WriteProfileString(REGISTRY_ENTRY_EMAIL, REG_OUTPUT_DIR, dest_path_);

	EndDialog(IDOK);
}


void CSendEMailDlg::OnChangeHeight()
{
	if (update_)
		return;
	update_ = true;
	int height= GetDlgItemInt(IDC_HEIGHT_PX);
	SetDlgItemInt(IDC_WIDTH_PX, int(height / ratio_));
	update_ = false;
}

void CSendEMailDlg::OnChangeWidth()
{
	if (update_)
		return;
	update_ = true;
	int width= GetDlgItemInt(IDC_WIDTH_PX);
	SetDlgItemInt(IDC_HEIGHT_PX, int(width * ratio_));
	update_ = false;
}


void CSendEMailDlg::UpdateSize()
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


void CSendEMailDlg::UpdateQuality()
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


void CSendEMailDlg::OnVScroll(UINT sb_code, UINT pos, CScrollBar* scroll_bar)
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


void CSendEMailDlg::OnChangeComprLevel()
{
	UpdateFmt();
}

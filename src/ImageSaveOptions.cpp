/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// ImageSaveOptions.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "ImageSaveOptions.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


// ImageSaveOptions dialog

ImageSaveOptions::ImageSaveOptions(CWnd* parent /*=NULL*/)
	: CDialog(ImageSaveOptions::IDD, parent)
{
	quality_ = 95;
	progressive_ = false;
	base_line_ = false;
	preserve_exif_ = true;
	suffix_ = _T("_modified");

	const TCHAR* section= _T("ImageAdjustmentOptions");
	profile_quality_.Register(section, _T("JpegQuality"), quality_);
	profile_progressive_.Register(section, _T("Progressive"), !!progressive_);
	profile_base_line_.Register(section, _T("BaseLine"), !!base_line_);
	profile_preserve_exif_.Register(section, _T("PreserveExif"), !!preserve_exif_);
	profile_suffix_.Register(section, _T("Suffix"), suffix_);
}


ImageSaveOptions::~ImageSaveOptions()
{
}


void ImageSaveOptions::DoDataExchange(CDataExchange* DX)
{
	CDialog::DoDataExchange(DX);
	DDX_Control(DX, IDC_QUALITY, slider_quality_);
	DDX_Control(DX, IDC_COMPR_LEVEL, edit_quality_);
	DDX_Control(DX, IDC_SPIN_COMPR, spin_wnd_);
	DDX_Slider(DX, IDC_QUALITY, quality_);
	DDX_Check(DX, IDC_BASELINE, base_line_);
	DDX_Check(DX, IDC_PROGRESSIVE, progressive_);
	DDX_Check(DX, IDC_EXIF, preserve_exif_);
	DDX_Text(DX, IDC_SUFFIX, suffix_);
	DDX_Control(DX, IDC_SUFFIX, suffix_wnd_);
	DDX_Control(DX, IDC_LABEL_2, label2_wnd_);
	DDX_Control(DX, IDC_HISTOGRAM, histogram_wnd_);
}


BEGIN_MESSAGE_MAP(ImageSaveOptions, CDialog)
	ON_WM_HSCROLL()
	ON_EN_CHANGE(IDC_COMPR_LEVEL, OnChangeComprLevel)
END_MESSAGE_MAP()


// ImageSaveOptions message handlers


bool ImageSaveOptions::Create(CWnd* parent)
{
	return !!CDialog::Create(IDD, parent);
}


BOOL ImageSaveOptions::OnInitDialog()
{
	suffix_ = profile_suffix_;
	quality_ = profile_quality_;
	progressive_ = profile_progressive_;
	preserve_exif_ = profile_preserve_exif_;
	base_line_ = profile_progressive_;

	CDialog::OnInitDialog();

	slider_quality_.SetRange(MIN_LEVEL, MAX_LEVEL);
	slider_quality_.SetTicFreq(MAX_LEVEL / 10);
	slider_quality_.SetPos(quality_);

	spin_wnd_.SetRange(MIN_LEVEL, MAX_LEVEL);

	SetDlgItemInt(IDC_COMPR_LEVEL, quality_);

	histogram_wnd_.SelectChannel(Histogram::RGBOverlaid);
	histogram_wnd_.SetDrawFlags(0);
//	histogram_wnd_.SetLogarithmic(true);
	histogram_wnd_.SetRGBOverlaidOnly();

	return true;
}


void ImageSaveOptions::OnHScroll(UINT sb_code, UINT pos, CScrollBar* scroll_bar)
{
	if (scroll_bar == 0)
		return;

	if (scroll_bar->m_hWnd == slider_quality_.m_hWnd)
		SetDlgItemInt(IDC_COMPR_LEVEL, slider_quality_.GetPos());
}


void ImageSaveOptions::OnChangeComprLevel()
{
	int level= GetDlgItemInt(IDC_COMPR_LEVEL);

	if (level > MAX_LEVEL)
		level = MAX_LEVEL;
	else if (level < MIN_LEVEL)
		level = MIN_LEVEL;

	if (slider_quality_.m_hWnd)
		slider_quality_.SetPos(level);
}


void ImageSaveOptions::SaveOptions()
{
	if (!UpdateData())
	{
		ASSERT(false);
		return;
	}

	profile_suffix_ = suffix_;
	profile_quality_ = quality_;
	profile_progressive_ = !!progressive_;
	profile_base_line_ = !!base_line_;
	profile_preserve_exif_ = !!preserve_exif_;
}

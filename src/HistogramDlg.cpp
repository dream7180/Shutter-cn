/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// HistogramDlg.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "HistogramDlg.h"
#include "JpegDecoder.h"
#include "FileDataSource.h"
#include "Config.h"
#include "Dib.h"
#include "RString.h"
#include "JPEGException.h"
#include "Path.h"
#include "PhotoInfo.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static const TCHAR* REGISTRY_ENTRY_HIST= _T("Histogram");
static const TCHAR* REG_SEL= _T("ChannelSelection");
static const TCHAR* REG_LOG= _T("LogScale");
//static const TCHAR* REG_POSX= _T("PosX");
//static const TCHAR* REG_POSY= _T("PosY");
//static const TCHAR* REG_OPENED= _T("Opened");

/////////////////////////////////////////////////////////////////////////////
// HistogramDlg dialog


HistogramDlg::HistogramDlg(const PhotoInfo& photo, PhotoCache* cache)
	: image_wnd_(&hist_wnd_, &rect_label_wnd_), DialogChild(HistogramDlg::IDD, 0), cache_(cache)
{
	CWaitCursor wait;

	if (LoadPhoto(photo))
	{
		image_wnd_.SetImage(bmp_.get());
		hist_wnd_.Build(*bmp_);
	}
	else
		bmp_ = 0;

	name_ = photo.GetName();

	//{{AFX_DATA_INIT(HistogramDlg)
	//}}AFX_DATA_INIT
}


void HistogramDlg::DoDataExchange(CDataExchange* DX)
{
	DialogChild::DoDataExchange(DX);
	//{{AFX_DATA_MAP(HistogramDlg)
	DDX_Control(DX, IDC_RECT, rect_label_wnd_);
	DDX_Control(DX, IDC_IMG, image_wnd_);
	DDX_Control(DX, IDC_GRAPH, hist_wnd_);
	DDX_Control(DX, IDC_LIST, channels_wnd_);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(HistogramDlg, DialogChild)
	//{{AFX_MSG_MAP(HistogramDlg)
	ON_CBN_SELCHANGE(IDC_CHANNEL, OnSelChangeChannel)
	ON_LBN_SELCHANGE(IDC_LIST, OnSelChangeChannel)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_LOG_SCALE, OnLogScale)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// HistogramDlg message handlers

void HistogramDlg::OnSelChangeChannel()
{
	Histogram::ChannelSel sel= Histogram::LumRGB;

	switch (channels_wnd_.GetCurSel())
	{
	case 0: sel = Histogram::LumRGB;		break;
	case 1: sel = Histogram::RGBOverlaid;	break;
	case 2: sel = Histogram::RGBLines;	break;
	case 3: sel = Histogram::RED;			break;
	case 4: sel = Histogram::GREEN;		break;
	case 5: sel = Histogram::BLUE;		break;
	case 6: sel = Histogram::LUM;			break;
	default:
		ASSERT(false);
		return;
	}

	hist_wnd_.SelectChannel(sel);
}


BOOL HistogramDlg::OnInitDialog()
{
	DialogChild::OnInitDialog();

	SubclassHelpBtn(_T("ToolHistogram.htm"));

	channels_wnd_.AddString(_T("亮度 & RGB"));
	channels_wnd_.AddString(_T("RGB 覆盖"));
	channels_wnd_.AddString(_T("RGB 线条"));
	channels_wnd_.AddString(_T("红"));
	channels_wnd_.AddString(_T("绿"));
	channels_wnd_.AddString(_T("蓝"));
	channels_wnd_.AddString(_T("亮度"));

	CString title;
	GetWindowText(title);
	title += _T(" for \"");
	title += name_.c_str();
	title += _T('"');
	SetWindowText(title);

	int chnl_sel= AfxGetApp()->GetProfileInt(REGISTRY_ENTRY_HIST, REG_SEL, 1);
	bool log_scale= AfxGetApp()->GetProfileInt(REGISTRY_ENTRY_HIST, REG_LOG, 0) != 0;
	hist_wnd_.SetLogarithmic(log_scale);
	CheckDlgButton(IDC_LOG_SCALE, log_scale);

	//if (::GetAsyncKeyState(VK_SHIFT) < 0)
	//{
	//	channels_wnd_.AddString(_T("Shape Histogram"));
	//	if (bmp_.get())
	//		hist_wnd_.BuildIndex(bmp_.get());
	//}

	channels_wnd_.SetCurSel(chnl_sel >= 0 ? chnl_sel : 1);
	if (channels_wnd_.GetCurSel() < 0)
		channels_wnd_.SetCurSel(0);

	OnSelChangeChannel();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void HistogramDlg::OnDestroy()
{
	CWinApp* app= AfxGetApp();

	if (channels_wnd_.m_hWnd)
		app->WriteProfileInt(REGISTRY_ENTRY_HIST, REG_SEL, channels_wnd_.GetCurSel());
	if (m_hWnd)
		app->WriteProfileInt(REGISTRY_ENTRY_HIST, REG_LOG, IsDlgButtonChecked(IDC_LOG_SCALE));

	DialogChild::OnDestroy();
}


Dib* HistogramDlg::LoadPhoto(const PhotoInfo& photo)
{
	try
	{
		CWaitCursor wait;

		bmp_ = AutoPtr<Dib>(new Dib);

		CacheImg* img= 0;

		if (cache_ && (img = cache_->FindEntry(&photo)) != 0 && img->Dib())
		{
			bmp_->Clone(*img->Dib());
			return bmp_.get();
		}
		else
		{
			CImageDecoderPtr decoder= photo.GetDecoder();
			CSize img_size(500, 500);
			if (decoder->DecodeImg(*bmp_, img_size, false) == IS_OK)
				return bmp_.get();
		}
	}
	catch (...)
	{
	}
/*
	try
	{
		CWaitCursor wait;
		CFileDataSource fsrc(path, offset);
		JPEGDecoder dec(fsrc, g_Settings.dct_method_);
		dec.SetFast(true, true);
		bmp_ = AutoPtr<Dib>(new Dib);
		if (dec.DecodeImg(*bmp_) != IS_OK)
			return 0;
		return bmp_.get();
	}
	catch (const JPEGException& ex)	// jpeg decoding error?
	{
		CString msg= path;
		msg += _T('\n');
		msg += ex.GetMessage();
		AfxGetMainWnd()->MessageBox(msg);
	}
	catch (...)
	{
	}
*/
	return 0;
}


void HistogramDlg::OnLogScale()
{
	hist_wnd_.SetLogarithmic(!!IsDlgButtonChecked(IDC_LOG_SCALE));
}

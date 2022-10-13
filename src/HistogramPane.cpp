/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// HistogramPane.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "HistogramPane.h"
#include "SnapFrame/SnapView.h"
#include "PhotoInfo.h"
#include "PhotoCtrl.h"
#include "MemoryDC.h"
#include "Config.h"
#include "GetDefaultGuiFont.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// HistogramPane

HistogramPane::HistogramPane()
{
	channels_ = 1;
	profile_channel_.Register(_T("HistogramPane"), _T("Channel"), channels_);
	rgb_backgnd_ = ::GetSysColor(COLOR_WINDOW);
}

HistogramPane::~HistogramPane()
{}


BEGIN_MESSAGE_MAP(HistogramPane, PaneWnd)
	//{{AFX_MSG_MAP(HistogramPane)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
	ON_CBN_SELCHANGE(IDC_CHANNEL, OnSelChangeChannel)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// HistogramPane message handlers


bool HistogramPane::Create(CWnd* parent)
{
	if (!PaneWnd::Create(AfxRegisterWndClass(0, ::LoadCursor(NULL, IDC_ARROW)), 0, WS_CHILD | WS_VISIBLE, CRect(0,0,0,0), parent, -1))
		return false;

	int w= 120;
	int h= 200;
	if (!channels_wnd_.Create(WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | CBS_HASSTRINGS, CRect(0, 0, w, h), this, IDC_CHANNEL))
		return false;

	channels_ = profile_channel_;
	
	LOGFONT lf;
	::GetDefaultGuiFont(lf);
	HFONT hfont = CreateFontIndirectW(&lf);
	channels_wnd_.SendMessage(WM_SETFONT, WPARAM(hfont));
	//channels_wnd_.SendMessage(WM_SETFONT, WPARAM(::GetStockObject(DEFAULT_GUI_FONT)));
	channels_wnd_.InitStorage(5, 15);
	channels_wnd_.AddString(_T("RGB"));
	channels_wnd_.AddString(_T("RGB 覆盖"));
	channels_wnd_.AddString(_T("RGB 线条"));
	channels_wnd_.AddString(_T("红"));
	channels_wnd_.AddString(_T("绿"));
	channels_wnd_.AddString(_T("蓝"));
	channels_wnd_.AddString(_T("亮度"));
	channels_wnd_.SetCurSel(channels_);
	channels_wnd_.SetDroppedWidth(w);
	channels_wnd_.SetExtendedUI();

	channels_wnd_.SetOwner(this);

	SetColors();

	int combo_min_width= 30;
	AddBand(&channels_wnd_, this, std::make_pair(combo_min_width, 0), true);
	return true;
}


BOOL HistogramPane::OnEraseBkgnd(CDC* dc)
{
	return true;
}


void HistogramPane::OnPaint()
{
	CPaintDC paint_dc(this); // device context for painting

	COLORREF rgb_back= rgb_backgnd_;

	MemoryDC dc(paint_dc, this, rgb_back);

	CRect rect;
	GetClientRect(rect);

	if (rect.Height() <= 0 || rect.Width() <= 0)
		return;

	histogram_.rgb_back_ = rgb_back;
	histogram_.rgb_fore_ = rgb_back;

	const UINT flags= Histogram::NO_BOTTOM_SPACE | Histogram::NO_FRAME;

	if (channels_ == 0)
	{

		Histogram::ChannelSel chnls[]= { Histogram::RED, Histogram::GREEN, Histogram::BLUE };

		const int CHNL= array_count(chnls);
		for (int i= 0; i < CHNL; ++i)
		{
			CRect r(rect.left, rect.top + i * rect.Height() / CHNL, rect.right, rect.top + (i + 1) * rect.Height() / CHNL);
			histogram_.Draw(&dc, r, chnls[i], flags);
		}
	}
	else
	{
		Histogram::ChannelSel sel= Histogram::LumRGB;
		switch (channels_)
		{
		case 1: sel = Histogram::RGBOverlaid;	break;
		case 2: sel = Histogram::RGBLines;		break;
		case 3: sel = Histogram::RED;			break;
		case 4: sel = Histogram::GREEN;			break;
		case 5: sel = Histogram::BLUE;			break;
		case 6: sel = Histogram::LUM;			break;
		default:
			ASSERT(false);
			break;
		}

		histogram_.Draw(&dc, rect, sel, flags);
	}
	dc.FillSolidRect(rect.left, rect.top, rect.Width(), 1, g_Settings.AppColors()[AppColors::SecondarySeparator]);

	dc.BitBlt();
}


void HistogramPane::Update(PhotoInfoPtr photo)
{
	// clear histogram data
	histogram_.BuildHistogram(0);

	if (!IsWindowVisible())
		return;

	if (photo != 0)
	{
		//TODO: revise?
		if (Dib* thumbnail_img= photo->GetThumbnail(PhotoInfo::GetThumbnailSize()))
		{
			histogram_.BuildHistogram(thumbnail_img);
		}
		else
		{
			Dib bmp;
			if (HistogramPane::LoadPhoto(*photo, bmp))
				histogram_.BuildHistogram(&bmp);
		}
	}

	Invalidate();
}


bool HistogramPane::LoadPhoto(const PhotoInfo& photo, Dib& bmp)
{
	try
	{
		CWaitCursor wait;
		CImageDecoderPtr decoder= photo.GetDecoder();
		CSize img_size(100, 100);
		return decoder->DecodeImg(bmp, img_size, false) == IS_OK;
	}
	catch (...)
	{
	}

	return 0;
}


void HistogramPane::OnSelChangeChannel()
{
	channels_ = channels_wnd_.GetCurSel();
	profile_channel_ = channels_;
	Invalidate();
}


void HistogramPane::CurrentChanged(PhotoInfoPtr photo)
{
	Update(photo);
}


void HistogramPane::SetColors()
{
	std::vector<COLORREF> colors= g_Settings.main_wnd_colors_.Colors();

	rgb_backgnd_ = colors[PhotoCtrl::C_EDIT_BACKGND];//colors[PhotoCtrl::C_BACKGND];
	if (m_hWnd)
		Invalidate();
}


void HistogramPane::OptionsChanged(OptionsDlg& dlg)
{
	SetColors();
}

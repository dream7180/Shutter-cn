/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// ImgLevelsPage.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "ImgLevelsPage.h"
#include "Dib.h"
#include "Block.h"
#include "PhotoInfo.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// ImgLevelsPage dialog

ImgLevelsPage::ImgLevelsPage(CWnd* parent /*=NULL*/)
	: ImgPage(ImgLevelsPage::IDD, parent)
{
	in_update_ = true;
}


ImgLevelsPage::~ImgLevelsPage()
{
}


void ImgLevelsPage::DoDataExchange(CDataExchange* DX)
{
	ImgPage::DoDataExchange(DX);
	DDX_Control(DX, IDC_HISTOGRAM, histogram_wnd_);
	DDX_Control(DX, IDC_CHANNELS, channel_wnd_);
	DDX_Control(DX, IDC_SPIN_1, spin_wnd_[0]);
	DDX_Control(DX, IDC_SPIN_2, spin_wnd_[1]);
	DDX_Control(DX, IDC_SPIN_3, spin_wnd_[2]);
}


BEGIN_MESSAGE_MAP(ImgLevelsPage, ImgPage)
	ON_BN_CLICKED(IDC_RESET, OnReset)
	ON_CONTROL_RANGE(EN_CHANGE, IDC_EDIT_1, IDC_EDIT_3, OnNumChanged)
	ON_CBN_SELCHANGE(IDC_CHANNELS, OnSelChangeChannels)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_2, OnDeltaPosGammaSpin)
END_MESSAGE_MAP()


// ImgLevelsPage message handlers


BOOL ImgLevelsPage::OnInitDialog()
{
	ImgPage::OnInitDialog();

	dlg_resize_map_.BuildMap(this);

	dlg_resize_map_.SetWndResizing(ID_LABEL_1, DlgAutoResize::MOVE_H, DlgAutoResize::HALF_MOVE_H);
	dlg_resize_map_.SetWndResizing(IDC_CHANNELS, DlgAutoResize::MOVE_H, DlgAutoResize::HALF_MOVE_H);
	dlg_resize_map_.SetWndResizing(IDC_RESET, DlgAutoResize::MOVE_H);
	dlg_resize_map_.SetWndResizing(IDC_HISTOGRAM, DlgAutoResize::RESIZE_H);
	dlg_resize_map_.SetWndResizing(IDC_GAMMA, DlgAutoResize::MOVE_H, DlgAutoResize::HALF_MOVE_H);
	dlg_resize_map_.SetWndResizing(IDC_EDIT_2, DlgAutoResize::MOVE_H, DlgAutoResize::HALF_MOVE_H);
	dlg_resize_map_.SetWndResizing(IDC_SPIN_2, DlgAutoResize::MOVE_H, DlgAutoResize::HALF_MOVE_H);
	dlg_resize_map_.SetWndResizing(IDC_EDIT_3, DlgAutoResize::MOVE_H);
	dlg_resize_map_.SetWndResizing(IDC_SPIN_3, DlgAutoResize::MOVE_H);

	spin_wnd_[0].SetRange(MIN_VAL, MAX_VAL);
	spin_wnd_[2].SetRange(MIN_VAL, MAX_VAL);

	// gamma
	spin_wnd_[1].SetRange(1, 10);

	channel_wnd_.SetCurSel(0);

	histogram_wnd_.SetHost(this);
	histogram_wnd_.SetDrawFlags(histogram_wnd_.GetDrawFlags() | Histogram::DRAW_SLIDER_ARROWS);

	in_update_ = false;

	OnSelChangeChannels();

	return true;
}


void ImgLevelsPage::Transform(Dib& dib, bool preview)
{
	levels_.CalcLookupTables(params_);
	levels_.Transform(dib);
}


void ImgLevelsPage::OnReset()
{
	params_.Reset();

	//TODO: synch controls

	OnSelChangeChannels();

	ParamChanged(this, true);
}


void ImgLevelsPage::OnNumChanged(UINT id)
{
	if (in_update_)
		return;

	if (CWnd* edit= GetDlgItem(id))
		edit->UpdateWindow();
	histogram_wnd_.UpdateWindow();

	int channel= channel_wnd_.GetCurSel();

	if (channel == 4)
		channel = 0;

	if (channel < 0 || channel > 3)
	{
		ASSERT(false);
		return;
	}

	int min= GetDlgItemInt(IDC_EDIT_1);
	int max= GetDlgItemInt(IDC_EDIT_3);

	if (min < MIN_VAL)
		min = MIN_VAL;
	else if (min > MAX_VAL)
		min = MAX_VAL;

	if (max < MIN_VAL)
		max = MIN_VAL;
	else if (max > MAX_VAL)
		max = MAX_VAL;

	params_.levels_[channel].min = min;
	params_.levels_[channel].max = max;

	CString gamma_str;
	GetDlgItemText(IDC_EDIT_2, gamma_str);

	double gamma= _tstof(gamma_str);
	// limit?
	params_.levels_[channel].gamma = gamma;

	histogram_wnd_.SetEdgeLines(min, max);

	params_.channels_ = LevelParams::Channels(channel);

	if (channel == LevelParams::RGB)	// overwrite individual channels if RGB is selected
	{
		for (size_t i= LevelParams::Red; i <= LevelParams::Blue; ++i)
		{
			params_.levels_[i].min = min;
			params_.levels_[i].max = max;
			params_.levels_[i].gamma = gamma;
		}
	}

	ParamChanged(this);
}


void ImgLevelsPage::OnSelChangeChannels()
{
	if (in_update_)
		return;

	int channel= channel_wnd_.GetCurSel();

	if (channel < 0 || channel > 4)
	{
		ASSERT(false);
		return;
	}

	Block update(in_update_);

	Histogram::ChannelSel sel= Histogram::LumRGB;
	switch (channel)
	{
	case 0: sel = Histogram::RGB;		break;
	case 1: sel = Histogram::RED;		break;
	case 2: sel = Histogram::GREEN;	break;
	case 3: sel = Histogram::BLUE;	break;
	case 4:
		sel = Histogram::RGBOverlaid;
		channel = 0;
		break;
	default:
		ASSERT(false);
		break;
	}

	histogram_wnd_.SelectChannel(sel);

	SetDlgItemInt(IDC_EDIT_1, params_.levels_[channel].min);
	SetDlgItemInt(IDC_EDIT_3, params_.levels_[channel].max);

	histogram_wnd_.SetEdgeLines(params_.levels_[channel].min, params_.levels_[channel].max);

	SetGamma(params_.levels_[channel].gamma);
}


void ImgLevelsPage::SetGamma(double gamma)
{
	oStringstream ost;
	ost.precision(3);
	ost << gamma;
	SetDlgItemText(IDC_EDIT_2, ost.str().c_str());
}


void ImgLevelsPage::Initialize(const Dib& dibOriginal)
{
	histogram_wnd_.Build(dibOriginal);
}


void ImgLevelsPage::MouseClicked(int position, bool left)
{
	int edit_id= left ? IDC_EDIT_1 : IDC_EDIT_3;

	{
		Block update(in_update_);
		SetDlgItemInt(edit_id, position);
	}

	OnNumChanged(edit_id);
}


void ImgLevelsPage::OnDeltaPosGammaSpin(NMHDR* nmhdr, LRESULT* result)
{
	NM_UPDOWN* NM_up_down= reinterpret_cast<NM_UPDOWN*>(nmhdr);

	CString temp;
	GetDlgItemText(IDC_EDIT_2, temp);
	double val= _tcstod(temp, 0);

	if (NM_up_down->iDelta < 0)
		val -= 0.1;
	else
		val += 0.1;

	if (val > 9.9)
		val = 9.9;
	else if (val < 0.1)
		val = 0.1;

	SetGamma(val);

	*result = 0;
}

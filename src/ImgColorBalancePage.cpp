/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// ImgColorBalancePage.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "ImgColorBalancePage.h"
#include "Dib.h"
#include "Block.h"
#include "PhotoInfo.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


// ImgColorBalancePage dialog

ImgColorBalancePage::ImgColorBalancePage(CWnd* parent /*=NULL*/)
	: ImgPage(ImgColorBalancePage::IDD, parent)
{
	range_ = 0;
	in_update_ = true;
	preserve_lum_ = 1;
}


ImgColorBalancePage::~ImgColorBalancePage()
{
}


void ImgColorBalancePage::DoDataExchange(CDataExchange* DX)
{
	ImgPage::DoDataExchange(DX);
	DDX_Control(DX, IDC_SLIDER_1, slider_wnd_[0]);
	DDX_Control(DX, IDC_SLIDER_2, slider_wnd_[1]);
	DDX_Control(DX, IDC_SLIDER_3, slider_wnd_[2]);
	DDX_Control(DX, IDC_SPIN_1, spin_wnd_[0]);
	DDX_Control(DX, IDC_SPIN_2, spin_wnd_[1]);
	DDX_Control(DX, IDC_SPIN_3, spin_wnd_[2]);
	DDX_Check(DX, IDC_PRESERVE_LUM, preserve_lum_);
}


BEGIN_MESSAGE_MAP(ImgColorBalancePage, ImgPage)
	ON_CONTROL_RANGE(BN_CLICKED, IDC_RADIO1, IDC_RADIO3, OnRangeChanged)
	ON_CONTROL_RANGE(EN_CHANGE, IDC_EDIT_1, IDC_EDIT_3, OnNumChanged)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_RESET, OnReset)
	ON_BN_CLICKED(IDC_PRESERVE_LUM, OnPreserveLuminosity)
END_MESSAGE_MAP()


// ImgColorBalancePage message handlers


BOOL ImgColorBalancePage::OnInitDialog()
{
	ImgPage::OnInitDialog();

	dlg_resize_map_.BuildMap(this);

	dlg_resize_map_.SetWndResizing(IDC_SEPARATOR, DlgAutoResize::RESIZE_H);
	dlg_resize_map_.SetWndResizing(IDC_SLIDER_1, DlgAutoResize::RESIZE_H);
	dlg_resize_map_.SetWndResizing(IDC_SLIDER_2, DlgAutoResize::RESIZE_H);
	dlg_resize_map_.SetWndResizing(IDC_SLIDER_3, DlgAutoResize::RESIZE_H);
	dlg_resize_map_.SetWndResizing(IDC_LABEL_1, DlgAutoResize::MOVE_H);
	dlg_resize_map_.SetWndResizing(IDC_LABEL_2, DlgAutoResize::MOVE_H);
	dlg_resize_map_.SetWndResizing(IDC_LABEL_3, DlgAutoResize::MOVE_H);
	dlg_resize_map_.SetWndResizing(IDC_EDIT_1, DlgAutoResize::MOVE_H);
	dlg_resize_map_.SetWndResizing(IDC_EDIT_2, DlgAutoResize::MOVE_H);
	dlg_resize_map_.SetWndResizing(IDC_EDIT_3, DlgAutoResize::MOVE_H);
	dlg_resize_map_.SetWndResizing(IDC_SPIN_1, DlgAutoResize::MOVE_H);
	dlg_resize_map_.SetWndResizing(IDC_SPIN_2, DlgAutoResize::MOVE_H);
	dlg_resize_map_.SetWndResizing(IDC_SPIN_3, DlgAutoResize::MOVE_H);
	dlg_resize_map_.SetWndResizing(IDC_RESET, DlgAutoResize::MOVE_H);

	CheckRadioButton(IDC_RADIO1, IDC_RADIO3, IDC_RADIO1);

	for (int i= 0; i < 3; ++i)
	{
		slider_wnd_[i].SetRange(MIN_VAL, MAX_VAL);
		slider_wnd_[i].SetTicFreq(100);
		slider_wnd_[i].SetPos(0);
		slider_wnd_[i].SetPageSize(10);

		spin_wnd_[i].SetRange(MIN_VAL, MAX_VAL);
	}

	balance_.PreserveLuminosity(preserve_lum_ != 0);

	in_update_ = false;

	SetRangeValues();

	return true;
}


void ImgColorBalancePage::OnRangeChanged(UINT id)
{
	int index= -1;

	switch (id)
	{
	case IDC_RADIO1:
		index = 0;
		break;

	case IDC_RADIO2:
		index = 1;
		break;

	case IDC_RADIO3:
		index = 2;
		break;
	}

	if (index >= 0 && index != range_)
	{
		range_ = index;
		SetRangeValues();
	}
}


void ImgColorBalancePage::OnNumChanged(UINT id)
{
	if (in_update_)
		return;

	int index= -1;

	switch (id)
	{
	case IDC_EDIT_1:
		index = 0;
		break;

	case IDC_EDIT_2:
		index = 1;
		break;

	case IDC_EDIT_3:
		index = 2;
		break;
	}

	if (index < 0)
		return;

	int val= GetDlgItemInt(id);
	if (val < MIN_VAL)
		val = MIN_VAL;
	if (val > MAX_VAL)
		val = MAX_VAL;

	Block update(in_update_);
	slider_wnd_[index].SetPos(val);
	shift_[range_].val[index] = val;

	ParamChanged(this);
}


void ImgColorBalancePage::OnHScroll(UINT sb_code, UINT pos, CScrollBar* scroll_bar)
{
	if (scroll_bar == 0 || in_update_)
		return;

	for (int i= 0; i < 3; ++i)
		if (scroll_bar->m_hWnd == slider_wnd_[i].m_hWnd)
		{
			Block update(in_update_);
			int val= slider_wnd_[i].GetPos();
			SetDlgItemInt(IDC_EDIT_1 + i, val);
			shift_[range_].val[i] = val;

			ParamChanged(this);

			break;
		}
}


void ImgColorBalancePage::SetRangeValues()
{
	Block update(in_update_);

	for (int i= 0; i < 3; ++i)
	{
		const Shift& s= shift_[range_];
		slider_wnd_[i].SetPos(s.val[i]);
		SetDlgItemInt(IDC_EDIT_1 + i, s.val[i]);
	}
}


void ImgColorBalancePage::OnReset()
{
	{
		Block update(in_update_);
		preserve_lum_ = 1;
		CheckDlgButton(IDC_PRESERVE_LUM, preserve_lum_);
	}
	// clear all ranges
	for (int i= 0; i < array_count(shift_); ++i)
		std::fill(shift_[i].val, shift_[i].val + array_count(shift_[i].val), 0);
	SetRangeValues();
	ParamChanged(this, true);
}


void ImgColorBalancePage::Transform(Dib& dib, bool preview)
{
	ColorBalanceParams params;

	double Max= 1.0;

	params.shadows.cyan_red			= shift_[0].val[0] / Max;
	params.shadows.magenta_green	= shift_[0].val[1] / Max;
	params.shadows.yellow_blue		= shift_[0].val[2] / Max;

	params.midtones.cyan_red		= shift_[1].val[0] / Max;
	params.midtones.magenta_green	= shift_[1].val[1] / Max;
	params.midtones.yellow_blue		= shift_[1].val[2] / Max;

	params.highlights.cyan_red		= shift_[2].val[0] / Max;
	params.highlights.magenta_green	= shift_[2].val[1] / Max;
	params.highlights.yellow_blue	= shift_[2].val[2] / Max;

	balance_.CalcLookupTables(params);

	balance_.Transform(dib);
}


void ImgColorBalancePage::OnPreserveLuminosity()
{
	balance_.PreserveLuminosity(IsDlgButtonChecked(IDC_PRESERVE_LUM) != 0);

	ParamChanged(this);
}

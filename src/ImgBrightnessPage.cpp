/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// ImgBrightnessPage.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "ImgBrightnessPage.h"
#include "Dib.h"
#include "Block.h"
#include "PhotoInfo.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


// ImgBrightnessPage dialog

ImgBrightnessPage::ImgBrightnessPage(CWnd* parent /*=NULL*/)
	: ImgPage(ImgBrightnessPage::IDD, parent)
{
	in_update_ = true;
}


ImgBrightnessPage::~ImgBrightnessPage()
{
}


void ImgBrightnessPage::DoDataExchange(CDataExchange* DX)
{
	ImgPage::DoDataExchange(DX);
	DDX_Control(DX, IDC_SLIDER_1, slider_wnd_[0]);
	DDX_Control(DX, IDC_SLIDER_2, slider_wnd_[1]);
	DDX_Control(DX, IDC_SPIN_1, spin_wnd_[0]);
	DDX_Control(DX, IDC_SPIN_2, spin_wnd_[1]);
}


BEGIN_MESSAGE_MAP(ImgBrightnessPage, ImgPage)
	ON_CONTROL_RANGE(EN_CHANGE, IDC_EDIT_1, IDC_EDIT_2, OnNumChanged)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_RESET, OnReset)
END_MESSAGE_MAP()


// ImgBrightnessPage message handlers


BOOL ImgBrightnessPage::OnInitDialog()
{
	ImgPage::OnInitDialog();

	dlg_resize_map_.BuildMap(this);

	dlg_resize_map_.SetWndResizing(IDC_SLIDER_1, DlgAutoResize::RESIZE_H);
	dlg_resize_map_.SetWndResizing(IDC_SLIDER_2, DlgAutoResize::RESIZE_H);
	dlg_resize_map_.SetWndResizing(IDC_EDIT_1, DlgAutoResize::MOVE_H);
	dlg_resize_map_.SetWndResizing(IDC_EDIT_2, DlgAutoResize::MOVE_H);
	dlg_resize_map_.SetWndResizing(IDC_SPIN_1, DlgAutoResize::MOVE_H);
	dlg_resize_map_.SetWndResizing(IDC_SPIN_2, DlgAutoResize::MOVE_H);
	dlg_resize_map_.SetWndResizing(IDC_RESET, DlgAutoResize::MOVE_H);

	for (int i= 0; i < 2; ++i)
	{
		slider_wnd_[i].SetRange(MIN_VAL, MAX_VAL);
		slider_wnd_[i].SetTicFreq(100);
		slider_wnd_[i].SetPos(0);
		slider_wnd_[i].SetPageSize(10);

		spin_wnd_[i].SetRange(MIN_VAL, MAX_VAL);
	}

	in_update_ = false;

	SetValues();

	return true;
}


void ImgBrightnessPage::Transform(Dib& dib, bool preview)
{
	br_cont_.CalcLookupTables(params_);
	br_cont_.Transform(dib);
}


void ImgBrightnessPage::OnNumChanged(UINT id)
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
	params_.val[index] = val / 100.0;

	ParamChanged(this);
}


void ImgBrightnessPage::OnHScroll(UINT sb_code, UINT pos, CScrollBar* scroll_bar)
{
	if (scroll_bar == 0 || in_update_)
		return;

	for (int i= 0; i < 2; ++i)
		if (scroll_bar->m_hWnd == slider_wnd_[i].m_hWnd)
		{
			Block update(in_update_);
			int val= slider_wnd_[i].GetPos();
			SetDlgItemInt(IDC_EDIT_1 + i, val);
			params_.val[i] = val / 100.0;

			ParamChanged(this);

			break;
		}
}


void ImgBrightnessPage::OnReset()
{
	params_.Reset();
	SetValues();
	ParamChanged(this, true);
}


void ImgBrightnessPage::SetValues()
{
	Block update(in_update_);

	for (int i= 0; i < 2; ++i)
	{
		int val= static_cast<int>(MAX_VAL * params_.val[i]);
		slider_wnd_[i].SetPos(val);
		SetDlgItemInt(IDC_EDIT_1 + i, val);
	}
}

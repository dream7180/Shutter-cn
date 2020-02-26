/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// SliderEditForm.cpp : implementation file
//

#include "stdafx.h"
#include "SliderEditForm.h"
#include "Block.h"


static const TCHAR* const WND_CLASS= _T("MonitorCtrlMiK");

static void RegisterWndClass(const TCHAR* class_name)
{
	HINSTANCE instance= AfxGetInstanceHandle();

	// see if the class already exists
	WNDCLASS wndcls;
	if (!::GetClassInfo(instance, class_name, &wndcls))
	{
		// otherwise we need to register a new class
		wndcls.style = 0;//CS_VREDRAW | CS_HREDRAW;
		wndcls.lpfnWndProc = ::DefWindowProc;
		wndcls.cbClsExtra = wndcls.cbWndExtra = 0;
		wndcls.hInstance = instance;
		wndcls.hIcon = 0;
		wndcls.hCursor = ::LoadCursor(NULL, IDC_ARROW);
		wndcls.hbrBackground = 0;
		wndcls.lpszMenuName = NULL;
		wndcls.lpszClassName = class_name;

		AfxRegisterClass(&wndcls);
	}
}

// SliderEditForm dialog

SliderEditForm::SliderEditForm() : CDialog(SliderEditForm::IDD, 0)
{
	RegisterWndClass(WND_CLASS);

	from_ = 0.0;
	to_ = 10.0;
	cur_val_ = 0.0;
	precision_ = 0;

	in_update_ = true;
}


SliderEditForm::~SliderEditForm()
{}


void SliderEditForm::DoDataExchange(CDataExchange* DX)
{
	CDialog::DoDataExchange(DX);
	DDX_Control(DX, IDC_SLIDER, slider_);
	DDX_Control(DX, IDC_SPIN, spin_);
}


BEGIN_MESSAGE_MAP(SliderEditForm, CDialog)
	ON_EN_CHANGE(IDC_EDIT, &SliderEditForm::OnChangeEdit)
	//ON_WM_SIZE()
	ON_WM_HSCROLL()
END_MESSAGE_MAP()


// SliderEditForm message handlers

BOOL SliderEditForm::OnInitDialog()
{
	CDialog::OnInitDialog();

	dlg_auto_resize_.BuildMap(this);

	dlg_auto_resize_.SetWndResizing(IDC_SLIDER, DlgAutoResize::RESIZE_H);
	dlg_auto_resize_.SetWndResizing(IDC_EDIT, DlgAutoResize::MOVE_H);
	dlg_auto_resize_.SetWndResizing(IDC_SPIN, DlgAutoResize::MOVE_H);
//	dlg_auto_resize_.SetWndResizing(IDC_RESET, DlgAutoResize::MOVE_H);

	for (int i= 0; i < 2; ++i)
	{
		//slider_wnd_[i].SetRange(MIN_VAL, MAX_VAL);
		//slider_wnd_[i].SetTicFreq(100);
		//slider_wnd_[i].SetPos(0);
		//slider_wnd_[i].SetPageSize(10);

		//spin_wnd_[i].SetRange(MIN_VAL, MAX_VAL);
	}

	in_update_ = false;

	SetValues();

	return true;
}


void SliderEditForm::OnChangeEdit()
{
	if (in_update_)
		return;

	int val= GetDlgItemInt(IDC_EDIT);
	if (val < from_)
		val = from_;
	if (val > to_)
		val = to_;

	Block update(in_update_);
	slider_.SetPos(val);
	cur_val_ = val;

//	ParamChanged(this);
}


void SliderEditForm::OnHScroll(UINT sb_code, UINT pos, CScrollBar* scroll_bar)
{
	if (scroll_bar == 0 || in_update_)
		return;

/*	for (int i= 0; i < 2; ++i)
		if (scroll_bar->m_hWnd == slider_wnd_[i].m_hWnd)
		{
			Block update(in_update_);
			int val= slider_wnd_[i].GetPos();
			SetDlgItemInt(IDC_EDIT_1 + i, val);
			params_.val[i] = val / 100.0;

//			ParamChanged(this);

			break;
		} */
}


void SliderEditForm::SetValues()
{
	Block update(in_update_);

	for (int i= 0; i < 2; ++i)
	{
	//	int val= static_cast<int>(MAX_VAL * params_.val[i]);
	//	slider_wnd_[i].SetPos(val);
	//	SetDlgItemInt(IDC_EDIT_1 + i, val);
	}
}

/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// SortingOptions.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "SortingOptions.h"
#include "WhistlerLook.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// SortingOptions dialog


SortingOptions::SortingOptions(CSortingOptionsNotification* ctrl, float weight)
	: CDialog(SortingOptions::IDD, 0), ctrl_(ctrl), weight_(weight)
{
	//{{AFX_DATA_INIT(SortingOptions)
	thumbnail_size_ = 0;
	//}}AFX_DATA_INIT
	ready_ = false;
	exit_ = false;
}


void SortingOptions::DoDataExchange(CDataExchange* DX)
{
	CDialog::DoDataExchange(DX);
	//{{AFX_DATA_MAP(SortingOptions)
	DDX_Control(DX, IDC_SLIDER, slider_wnd_);
	DDX_Slider(DX, IDC_SLIDER, thumbnail_size_);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(SortingOptions, CDialog)
	//{{AFX_MSG_MAP(SortingOptions)
	ON_WM_KILLFOCUS()
	ON_WM_ACTIVATE()
	ON_WM_VSCROLL()
	ON_COMMAND(SC_CLOSE, OnClose)
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// SortingOptions message handlers

BOOL SortingOptions::OnInitDialog()
{
	CDialog::OnInitDialog();

	close_wnd_.Create(this, false);
	CRect rect;
	GetClientRect(rect);
	CRect bar_rect;
	close_wnd_.GetWindowRect(bar_rect);
	close_wnd_.SetWindowPos(0, rect.right - bar_rect.Width() - 2, rect.top + 2, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

//	SetWindowPos(0, left_top_.x, left_top_.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

	slider_wnd_.SetRange(0, 100, true);
	slider_wnd_.SetLineSize(10);
	slider_wnd_.SetPageSize(10);
	slider_wnd_.SetTicFreq(10);
//	slider_wnd_.SetTic(0);
//	slider_wnd_.SetTic(50);
//	slider_wnd_.SetTic(100);
	thumbnail_size_ = static_cast<int>(weight_ * 100);
	slider_wnd_.SetPos(thumbnail_size_);

	ready_ = true;

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void SortingOptions::OnKillFocus(CWnd* new_wnd)
{
	CDialog::OnKillFocus(new_wnd);
	if (ready_)
	{
		Finished();
	}
}

void SortingOptions::OnActivate(UINT state, CWnd* wnd_other, BOOL minimized)
{
	CDialog::OnActivate(state, wnd_other, minimized);
	if (state == WA_INACTIVE && ready_)
	{
		Finished();
	}
}


bool SortingOptions::Create(CWnd* parent)
{
	m_pParentWnd = parent;

	exit_ = false;

	if (!CDialog::Create(IDD, m_pParentWnd))
		return false;

//	ShowWindow(SW_SHOW);
/*
	while (!exit_)
	{
		MSG msg;
		if (!::GetMessage(&msg, NULL, NULL, NULL))
			break;

		// process this message
		if (::IsDialogMessage(*this, &msg))
			continue;

		//    if (msg.message != WM_KICKIDLE && !PreTranslateMessage(&msg_cur_))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
	}
*/
	return true;
}


void SortingOptions::OnCancel()
{
	Finished();
}

void SortingOptions::OnOK()
{
	Finished();
}


void SortingOptions::Finished()
{
	if (!exit_)
	{
		exit_ = true;
		//DestroyWindow();
		GetParent()->SendMessage(WM_USER);
	}
}


void SortingOptions::OnVScroll(UINT sb_code, UINT pos, CScrollBar* scroll_bar)
{
	switch (sb_code)
	{
	case TB_LINEUP:
	case TB_LINEDOWN:
	case TB_PAGEUP:
	case TB_PAGEDOWN:
	case TB_THUMBPOSITION:
	case TB_TOP:
	case TB_BOTTOM:
	case TB_THUMBTRACK:
		break;

	case TB_ENDTRACK:
//		TRACE("slider: %d\n", slider_wnd_.GetPos());
		ctrl_->SetColorVsShapeWeight(slider_wnd_.GetPos() / 100.0f);
		break;
	}
}


void SortingOptions::OnClose()
{
	Finished();
}


BOOL SortingOptions::OnEraseBkgnd(CDC* dc)
{
	CRect rect;
	GetClientRect(rect);

	if (dc)
	{
		COLORREF rgb_outline= ::GetSysColor(COLOR_3DSHADOW);
		dc->Draw3dRect(rect, rgb_outline, rgb_outline);
		rect.DeflateRect(1, 1);
		dc->FillSolidRect(rect, ::GetSysColor(COLOR_MENU));
	}

	return true;
}


HBRUSH SortingOptions::OnCtlColor(CDC* dc, CWnd* wnd, UINT ctl_color)
{
	HBRUSH hbr= CDialog::OnCtlColor(dc, wnd, ctl_color);

	if (wnd)
	{
		COLORREF rgb_back= ::GetSysColor(COLOR_MENU);
		br_back_.DeleteObject();
		br_back_.CreateSolidBrush(rgb_back);
		hbr = br_back_;
		dc->SetBkColor(rgb_back);
	}

	return hbr;
}


///////////////////////////////////////////////////////////////////////////////


BEGIN_MESSAGE_MAP(CSortingOptionsPopup, CMiniFrameWnd)
	//{{AFX_MSG_MAP(CSortingOptionsPopup)
//	ON_WM_ERASEBKGND()
	ON_WM_ACTIVATE()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_USER, OnFinish)
END_MESSAGE_MAP()

CSortingOptionsPopup* CSortingOptionsPopup::running_instance_= 0;

bool CSortingOptionsPopup::IsPopupActive()
{
	return running_instance_ != 0;
}


CSortingOptionsPopup::CSortingOptionsPopup(CPoint left_top, CWnd* parent, CSortingOptionsNotification* ctrl, float weight)
 : dlg_options_(ctrl, weight), left_top_(left_top)
{
	ASSERT(running_instance_ == 0);
	run_ = true;
	running_instance_ = this;
	Create();
}

CSortingOptionsPopup::~CSortingOptionsPopup()
{
//	TRACE(L"options gone\n");
	running_instance_ = 0;
}


bool CSortingOptionsPopup::Create()
{
	UINT class_style= CS_SAVEBITS | (WhistlerLook::IsAvailable() ? CS_DROPSHADOW : 0);

	if (!CreateEx(WS_EX_WINDOWEDGE | WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
		AfxRegisterWndClass(class_style, ::LoadCursor(NULL, IDC_ARROW)), 0,
		WS_POPUP | WS_VISIBLE | WS_CLIPCHILDREN | MFS_SYNCACTIVE, CRect(0,0,1,1), AfxGetMainWnd()))
		return false;

	if (!dlg_options_.Create(this))
		return false;

	CRect rect;
	dlg_options_.GetWindowRect(rect);

	SetWindowPos(0, left_top_.x, left_top_.y, rect.Width(), rect.Height(), SWP_NOZORDER);

	ShowWindow(SW_SHOW);

	RunModalLoop();

	DestroyWindow();

	delete this;

	return true;
}


BOOL CSortingOptionsPopup::ContinueModal()
{
	return run_;
}


void CSortingOptionsPopup::OnKillFocus(CWnd* new_wnd)
{
//	run_ = false;
}


LRESULT CSortingOptionsPopup::OnFinish(WPARAM, LPARAM)
{
	run_ = false;
	return 0;
}


void CSortingOptionsPopup::OnActivate(UINT state, CWnd* wnd_other, BOOL minimized)
{
	CMiniFrameWnd::OnActivate(state, wnd_other, minimized);
	if (state == WA_INACTIVE)
		run_ = false;
}


void CSortingOptionsPopup::PostNcDestroy()
{}

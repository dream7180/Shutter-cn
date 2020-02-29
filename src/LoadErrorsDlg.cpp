/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// LoadErrorsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "LoadErrorsDlg.h"
#include "MultiMonitor.h"
#include "CatchAll.h"
#include "ImgLogger.h"

// CLoadErrorsDlg dialog

CLoadErrorsDlg::CLoadErrorsDlg(CWnd* parent, const ImgLogger& logger)
	: CDialog(CLoadErrorsDlg::IDD, parent), logger_(logger)
{
	min_size_ = CSize(0, 0);
}

CLoadErrorsDlg::~CLoadErrorsDlg()
{
}

void CLoadErrorsDlg::DoDataExchange(CDataExchange* DX)
{
	CDialog::DoDataExchange(DX);
	DDX_Control(DX, IDC_LIST, list_ctrl_);
}


BEGIN_MESSAGE_MAP(CLoadErrorsDlg, CDialog)
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
	ON_NOTIFY_EX(LVN_GETDISPINFO, IDC_LIST, &CLoadErrorsDlg::OnGetDispInfo)
END_MESSAGE_MAP()


// CLoadErrorsDlg message handlers

BOOL CLoadErrorsDlg::OnInitDialog()
{
	try
	{
		CDialog::OnInitDialog();

		SetIcon(AfxGetApp()->LoadIcon(IDR_MAINFRAME), false);

		resize_.BuildMap(this);

		resize_.SetWndResizing(IDC_LIST, DlgAutoResize::RESIZE);
		resize_.SetWndResizing(IDCANCEL, DlgAutoResize::MOVE);

		CRect rect(0,0,0,0);
		GetWindowRect(rect);

		min_size_.cx = rect.Width() / 2;
		min_size_.cy = rect.Height() / 2;

		wnd_pos_.reset(new WindowPosition(_T("ImageLoadErrorsDlg"), &rect));

		rect = wnd_pos_->GetLocation(true);
		MoveWindow(rect);

		{
			CRect list;
			list_ctrl_.GetClientRect(list);
			int w= std::max<int>(50, (list.Width() - 25) / 2);
			list_ctrl_.InsertColumn(0, _T("错误消息"), LVCFMT_LEFT, w);
			list_ctrl_.InsertColumn(1, _T("图像路径"), LVCFMT_LEFT, w);
		}

		const size_t count= logger_.GetCount();
		list_ctrl_.SetItemCount(static_cast<int>(count));

		return TRUE;  // return TRUE unless you set the focus to a control
		// EXCEPTION: OCX Property Pages should return FALSE
	}
	CATCH_ALL

	EndDialog(IDCANCEL);
	return true;
}


void CLoadErrorsDlg::OnSize(UINT type, int cx, int cy)
{
	CDialog::OnSize(type, cx, cy);

	resize_.Resize();
}


void CLoadErrorsDlg::OnOK()
{
	OnCancel();
}


void CLoadErrorsDlg::OnCancel()
{
	if (wnd_pos_.get())
		wnd_pos_->StoreState(*this);

	CDialog::OnCancel();
}


void CLoadErrorsDlg::OnGetMinMaxInfo(MINMAXINFO* MMI)
{
	CDialog::OnGetMinMaxInfo(MMI);

	if (MMI)
	{
		MMI->ptMinTrackSize.x = min_size_.cx;
		MMI->ptMinTrackSize.y = min_size_.cy;
	}
}


BOOL CLoadErrorsDlg::OnGetDispInfo(UINT id, NMHDR* nmhdr, LRESULT* result)
{
	NMLVDISPINFO* disp_info= reinterpret_cast<NMLVDISPINFO*>(nmhdr);
	*result = 0;

	if ((disp_info->item.mask & LVIF_TEXT) == 0)
		return false;

	int line= disp_info->item.iItem;

	try
	{
		ImgLogger::LogEntry entry= logger_.GetItem(line);

		switch (disp_info->item.iSubItem)
		{
		case 0:	// error message
			_tcsncpy(disp_info->item.pszText, entry.first.c_str(), disp_info->item.cchTextMax);
			break;

		case 1:	// path
			_tcsncpy(disp_info->item.pszText, entry.second.c_str(), disp_info->item.cchTextMax);
			break;
		}
	}
	catch(...)
	{
		ASSERT(false);
	}

	return true;
}

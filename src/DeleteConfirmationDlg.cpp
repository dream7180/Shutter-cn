/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// DeleteConfirmationDlg.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "DeleteConfirmationDlg.h"
//#include "LoadJpeg.h"
//extern AutoPtr<Dib> LoadJpeg(UINT id_img);

// DeleteConfirmationDlg dialog

const COLORREF BACKGND_COLOR= ::GetSysColor(COLOR_3DFACE);	// color taken from background image


DeleteConfirmationDlg::DeleteConfirmationDlg(CWnd* parent, CString msg)
	: CDialog(DeleteConfirmationDlg::IDD, parent), msg_(msg)
{
	//background_ = LoadJpeg(IDB_TRASHCAN);
	back_.CreateSolidBrush(BACKGND_COLOR);
}

DeleteConfirmationDlg::~DeleteConfirmationDlg()
{
}

void DeleteConfirmationDlg::DoDataExchange(CDataExchange* DX)
{
	CDialog::DoDataExchange(DX);
}


BEGIN_MESSAGE_MAP(DeleteConfirmationDlg, CDialog)
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()


// DeleteConfirmationDlg message handlers

BOOL DeleteConfirmationDlg::OnEraseBkgnd(CDC* dc)
{
	/*if (background_ && background_->IsValid())
	{
		CRect rect(0,0,0,0);
		GetClientRect(rect);

		if (!rect.IsRectEmpty())
		{
			DrawPreserveAspectRatio(*background_, dc, rect, 0, false, true);
			return true;
		}
	}
*/
	return CDialog::OnEraseBkgnd(dc);
}


BOOL DeleteConfirmationDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetDlgItemText(IDC_LABEL, msg_);

	this->GotoDlgCtrl(GetDlgItem(IDOK));

	return false;
}


HBRUSH DeleteConfirmationDlg::OnCtlColor(CDC* dc, CWnd* wnd, UINT ctl_color)
{
	if (ctl_color == CTLCOLOR_STATIC)
	{
		// hollow brush to make label ctrl transparent
		HBRUSH hbr= (HBRUSH)::GetStockObject(NULL_BRUSH);

		dc->SetBkColor(BACKGND_COLOR);
		dc->SetBkMode(TRANSPARENT);

		return hbr;
	}
	else
	{
		dc->SetBkColor(BACKGND_COLOR);
		return back_;
	}
}

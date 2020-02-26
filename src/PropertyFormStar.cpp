/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// PropertyFormStar.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "PropertyFormStar.h"


// PropertyFormStar dialog

PropertyFormStar::PropertyFormStar(CWnd* parent /*=NULL*/) : CDialog(PropertyFormStar::IDD, parent)
{
	ready_ = false;
	modified_ = false;
}

PropertyFormStar::~PropertyFormStar()
{
}

void PropertyFormStar::DoDataExchange(CDataExchange* DX)
{
	CDialog::DoDataExchange(DX);
	DDX_Control(DX, IDC_STARS, star_);
	DDX_Control(DX, IDC_SPIN, spin_);
}

BOOL PropertyFormStar::OnInitDialog()
{
	CDialog::OnInitDialog();

	star_.SetWindowPos(0, 0, 1, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOREDRAW | SWP_NOACTIVATE);
	spin_.SetRange(0, star_.GetStarCount());

	star_.SetClickCallback(boost::bind(&PropertyFormStar::RatingChanged, this, _1));

	return TRUE;  // return TRUE  unless you set the focus to a control
}


BEGIN_MESSAGE_MAP(PropertyFormStar, CDialog)
	ON_EN_CHANGE(IDC_EDIT, OnNumChange)
	ON_EN_SETFOCUS(IDC_EDIT, OnFocusSet)
END_MESSAGE_MAP()


bool PropertyFormStar::Create(CWnd* parent, String* init)
{
	if (!CDialog::Create(IDD, parent))
		return false;

	if (init && !init->empty())
	{
		SetDlgItemText(IDC_EDIT, init->c_str());
		OnNumChange();
	}

	ready_ = true;

	return true;
}


void PropertyFormStar::RatingChanged(int stars)
{
	if (star_.GetRating() == stars && stars > 0)
		stars--;

	SetDlgItemInt(IDC_EDIT, stars);
}


void PropertyFormStar::OnNumChange()
{
	if (star_.m_hWnd == 0)
		return;

	int n= GetDlgItemInt(IDC_EDIT);
	if (n >= 0 && n <= star_.GetStarCount())
		star_.SetRating(n);
	else
		star_.SetRating(0);

	modified_ = true;

	if (ready_)
		GetParent()->SendMessage(WM_COMMAND, MAKELONG(GetDlgCtrlID(), EN_CHANGE), 0);//msg->lParam);
}


void PropertyFormStar::OnFocusSet()
{
	if (ready_)
	{
		const MSG* msg= GetCurrentMessage();
		GetParent()->SendMessage(WM_COMMAND, MAKELONG(GetDlgCtrlID(), EN_SETFOCUS), msg->lParam);
	}
}


void PropertyFormStar::Reset(String* init)
{
	if (init)
	{
		SetDlgItemText(IDC_EDIT, init->c_str());
		OnNumChange();
	}
	modified_ = false;
}


bool PropertyFormStar::IsModified() const
{
	return modified_;
}


String PropertyFormStar::Read() const
{
	CString str;
	GetDlgItemText(IDC_EDIT, str);
	return String(str);
}

/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// ImageInfoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "ImageInfoDlg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

bool LoadImageList(CImageList& il, UINT id, int img_width);


// ImageInfoDlg dialog

ImageInfoDlg::ImageInfoDlg(CWnd* parent /*=NULL*/)
	: CDialog(ImageInfoDlg::IDD, parent)
{
}


ImageInfoDlg::~ImageInfoDlg()
{
}


void ImageInfoDlg::DoDataExchange(CDataExchange* DX)
{
	CDialog::DoDataExchange(DX);
	DDX_Control(DX, IDC_EFFECTS, ctrl_image_);
}


BEGIN_MESSAGE_MAP(ImageInfoDlg, CDialog)
END_MESSAGE_MAP()


// ImageInfoDlg message handlers


bool ImageInfoDlg::Create(CWnd* parent)
{
	return !!CDialog::Create(IDD, parent);
}


void ImageInfoDlg::ShowColors(RGBQUAD original, RGBQUAD new_color, bool hide_colors)
{
	if (hide_colors)
	{
		const TCHAR* empty= _T("- / -");
		SetDlgItemText(IDC_RED, empty);
		SetDlgItemText(IDC_GREEN, empty);
		SetDlgItemText(IDC_BLUE, empty);
	}
	else
	{
		CString str;

		str.Format(_T("%03d / %03d"), int(original.rgbRed), int(new_color.rgbRed));
		SetDlgItemText(IDC_RED, str);

		str.Format(_T("%03d / %03d"), int(original.rgbGreen), int(new_color.rgbGreen));
		SetDlgItemText(IDC_GREEN, str);

		str.Format(_T("%03d / %03d"), int(original.rgbBlue), int(new_color.rgbBlue));
		SetDlgItemText(IDC_BLUE, str);
	}
}


BOOL ImageInfoDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	eye_drop_bmp_.LoadMappedBitmap(IDB_EYE_DROP);

	SendDlgItemMessage(IDC_EYE_DROP, STM_SETIMAGE, IMAGE_BITMAP, LPARAM(eye_drop_bmp_.m_hObject));

	LoadImageList(image_list_, IDB_IMG_EFFECTS, 18);

	ctrl_image_.SetImageList(&image_list_);
	ctrl_image_.SetImageSpace(3);

	return true;
}

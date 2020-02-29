/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// ImgCropPage.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "ImgCropPage.h"
#include "Dib.h"
#include "Block.h"
#include "Transform.h"
#include "Path.h"
#include "PhotoInfo.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


// ImgCropPage dialog

ImgCropPage::ImgCropPage(CPreviewCtrl& preview_wnd, PhotoInfoPtr photo, CWnd* parent /*=NULL*/)
	: ImgPage(ImgCropPage::IDD, parent, false), preview_wnd_(preview_wnd)
{
	in_update_ = true;
	lossless_crop_possible_ = photo->IsLosslessJPEGCropPossible();
	original_aspect_ratio_ = CSize(photo->GetWidth(), photo->GetHeight());
	selected_ratio_index_ = 0;
	reverse_constraints_ = false;
	preview_wnd_.SetHost(this);
}


ImgCropPage::~ImgCropPage()
{
}


void ImgCropPage::DoDataExchange(CDataExchange* DX)
{
	DDX_Control(DX, IDC_CROP_SIZE, size_wnd_);
	DDX_Control(DX, IDC_RATIOS, comboRatios_);
	DDX_Control(DX, IDC_REVERSE, reverseBtn_);
	ImgPage::DoDataExchange(DX);
}


BEGIN_MESSAGE_MAP(ImgCropPage, ImgPage)
	ON_BN_CLICKED(IDC_RESET, OnReset)
	ON_BN_CLICKED(IDC_LOSSLESS, OnLosslessCrop)
	ON_BN_CLICKED(IDC_FREE, OnFreeCrop)
	ON_CBN_SELCHANGE(IDC_RATIOS, OnConstrain)
	ON_BN_CLICKED(IDC_REVERSE, OnReverse)
END_MESSAGE_MAP()


// ImgCropPage message handlers

static struct { const TCHAR* name; SIZE size; } ratios[]=
{
	{ _T("No constraints"), { 0, 0 } },
	{ _T("3\"x5\""),	{ 3, 5 } },
	{ _T("3.5\"x5\""),	{ 35, 50 } },
	{ _T("4\"x6\""),	{ 4, 6 } },
	{ _T("5\"x7\""),	{ 5, 7 } },
	{ _T("8\"x10\""),	{ 8, 10 } },
	{ _T("8\"x12\""),	{ 8, 12 } },
	{ _T("11\"x14\""),	{ 11, 14 } },
	{ _T("12\"x18\""),	{ 12, 18 } },
	{ _T("16\"x20\""),	{ 16, 20 } },
	{ _T("20\"x24\""),	{ 20, 24 } },
	{ _T("20\"x30\""),	{ 20, 30 } },
	{ _T("5 cm x 8 cm"),	{ 5, 8 } },
	{ _T("10 cm x 15 cm"),	{ 10, 15 } },
	{ _T("13 cm x 18 cm"),	{ 13, 18 } },
	{ _T("15 cm x 20 cm"),	{ 15, 20 } },
	{ _T("20 cm x 25 cm"),	{ 20, 25 } },
	{ _T("20 cm x 30 cm"),	{ 20, 30 } },
	{ _T("Square (1:1)"),	{ 1, 1 } },
	{ _T("Screen (4:3)"),	{ 3, 4 } },
//	{ _T("Wide screen (19:12)"),	{ 12, 19 } },
	{ _T("Wide screen (16:9)"),		{ 9, 16 } },
	{ _T("Wide screen (16:10)"),	{ 10, 16 } },
	{ _T("Image aspect ratio"),	{ 0, 0 } }
};


BOOL ImgCropPage::OnInitDialog()
{
	ImgPage::OnInitDialog();

	dlg_resize_map_.BuildMap(this);

	dlg_resize_map_.SetWndResizing(IDC_LABEL_1, DlgAutoResize::RESIZE_H);
	dlg_resize_map_.SetWndResizing(IDC_RESET, DlgAutoResize::MOVE_H);

	comboRatios_.ResetContent();

	for (size_t i= 0; i < array_count(ratios); ++i)
		comboRatios_.AddString(ratios[i].name);

	comboRatios_.SetCurSel(selected_ratio_index_);
	reverseBtn_.SetCheck(reverse_constraints_ ? 1 : 0);

	in_update_ = false;

	if (lossless_crop_possible_)
	{
		CheckRadioButton(IDC_LOSSLESS, IDC_FREE, IDC_LOSSLESS);
		preview_wnd_.EnableMagneticGrid(true);
	}
	else
	{
		CheckRadioButton(IDC_LOSSLESS, IDC_FREE, IDC_FREE);
		preview_wnd_.EnableMagneticGrid(false);

		if (CWnd* wnd= GetDlgItem(IDC_LOSSLESS))
			wnd->EnableWindow(false);
	}

	OnConstrain();

	SetValues();

	return true;
}


void ImgCropPage::Transform(Dib& dib, bool preview)
{
	if (preview)
		return;

	CRect rect= preview_wnd_.GetSelectionRect();

	if (rect.IsRectEmpty())
		return;

	Dib crop(rect.Width(), rect.Height(), dib.GetBitsPerPixel());

	CDC dc;
	dc.CreateCompatibleDC(0);
	dc.SelectObject(crop.GetBmp());

	dib.Draw(&dc, CRect(CPoint(0, 0), rect.Size()), &rect, false, false);

	dib.Swap(crop);
}


void ImgCropPage::TransformFile(Path& photo, Path& output)
{
	CRect rect= preview_wnd_.GetSelectionRect();
	if (rect.IsRectEmpty())
		return;
	::JPEG_Crop_raw(photo, output, rect);
}


bool ImgCropPage::IsExclusive(PhotoInfoPtr photo) const
{
	return photo->IsLosslessJPEGCropPossible() &&
		preview_wnd_.IsMagneticGridEnabled();
}


void ImgCropPage::OnReset()
{
	preview_wnd_.SetSelectionRect(CRect(0,0,0,0));
	//params_.Reset();
	SetValues();
	ParamChanged(this, true);
}


void ImgCropPage::SetValues()
{
//	Block update(in_update_);

	CRect rect= preview_wnd_.GetSelectionRect();

	if (rect.IsRectEmpty())
		size_wnd_.SetWindowText(_T("无裁切"));
	else
	{
		oStringstream ost;
		ost << rect.Width() << _T(" x ") << rect.Height();
		size_wnd_.SetWindowText(ost.str().c_str());
	}

	//for (int i= 0; i < 2; ++i)
	//{
	//	int val= static_cast<int>(MAX_VAL * params_.val[i]);
	//	slider_wnd_[i].SetPos(val);
	//	SetDlgItemInt(IDC_EDIT_1 + i, val);
	//}
}


void ImgCropPage::OnLosslessCrop()
{
	preview_wnd_.EnableMagneticGrid(true);
	SetValues();
	if (!preview_wnd_.GetSelectionRect().IsRectEmpty())
		ParamChanged(this);
}


void ImgCropPage::OnFreeCrop()
{
	preview_wnd_.EnableMagneticGrid(false);
	SetValues();
	if (!preview_wnd_.GetSelectionRect().IsRectEmpty())
		ParamChanged(this);
}


void ImgCropPage::SelectionRectChanged(const CRect& rect)
{
	SetValues();
	ParamChanged(this, !!preview_wnd_.GetSelectionRect().IsRectEmpty());
}


void ImgCropPage::OnConstrain()
{
	if (in_update_)
		return;

	size_t index= comboRatios_.GetCurSel();
	if (index < array_count(ratios))
	{
		CSize s= ratios[index].size;

		reverse_constraints_ = reverseBtn_.GetCheck() != 0;

		if (index == array_count(ratios) - 1)	// image's aspect ratio?
		{
			s = original_aspect_ratio_;

			if (reverse_constraints_)
				std::swap(s.cx, s.cy);
		}
		else
		{
			// Note: this 'not' condition here is due to the ratio values being in H x W form
			if (!reverse_constraints_)
				std::swap(s.cx, s.cy);
		}

		preview_wnd_.SetRatioConstrain(s);

		selected_ratio_index_ = static_cast<int>(index);
	}

	reverseBtn_.EnableWindow(index != 0);
}


void ImgCropPage::OnReverse()
{
	OnConstrain();
}


void ImgCropPage::RestoreSettings(const TCHAR* key)
{
	selected_ratio_index_ = AfxGetApp()->GetProfileInt(key, _T("CropRatioIndex"), selected_ratio_index_);
	reverse_constraints_ = !!AfxGetApp()->GetProfileInt(key, _T("PortraitCrop"), reverse_constraints_);
}


void ImgCropPage::StoreSettings(const TCHAR* key)
{
	AfxGetApp()->WriteProfileInt(key, _T("CropRatioIndex"), selected_ratio_index_);
	AfxGetApp()->WriteProfileInt(key, _T("PortraitCrop"), reverse_constraints_);
}

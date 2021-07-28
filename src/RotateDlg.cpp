/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// RotateDlg.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "RotateDlg.h"
#include "BmpFunc.h"
#include "ImageDraw.h"
#include "Block.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static const TCHAR* REGISTRY_ENTRY_ROTATE=	_T("RotateDlg");


PhotoPreview::PhotoPreview()
{
	rotation_ = ROTATE_NONE;
	mirror_ = false;
	show_margins_ = show_prinatable_area_ = print_page_number_ = show_image_space_ = false;
	SetPageSize(CSize(200, 200), CRect(10,10,10,10));
	SetItemsAcross(1);
}

PhotoPreview::~PhotoPreview()
{}

int PhotoPreview::GetPageCount(int items_count) const
{
	return 1;
}

CSize PhotoPreview::GetImageSize() const
{
	return CSize(200, 200);
}

bool PhotoPreview::PrintFn(CDC& dc, CRect page_rect, VectPhotoInfo& photos, int page)
{
	if (photos.empty())
		return true;

	page_rect.DeflateRect(margins_rect_);

	if (!image_.IsValid())
	{
		Dib* bmp= photos.front()->GetThumbnail();
		if (bmp == 0)
			return true;

		image_.Clone(*bmp);

		if (mirror_)
			::FlipBitmap(image_, mirror_, false);

		if (rotation_ == ROTATE_90_DEG_CW || rotation_ == ROTATE_90_DEG_COUNTERCW)
			image_.RotateInPlace(rotation_ == ROTATE_90_DEG_CW);
		else if (rotation_ == ROTATE_180_DEG)
		{
			image_.RotateInPlace(true);
			image_.RotateInPlace(true);
		}
	}

	if (image_.IsValid())
		ImageDraw::Draw(&image_, &dc, page_rect, RGB(255,255,255), 0, 0, 0, 0, ImageDraw::DRAW_HALFTONE | ImageDraw::DRAW_SHADOW, nullptr);

	return true;
}


void PhotoPreview::SetTransformation(RotationTransformation rotation, bool horz_flip, bool vert_flip)
{
	rotation_ = rotation;
	mirror_ = horz_flip;

	image_.Swap(Dib());	// empty
}


/////////////////////////////////////////////////////////////////////////////
// CRotateDlg dialog


CRotateDlg::CRotateDlg(VectPhotoInfo& selected, bool all, CWnd* parent /*=NULL*/)
	: DialogChild(CRotateDlg::IDD, parent), all_(all), preview_(selected, &print_)
{
	rotate_180_ = rotate_cw_ = rotate_ccw_ = false;
	rotate_auto_ = true;
	mirror_ = false;
	fix_rotated_photos_ = false;
	update_ = false;

	operation_.Register(REGISTRY_ENTRY_ROTATE, L"Operation", AUTO_ROTATE);
	horz_flip_.Register(REGISTRY_ENTRY_ROTATE, L"HorzFlip", false);
}


void CRotateDlg::DoDataExchange(CDataExchange* DX)
{
	DialogChild::DoDataExchange(DX);

	DDX_Check(DX, IDC_90_CW, rotate_cw_);
	DDX_Check(DX, IDC_90_CCW, rotate_ccw_);
	DDX_Check(DX, IDC_180, rotate_180_);
	DDX_Check(DX, IDC_AUTO, rotate_auto_);
	DDX_Check(DX, IDC_HORZ_FLIP, mirror_);

	DDX_Control(DX, IDOK, ok_button_);
	//DDX_Control(DX, IDC_SEPARATOR, separator_);
	DDX_Control(DX, IDC_IMAGE_1, images_[0]);
	DDX_Control(DX, IDC_IMAGE_2, images_[1]);
	DDX_Control(DX, IDC_IMAGE_3, images_[2]);
	DDX_Control(DX, IDC_IMAGE_4, images_[3]);
	DDX_Control(DX, IDC_IMAGE_5, images_[4]);
}


BEGIN_MESSAGE_MAP(CRotateDlg, DialogChild)
	//{{AFX_MSG_MAP(CRotateDlg)
	ON_CONTROL_RANGE(BN_CLICKED, IDC_90_CW, IDC_90_CW, UpdateCheckBoxes)
	ON_CONTROL_RANGE(BN_CLICKED, IDC_90_CCW, IDC_90_CCW, UpdateCheckBoxes)
	ON_CONTROL_RANGE(BN_CLICKED, IDC_AUTO, IDC_AUTO, UpdateCheckBoxes)
	ON_CONTROL_RANGE(BN_CLICKED, IDC_HORZ_FLIP, IDC_HORZ_FLIP, UpdateCheckBoxes)
	ON_CONTROL_RANGE(BN_CLICKED, IDC_180, IDC_180, UpdateCheckBoxes)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRotateDlg message handlers

BOOL CRotateDlg::OnInitDialog()
{
	int operation= operation_;

	rotate_cw_ = rotate_ccw_ = rotate_auto_ = false;

	switch (operation)
	{
	case ROTATE_90_DEG_CW:
		rotate_cw_ = true;
		break;
	case ROTATE_90_DEG_COUNTERCW:
		rotate_ccw_ = true;
		break;
	case AUTO_ROTATE:
		rotate_auto_ = true;
		break;
	case ROTATE_180_DEG:
		rotate_180_ = true;
		break;
	}

	mirror_ = horz_flip_;

	DialogChild::OnInitDialog();

	//SubclassHelpBtn(_T("ToolRotate.htm"));

	if (all_)
		SetDlgItemText(IDOK, L"全部旋转!");

	GetDlgItemText(IDOK, original_ok_btn_text_);

	images_[0].SetImage(IDB_ROTATE_LEFT);
	images_[1].SetImage(IDB_ROTATE_RIGHT);
	images_[2].SetImage(IDB_ROTATE_AUTO);
	images_[3].SetImage(IDB_ROTATE_180);
	images_[4].SetImage(IDB_FLIP_HORZ);

	if (CWnd* wnd= GetDlgItem(IDC_PREVIEW))
	{
		WINDOWPLACEMENT wp;
		wnd->GetWindowPlacement(&wp);
		wnd->DestroyWindow();

		preview_.Create(this, wp.rcNormalPosition);
		preview_.SetDlgCtrlID(IDC_PREVIEW);
	}

	UpdateExample();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


RotationTransformation CRotateDlg::GetOperation() const
{
	int operation= GetOperationEx();

	if (fix_rotated_photos_)
		operation = FIX_ROTATED_THUMBNAIL;

	return static_cast<RotationTransformation>(operation);
}


int CRotateDlg::GetOperationEx() const
{
	int operation= ROTATE_NONE;

	if (rotate_cw_)
		operation = ROTATE_90_DEG_CW;

	if (rotate_ccw_)
		operation = ROTATE_90_DEG_COUNTERCW;

	if (rotate_180_)
		operation = ROTATE_180_DEG;

	if (rotate_auto_)
		operation = AUTO_ROTATE;

	return static_cast<RotationTransformation>(operation);
}


void CRotateDlg::OnOK()
{
	if (UpdateData())
	{
		operation_ = GetOperationEx();
		horz_flip_ = !!mirror_;
	}

	EndDialog(IDOK);
}


BOOL CRotateDlg::ContinueModal()
{
	if (!DialogChild::ContinueModal())
		return false;

	if (MSG *msg = AfxGetCurrentMessage())
	{
		if (msg->message == WM_KEYDOWN || msg->message == WM_KEYUP ||
			msg->message == WM_SYSKEYDOWN || msg->message == WM_SYSKEYUP)
		{
			fix_rotated_photos_ = ::GetKeyState(VK_SHIFT) < 0 && ::GetKeyState(VK_CONTROL) < 0 && ::GetKeyState(VK_MENU) < 0;

			if (ok_button_.m_hWnd)
			{
				CString str;
				ok_button_.GetWindowText(str);

				const TCHAR* FIX= _T("Fix");

				if (fix_rotated_photos_ && str != FIX)
				{
					ok_button_.SetWindowText(FIX);
					ok_button_.Invalidate();
				}
				else if (!fix_rotated_photos_ && str == FIX)
				{
					ok_button_.SetWindowText(original_ok_btn_text_);
					ok_button_.Invalidate();
				}
			}
		}
	}

	return true;
}


void CRotateDlg::UpdateCheckBoxes(UINT id)
{
	if (update_)
		return;

	Block b(update_);

	switch (id)
	{
	case IDC_90_CW:		// handle mutually exclusive buttons
	case IDC_90_CCW:
	case IDC_180:
	case IDC_AUTO:
		{
			if (IsDlgButtonChecked(id))
			{
				int ids[]= { IDC_90_CW, IDC_90_CCW, IDC_180, IDC_AUTO };
				for (int i= 0; i < array_count(ids); ++i)
					if (ids[i] != id)
						CheckDlgButton(ids[i], 0);
			}
		break;
		}
	}

	UpdateExample();
}


void CRotateDlg::UpdateExample()
{
	bool horz_flip= !!IsDlgButtonChecked(IDC_HORZ_FLIP);
	bool vert_flip= false;

	RotationTransformation rotation= ROTATE_NONE;

	if (IsDlgButtonChecked(IDC_90_CW))
		rotation = ROTATE_90_DEG_CW;
	else if (IsDlgButtonChecked(IDC_90_CCW))
		rotation = ROTATE_90_DEG_COUNTERCW;
	else if (IsDlgButtonChecked(IDC_180))
		rotation = ROTATE_180_DEG;
	else if (IsDlgButtonChecked(IDC_AUTO))
		rotation = AUTO_ROTATE;

	print_.SetTransformation(rotation, horz_flip, vert_flip);

	preview_.Invalidate();

	ok_button_.EnableWindow(rotation != ROTATE_NONE || horz_flip || vert_flip);
}

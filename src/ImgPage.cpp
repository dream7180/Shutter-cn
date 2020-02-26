/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// ImgPage.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "ImgPage.h"
#include "WhistlerLook.h"
#include "CatchAll.h"
#include "PhotoInfo.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


// ImgPage dialog

ImgPage::ImgPage(UINT dlg_id, CWnd* parent /*=NULL*/, bool need_preview/*= true*/)
	: CDialog(dlg_id, parent), bitmap_size_(0, 0)
{
	host_ = 0;
	need_preview_ = need_preview;
//	exclusive_ = false;
}


ImgPage::~ImgPage()
{
	// delete DC before the bitmap selected into it
	if (background_dc_.m_hDC)
		background_dc_.DeleteDC();
}


void ImgPage::DoDataExchange(CDataExchange* DX)
{
	CDialog::DoDataExchange(DX);
}


BEGIN_MESSAGE_MAP(ImgPage, CDialog)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_CTLCOLOR()
	ON_WM_SIZE()
END_MESSAGE_MAP()


// ImgPage message handlers


bool ImgPage::Create(CWnd* parent)
{
	return !!CDialog::Create(m_lpszTemplateName, parent);
}


BOOL ImgPage::OnEraseBkgnd(CDC* dc)
{
	CRect rect;
	GetClientRect(rect);

	if (background_dc_.m_hDC)
	{
		dc->BitBlt(0, 0, bitmap_size_.cx, bitmap_size_.cy, &background_dc_, 0, 0, SRCCOPY);
		return true;
	}

	return CDialog::OnEraseBkgnd(dc);
}


void ImgPage::OnPaint()
{
	CPaintDC dc(this); // device context for painting
}

/*
LPARAM ImgPage::OnPrintClient(WPARAM HDC, LPARAM flags)
{
	if (flags == PRF_CLIENT && HDC != 0 && background_dc_.m_hDC != 0)
	{
		CRect rect;
		GetClientRect(rect);
		HDC dc= HDC(HDC);
//		::SetBkMode(dc, OPAQUE);
//	::SetBkColor(dc, 0xc0c0c0);
//	::ExtTextOut(dc, 0, 0, ETO_OPAQUE, rect, NULL, 0, NULL);
		::BitBlt(dc, 0, 0, bitmap_size_.cx, bitmap_size_.cy, background_dc_, 0, 0, SRCCOPY);
		return 0;
	}
	else
		return Default();
} */


//LPARAM ImgPage::OnCtrlColor(WPARAM HDC, LPARAM ctrl)
//{
//	HDC dc= HDC(HDC);
//	HWND ctrl= HWND(ctrl);
//
//	WINDOWPLACEMENT wp;
//	wp.length = sizeof wp;
//	if (::GetWindowPlacement(ctrl, &wp))
//		::SetBrushOrgEx(dc, wp.rcNormalPosition.left, bitmap_size_.cy - wp.rcNormalPosition.top, 0);
//
//	::SetBkMode(dc, TRANSPARENT);
//	return LPARAM(br_background_.m_hObject);
//}


HBRUSH ImgPage::OnCtlColor(CDC* dc, CWnd* wnd, UINT flags)
{
	UINT msg= GetCurrentMessage()->message;
	if (wnd == 0 || dc == 0 || msg == WM_CTLCOLOREDIT || msg == WM_CTLCOLORLISTBOX || !WhistlerLook::IsAvailable())
		return HBRUSH(Default());

	WINDOWPLACEMENT wp;
	if (wnd->GetWindowPlacement(&wp))
		dc->SetBrushOrg(-wp.rcNormalPosition.left, bitmap_size_.cy - wp.rcNormalPosition.top);

	dc->SetBkMode(TRANSPARENT);

	return HBRUSH(br_background_.m_hObject);
}


void ImgPage::OnSize(UINT type, int cx, int cy)
{
	CDialog::OnSize(type, cx, cy);

	if (type != SIZE_MINIMIZED && cx > 0 && cy > 0)
	{
		if (CWnd* parent= GetParent())
		{
			try
			{
				CClientDC dc(this);
				if (background_dc_.m_hDC == 0)
					background_dc_.CreateCompatibleDC(&dc);

				background_bmp_.DeleteObject();
				background_bmp_.CreateCompatibleBitmap(&dc, cx, cy);
				background_dc_.SelectObject(&background_bmp_);

				bitmap_size_ = CSize(cx, cy);

				WINDOWPLACEMENT wp;
				GetWindowPlacement(&wp);

				// prepare copy of background
				background_dc_.SetWindowOrg(wp.rcNormalPosition.left, wp.rcNormalPosition.top);
				parent->Print(&background_dc_, PRF_ERASEBKGND | PRF_CLIENT);

				background_dc_.SetWindowOrg(0, 0);

				// prepare bmp brush for CTRLCOLOR message
				br_background_.DeleteObject();
				br_background_.CreatePatternBrush(&background_bmp_);
			}
			CATCH_ALL
		}

		dlg_resize_map_.Resize();
	}
}


void ImgPage::ParamChanged(ImgPage* wnd, bool reset/*= false*/)
{
	if (host_)
		host_->ParamChanged(wnd, reset);

	UpdateWindow();
}


void ImgPage::ParamChanged()
{
	if (host_)
		host_->ParamChanged(this, false);
}


void ImgPage::Initialize(const Dib& dibOriginal)
{}


void ImgPage::TransformFile(Path& pathPhoto, Path& pathOutput)
{}

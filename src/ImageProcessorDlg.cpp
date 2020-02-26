/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// ImageProcessorDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CatchAll.h"
#include "ImageProcessorDlg.h"
#include "DlgListCtrl.h"
#include "PhotoCtrl.h"
#include "ProcessorResizeForm.h"
#include "ProcessorCropForm.h"
#include "ProcessorSharpenForm.h"
#include "LoadImageList.h"
#include "MonitorCtrl.h"
#include "ViewPane.h"
//#include <boost/bind.hpp>

//#define _MAGICKLIB_	1
#define MAGICK_STATIC_LINK 1
#define MAGICKCORE_LQR_DELEGATE 1
#ifdef _DEBUG
// #include "C:\data\ImageMagick-6.4.4\magick\MagickCore.h"
#endif


struct ImageProcessorDlg::Impl : PhotoCtrlNotification
{
	Impl(VectPhotoInfo& photos) : photos_(photos)
	{}

	DlgListCtrl dlg_container_;
	PhotoCtrl input_photos_;
	MonitorCtrl monitor_ctrl;
	ViewPane view_;
	VectPhotoInfo& photos_;
	CImageList icons_;
	ProcessorResizeForm resize_form_;
	ProcessorCropForm crop_form_;
	ProcessorSharpenForm sharpen_form_;

	// photo ctrl
	virtual Dib* RequestThumbnail(PhotoInfoPtr photo, CSize image_size, bool return_if_not_available);
	virtual void CurrentItemChanged(PhotoInfoPtr photo);

	void test(ViewPane& view);

	void OnMonitorResized();
};


ImageProcessorDlg::ImageProcessorDlg(CWnd* parent, VectPhotoInfo& photos)
  : impl_(new Impl(photos)), DialogChild(ImageProcessorDlg::IDD, parent)
{
}


ImageProcessorDlg::~ImageProcessorDlg()
{}


void ImageProcessorDlg::DoDataExchange(CDataExchange* DX)
{
	DialogChild::DoDataExchange(DX);

	DDX_Control(DX, IDC_CONTAINER, impl_->dlg_container_);
	//DDX_Control(DX, IDC_PHOTOS, impl_->input_photos_);
	DDX_Control(DX, IDC_MONITOR, impl_->monitor_ctrl);

}


BEGIN_MESSAGE_MAP(ImageProcessorDlg, DialogChild)
//	ON_WM_SIZE()
END_MESSAGE_MAP()


Dib* ImageProcessorDlg::Impl::RequestThumbnail(PhotoInfoPtr photo, CSize image_size, bool return_if_not_available)
{
	return photo->GetThumbnail(image_size);
}

void ImageProcessorDlg::Impl::CurrentItemChanged(PhotoInfoPtr photo)
{
	//if (photo)
	//	view_.LoadPhoto(*photo, 0, 0);
	//else
	//	view_.Reset();
}


void ImageProcessorDlg::Impl::test(ViewPane& view)
{
#if 0
	CImageDecoderPtr d= photos_[0]->GetDecoder();

	Dib bmp;
	CSize size(500,500);
	if (d->DecodeImg(bmp, size, false) != IS_OK)
		return;

	size = CSize(bmp.GetWidth(), bmp.GetHeight());

	ExceptionInfo exc;

	GetExceptionInfo(&exc);

	Image* img= ConstituteImage(bmp.GetWidth(), bmp.GetHeight(), "RGB", CharPixel, bmp.GetBuffer(), &exc);

	if (img)
	{
		double radius= 10.0;
		double sigma= 1.0;
		double amount= 1.0;
		double threshold= 0.01;

	//	Image* sharp= UnsharpMaskImage(img, radius, sigma, amount, threshold, &exc);
		int w= size.cy * 16 / 9;
		Image* sharp= LiquidRescaleImage(img, w, size.cy, 1.0, 0.0, &exc);

		if (sharp)
		{
			//ImageInfo ii;
			//GetImageInfo(&ii);
			//strcpy(sharp->filename, "c:\\sharp.img");
			size.cx = sharp->columns;
			size.cy = sharp->rows;

			const PixelPacket* pix= AcquireImagePixels(sharp, 0, 0, size.cx, size.cy, &exc);

			Dib b(size.cx, size.cy, 24);
			for (int y= size.cy - 1; y >= 0; --y)
			{
				BYTE* line= b.LineBuffer(y);
				for (int x= 0; x < size.cx; ++x)
				{
					line[0] = pix->red;
					line[1] = pix->green;
					line[2] = pix->blue;

					line += 3;
					pix++;
				}
			}

			b.Swap(bmp);

			DestroyImage(sharp);
		}

		DestroyImage(img);
	}

	view.DisplayBitmap(bmp);
	bmp.Save(L"c:\\resized.bmp");
#endif
}


// ImageProcessorDlg message handlers

BOOL ImageProcessorDlg::OnInitDialog()
{
	try
	{
//		CRect client(0,0,0,0);
//		GetClientRect(client);
//		CSize minimal= client.Size();
//		minimal.cy /= 2;
//		SetMinimalDlgSize(minimal);

		DialogChild::OnInitDialog();

		// add monitor display first, then list of photos

		impl_->monitor_ctrl.SetResizeCallback(boost::bind(&Impl::OnMonitorResized, impl_.get()));

		impl_->view_.Create(&impl_->monitor_ctrl);
		impl_->view_.SetBackgndColor(RGB(0,0,0));
		impl_->view_.UseScrollBars(false);
		impl_->view_.CursorStayVisible(true);
		impl_->view_.SetLogicalZoom(0.0, false);
		impl_->view_.EnablePhotoDesc(false);
		impl_->view_.MoveWindow(impl_->monitor_ctrl.GetDisplayArea());

		if (CWnd* frm= GetDlgItem(IDC_PHOTOS))
		{
			WINDOWPLACEMENT wp;
			if (frm->GetWindowPlacement(&wp))
			{
				CRect r= wp.rcNormalPosition;
				impl_->input_photos_.Create(this, impl_.get(), IDC_PHOTOS);
				//impl_->input_photos_.ModifyStyleEx(0, WS_EX_);
				impl_->input_photos_.SetWindowPos(frm, r.left, r.top, r.Width(), r.Height(), SWP_NOACTIVATE);

				impl_->input_photos_.SetImageSize(60);
				impl_->input_photos_.AddItems(impl_->photos_, String(), PhotoCtrl::NO_ICON, 1);
				impl_->input_photos_.ShowItemLabel(false);

				frm->DestroyWindow();
			}
		}

//		impl_->resize_form_.Create(&impl_->dlg_container_);

		LoadPngImageList(impl_->icons_, IDB_TASK_BAR, RGB(255,255,255), true, 20);


		impl_->dlg_container_.SetImageList(&impl_->icons_);

		impl_->dlg_container_.DrawCheckboxes(true);
		impl_->dlg_container_.AddSubDialog(&impl_->crop_form_, 6, 0, true);
		impl_->dlg_container_.AddSubDialog(&impl_->resize_form_, 7, 0, true);
		impl_->dlg_container_.AddSubDialog(&impl_->sharpen_form_, 8, 0, true);

/*
		GetWindowText(impl_->title_);

		impl_->Notify(this, Load);

		impl_->InitDlg(this);
*/
		BuildResizingMap();

		SetWndResizing(IDC_PHOTOS, DlgAutoResize::RESIZE_V);
		SetWndResizing(IDC_CONTAINER, DlgAutoResize::RESIZE_V);
		SetWndResizing(IDC_MONITOR, DlgAutoResize::RESIZE);

		SetWndResizing(IDC_HELP_BTN, DlgAutoResize::MOVE_V);
		SetWndResizing(IDC_LABEL_1, DlgAutoResize::MOVE_V);
		SetWndResizing(IDC_TEMPLATES, DlgAutoResize::MOVE_V);
		SetWndResizing(IDC_PATH, DlgAutoResize::MOVE_V_RESIZE_H);
		SetWndResizing(IDC_FOLDER, DlgAutoResize::MOVE);

		SetWndResizing(IDC_PREVIOUS, DlgAutoResize::MOVE);
		SetWndResizing(IDC_NEXT, DlgAutoResize::MOVE);
		SetWndResizing(IDCANCEL, DlgAutoResize::MOVE);
		SetWndResizing(IDOK, DlgAutoResize::MOVE);

		impl_->test(impl_->view_);

		return true;
	}
	CATCH_ALL

	EndDialog(IDCANCEL);

	return true;
}


void ImageProcessorDlg::OnOK()
{
}


//void ImageProcessorDlg::OnSize(UINT type, int cx, int cy)
//{
//	DialogChild::OnSize(type, cx, cy);
//}


void ImageProcessorDlg::Impl::OnMonitorResized()
{
	if (view_.m_hWnd)
		view_.MoveWindow(monitor_ctrl.GetDisplayArea());
}

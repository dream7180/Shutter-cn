/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// ImageCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "ImageCtrl.h"
#include "MemoryDC.h"
#include "PNGImage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


static const TCHAR* const WND_CLASS= _T("ImageCtrlMiK");

static void RegisterWndClass(const TCHAR* class_name)
{
	HINSTANCE instance= AfxGetInstanceHandle();

	// see if the class already exists
	WNDCLASS wndcls;
	if (!::GetClassInfo(instance, class_name, &wndcls))
	{
		// otherwise we need to register a new class
		wndcls.style = CS_VREDRAW | CS_HREDRAW;
		wndcls.lpfnWndProc = ::DefWindowProc;
		wndcls.cbClsExtra = wndcls.cbWndExtra = 0;
		wndcls.hInstance = instance;
		wndcls.hIcon = 0;
		wndcls.hCursor = 0;//AfxGetApp()->LoadCursor(IDC_CURSOR_POINT);//IDC_VERT_RESIZE);
		wndcls.hbrBackground = 0;
		wndcls.lpszMenuName = NULL;
		wndcls.lpszClassName = class_name;

		AfxRegisterClass(&wndcls);
	}
}


ImageCtrl::ImageCtrl()
{
	RegisterWndClass(WND_CLASS);
}


ImageCtrl::~ImageCtrl()
{}


BEGIN_MESSAGE_MAP(ImageCtrl, CWnd)
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// ImageCtrl message handlers

void ImageCtrl::SetImage(int rsrc_id)
{
	PNGImage png;
	png.LoadImageList(rsrc_id, 0, image_, 0.0f, 0.0f, true, 1);
}


bool ImageCtrl::Create(CWnd* parent)
{
	if (!CWnd::Create(WND_CLASS, _T(""), WS_CHILD | WS_VISIBLE, CRect(0,0,0,0), parent, -1))
		return false;

	return true;
}


BOOL ImageCtrl::OnEraseBkgnd(CDC* dc)
{
	CRect rect(0,0,0,0);
	GetClientRect(rect);

	MemoryDC mem(*dc, this, ::GetSysColor(COLOR_3DFACE));

	if (image_.m_hImageList && image_.GetImageCount() > 0)
	{
		IMAGEINFO ii;
		if (image_.GetImageInfo(0, &ii))
		{
			CRect r= ii.rcImage;
			// center image
			CPoint pos((rect.Width() - r.Width()) / 2, (rect.Height() - r.Height()) / 2);
			image_.Draw(&mem, 0, pos, ILD_NORMAL);
		}
	}

	mem.BitBlt();

	return true;
}

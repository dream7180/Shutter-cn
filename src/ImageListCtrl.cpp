/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// ImageListCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "ImageListCtrl.h"
#include "MemoryDC.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


/////////////////////////////////////////////////////////////////////////////
// CImageListCtrl

CImageListCtrl::CImageListCtrl()
{
	const TCHAR* cls_name= _T("ImageListCtrl");

	HINSTANCE inst= AfxGetResourceHandle();
	// force same dll as resources
//	HINSTANCE inst= AfxFindResourceHandle(MAKEINTRESOURCE(IDD_FORM_DOUBLE), RT_DIALOG);

	// see if the class already exists
	WNDCLASS wndcls;
	if (!::GetClassInfo(inst, cls_name, &wndcls))
	{
		// otherwise we need to register a new class
		wndcls.style = 0;
		wndcls.lpfnWndProc = ::DefWindowProc;
		wndcls.cbClsExtra = wndcls.cbWndExtra = 0;
		wndcls.hInstance = inst;
		wndcls.hIcon = 0;
		wndcls.hCursor = ::LoadCursor(NULL, IDC_ARROW);
		wndcls.hbrBackground = 0;
		wndcls.lpszMenuName = NULL;
		wndcls.lpszClassName = cls_name;

		AfxRegisterClass(&wndcls);
	}

	image_list_ = 0;
	extra_space_ = 0;
}


CImageListCtrl::~CImageListCtrl()
{
}


BEGIN_MESSAGE_MAP(CImageListCtrl, CWnd)
	//{{AFX_MSG_MAP(CImageListCtrl)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CImageListCtrl message handlers

//bool CImageListCtrl::Create(CWnd* parent, CRect rect)
//{
////	pattern_bmp_.LoadBitmap(IDB_GRID_PATTERN);
//
//	if (!CWnd::Create(AfxRegisterWndClass(CS_VREDRAW | CS_HREDRAW, ::LoadCursor(NULL, IDC_ARROW)),
//		0, WS_CHILD | WS_VISIBLE, rect, parent, -1))
//		return false;
//
//	return true;
//}


void CImageListCtrl::OnPaint()
{
	CPaintDC paint_dc(this); // device context for painting

	MemoryDC dc(paint_dc, this, ::GetSysColor(COLOR_3DFACE));

	if (image_list_ != 0 && !images_.empty())
	{
		CRect rect;
		GetClientRect(rect);

		CPoint point= rect.TopLeft();

		IMAGEINFO ii;
		if (!image_list_->GetImageInfo(0, &ii))
			return;

		size_t count= images_.size();
		for (size_t i= 0; i < count; ++i)
		{
			image_list_->Draw(&dc, images_[i], point, ILD_TRANSPARENT);
			point.x += ii.rcImage.right - ii.rcImage.left + extra_space_;
		}
	}

	dc.BitBlt();
}


BOOL CImageListCtrl::OnEraseBkgnd(CDC* dc)
{
	return true;
}


void CImageListCtrl::SetImageList(CImageList* image_list)
{
	image_list_ = image_list;

	if (m_hWnd)
		Invalidate(false);
}


void CImageListCtrl::SelectImages(const std::vector<int>& images)
{
	try
	{
		images_ = images;

		if (m_hWnd)
			Invalidate(false);
	}
	catch (std::exception&)
	{}
}


void CImageListCtrl::SetImageSpace(int space)
{
	extra_space_ = space;

	if (m_hWnd)
		Invalidate(false);
}

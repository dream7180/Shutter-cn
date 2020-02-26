/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "resource.h"
#include "MonitorCtrl.h"
#include "MemoryDC.h"
#include "LoadImageList.h"
#include "BmpFunc.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


BEGIN_MESSAGE_MAP(MonitorCtrl, CWnd)
	ON_WM_ERASEBKGND()
//	ON_WM_LBUTTONDOWN()
	ON_WM_SIZE()
END_MESSAGE_MAP()


static const TCHAR* const MON_CTRL_CLASS= _T("MonitorCtrlMiK");

//LRESULT WINAPI MyDefWindowProc(
//    __in HWND wnd,
//    __in UINT Msg,
//    __in WPARAM wParam,
//    __in LPARAM lParam)
//{
//	TRACE(L"msg: %d\n", Msg);
//	return ::CallWindowProc(&::DefWindowProc, wnd, Msg, wParam, lParam);
//}


static void RegisterWndClass(const TCHAR* class_name)
{
	HINSTANCE instance= AfxGetInstanceHandle();

	// see if the class already exists
	WNDCLASS wndcls;
	if (!::GetClassInfo(instance, class_name, &wndcls))
	{
		// otherwise we need to register a new class
		wndcls.style = CS_VREDRAW | CS_HREDRAW;
		wndcls.lpfnWndProc = &::DefWindowProc;
		wndcls.cbClsExtra = wndcls.cbWndExtra = 0;
		wndcls.hInstance = instance;
		wndcls.hIcon = 0;
		wndcls.hCursor = ::LoadCursor(NULL, IDC_ARROW);
		wndcls.hbrBackground = 0;
		wndcls.lpszMenuName = NULL;
		wndcls.lpszClassName = class_name;

		AfxRegisterClass(&wndcls);
	}
}


MonitorCtrl::MonitorCtrl()
{
	RegisterWndClass(MON_CTRL_CLASS);

	aspect_ = CSize(16, 9);
	bezel_thickness_ = 10;
	monitor_rect_.SetRectEmpty();
	display_area_.SetRectEmpty();

	//if (star_img_.m_hImageList == 0)
	//{
	//	::LoadImageList(star_img_, IDB_STARS, 22, ::GetSysColor(COLOR_3DFACE));

	//	IMAGEINFO ii;
	//	if (star_img_.GetImageInfo(0, &ii))
	//	{
	//		star_size_.cx = ii.rcImage.right - ii.rcImage.left;
	//		star_size_.cy = ii.rcImage.bottom;
	//	}
	//	else
	//	{ ASSERT(false); }
	//}

	//stars_ = 5;
	//rating_ = 0;
}


//bool MonitorCtrl::Create(CWnd* parent, int count, int rating, const boost::function<void (int)>& on_clicked, int id)
//{
//	stars_ = count;
//	rating_ = rating;
//	on_clicked_ = on_clicked;
//
//	CRect rect(0,0,0,0);
//	rect.bottom = star_size_.cy;
//	rect.right = (star_size_.cx + HORZ_SPACE) * count - HORZ_SPACE;
//
//	if (!CWnd::Create(MON_CTRL_CLASS, 0, WS_CHILD | WS_VISIBLE, rect, parent, id))
//		return false;
//
//	return true;
//}


void MonitorCtrl::PreSubclassWindow()
{
	if (CFont* font= GetParent()->GetFont())
		SetFont(font);

	CalcSize();

	ModifyStyle(0, WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
	ModifyStyleEx(0, WS_EX_CONTROLPARENT);

	CWnd::PreSubclassWindow();
}


BOOL MonitorCtrl::OnEraseBkgnd(CDC* dc)
{
	CRect rect(0,0,0,0);
	GetClientRect(rect);

	COLORREF back= ::GetSysColor(COLOR_3DFACE);

	if (!monitor_rect_.IsRectEmpty())
	{
		MemoryDC dc(*dc, rect);

		dc.FillSolidRect(rect, back);

		dc.FillSolidRect(monitor_rect_, RGB(70,70,70));
		dc.FillSolidRect(display_area_, 0);

		COLORREF dark= RGB(96,96,96);
		COLORREF light= RGB(120,120,120);

		CRect r= display_area_;
		r.InflateRect(1, 1);
		dc.Draw3dRect(r, dark, light);
		r.InflateRect(1, 1);
		dc.Draw3dRect(r, dark, light);

		dc.BitBlt();
	}
	else
		dc->FillSolidRect(rect, back);

	return true;
}


//void MonitorCtrl::OnLButtonDown(UINT flags, CPoint point)
//{
//	if (star_size_.cx > 0)
//	{
//		int w= star_size_.cx + HORZ_SPACE;
//		int star= point.x / w + 1;
//		if (point.y <= star_size_.cy && star >= 0 && star <= stars_ && on_clicked_)
//			on_clicked_(star);
//	}
//}


//void MonitorCtrl::SetRating(int rating)
//{
//	rating_ = rating;
//	Invalidate();
//}


void MonitorCtrl::SetResizeCallback(const boost::function<void ()>& on_resized)
{
	on_resized_ = on_resized;
}


//int MonitorCtrl::GetStarCount() const
//{
//	return stars_;
//}
//
//
//int MonitorCtrl::GetRating() const
//{
//	return rating_;
//}


void MonitorCtrl::SetAspect(int w, int h)
{
	aspect_.cx = w;
	aspect_.cy = h;

	CalcSize();

	Invalidate();
}


void MonitorCtrl::OnSize(UINT type, int cx, int cy)
{
	CWnd::OnSize(type, cx, cy);
	CalcSize();
	Invalidate();
}


std::pair<CRect, CRect> CalcRects(CRect client, CSize aspect, int bezel_thickness)
{
	CRect rect= client;

	CRect monitor_rect(0,0,0,0), display_area(0,0,0,0);

	if (!rect.IsRectEmpty())
	{
		rect.DeflateRect(bezel_thickness, bezel_thickness);

		if (rect.Width() > 0 && rect.Height() > 0)
		{
			CSize size= SizeToFit(aspect, rect.Size(), 0.0);

			if (size.cx < rect.Width())
				display_area.left = rect.left + (rect.Width() - size.cx) / 2;
			else
				display_area.left = rect.left;

			if (size.cy < rect.Height())
				display_area.top = rect.top + (rect.Height() - size.cy) / 2;
			else
				display_area.top = rect.top;

			display_area.right = display_area.left + size.cx;
			display_area.bottom = display_area.top + size.cy;

			monitor_rect = display_area;
			monitor_rect.InflateRect(bezel_thickness, bezel_thickness);
		}
	}

	return std::make_pair(monitor_rect, display_area);
}


void MonitorCtrl::CalcSize()
{
	CRect client(0,0,0,0);
	GetClientRect(client);

	std::pair<CRect, CRect> rects= CalcRects(client, aspect_, bezel_thickness_);

	if (rects.first != monitor_rect_ || rects.second != display_area_)
	{
		monitor_rect_ = rects.first;
		display_area_ = rects.second;

		if (on_resized_)
			on_resized_();
	}
}


CRect MonitorCtrl::GetDisplayArea() const
{
	return display_area_;
}

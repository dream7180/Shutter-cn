/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// DragControl.cpp: implementation of the DragControl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
//#include "snapper.h"
#include "DragControl.h"
#include "../UIElements.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

DragControl::DragControl()
{
	wnd_ = 0;
	dc_ = 0;
}

void DragControl::StartDrag(CWnd* wnd)
{
	ASSERT_VALID(wnd);
	wnd_ = wnd;

	dragging_ = TRUE;

	InitLoop();

	CRect rect;
	wnd->GetWindowRect(rect);

	MoveTo(rect);
}


void DragControl::InitLoop()
{
	// handle pending WM_PAINT messages
	MSG msg;
	while (::PeekMessage(&msg, NULL, WM_PAINT, WM_PAINT, PM_NOREMOVE))
	{
		if (!GetMessage(&msg, NULL, WM_PAINT, WM_PAINT))
			return;
		DispatchMessage(&msg);
	}

	// initialize state
	last1_rect_.SetRectEmpty();
	last2_rect_.SetRectEmpty();
	last3_rect_.SetRectEmpty();
	last_size_.cx = last_size_.cy = 0;
	force_frame_ = flip_ = dither_last_ = FALSE;

	// lock window update while dragging
	ASSERT(dc_ == NULL);
	CWnd* wnd = CWnd::GetDesktopWindow();
	if (wnd->LockWindowUpdate())
		dc_ = wnd->GetDCEx(NULL, DCX_WINDOW | DCX_CACHE | DCX_LOCKWINDOWUPDATE);
	else
		dc_ = wnd->GetDCEx(NULL, DCX_WINDOW | DCX_CACHE);
	ASSERT(dc_ != NULL);
}


void DragControl::CancelLoop()
{
	DrawFocusRect(CRect(0,0,0,0), true);    // gets rid of focus rect
	ReleaseCapture();

	CWnd* wnd = CWnd::GetDesktopWindow();
	wnd->UnlockWindowUpdate();
	if (dc_ != NULL)
	{
		wnd->ReleaseDC(dc_);
		dc_ = NULL;
	}
}


const int CX_BORDER= 1;
const int CY_BORDER= 1;


void DragControl::DrawFocusRect(const CRect& rect, bool remove_rect/*= false*/)
{
	ASSERT(dc_ != NULL);

	// default to thin frame
	CSize size(2, 2);

	CBrush* white_brush = CBrush::FromHandle((HBRUSH)::GetStockObject(WHITE_BRUSH));
	CBrush* dither_brush = CDC::GetHalftoneBrush();
	CBrush* brush = dither_brush;// white_brush;

	if (remove_rect)
		size.cx = size.cy = 0;

	CRect in_rect= rect;
	in_rect.DeflateRect(2, 2);
	CRect top_rect= in_rect;
	top_rect.bottom = top_rect.top + Pixels(26);
	CRect bottom_rect= in_rect;
	bottom_rect.top = top_rect.bottom;

	// draw it
	if (dc_->GetSafeHdc() != 0)
	{
		CBrush* brush_last= dither_last_ ? dither_brush : white_brush;

		dc_->DrawDragRect(&top_rect,	 size, &last1_rect_, last_size_, brush, brush_last);
		dc_->DrawDragRect(&rect,		 size, &last2_rect_, last_size_, brush, brush_last);
		dc_->DrawDragRect(&bottom_rect, size, &last3_rect_, last_size_, brush, brush_last);
	}

	// remember last size
	last1_rect_ = top_rect;
	last2_rect_ = rect;
	last3_rect_ = bottom_rect;
	last_size_ = size;
	dither_last_ = (brush == dither_brush);
}


void DragControl::MoveTo(const CRect& new_pos_rect)
{
	DrawFocusRect(new_pos_rect);
}


bool DragControl::Track()
{
	// don't handle if capture already set
//	if (::GetCapture() != NULL)
//		return false;

	// set capture to the window which received this message
	wnd_->SetCapture();
	ASSERT(wnd_ == CWnd::GetCapture());

	// get messages until capture lost or cancelled/accepted
	while (CWnd::GetCapture() == wnd_)
	{
		MSG msg;
		if (!::GetMessage(&msg, NULL, 0, 0))
		{
			AfxPostQuitMessage(static_cast<int>(msg.wParam));
			break;
		}

		switch (msg.message)
		{
		case WM_LBUTTONUP:
//			if (dragging_)
//				EndDrag();
//			else
//				EndResize();
			return true;

		case WM_MOUSEMOVE:
			MouseMoved();
//			if (dragging_)
//				Move(msg.pt);
//			else
//				Stretch(msg.pt);
//			break;
		case WM_KEYUP:
//			if (dragging_)
//				OnKey((int)msg.wParam, FALSE);
			break;
		case WM_KEYDOWN:
//			if (dragging_)
//				OnKey((int)msg.wParam, TRUE);
			if (msg.wParam == VK_ESCAPE)
			{
//				CancelLoop();
				return false;
			}
			break;
		case WM_RBUTTONDOWN:
//			CancelLoop();
			return false;

		// just dispatch rest of the messages
		default:
			DispatchMessage(&msg);
			break;
		}
	}

//	CancelLoop();

	return false;
}


void DragControl::EndDrag()
{
	CancelLoop();
}


void DragControl::ScreenDraw(const CRect& rect, COLORREF rgb_fill)
{
	if (dc_ != 0)
	{
		int mode= dc_->GetBkMode();
		dc_->FillSolidRect(rect, rgb_fill);
		dc_->SetBkMode(mode);
	}
}

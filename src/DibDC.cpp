/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// DibDC.cpp: implementation of the off-screen bitmap drawing using dib

#include "stdafx.h"
#include "DibDC.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


DibDC::DibDC(CDC& dc, CWnd* wnd, COLORREF rgb_clr_back/*= -1*/)
{
	CRect rect;
	wnd->GetClientRect(rect);
	Init(dc, rect);
	if (rgb_clr_back != -1)
		FillSolidRect(rect, rgb_clr_back);
}

DibDC::DibDC(CDC& dc, const CRect& rect, COLORREF rgb_clr_back/*= -1*/)
{
	Init(dc, rect);
	if (rgb_clr_back != -1)
		FillSolidRect(rect, rgb_clr_back);
}


void DibDC::BitBlt()
{
	CPoint org(0,0);//= GetWindowOrg();
	SetViewportOrg(0,0);
	dc_->BitBlt(org.x + pos_.x, org.y + pos_.y, size_.cx, size_.cy, this, org.x, org.y, SRCCOPY);
}


void DibDC::BitBlt(int height)
{
	if (screen_bmp_.IsValid())
	{
		POINT org= GetWindowOrg();
		SetViewportOrg(0,0);
		dc_->BitBlt(org.x + pos_.x, org.y + pos_.y,
			size_.cx, std::min(static_cast<int>(size_.cy), height), this, org.x, org.y, SRCCOPY);
	}
}


DibDC::~DibDC()
{
	if (screen_bmp_.IsValid())
	{
		DeleteDC();		// delete in this order!
//		screen_bmp_..DeleteObject();
	}
	else
	{
		m_hDC = 0;
		m_hAttribDC = 0;
	}
}



void DibDC::Init(CDC& dc, const CRect& rect)
{
	size_ = rect.Size();
	pos_ = rect.TopLeft();
	dc_ = &dc;
	CPoint org(0,0);//= dc.GetWindowOrg();
	// try to allocate offscreen bitmap
	screen_bmp_.Create(size_.cx, size_.cy, 32);
	if (screen_bmp_.IsValid())
	{
		CreateCompatibleDC(&dc);
		SelectObject(screen_bmp_.GetBmp());
	}
	else
	{
		// no more memory/resources
		m_hDC = dc.m_hDC;
		m_hAttribDC = dc.m_hAttribDC;
	}
	SetWindowOrg(org);

	SetViewportOrg(-rect.left, -rect.top);
//	SetBrushOrg(rect.left % 8, rect.top % 8);
}


void DibDC::OffsetTo(const CRect& rect)
{
//	ASSERT(size_ == rect.Size());
	pos_ = rect.TopLeft();
	SetViewportOrg(-rect.left, -rect.top);
}


void DibDC::OffsetTo(const CPoint& point)
{
	pos_ = point;
	SetViewportOrg(-point.x, -point.y);
}


void DibDC::OffsetYTo(int y_pos)
{
	pos_.y = y_pos;
	SetViewportOrg(-pos_.x, -pos_.y);
}

/*
BOOL DibDC::RectVisible(LPCRECT rect) const
{
	if (rect == 0)
		return false;

	CRect rect= *rect;
	rect.OffsetRect(pos_);

	return CDC::RectVisible(rect);
}
*/

Dib& DibDC::GetDib()
{
	return screen_bmp_;
}

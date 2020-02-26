/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// MemoryDC.cpp: implementation of the MemoryDC class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MemoryDC.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

MemoryDC::MemoryDC(CDC& dc, CWnd* wnd, COLORREF rgb_clr_back/*= -1*/) : old_bmp_(0)
{
	CRect rect;
	wnd->GetClientRect(rect);
	Init(dc, rect);
	if (rgb_clr_back != -1)
		FillSolidRect(rect, rgb_clr_back);
}

MemoryDC::MemoryDC(CDC& dc, const CRect& rect, COLORREF rgb_clr_back/*= -1*/) : old_bmp_(0)
{
	Init(dc, rect);
	if (rgb_clr_back != -1)
		FillSolidRect(rect, rgb_clr_back);
}


void MemoryDC::BitBlt()
{
	POINT org= GetWindowOrg();
	SetViewportOrg(0,0);
	dc_->BitBlt(org.x + pos_.x, org.y + pos_.y, size_.cx, size_.cy, this, org.x, org.y, SRCCOPY);
}


void MemoryDC::BitBlt(int height)
{
	if (screen_bmp_.m_hObject)
	{
		POINT org= GetWindowOrg();
		SetViewportOrg(0,0);
		dc_->BitBlt(org.x + pos_.x, org.y + pos_.y,
			size_.cx, std::min(static_cast<int>(size_.cy), height), this, org.x, org.y, SRCCOPY);
	}
}


MemoryDC::~MemoryDC()
{
	//if (old_bmp_)
	//	SelectObject(old_bmp_);

	if (screen_bmp_.m_hObject)
	{
		DeleteDC();		// delete in this order!
		screen_bmp_.DeleteObject();
	}
	else
	{
		m_hDC = 0;
		m_hAttribDC = 0;
	}
}



void MemoryDC::Init(CDC& dc, const CRect& rect)
{
	size_ = rect.Size();
	pos_ = rect.TopLeft();
	dc_ = &dc;
	POINT org= dc.GetWindowOrg();
	// try to allocate offscreen bitmap
	screen_bmp_.CreateCompatibleBitmap(&dc, size_.cx, size_.cy);
	if (screen_bmp_.m_hObject)
	{
		CreateCompatibleDC(&dc);
		old_bmp_ = SelectObject(&screen_bmp_);
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


void MemoryDC::OffsetTo(const CRect& rect)
{
//	ASSERT(size_ == rect.Size());
	pos_ = rect.TopLeft();
	SetViewportOrg(-rect.left, -rect.top);
}


void MemoryDC::OffsetTo(const CPoint& point)
{
	pos_ = point;
	SetViewportOrg(-point.x, -point.y);
}


void MemoryDC::OffsetYTo(int y_pos)
{
	pos_.y = y_pos;
	SetViewportOrg(-pos_.x, -pos_.y);
}

/*
BOOL MemoryDC::RectVisible(LPCRECT rect) const
{
	if (rect == 0)
		return false;

	CRect rect= *rect;
	rect.OffsetRect(pos_);

	return CDC::RectVisible(rect);
}
*/

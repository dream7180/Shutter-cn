/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// ViewerExampleWnd.cpp : implementation file
//

#include "stdafx.h"
//#include "resource.h"
#include "ViewerExampleWnd.h"
#include "DrawFunctions.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// ViewerExampleWnd

ViewerExampleWnd::ViewerExampleWnd(VectPhotoInfo& photos) : photos_(photos)
// : list_bar_wnd_(photos)
{
	rgb_tag_text_ = rgb_tag_bkgnd_ = 0;
}

ViewerExampleWnd::~ViewerExampleWnd()
{
}


BEGIN_MESSAGE_MAP(ViewerExampleWnd, CWnd)
	ON_WM_CREATE()
	ON_WM_SIZE()
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// ViewerExampleWnd message handlers


bool ViewerExampleWnd::Create(CWnd* parent, const RECT& rect)
{
	if (!CWnd::Create(AfxRegisterWndClass(0, ::LoadCursor(NULL, IDC_ARROW)),
		0, WS_CHILD | WS_VISIBLE, rect, parent, -1))
		return false;

	//display_wnd_.LoadPhoto(photo, 0, false);

	return true;
}


static std::wstring example_str= L"Sample description text (scaled down)";

int ViewerExampleWnd::OnCreate(LPCREATESTRUCT create_struct)
{
	if (CWnd::OnCreate(create_struct) == -1)
		return -1;

	//VERIFY(list_bar_wnd_.Create(this, false));
	//list_bar_wnd_.EnableBalloons(false);

	//VERIFY(preview_bar_wnd_.Create(this, &list_bar_wnd_));

	preview_.Create(this);
	preview_.EnableToolTips(false);
	preview_.SetItemDrawCallback(boost::bind(&ViewerExampleWnd::DrawItem, this, _1, _2, _3, _4));
	preview_.SetOrientation(true);

	for (size_t i= 0; i < photos_.size(); ++i)
	{
		PhotoInfoPtr p= photos_[i];
		CSize size(p->bmp_->GetWidth(), p->bmp_->GetHeight());
		preview_.AddItem(size, p);
	}

	preview_.Invalidate();
	preview_.ResetScrollBar(false);

	preview_.SelectionVisible(2, false);

	VERIFY(display_wnd_.Create(this));
	display_wnd_.CursorStayVisible(true);
	display_wnd_.SetDescriptionText(&example_str);

	//VERIFY(separator_wnd_.Create(this, 0));

	//preview_bar_wnd_.Resize(60);
	Resize();

	return 0;
}


void ViewerExampleWnd::DrawItem(CDC& dc, CRect rect, size_t item, AnyPointer key)
{
	if (item >= photos_.size())
		return;

	PhotoInfoPtr photo= photos_[item];

	::DrawPhoto(dc, rect, photo);

	//if (!photo->exif_data_present_)		  // missing EXIF indicator
	//	::DrawNoExifIndicator(dc, rect);

	//::DrawPhotoTags(dc, rect, photo->GetTags(), photo->GetRating(), rgb_tag_text_, rgb_tag_bkgnd_);
}


void ViewerExampleWnd::Resize()
{
	CRect cl_rect;
	GetClientRect(cl_rect);
	CSize wnd_size= cl_rect.Size();

	display_wnd_.Invalidate();

	int y_pos= 0;

	CRect rect;
	int h= 70;
	preview_.SetWindowPos(0, 0, y_pos, wnd_size.cx, h, SWP_NOZORDER | SWP_NOACTIVATE);
	y_pos += h;

	//separator_wnd_.SetWindowPos(0, 0, y_pos, wnd_size.cx, separator_wnd_.GetHeight(), SWP_NOZORDER | SWP_NOACTIVATE);
	//separator_wnd_.Invalidate();
	//y_pos += separator_wnd_.GetHeight();

	display_wnd_.SetWindowPos(0, 0, y_pos, wnd_size.cx, MAX(wnd_size.cy - y_pos, 0), SWP_NOZORDER | SWP_NOACTIVATE);
}


void ViewerExampleWnd::ResetColors()
{
	display_wnd_.SetBackgndColor(RGB(0,0,0));
	display_wnd_.SetTextColor(RGB(255,138,22));
	display_wnd_.SetDescriptionText(&example_str);
	display_wnd_.Invalidate();

	rgb_tag_text_ = RGB(255,255,255);
	rgb_tag_bkgnd_ = RGB(247, 123, 0);
	preview_.SetSelectionColor(RGB(247, 123, 0));//(::GetSysColor(COLOR_HIGHLIGHT));
	//list_bar_wnd_.ResetColors();
	//list_bar_wnd_.Invalidate();
}


void ViewerExampleWnd::SetColors(const std::vector<COLORREF>& colors)
{
	display_wnd_.SetBackgndColor(colors[0]);
	display_wnd_.SetTextColor(colors[1]);
	display_wnd_.SetDescriptionText(&example_str);
	display_wnd_.Invalidate();

	rgb_tag_text_ = colors[5];
	rgb_tag_bkgnd_ = colors[4];
	preview_.SetSelectionColor(colors[3]);
	//list_bar_wnd_.rgb_back_ = colors[2];
	//list_bar_wnd_.rgb_cur_selection_ = colors[3];
	//list_bar_wnd_.rgb_tag_bkgnd_ = colors[4];
	//list_bar_wnd_.rgb_tag_text_ = colors[5];
	//list_bar_wnd_.Invalidate();
}


void ViewerExampleWnd::SetUIBrightness(double gamma)
{
	preview_.SetUIBrightness(gamma);
}


void ViewerExampleWnd::SetDescriptionFont(const LOGFONT& lf)
{
	display_wnd_.SetDescriptionFont(lf);
	display_wnd_.SetDescriptionText(&example_str);
	display_wnd_.Invalidate();
}


void ViewerExampleWnd::OnSize(UINT type, int cx, int cy)
{
	CWnd::OnSize(type, cx, cy);

	if (type != SIZE_MINIMIZED)
		Resize();
}

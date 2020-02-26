/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "resource.h"
#include "TaggingPopup.h"
#include "WhistlerLook.h"
#include "MultiMonitor.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(TaggingPopup, CMiniFrameWnd)
//	ON_WM_ERASEBKGND()
	ON_WM_ACTIVATE()
	ON_MESSAGE(WM_USER, OnFinish)
	ON_COMMAND(SC_CLOSE, OnClose)
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

TaggingPopup* TaggingPopup::running_instance_= 0;

bool TaggingPopup::IsPopupActive()
{
	return running_instance_ != 0;
}

void TaggingPopup::Refresh(const VectPhotoInfo& photos)
{
	if (running_instance_)
		running_instance_->RefreshTags(photos);
}

void TaggingPopup::RefreshTags(const VectPhotoInfo& photos)
{
	tag_wnd_.PhotosSelected(photos);
}


TaggingPopup::TaggingPopup(PhotoTagsCollection& tagCollection, const VectPhotoInfo& photos,
		const TagsBarCommon::ApplyTagsFn& apply, const TagsBarCommon::ApplyRatingFn& rate,
		CPoint left_top, CWnd* parent)
 : tag_wnd_(tagCollection, L"TagPopup"), left_top_(left_top)
{
	tag_wnd_.SetTagCallback(apply);
	tag_wnd_.SetRatingCallback(rate);

	ASSERT(running_instance_ == 0);
	run_ = true;
	running_instance_ = this;
	Create(parent, photos);
}

TaggingPopup::~TaggingPopup()
{
	running_instance_ = 0;
}


bool TaggingPopup::Create(CWnd* parent, const VectPhotoInfo& photos)
{
	UINT class_style= CS_SAVEBITS | (WhistlerLook::IsAvailable() ? CS_DROPSHADOW : 0);

	if (!CreateEx(WS_EX_WINDOWEDGE | WS_EX_TOOLWINDOW | WS_EX_TOPMOST,
		AfxRegisterWndClass(class_style, ::LoadCursor(NULL, IDC_ARROW)), 0,
		WS_POPUP | WS_VISIBLE | WS_CLIPCHILDREN | MFS_SYNCACTIVE, CRect(0,0,1,1), parent))
		return false;

	if (!tag_wnd_.Create(this))
		return false;

	close_wnd_.Create(this, false);
	CRect bar_rect;
	close_wnd_.GetWindowRect(bar_rect);

	tag_wnd_.PhotosSelected(photos);

	CSize ideal= tag_wnd_.GetIdealSize();
	ideal.cx = std::min<int>(400, ideal.cx);	// sanity limits
	ideal.cy = std::min<int>(750, ideal.cy);

	int top= bar_rect.Height() - 4;

	tag_wnd_.MoveWindow(0, top, ideal.cx, ideal.cy);

	int width= ideal.cx;
	int height= top + ideal.cy;

	CRect wnd(left_top_, CSize(width, height));
	::RectToWorkArea(wnd, false);

	SetWindowPos(0, wnd.left, wnd.top, wnd.Width(), wnd.Height(), SWP_NOZORDER);

	close_wnd_.SetWindowPos(0, width - bar_rect.Width(), 0, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

	ShowWindow(SW_SHOW);

	RunModalLoop();

	DestroyWindow();

	delete this;

	return true;
}


BOOL TaggingPopup::ContinueModal()
{
	return run_;
}


void TaggingPopup::OnClose()
{
	run_ = false;
}


void TaggingPopup::OnKillFocus(CWnd* new_wnd)
{
	run_ = false;
}


LRESULT TaggingPopup::OnFinish(WPARAM, LPARAM)
{
	run_ = false;
	return 0;
}


void TaggingPopup::OnActivate(UINT state, CWnd* wnd_other, BOOL minimized)
{
	CMiniFrameWnd::OnActivate(state, wnd_other, minimized);
	if (state == WA_INACTIVE)
		run_ = false;
}


void TaggingPopup::PostNcDestroy()
{}


BOOL TaggingPopup::OnEraseBkgnd(CDC* dc)
{
	CRect rect(0,0,0,0);
	GetClientRect(rect);

	dc->FillSolidRect(rect, tag_wnd_.GetBackgndColor());

	return true;
}

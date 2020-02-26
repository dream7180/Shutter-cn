/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// TagBarView.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "TagBarView.h"
#include "Color.h"
#include "MemoryDC.h"
#include "TagsCommonCode.h"
#include "Tasks.h"
#include "AppColors.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// TagBarView

TagBarView::TagBarView()
{
}

TagBarView::~TagBarView()
{
}


BEGIN_MESSAGE_MAP(TagBarView, PaneWnd)
	//{{AFX_MSG_MAP(TagBarView)
//	ON_WM_ERASEBKGND()
//	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_SETCURSOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// TagBarView message handlers

// draw task bar background
//
BOOL TagBarView::OnEraseBkgnd(CDC* dc)
{
	return true;
}


void TagBarView::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// Do not call CWnd::OnPaint() for painting messages
}


bool TagBarView::Create(CWnd* parent)
{
	const TCHAR* className= AfxRegisterWndClass(CS_VREDRAW | CS_HREDRAW, ::LoadCursor(NULL, IDC_ARROW));

	if (!CWnd::Create(className, _T(""), WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN, CRect(0,0,0,0), parent, -1))
		return false;

//	if (!rating_wnd_.Create(MAKEINTRESOURCE(IDD_RATING_FORM), this))
//		return false;
//	rating_wnd_.ShowWindow(SW_SHOWNA);

//	ASSERT(dynamic_cast<SnapView*>(parent));
	//BrowserFrame* frame= dynamic_cast<BrowserFrame*>(static_cast<SnapView*>(parent)->GetFrame()->GetParent());
	//if (frame == 0)
	//{
	//	ASSERT(false);
	//	return false;
	//}

	tags_ctrl_.reset(new TagsBarCommon(Tags::GetTagCollection(), L"TagBarPane"));
	tags_ctrl_->tool_bar_at_top_ = false;
	tags_ctrl_->Create(this, -1);//, IsCaptionBig());
	tags_ctrl_->SetTagCallback(&::ApplyTagToPhotos);
	tags_ctrl_->SetRatingCallback(&::ApplyRatingToPhotos);

	// margin
	tags_ctrl_->left_indent_ = 1;
	tags_ctrl_->top_indent_ = 3;

	SetColors();

	AddBand(&tags_ctrl_->GetToolBar(), tags_ctrl_.get());

	return true;
}


void TagBarView::Resize()
{
	if (tags_ctrl_.get() != 0 && tags_ctrl_->m_hWnd)
	{
		CRect rect(0,0,0,0);
		GetClientRect(rect);
		tags_ctrl_->MoveWindow(rect);

		//CRect w(0,0,0,0);
		//rating_wnd_.GetClientRect(w);
		//rating_wnd_.MoveWindow(rect.left, rect.bottom - w.Height(), rect.Width(), w.Height());

		//tags_ctrl_->MoveWindow(rect.left, rect.top, rect.Width(), std::max(0, rect.Height() - w.Height()));
	}
}


void TagBarView::OnSize(UINT type, int cx, int cy)
{
	CWnd::OnSize(type, cx, cy);
	Resize();
}


void TagBarView::SelectionHasChanged(VectPhotoInfo& selected)
{
	if (tags_ctrl_.get())
		tags_ctrl_->PhotosSelected(selected);
}


void TagBarView::AssignTag(int index)
{
	if (IsPaneVisible() && tags_ctrl_.get() != 0)
		tags_ctrl_->AssignTag(index);
}

/*
void TagBarView::CaptionHeightChanged(bool big)
{
	tags_ctrl_->GetToolBar().ReplaceImageList(big ? IDB_TAGS_TOOLBAR_BIG : IDB_TAGS_TOOLBAR);
	ResetBandsWidth();
}
*/

BOOL TagBarView::OnSetCursor(CWnd* wnd, UINT hit_test, UINT message)
{
	SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));
	return true;
}


void TagBarView::OptionsChanged(OptionsDlg& dlg)
{
	SetColors();
}


void TagBarView::SetColors()
{
	tags_ctrl_->SetColors(GetAppColors());
}


void TagBarView::ActivatePane(bool active)
{
	if (active)
		if (IsTopParentActive())	// take the focus if this frame/view/pane is now active
			tags_ctrl_->SetFocus();
}


bool TagBarView::HasFocus() const
{
	return tags_ctrl_->GetFilterEditBox().m_hWnd == ::GetFocus();
}

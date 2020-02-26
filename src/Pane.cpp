/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// Pane.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "Pane.h"
//#include "PaneNotification.h"
#include "SnapFrame/SnapFrame.h"
#include "SnapFrame/SnapMsg.h"
#include "SnapFrame/CaptionWindow.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// PaneWnd

PaneWnd::PaneWnd()
{
	visible_ = false;
	current_photo_ = 0;
}

PaneWnd::~PaneWnd()
{}


BEGIN_MESSAGE_MAP(PaneWnd, CWnd)
	//{{AFX_MSG_MAP(PaneWnd)
//	ON_WM_CREATE()
	//}}AFX_MSG_MAP
	ON_MESSAGE(SNAP_WND_MSG_SHOW, OnShowPane)
	ON_MESSAGE(SNAP_WND_MSG_ACTIVATE, OnActivatePane)
	//ON_MESSAGE(SNAP_WND_MSG_CAPTION_HEIGHT_CHANGED, OnCaptionHeightChanged)
	ON_WM_KEYDOWN()
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// PaneWnd message handlers


LRESULT PaneWnd::OnShowPane(WPARAM visible, LPARAM notification)
{
	visible_ = visible != 0;

	if (visible_)
		CurrentChanged(current_photo_);
	else
		PaneHidden();

	return 0;
}


LRESULT PaneWnd::OnActivatePane(WPARAM active, LPARAM)
{
	if (visible_ && active)
		ActivatePane(true);
	else
		ActivatePane(!!active);

	return 0;
}


void PaneWnd::CurrentPhotoChanged(PhotoInfoPtr photo)
{
	current_photo_ = photo;

	if (visible_)
		CurrentChanged(photo);
	else if (current_photo_ == 0)	// hacky: if no photo, then refresh even hidden panes;
		CurrentChanged(0);			// that is to make sure they do not keep hanging pointers to photos
}


void PaneWnd::CurrentPhotoModified(PhotoInfoPtr photo)
{
	if (visible_)
		CurrentModified(photo);
}


void PaneWnd::PhotoSelectionChanged(VectPhotoInfo& selected)
{
	if (visible_)
		SelectionHasChanged(selected);
}


void PaneWnd::PhotoDescriptionChanged(std::wstring& descr)
{
	if (visible_)
		DescriptionChanged(descr);
}


void PaneWnd::SaveSettings()
{}


void PaneWnd::OpenCurrentPhoto()
{}


// main window activated
void PaneWnd::MainWndActivated()
{
	if (visible_)
		ActiveMainWnd();
}

//=============================================================================


void PaneWnd::CurrentChanged(PhotoInfoPtr photo)
{}


void PaneWnd::SelectionHasChanged(VectPhotoInfo& selected)
{}


void PaneWnd::InitialUpdate()
{}


void PaneWnd::OptionsChanged(OptionsDlg&)
{}


void PaneWnd::PaneHidden()
{}


void PaneWnd::CurrentModified(PhotoInfoPtr photo)
{}


void PaneWnd::DescriptionChanged(std::wstring& descr)
{}


void PaneWnd::ActivatePane(bool active)
{
	if (active)
		if (IsTopParentActive())	// take the focus if this frame/view/pane is now active
		{
			TRACE("focus\n");
			SetFocus();
		}
}


void PaneWnd::SyncVisibilityFlag()
{
	if (SnapView* parent= dynamic_cast<SnapView*>(GetParent()))
	{
		visible_ = parent->IsPaneOpen();

		if (visible_)
			OnShowPane(visible_, 0);
	}
}


SnapFrame* PaneWnd::GetSnapFrame() const
{
	if (SnapView* parent= dynamic_cast<SnapView*>(GetParent()))
		return parent->GetFrame();
	return 0;
}


void PaneWnd::AddBand(CWnd* toolbar, CWnd* owner, std::pair<int, int> min_max_width/*=(0,0)*/, bool resizable/*= false*/)
{
	if (SnapView* parent= dynamic_cast<SnapView*>(GetParent()))
		parent->AddBand(toolbar, owner, min_max_width, resizable);
}


void PaneWnd::ResetBandsWidth(std::pair<int, int> min_max_width)
{
	if (SnapView* parent= dynamic_cast<SnapView*>(GetParent()))
		parent->ResetBandsWidth(min_max_width);
}


void PaneWnd::ResetBandsWidth()
{
	if (SnapView* parent= dynamic_cast<SnapView*>(GetParent()))
		parent->ResetBandsWidth();
}


void PaneWnd::ActiveMainWnd()
{}


void PaneWnd::UpdateSortOrderPopup(CMenu& popup)
{}

void PaneWnd::ChangeSortOrder(UINT cmd_id)
{}

/*
LRESULT PaneWnd::OnCaptionHeightChanged(WPARAM big, LPARAM)
{
	CaptionHeightChanged(!!big);
	return 0;
}


void PaneWnd::CaptionHeightChanged(bool big)
{}


bool PaneWnd::IsCaptionBig() const
{
	return CaptionWindow::IsCaptionBig();
}
*/

void PaneWnd::AssignTag(int index)
{}


void PaneWnd::OnKeyDown(UINT chr, UINT rep_cnt, UINT flags)
{
	if (chr == VK_ESCAPE && ::GetKeyState(VK_SHIFT) >= 0 && ::GetKeyState(VK_CONTROL) >= 0 && ::GetKeyState(VK_MENU) >= 0)
	{
		if (CWnd* wnd= GetSnapFrame())
			wnd->SendMessage(WM_COMMAND, ID_ESCAPE);
		return;
	}

	CWnd::OnKeyDown(chr, rep_cnt, flags);
}


void PaneWnd::SetFaintCaptionEdge(bool faint)
{
	if (SnapView* parent= dynamic_cast<SnapView*>(GetParent()))
		parent->SetFaintCaptionEdge(faint);
}


void PaneWnd::UpdatePane()
{
	UpdateWindow();
}

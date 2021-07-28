/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// PreviewPane.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "PreviewPane.h"
#include "Config.h"
#include "MagnifierWnd.h"
#include "Transform.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern AutoPtr<PhotoCache> global_photo_cache;		// one central photo cache

namespace {
	const TCHAR* REGISTRY_ENTRY_PREVIEW= _T("PreviewPane");
	const TCHAR* REG_SHOW_DESCRIPTION= _T("ShowDescription");
}

/////////////////////////////////////////////////////////////////////////////
// PreviewPane

PreviewPane::PreviewPane()
{
	timer_id_ = 0;
	show_magnifier_lens_ = false;
	metadata_changed_ =	PhotoCollection::Instance().ConnectOnMetadataChanged(boost::bind(&PreviewPane::RefreshAfterChange, this, _1));
	rotation_feasible_ = false;
}

PreviewPane::~PreviewPane()
{
}


BEGIN_MESSAGE_MAP(PreviewPane, PaneWnd)
	//{{AFX_MSG_MAP(PreviewPane)
	ON_WM_SIZE()
	ON_COMMAND(ID_ZOOM_100, OnZoom100)
	ON_UPDATE_COMMAND_UI(ID_ZOOM_100, OnUpdateZoom100)
	ON_COMMAND(ID_ZOOM_FIT, OnZoomFit)
	ON_UPDATE_COMMAND_UI(ID_ZOOM_FIT, OnUpdateZoomFit)
	ON_COMMAND(ID_ZOOM_IN, OnZoomIn)
	ON_UPDATE_COMMAND_UI(ID_ZOOM_IN, OnUpdateZoomIn)
	ON_COMMAND(ID_ZOOM_OUT, OnZoomOut)
	ON_UPDATE_COMMAND_UI(ID_ZOOM_OUT, OnUpdateZoomOut)
	ON_COMMAND(ID_PREVIEW_OPTIONS, OnPreviewOptions)
	ON_UPDATE_COMMAND_UI(ID_PREVIEW_OPTIONS, OnUpdatePreviewOptions)
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_COMMAND(ID_MAGNIFIER_LENS, OnMagnifierLens)
	ON_UPDATE_COMMAND_UI(ID_MAGNIFIER_LENS, OnUpdateMagnifierLens)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_USER, OnDblClick)
	ON_WM_CONTEXTMENU()
	ON_WM_INITMENUPOPUP()
	ON_COMMAND(ID_TOGGLE_DESCRIPTION, OnToggleDispDescription)
	ON_UPDATE_COMMAND_UI(ID_TOGGLE_DESCRIPTION, OnUpdateToggleDispDescription)
	ON_COMMAND(ID_SET_WALLPAPER, OnSetAsWallpaper)
	ON_UPDATE_COMMAND_UI(ID_SET_WALLPAPER, OnUpdateSetAsWallpaper)
	ON_COMMAND(ID_IMG_ROTATE_90_CCW, OnJpegRotate90CCW)
	ON_UPDATE_COMMAND_UI(ID_IMG_ROTATE_90_CCW, OnUpdateJpegRotate)
	ON_COMMAND(ID_IMG_ROTATE_90_CW, OnJpegRotate90CW)
	ON_UPDATE_COMMAND_UI(ID_IMG_ROTATE_90_CW, OnUpdateJpegRotate)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// PreviewPane message handlers


bool PreviewPane::Create(CWnd* parent)
{
	CString wnd_class= AfxRegisterWndClass(CS_VREDRAW | CS_HREDRAW, ::LoadCursor(NULL, IDC_ARROW));

	if (!PaneWnd::CreateEx(0, wnd_class, _T(""), WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN,
		CRect(0,0,0,0), parent, -1))
		return false;

	VERIFY(display_.Create(this));
	VERIFY(bar_wnd_.Create(this, -1));//, IsCaptionBig()));

	display_.CursorStayVisible(true);
	display_.SetHost(this);
	display_.SetPhotoCache(global_photo_cache.get());

	bool desc= AfxGetApp()->GetProfileInt(REGISTRY_ENTRY_PREVIEW, REG_SHOW_DESCRIPTION, true) != 0;
	display_.EnablePhotoDesc(desc);

	transform_.Reset(g_Settings.default_photo_->profile_, g_Settings.main_wnd_->profile_, g_Settings.main_wnd_->rendering_);

 	display_.SetGamma(transform_);
	display_.EnableGamma(g_Settings.main_wnd_->enabled_);

//	yellow_text_.ResetFont();

//	transparent_bar_wnd_.Create(this, -1);

	SetColors();

	AddBand(&bar_wnd_, this, bar_wnd_.GetMinMaxWidth());

	return true;

}


void PreviewPane::OnSize(UINT type, int cx, int cy)
{
	PaneWnd::OnSize(type, cx, cy);

	Resize();
}


void PreviewPane::Resize()
{
	if (bar_wnd_.m_hWnd && display_.m_hWnd)
	{
		CRect cl_rect;
		GetClientRect(cl_rect);
		CSize wnd_size= cl_rect.Size();

//		display_.Invalidate();
/*
		int y_pos= 0;
		int toolbar_h= tool_rebar_wnd_.GetHeight();
		tool_rebar_wnd_.SetWindowPos(0, 0, y_pos, wnd_size.cx, toolbar_h, SWP_NOZORDER | SWP_NOACTIVATE);
		toolbar_h = tool_rebar_wnd_.GetHeight();
		y_pos += toolbar_h;
		if (preview_bar_wnd_.GetStyle() & WS_VISIBLE)
		{
			CRect rect;
			preview_bar_wnd_.GetWindowRect(rect);
			preview_bar_wnd_.SetWindowPos(0, 0, y_pos, wnd_size.cx, rect.Height(), SWP_NOZORDER | SWP_NOACTIVATE);
			y_pos += rect.Height() + 0;
		}
		display_.SetWindowPos(0, 0, y_pos, wnd_size.cx, max(wnd_size.cy - y_pos, 0), SWP_NOZORDER | SWP_NOACTIVATE);
*/
		//int bar_width= 26;
		//bar_wnd_.SetWindowPos(0, 0, 0, bar_width, wnd_size.cy, SWP_NOZORDER | SWP_NOACTIVATE);
		//display_.SetWindowPos(0, bar_width, 0, wnd_size.cx - bar_width, wnd_size.cy, SWP_NOZORDER | SWP_NOACTIVATE);
		display_.MoveWindow(cl_rect);

		// resizing may change photo's zoom, so adjust slider's position
		SetSlider();
	}
}


void PreviewPane::OnZoom100()
{
	MagnifierWnd::Close();
	display_.SetLogicalZoom(1.0, true);
	SetSlider();
}

void PreviewPane::OnUpdateZoom100(CCmdUI* cmd_ui)
{
	cmd_ui->Enable();
	cmd_ui->SetCheck(!display_.IsZoomToFit() && display_.GetLogicalZoom() == 1.0 ? 1 : 0);
}

void PreviewPane::OnZoomFit()
{
	MagnifierWnd::Close();
	display_.SetLogicalZoom(0.0, g_Settings.allow_magnifying_above100_);
	SetSlider();
}

void PreviewPane::OnUpdateZoomFit(CCmdUI* cmd_ui)
{
	cmd_ui->Enable();
	cmd_ui->SetCheck(display_.IsZoomToFit() ? 1 : 0);
}


static uint16 zoom_[]=	// zoom in percents; progression using 2^1/3 factor
{ 5, 6, 8, 10, 13, 16, 20, 25, 31, 40, 50, 63, 79, 100, 126, 159, 200, 252, 317, 400, 504, 635, 800 };
//{ 5, 10, 25, 50, 75, 100, 150, 200, 300, 400, 600 };
static const int SIZE_= array_count(zoom_);


void PreviewPane::SetSlider()
{
	double zoom= display_.GetLogicalZoom();
	if (zoom == 0.0)
	{
		// zoom factor not yet ready
		if (timer_id_ == 0)
			timer_id_ = SetTimer(2, 100, NULL);
	}
	else
		bar_wnd_.SetSlider(zoom_, SIZE_, static_cast<int>(zoom * 100.0));
}


void PreviewPane::OnZoomIn()
{
	MagnifierWnd::Close();
	display_.ZoomIn(zoom_, SIZE_);
	SetSlider();
}

void PreviewPane::OnUpdateZoomIn(CCmdUI* cmd_ui)
{
	cmd_ui->Enable(display_.GetLogicalZoom() < zoom_[SIZE_ - 1] / 100.0);
}


void PreviewPane::OnZoomOut()
{
	MagnifierWnd::Close();
	display_.ZoomOut(zoom_, SIZE_);
	SetSlider();
}

void PreviewPane::OnUpdateZoomOut(CCmdUI* cmd_ui)
{
	cmd_ui->Enable(display_.GetLogicalZoom() > zoom_[0] / 100.0);
}


void PreviewPane::OnPreviewOptions()
{
}

void PreviewPane::OnUpdatePreviewOptions(CCmdUI* cmd_ui)
{
	cmd_ui->Enable();
}


void PreviewPane::SliderChanged(int pos)		// notification from zoom slider
{
	int index= pos / 2;
	if (pos < 0 || pos >= SIZE_ * 2)
	{
		ASSERT(false);
		return;
	}

	int zoom= pos & 1 ? (zoom_[index] + zoom_[index + 1]) / 2 : zoom_[index];

	display_.SetLogicalZoom(zoom / 100.0, true);
//	SetSlider();
}


void PreviewPane::SetTextColor(COLORREF rgb_text)
{
	display_.SetTextColor(rgb_text);
	if (display_.m_hWnd)
		display_.Invalidate();
}


void PreviewPane::SetBkColor(COLORREF rgb_back)
{
	display_.SetBackgndColor(rgb_back);
	if (display_.m_hWnd)
		display_.Invalidate();
}


LRESULT PreviewPane::OnDblClick(WPARAM, LPARAM)
{
	SendNotification(&PaneWnd::OpenCurrentPhoto);

	return 0;
}


void PreviewPane::ResetGamma()
{
	transform_.Reset(g_Settings.default_photo_->profile_, g_Settings.main_wnd_->profile_, g_Settings.main_wnd_->rendering_);
	display_.SetGamma(transform_);
	display_.EnableGamma(g_Settings.main_wnd_->enabled_ && g_Settings.default_photo_->enabled_);
}


void PreviewPane::LoadPhoto(const PhotoInfo& inf, bool force_reload/*= false*/)
{
	show_magnifier_lens_ = false;
	rotation_feasible_ = false;

	UINT flags= 0;
	if (!inf.OrientationAltered())
		flags |= ViewPane::AUTO_ROTATE;
	if (force_reload)
		flags |= ViewPane::FORCE_RELOADING;

	display_.LoadPhoto(inf, 0, flags);
	rotation_feasible_ = inf.IsRotationFeasible() && inf.IsFileEditable();	// cache this info, it may be expensive

	SetSlider();
}


void PreviewPane::OnDestroy()
{
	if (timer_id_)
		KillTimer(timer_id_);

	bool enabled= display_.IsPhotoDescDisplayed();
	AfxGetApp()->WriteProfileInt(REGISTRY_ENTRY_PREVIEW, REG_SHOW_DESCRIPTION, enabled ? 1 : 0);

	PaneWnd::OnDestroy();
}


void PreviewPane::OnTimer(UINT_PTR id_event)
{
	CWnd::OnTimer(id_event);

	if (id_event == timer_id_)
	{
		KillTimer(timer_id_);
		timer_id_ = 0;
		SetSlider();
	}
}


void PreviewPane::ResetColors()
{
	// do not touch background
	display_.SetTextColor(RGB(255, 138, 22));
	display_.SetDescriptionText(0);
	display_.Invalidate();
}


void PreviewPane::SetColors(const std::vector<COLORREF>& colors)
{
	// do not touch background
	display_.SetTextColor(colors[1]);
	display_.SetDescriptionText(0);
	display_.Invalidate();
}


void PreviewPane::OptionsChanged(OptionsDlg& dlg)
{
	SetColors();
	ResetDescriptionFont();
	ResetGamma();
	Invalidate();
}


void PreviewPane::SetColors()
{
	ResetColors();
	std::vector<COLORREF> colors= g_Settings.main_wnd_colors_.Colors();
	SetBkColor(RGB(150, 150, 150));//SetBkColor(colors[0]);	// same backgnd as main wnd
	COLORREF descr_text= g_Settings.viewer_wnd_colors_[1].SelectedColor();
	SetTextColor(descr_text);	// description text color

	bar_wnd_.SetBackgroundColor(g_Settings.pane_caption_colors_[SnapFrame::C_BAR].SelectedColor());
/*
	if (g_Settings.list_ctrl_sys_colors_)
	{
		ResetColors();
		SetBkColor(::GetSysColor(COLOR_WINDOW));
	}
	else
	{
		SetBkColor(g_Settings.list_ctrl_colors_[0]);	// same backgnd as main wnd
		SetTextColor(g_Settings.viewer_colors_[1]);	// description text color
	}
*/
}


void PreviewPane::PaneHidden()
{
	// remove photo to free memory
	Clear();
}


void PreviewPane::CurrentChanged(PhotoInfoPtr photo)
{
TRACE(_T("当前照片: %s\n"), photo ? photo->GetName().c_str() : _T("-无-"));
	if (photo)
		LoadPhoto(*photo);
	else
		Clear();
}


void PreviewPane::CurrentModified(PhotoInfoPtr photo)
{
	if (photo)
		LoadPhoto(*photo, true);
}


BOOL PreviewPane::IsFrameWnd() const
{
	return true;	// true so PreviewPane can handle UI messages sent by ToolBarWnd
}


void PreviewPane::OnMagnifierLens()
{
	if (!IsWindowVisible())
		return;

	if (DibPtr bmp= display_.GetDibPtr())
	{
		if (display_.ReloadWithoutReduction())
		{
			// wait till photo is reloaded in 1:1
			show_magnifier_lens_ = true;
		}
		else
		{
			show_magnifier_lens_ = false;
			CRect rect= display_.GetImageRect();
			display_.ClientToScreen(rect);
			MagnifierWnd::Close();
			new MagnifierWnd(bmp, rect, this);
		}
	}
}

void PreviewPane::OnUpdateMagnifierLens(CCmdUI* cmd_ui)
{
	cmd_ui->Enable(display_.GetDibPtr() != 0 && !display_.StillLoading());
}


void PreviewPane::DecodingStarted(ViewPane* view)
{}

void PreviewPane::DecodingFinished(ViewPane* view)
{}

void PreviewPane::DecodingThreadFinished(ViewPane* view)
{
	if (show_magnifier_lens_)
	{
		show_magnifier_lens_ = false;
		OnMagnifierLens();
	}
}


void PreviewPane::CaptionHeightChanged(bool big)
{
	bar_wnd_.SetBitmapSize(big);
	ResetBandsWidth(bar_wnd_.GetMinMaxWidth());
}


void PreviewPane::OnContextMenu(CWnd* wnd, CPoint pos)
{
	CMenu menu;
	if (!menu.LoadMenu(IDR_PREVIEW_CONTEXT))
		return;

	if (CMenu* popup= menu.GetSubMenu(0))
	{
		if (pos.x == -1 && pos.y == -1)
		{
			CRect rect;
			GetClientRect(rect);
			ClientToScreen(rect);
			pos = rect.CenterPoint();
		}
		popup->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, pos.x, pos.y, this);
	}
}


void PreviewPane::OnInitMenuPopup(CMenu* popup_menu, UINT index, BOOL sys_menu)
{
	if (popup_menu)
		popup_menu->CheckMenuItem(ID_TOGGLE_DESCRIPTION, MF_BYCOMMAND |
			(display_.IsPhotoDescDisplayed() ? MF_CHECKED : MF_UNCHECKED) );
}



void PreviewPane::OnToggleDispDescription()
{
	bool enable= !display_.IsPhotoDescDisplayed();
	display_.EnablePhotoDesc(enable);
}


void PreviewPane::OnUpdateToggleDispDescription(CCmdUI* cmd_ui)
{
	cmd_ui->Enable();
	cmd_ui->SetCheck(display_.IsPhotoDescDisplayed());
}


void PreviewPane::PhotoDescriptionChanged(std::wstring& descr)
{
	if (display_.IsPhotoDescDisplayed())
	{
		display_.ResetDescription(descr);
		display_.Invalidate();
	}
}


void PreviewPane::OnSetAsWallpaper()
{
	bool SetWallpaper(const PhotoInfo& photo);

	if (ConstPhotoInfoPtr photo= display_.GetCurrentPhoto())
		SetWallpaper(*photo);
}

void PreviewPane::OnUpdateSetAsWallpaper(CCmdUI* cmd)
{
	cmd->Enable(display_.GetCurrentPhoto() != 0);
}


void PreviewPane::MiddleButtonDown(CPoint pos)
{
	OnMagnifierLens();
}


void PreviewPane::RefreshAfterChange(const VectPhotoInfo& photos)
{
	if (display_.IsPhotoDescDisplayed())
		if (ConstPhotoInfoPtr photo= display_.GetCurrentPhoto())
			display_.SetDescriptionText(&photo->PhotoDescription());
}


void PreviewPane::OnJpegRotate90CCW()
{
	RotatePhoto(this, ROTATE_90_DEG_COUNTERCW);
}

void PreviewPane::OnUpdateJpegRotate(CCmdUI* cmd_ui)
{
	cmd_ui->Enable(display_.GetCurrentPhoto() != 0 && rotation_feasible_ && !display_.StillLoading());
}


void PreviewPane::OnJpegRotate90CW()
{
	RotatePhoto(this, ROTATE_90_DEG_CW);
}


void PreviewPane::RotatePhoto(CWnd* wnd, RotationTransformation transform)
{
	PhotoInfoPtr photo= ConstCast(display_.GetCurrentPhoto());
//	PhotoInfoPtr photo= const_cast<PhotoInfoPtr>(display_.GetCurrentPhoto());

	if (photo == 0 || !rotation_feasible_)
		return;

	CWaitCursor wait;

	if (::RotatePhoto(*photo, transform, false, wnd) > 0)
	{
		if (global_photo_cache)
			global_photo_cache->Remove(photo);

		LoadPhoto(*photo, true);

		SendNotification(&PaneWnd::CurrentPhotoModified, photo);
	}
}


void PreviewPane::AnimationStarts()
{
	// forward...
	SendNotification(&PaneWnd::UpdatePane);
}


void PreviewPane::UpdatePane()
{
	// when animation is about to start perform and complete any possible lengthy operations right now, so it can run smoothly
	display_.UpdateWindow();
}

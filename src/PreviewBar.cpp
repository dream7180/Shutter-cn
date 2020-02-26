/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// PreviewBar.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "PreviewBar.h"
#include "PreviewPane.h"
#include "Color.h"
#include "UIElements.h"
#include "WhistlerLook.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// PreviewBar

PreviewBar::PreviewBar()
{
	parent_ = 0;
	vertical_ = true;
}

PreviewBar::~PreviewBar()
{}


BEGIN_MESSAGE_MAP(PreviewBar, CWnd)
	//{{AFX_MSG_MAP(PreviewBar)
	ON_WM_SIZE()
	ON_WM_VSCROLL()
	ON_WM_HSCROLL()
	ON_COMMAND(ID_VIEWER_OPTIONS, OnViewerOptions)
	ON_UPDATE_COMMAND_UI(ID_VIEWER_OPTIONS, OnUpdateViewerOptions)
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_IDLEUPDATECMDUI, OnIdleUpdateCmdUI)
	ON_MESSAGE(WM_USER + 100, OnTbClicked)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// PreviewBar message handlers

static const int TB_IMAGES= 9;		// no of images in the toolbar's bitmap

bool PreviewBar::Create(PreviewPane* parent, bool big)
{
	if (!CWnd::Create(AfxRegisterWndClass(0, 0, reinterpret_cast<HBRUSH>(COLOR_3DFACE + 1)),
		_T(""), WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN, CRect(0,0,0,0), parent, -1))
		return false;

	parent_ = parent;

	static const int cmd[]= { ID_ZOOM_100, ID_ZOOM_FIT, ID_ZOOM_OUT, ID_ZOOM_IN, ID_PREVIEW_OPTIONS, ID_MAGNIFIER_LENS, ID_SET_WALLPAPER, ID_IMG_ROTATE_90_CCW, ID_IMG_ROTATE_90_CW };

	int bmp= IDB_ZOOMBAR;
	toolbar_top_wnd_.SetPadding(8, 8);
	toolbar_top_wnd_.SetOwnerDraw(true);
	VERIFY(toolbar_top_wnd_.Create("XXp......", cmd, bmp, vertical_ ? 0 : IDS_ZOOMBAR, this, -1, vertical_));
	toolbar_top_wnd_.CreateDisabledImageList(bmp);

	if (vertical_)
		if (!WhistlerLook::IsAvailable())
			toolbar_top_wnd_.SetRows(3, false, 0);

	toolbar_btm_wnd_.SetPadding(10, 8);
	toolbar_btm_wnd_.SetOwnerDraw(true);
	VERIFY(toolbar_btm_wnd_.Create("::.p|pPP|pp", cmd, bmp, vertical_ ? 0 : IDS_ZOOMBAR, this, -1, vertical_));
	toolbar_btm_wnd_.CreateDisabledImageList(bmp);
	toolbar_btm_wnd_.DeleteButton(ID_PREVIEW_OPTIONS);

	VERIFY(zoom_wnd_.Create(WS_CHILD | WS_VISIBLE | TBS_BOTH | TBS_NOTICKS /*TBS_AUTOTICKS*/ | (vertical_ ? TBS_VERT : TBS_HORZ), CRect(0,0,0,0), this, -1));

	toolbar_top_wnd_.SetOwner(parent);
	toolbar_top_wnd_.CWnd::SetOwner(parent);

	toolbar_btm_wnd_.SetOwner(parent);
	toolbar_btm_wnd_.CWnd::SetOwner(parent);

//	zoom_wnd_.SetOwner(parent);

	zoom_wnd_.SetRange(0, 20);
	zoom_wnd_.SetTicFreq(2);
	zoom_wnd_.SetPos(0);

	if (!vertical_)
	{
		int width= GetMinMaxWidth().second;
		CRect T_rect;
		toolbar_top_wnd_.GetWindowRect(T_rect);
		SetWindowPos(0, 0, 0, width, T_rect.Height(), SWP_NOZORDER | SWP_NOACTIVATE);
	}

	return true;
}


void PreviewBar::OnSize(UINT type, int cx, int cy)
{
	CWnd::OnSize(type, cx, cy);

	Resize();
}


namespace {
	const int MIN_WIDTH= 50;
	const int MAX_WIDTH= 150;
}


void PreviewBar::Resize()
{
	if (toolbar_top_wnd_.m_hWnd && toolbar_btm_wnd_.m_hWnd && zoom_wnd_.m_hWnd)
	{
		CRect rect;
		GetClientRect(rect);

		CRect T_rect;
		toolbar_top_wnd_.GetWindowRect(T_rect);
		CRect B_rect;
		toolbar_btm_wnd_.GetWindowRect(B_rect);

		if (vertical_)
		{
			int height= rect.Height();

			int y_pos= 0;
			height -= T_rect.Height() + B_rect.Height();
			const int MIN_HEIGHT= 50;
			const int MAX_HEIGHT= 150;
			if (height < MIN_HEIGHT)
				height = MIN_HEIGHT;
			else if (height > MAX_HEIGHT)
			{
				y_pos += (height - MAX_HEIGHT) / 2;
				height = MAX_HEIGHT;
			}

			toolbar_top_wnd_.SetWindowPos(0, 0, y_pos, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

			y_pos += T_rect.Height();

			zoom_wnd_.SetWindowPos(0, 0, y_pos, rect.Width() - 5, height, SWP_NOZORDER | SWP_NOACTIVATE);

			toolbar_btm_wnd_.SetWindowPos(0, 0, y_pos + height, rect.Width(), height, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

			UpdateWindow();
		}
		else	// horizontal layout
		{
			int width= rect.Width();

			int x_pos= 0;
			width -= T_rect.Width() + B_rect.Width();
			if (width < MIN_WIDTH)
				width = MIN_WIDTH;
			else if (width > MAX_WIDTH)
			{
				//x_pos += (width - MAX_WIDTH) / 2;
				width = MAX_WIDTH;
			}

			toolbar_top_wnd_.SetWindowPos(0, x_pos, 0, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

			x_pos += T_rect.Width();

			zoom_wnd_.SetWindowPos(0, x_pos, 3, width, rect.Height() - (WhistlerLook::IsAvailable() ? 5 : 6), SWP_NOZORDER | SWP_NOACTIVATE);

			toolbar_btm_wnd_.SetWindowPos(0, x_pos + width, 0, width, rect.Height(), SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

			UpdateWindow();
		}
	}
}


void PreviewBar::SetSliderPos(int pos)
{
	if (zoom_wnd_.m_hWnd)
	{
		//int max= zoom_wnd_.GetRangeMax();
		//zoom_wnd_.SetPos(max - pos);		// reverse (for vertical slider)

		zoom_wnd_.SetPos(pos);		// no reverse for horizontal slider
	}
}

void PreviewBar::SetSlider(const uint16* zoom_table, int count, int zoom)
{
	if (zoom_wnd_.m_hWnd)
	{
		int max= (count - 1) * 2;
		if (zoom_wnd_.GetRangeMax() != max)
			zoom_wnd_.SetRange(0, max);

		if (zoom < zoom_table[0])
			SetSliderPos(0);
		else if (zoom > zoom_table[count - 1])
			SetSliderPos(max);
		else
		{
			for (int i= 0; i < count; ++i)
			{
				if (zoom == zoom_table[i])
				{
					SetSliderPos(i * 2);
					break;
				}
				else if (zoom < zoom_table[i])
				{
					SetSliderPos(i * 2 - 1);
					break;
				}
			}
		}

//		zoom_wnd_.SetPos(zoom);
	}
}


void PreviewBar::OnVScroll(UINT sb_code, UINT pos, CScrollBar* scroll_bar)
{
	OnScroll(sb_code, pos, scroll_bar);
}

void PreviewBar::OnHScroll(UINT sb_code, UINT pos, CScrollBar* scroll_bar)
{
	OnScroll(sb_code, pos, scroll_bar);
}

void PreviewBar::OnScroll(UINT sb_code, UINT pos, CScrollBar* scroll_bar)
{
	switch (sb_code)
	{
	case SB_LINEUP:		// SB_LINELEFT
	case SB_LINEDOWN:	// SB_LINERIGHT
	case SB_PAGEUP:
	case SB_PAGEDOWN:
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		if (parent_ && zoom_wnd_.m_hWnd)
		{
			int pos= zoom_wnd_.GetPos();//zoom_wnd_.GetRangeMax() - zoom_wnd_.GetPos();
			parent_->SliderChanged(pos);
		}
		break;
	default:
		break;
	}
}


void PreviewBar::OnViewerOptions()
{
	// TODO: Add your command handler code here
	
}

void PreviewBar::OnUpdateViewerOptions(CCmdUI* cmd_ui)
{
	cmd_ui->Enable();
}


BOOL PreviewBar::OnEraseBkgnd(CDC* dc)
{
	CRect rect;
	GetClientRect(rect);

	COLORREF rgb_back= ::GetSysColor(COLOR_3DFACE);

	if (vertical_)
	{
		dc->FillSolidRect(rect, rgb_back);

		int GRADIENT= 4;

		rect.right -= GRADIENT - 1;
		rect.left = rect.right - 1;

		int r= GetRValue(rgb_back);
		int g= GetGValue(rgb_back);
		int b= GetBValue(rgb_back);

		COLORREF rgb_wnd= CalcNewColor(rgb_back, 0.12f, 0.69f);
		int RW= GetRValue(rgb_wnd);
		int GW= GetGValue(rgb_wnd);
		int BW= GetBValue(rgb_wnd);

		for (int i= 0; i < GRADIENT; ++i)
		{
			COLORREF rgb_color= RGB(
				r + (RW - r) * i / GRADIENT,
				g + (GW - g) * i / GRADIENT,
				b + (BW - b) * i / GRADIENT);
			dc->FillSolidRect(rect, rgb_color);
			rect.OffsetRect(1, 0);
		}
	}
	else if (CaptionWindow* parent= dynamic_cast<CaptionWindow*>(GetParent()))
		parent->EraseBackground(*dc, rect);
	else
		::DrawPanelBackground(*dc, rect);

	return true;
}


std::pair<int, int> PreviewBar::GetMinMaxWidth() const
{
	CRect T_rect;
	toolbar_top_wnd_.GetWindowRect(T_rect);
	CRect B_rect;
	toolbar_btm_wnd_.GetWindowRect(B_rect);

	int width= T_rect.Width() + B_rect.Width();

	return std::make_pair(width + MIN_WIDTH, width + MAX_WIDTH);
}


LRESULT PreviewBar::OnIdleUpdateCmdUI(WPARAM wParam, LPARAM lParam)
{
	toolbar_top_wnd_.SendMessage(WM_IDLEUPDATECMDUI, wParam, lParam);
	toolbar_btm_wnd_.SendMessage(WM_IDLEUPDATECMDUI, wParam, lParam);

	return 0;
}


LRESULT PreviewBar::OnTbClicked(WPARAM hwnd, LPARAM code)
{
	if (CWnd* parent= GetParent())
		parent->SendMessage(WM_USER + 100, hwnd, code);

	return 0;
}


void PreviewBar::SetBitmapSize(bool big)
{
	int bmp= IDB_ZOOMBAR;

	toolbar_top_wnd_.ReplaceImageList(bmp, TB_IMAGES);
	toolbar_top_wnd_.CreateDisabledImageList(bmp);

	toolbar_btm_wnd_.ReplaceImageList(bmp, TB_IMAGES);
	toolbar_btm_wnd_.CreateDisabledImageList(bmp);

	if (!vertical_)
	{
		int width= GetMinMaxWidth().second;
		CRect rect(0,0,0,0);
		toolbar_top_wnd_.GetWindowRect(rect);
		SetWindowPos(0, 0, 0, width, rect.Height(), SWP_NOZORDER | SWP_NOACTIVATE);

		Resize();
	}
}


void PreviewBar::SetBackgroundColor(COLORREF backgnd)
{
	zoom_wnd_.SetBackgroundColor(backgnd);
	// to force slider bar to redraw:
	zoom_wnd_.SetRangeMin(zoom_wnd_.GetRangeMin(), true);

	if (m_hWnd)
		Invalidate();
	if (toolbar_top_wnd_.m_hWnd)
		toolbar_top_wnd_.Invalidate();
	if (toolbar_btm_wnd_.m_hWnd)
		toolbar_btm_wnd_.Invalidate();
}

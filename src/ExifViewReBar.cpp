/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// ExifViewReBar.cpp: implementation of the ExifViewReBar class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "resource.h"
#include "ExifViewReBar.h"
#include "SnapFrame/CaptionWindow.h"
#include "UIElements.h"
#include "WhistlerLook.h"
#include "Config.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ExifViewReBar::ExifViewReBar() : owner_(0)
{
	tool_bar_pos_.reserve(6);
}

ExifViewReBar::~ExifViewReBar()
{
}


BEGIN_MESSAGE_MAP(ExifViewReBar, CWnd)
	//{{AFX_MSG_MAP(ExifViewReBar)
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
//	ON_MESSAGE(WM_IDLEUPDATECMDUI, OnIdleUpdateCmdUI)
	ON_MESSAGE(WM_USER + 100, OnTbClicked)
END_MESSAGE_MAP()


namespace {
	const int EXTRASPACE= Pixels(8); // extra space between toolbars
	const int SLIDER_WIDTH= 80;
}


static const int g_cmd[]=
{
	ID_FILTER_SWITCH, ID_VIEW, ID_STICKY_SELECTION, //ID_MARK_ALL, ID_MARK_NONE,
	ID_VIEW_THUMBNAILS, ID_VIEW_DETAILS, ID_VIEW_TILES, ID_VIEW_PICTURES, ID_SHOW_OPTIONS,
	ID_VIEW_THUMB_SMALLER, IDOK, ID_VIEW_THUMB_BIGGER,
	ID_VIEW_SORT, ID_SORT_BY_SIMILARITY, ID_CANCEL_SORT_BY_SIMILARITY,
	ID_GROUP_STARS, ID_GROUP_TAGS, ID_GROUP_FOLDER, ID_GROUP_DATE//,
	//ID_FIND //filter/search
};


bool ExifViewReBar::Create(CWnd* parent, int thumb_index_range, bool)
{
/* test: this ui update is expensive/slow
toolbar_first_.SetOnIdleUpdateState(false);
tool_bar_view_wnd_.SetOnIdleUpdateState(false);
tool_bar_thumb_wnd_.SetOnIdleUpdateState(false);
tool_bar_group_wnd_.SetOnIdleUpdateState(false);
tool_bar_sort_wnd_.SetOnIdleUpdateState(false);*/

	toolbar_first_.SetOwnerDraw(true);
	tool_bar_view_wnd_.SetOwnerDraw(true);
	tool_bar_thumb_wnd_.SetOwnerDraw(true);
	tool_bar_group_wnd_.SetOwnerDraw(true);
	tool_bar_sort_wnd_.SetOwnerDraw(true);
	//toolbar_filter_.SetOwnerDraw(true);

//	if (!CHorzReBar::Create(parent, false, RBS_FIXEDORDER/*RBS_VARHEIGHT | RBS_AUTOSIZE | RBS_DBLCLKTOGGLE | RBS_BANDBORDERS*/, AFX_IDW_REBAR))
//		return false;
	if (!CWnd::Create(AfxRegisterWndClass(0, ::LoadCursor(NULL, IDC_ARROW)), 0, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN,
		CRect(0,0,0,0), parent, -1))
		return false;

	owner_ = parent;
	SetOwner(parent);

	int bmp_id= IDB_PHOTOS_TB;

	if (WhistlerLook::IsAvailable())
		toolbar_first_.SetPadding(-1, -1);	// btn with down arrow section inflates total buttons height
	//toolbar_first_.Create("xm|xpp...............", g_cmd, bmp_id, 0, this, AFX_IDW_CONTROLBAR_LAST);
	toolbar_first_.Create("xm|x...............", g_cmd, bmp_id, 0, this, AFX_IDW_CONTROLBAR_LAST);
	toolbar_first_.CreateDisabledImageList(bmp_id);
	//toolbar_first_.DeleteButton(ID_MARK_ALL);
	//toolbar_first_.DeleteButton(ID_MARK_NONE);
	toolbar_first_.AutoResize();
	toolbar_first_.SetOwner(parent);
	toolbar_first_.CWnd::SetOwner(parent);
	tool_bar_pos_.push_back(PositionBar(toolbar_first_, 0));

	tool_bar_view_wnd_.Create("...gggg|v..........", g_cmd, bmp_id, 0, this, AFX_IDW_CONTROLBAR_LAST - 1);
	tool_bar_view_wnd_.CreateDisabledImageList(bmp_id);
	tool_bar_view_wnd_.SetOwner(parent);
	tool_bar_view_wnd_.CWnd::SetOwner(parent);
	tool_bar_pos_.push_back(PositionBar(tool_bar_view_wnd_, _T("查看")));

	tool_bar_thumb_wnd_.SetPadding(0, 7);
	tool_bar_thumb_wnd_.Create("........ppp.......", g_cmd, bmp_id, 0, this, AFX_IDW_CONTROLBAR_LAST - 2);
	tool_bar_thumb_wnd_.CreateDisabledImageList(bmp_id);
	tool_bar_thumb_wnd_.SetScrollInfoReceiver(parent);
	tool_bar_thumb_wnd_.SetOwner(parent);
	tool_bar_thumb_wnd_.CWnd::SetOwner(parent);
	// make space for trackbar
	tool_bar_thumb_wnd_.EnableButton(IDOK, false);

/*	TBBUTTONINFO tbi;
	tbi.cbSize = sizeof tbi;
	tbi.mask = TBIF_SIZE;
	tbi.cx = SLIDER_WIDTH;
	tool_bar_thumb_wnd_.SetButtonInfo(IDOK, &tbi);
	CRect rect(0,0,0,0);
	int space_idx= 1;
	tool_bar_thumb_wnd_.GetItemRect(space_idx, rect);
	rect.top += 3;
	rect.bottom -= 2;
	if (!WhistlerLook::IsAvailable())
		++rect.top;
*/
	DWORD dwStyle= WS_CHILD | WS_VISIBLE | TBS_NOTICKS | TBS_BOTH | /*TBS_AUTOTICKS | TBS_BOTTOM |*/ TBS_HORZ; // | TBS_TOOLTIPS;
	thumb_size_wnd_.Create(dwStyle, CRect(0,0,0,0), &tool_bar_thumb_wnd_, ID_THUMB_SIZE);
	SetSliderPos();

	tool_bar_pos_.push_back(PositionBar(tool_bar_thumb_wnd_, _T("缩略图")));

	tool_bar_group_wnd_.Create("..............xxxx", g_cmd, bmp_id, 0, this, AFX_IDW_CONTROLBAR_LAST - 4);
	tool_bar_group_wnd_.CreateDisabledImageList(bmp_id);
	tool_bar_group_wnd_.SetOwner(parent);
	tool_bar_group_wnd_.CWnd::SetOwner(parent);
	tool_bar_pos_.push_back(PositionBar(tool_bar_group_wnd_, _T("分组")));

	if (WhistlerLook::IsAvailable())
		tool_bar_sort_wnd_.SetPadding(-1, -1);	// btn with down arrow section inflates total buttons height
	tool_bar_sort_wnd_.Create("...........vmp....", g_cmd, bmp_id, 0, this, AFX_IDW_CONTROLBAR_LAST - 3);
	tool_bar_sort_wnd_.HideButton(ID_CANCEL_SORT_BY_SIMILARITY);
	tool_bar_sort_wnd_.CreateDisabledImageList(bmp_id);
	tool_bar_sort_wnd_.SetOwner(parent);
	tool_bar_sort_wnd_.CWnd::SetOwner(parent);
	tool_bar_pos_.push_back(PositionBar(tool_bar_sort_wnd_, _T("排序")));
	tool_bar_pos_.back().tool_bar_width -= EXTRASPACE;

	thumb_size_wnd_.SetRange(0, thumb_index_range - 1);
	thumb_size_wnd_.SetTicFreq(1);
	thumb_size_wnd_.SetLineSize(1);
	thumb_size_wnd_.SetPageSize(2);
	thumb_size_wnd_.SetTipSide(TBTS_BOTTOM);

	//toolbar_filter_.Create("...................X", g_cmd, bmp_id, IDS_FILTER, this, AFX_IDW_CONTROLBAR_LAST - 5);
	//toolbar_filter_.CWnd::SetOwner(parent);
	//tool_bar_pos_.push_back(PositionBar(toolbar_filter_, 0));

	CRect rect;
	toolbar_first_.GetWindowRect(rect);
	SetWindowPos(0, 0, 0, GetTotalBarWidth(), rect.Height() /* CaptionWindow::GetHeight()*/, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
	Resize();

	return true;
}


CRect ExifViewReBar::GetSortBtnRect(int cmd_id) const
{
	CRect rect;
	tool_bar_sort_wnd_.GetRect(cmd_id, rect);
	tool_bar_sort_wnd_.ClientToScreen(rect);
	return rect;
}


CRect ExifViewReBar::GetMainBtnRect(int cmd_id) const
{
	CRect rect;
	toolbar_first_.GetRect(cmd_id, rect);
	toolbar_first_.ClientToScreen(rect);
	return rect;
}


CRect ExifViewReBar::GetViewBtnRect(int cmd_id) const
{
	CRect rect;
	tool_bar_view_wnd_.GetRect(cmd_id, rect);
	tool_bar_view_wnd_.ClientToScreen(rect);
	return rect;
}


BOOL ExifViewReBar::OnEraseBkgnd(CDC* dc)
{
	CRect rect;
	GetClientRect(rect);
	if (CaptionWindow* parent= dynamic_cast<CaptionWindow*>(GetParent()))
		parent->EraseBackground(*dc, rect);
	else
		::DrawPanelBackground(*dc, rect);

	int count= static_cast<int>(tool_bar_pos_.size());
	int x_pos= 0;

	g_Settings.SelectDefaultFont(*dc);
	dc->SetTextColor(g_Settings.AppColors()[AppColors::DimText]);
	dc->SetBkMode(TRANSPARENT);

	for (int i= 0; i < count; ++i)
	{
		if (tool_bar_pos_[i].show_label && tool_bar_pos_[i].label)
		{
			CRect text_rect= rect;
			text_rect.left = x_pos;
			text_rect.right = text_rect.left + tool_bar_pos_[i].label_width - 4;

			dc->DrawText(tool_bar_pos_[i].label, static_cast<int>(_tcslen(tool_bar_pos_[i].label)), text_rect,
				DT_RIGHT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

			x_pos += tool_bar_pos_[i].label_width;
		}
		x_pos += tool_bar_pos_[i].tool_bar_width;
	}

	return true;
}


ExifViewReBar::Part ExifViewReBar::PositionBar(ToolBarWnd& tool_bar_wnd, const TCHAR* label)
{
	int label_width= 0;

	if (label)
	{
		CClientDC dc(this);
		LOGFONT lf;
		HFONT hfont = static_cast<HFONT>(::GetStockObject(DEFAULT_GUI_FONT));
		::GetObject(hfont, sizeof(lf), &lf);
		//lf.lfQuality = ANTIALIASED_QUALITY;
		lf.lfHeight += 1;
		_tcscpy(lf.lfFaceName, _T("Tahoma"));
		CFont _font;
		_font.CreateFontIndirect(&lf);
		dc.SelectObject(&_font);
		//dc.SelectStockObject(DEFAULT_GUI_FONT);
		int width= dc.GetTextExtent(label, static_cast<int>(_tcslen(label))).cx;
		int space= dc.GetTextExtent(_T(" "), 1).cx;
		label_width = width + 2 * space;
	}

	CRect rect;
	tool_bar_wnd.GetWindowRect(rect);

	return Part(label_width, rect.Width() + EXTRASPACE, label);
}


LRESULT ExifViewReBar::OnIdleUpdateCmdUI(WPARAM wParam, LPARAM lParam)
{
	toolbar_first_.SendMessage(WM_IDLEUPDATECMDUI, wParam, lParam);
	tool_bar_view_wnd_.SendMessage(WM_IDLEUPDATECMDUI, wParam, lParam);
	tool_bar_thumb_wnd_.SendMessage(WM_IDLEUPDATECMDUI, wParam, lParam);
	tool_bar_group_wnd_.SendMessage(WM_IDLEUPDATECMDUI, wParam, lParam);
	tool_bar_sort_wnd_.SendMessage(WM_IDLEUPDATECMDUI, wParam, lParam);

	return 0;
}


LRESULT ExifViewReBar::OnTbClicked(WPARAM hwnd, LPARAM code)
{
	if (CWnd* parent= GetParent())
		parent->SendMessage(WM_USER + 100, hwnd, code);

	return 0;
}


void ExifViewReBar::OnSize(UINT type, int cx, int cy)
{
	if (cx > 0 && toolbar_first_.m_hWnd && tool_bar_view_wnd_.m_hWnd && tool_bar_thumb_wnd_.m_hWnd &&
		tool_bar_group_wnd_.m_hWnd && tool_bar_sort_wnd_.m_hWnd)// && toolbar_filter_.m_hWnd)
	{
		Resize();
	}
}


void ExifViewReBar::Resize()
{
	CRect rect;
	GetClientRect(rect);

	// resize takes care of turning labels on and off depending on available space

	CWnd* wnd[]=
	{
		&toolbar_first_, &tool_bar_view_wnd_, &tool_bar_thumb_wnd_, &tool_bar_group_wnd_, &tool_bar_sort_wnd_//, &toolbar_filter_
	};

	ASSERT(array_count(wnd) == tool_bar_pos_.size());

	int count= static_cast<int>(tool_bar_pos_.size());

	if (GetTotalBarWidth() > rect.Width())	// toolbar only partially visible?
	{
		// turn off labels
		for (int i= 0; i < count; ++i)
			tool_bar_pos_[i].show_label = false;

		int min_total_width= GetMinTotalBarWidth();
		if (min_total_width < rect.Width())
		{
			int available_width= rect.Width() - min_total_width;

			for (int i= count - 1; i >= 0; --i)
				if (tool_bar_pos_[i].label != 0)
					if (tool_bar_pos_[i].label_width < available_width)
					{
						available_width -= tool_bar_pos_[i].label_width;
						tool_bar_pos_[i].show_label = true;
					}
					else
						break;
		}
	}
	else
	{
		// turn on labels
		for (int i= 0; i < count; ++i)
			tool_bar_pos_[i].show_label = true;
	}

	Invalidate();

	int x_pos= 0;

	for (int i= 0; i < count; ++i)
	{
		if (tool_bar_pos_[i].show_label && tool_bar_pos_[i].label != 0)
			x_pos += tool_bar_pos_[i].label_width;

		wnd[i]->SetWindowPos(0, x_pos, 0, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

		x_pos += tool_bar_pos_[i].tool_bar_width;
	}
}


void ExifViewReBar::SetBitmapSize(bool)
{
	int bmp_id= IDB_PHOTOS_TB;

	int btn_count= array_count(g_cmd);

	toolbar_first_.ReplaceImageList(bmp_id, btn_count);
	tool_bar_view_wnd_.ReplaceImageList(bmp_id, btn_count);
	tool_bar_thumb_wnd_.ReplaceImageList(bmp_id, btn_count);
	tool_bar_group_wnd_.ReplaceImageList(bmp_id, btn_count);
	tool_bar_sort_wnd_.ReplaceImageList(bmp_id, btn_count);

	toolbar_first_.CreateDisabledImageList(bmp_id);
	tool_bar_view_wnd_.CreateDisabledImageList(bmp_id);
	tool_bar_thumb_wnd_.CreateDisabledImageList(bmp_id);
	tool_bar_group_wnd_.CreateDisabledImageList(bmp_id);
	tool_bar_sort_wnd_.CreateDisabledImageList(bmp_id);

	SetSliderPos();
	tool_bar_thumb_wnd_.AutoResize();

	CWnd* wnd[]=
	{
		&toolbar_first_, &tool_bar_view_wnd_, &tool_bar_thumb_wnd_, &tool_bar_group_wnd_, &tool_bar_sort_wnd_
	};

	ASSERT(array_count(wnd) == tool_bar_pos_.size());

	int count= static_cast<int>(tool_bar_pos_.size());

	for (int i= 0; i < count; ++i)
	{
		CRect rect;
		wnd[i]->GetWindowRect(rect);
		tool_bar_pos_[i].tool_bar_width = rect.Width();
	}

	SetWindowPos(0, 0, 0, GetTotalBarWidth(), CaptionWindow::GetHeight(), SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

	Resize();
}


int ExifViewReBar::GetTotalBarWidth() const
{
	int sum= 0;

	int count= static_cast<int>(tool_bar_pos_.size());
	for (int i= 0; i < count; ++i)
		sum += tool_bar_pos_[i].tool_bar_width + tool_bar_pos_[i].label_width;

	return sum;
}


int ExifViewReBar::GetMinTotalBarWidth() const
{
	int sum= 0;

	int count= static_cast<int>(tool_bar_pos_.size());
	for (int i= 0; i < count; ++i)
		sum += tool_bar_pos_[i].tool_bar_width;

	return sum;
}


void ExifViewReBar::SetSliderPos()
{
	TBBUTTONINFO tbi;
	tbi.cbSize = sizeof tbi;
	tbi.dwMask = TBIF_SIZE;
	tbi.cx = Pixels(SLIDER_WIDTH);
	tool_bar_thumb_wnd_.SetButtonInfo(IDOK, &tbi);
	CRect rect(0,0,0,0);
	int space_idx= 1;
	tool_bar_thumb_wnd_.GetItemRect(space_idx, rect);
	rect.top += 3;
	rect.bottom -= 2;
	if (!WhistlerLook::IsAvailable())
		++rect.top;

	thumb_size_wnd_.SetWindowPos(0, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOZORDER);

	tool_bar_thumb_wnd_.GetItemRect(tool_bar_thumb_wnd_.GetButtonCount() - 1, rect);
	tool_bar_thumb_wnd_.SetWindowPos(0, 0, 0, rect.right, rect.bottom, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
}


void ExifViewReBar::ImgSizeLabel(bool thumbs)
{
	if (tool_bar_pos_.size() > 2)
	{
		tool_bar_pos_[2].label = thumbs ? _T("缩略图") : _T("图像");
		Invalidate();
	}
}


void ExifViewReBar::Invalidate()
{
	if (m_hWnd)
		CWnd::Invalidate();
	if (toolbar_first_.m_hWnd)
		toolbar_first_.Invalidate();
	if (tool_bar_view_wnd_.m_hWnd)
		tool_bar_view_wnd_.Invalidate();
	if (tool_bar_thumb_wnd_.m_hWnd)
		tool_bar_thumb_wnd_.Invalidate();
	if (tool_bar_group_wnd_.m_hWnd)
		tool_bar_group_wnd_.Invalidate();
	if (tool_bar_sort_wnd_.m_hWnd)
		tool_bar_sort_wnd_.Invalidate();
	if (thumb_size_wnd_.m_hWnd)
	{
		thumb_size_wnd_.Invalidate();
		// to force slider bar to redraw:
		thumb_size_wnd_.SetRangeMin(thumb_size_wnd_.GetRangeMin(), true);
	}
}


void ExifViewReBar::SetBackgroundColor(COLORREF backgnd)
{
	thumb_size_wnd_.SetBackgroundColor(backgnd);
	Invalidate();
}

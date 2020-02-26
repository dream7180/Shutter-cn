/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// HorzReBar.cpp : implementation file
//

#include "stdafx.h"
#include "HorzReBar.h"
#include <afxpriv.h>
#include "ProfileVector.h"
#include "UIElements.h"
#include "Color.h"
#include "Config.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// HorzReBar

HorzReBar::HorzReBar()
{
	resizing_border_ = true;
	bottom_margin_ = 0;
	height_requested_ = 0;
	background_ = RGB(255, 0, 0);// ::GetSysColor(COLOR_3DFACE);
	text_color_ = ::GetSysColor(COLOR_BTNTEXT);
}

HorzReBar::~HorzReBar()
{}

CString HorzReBar::wnd_class_;


BEGIN_MESSAGE_MAP(HorzReBar, CWnd)
	//{{AFX_MSG_MAP(HorzReBar)
	ON_WM_SIZE()
	ON_WM_NCCALCSIZE()
	ON_WM_NCHITTEST()
	ON_WM_NCPAINT()
	ON_WM_GETMINMAXINFO()
	ON_WM_SIZING()
	ON_WM_WINDOWPOSCHANGING()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_SIZEPARENT, OnSizeParent)
	ON_NOTIFY_EX(RBN_HEIGHTCHANGE, HREBAR_CTRL_ID, OnReBarHeightChange)
	ON_NOTIFY_EX(RBN_ENDDRAG, HREBAR_CTRL_ID, OnReBarHeightChange)
	ON_NOTIFY(NM_CUSTOMDRAW, HREBAR_CTRL_ID, RebarCustomDraw)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// HorzReBar message handlers

const int REBAR_BORDER= 0;


LRESULT HorzReBar::OnSizeParent(WPARAM, LPARAM lParam)
{
	AFX_SIZEPARENTPARAMS* layout = (AFX_SIZEPARENTPARAMS*)lParam;
	bool visible= !!(GetStyle() & WS_VISIBLE);

	// align the control bar
	CRect rect= layout->rect;

	CSize avail_size = rect.Size();  // maximum size available

	// get maximum requested size

	CSize size(0, 0);

	if (visible)
	{
		size.cy = REBAR_BORDER + rebar_wnd_.GetBarHeight() + bottom_margin_;
		size.cx = MIN(99999, avail_size.cx);
	}
	layout->sizeTotal.cy += size.cy;
	layout->sizeTotal.cx = std::max(layout->sizeTotal.cx, size.cx);
	layout->rect.top += size.cy;

	rect.right = rect.left + size.cx;
	rect.bottom = rect.top + size.cy;

	// only resize the window if doing layout and not just rect query
	if (layout->hDWP != NULL)
		AfxRepositionWindow(layout, m_hWnd, &rect);

	return 0;
}


bool HorzReBar::Create(CWnd* parent, bool resizing_border, UINT id/*= AFX_IDW_REBAR*/)
{
	return Create(parent, resizing_border, RBS_VARHEIGHT | RBS_AUTOSIZE | RBS_DBLCLKTOGGLE | RBS_BANDBORDERS, id);
}

bool HorzReBar::Create(CWnd* parent, bool resizing_border, DWORD styles, UINT id)
{
	resizing_border_ = resizing_border;
	if (!resizing_border_)
		bottom_margin_ = 0;

	if (wnd_class_.IsEmpty())
		wnd_class_ = AfxRegisterWndClass(CS_VREDRAW | CS_HREDRAW, ::LoadCursor(NULL, IDC_ARROW));// , HBRUSH(COLOR_3DFACE + 1));

	if (!CWnd::Create(wnd_class_, _T(""), WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN,
		CRect(0,0,0,0), parent, id))
		return false;

	styles |= WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | /*CCS_TOP |*/ CCS_NODIVIDER | CCS_NOPARENTALIGN;

	if (!rebar_wnd_.Create(styles, CRect(0,0,0,0), this, HREBAR_CTRL_ID))
		return false;

	rebar_wnd_.SendMessage(WM_SETFONT, WPARAM(g_Settings.GetDefaultFont()));

	REBARINFO ri;
	ri.cbSize = sizeof ri;
	ri.fMask = 0;
	ri.himl = 0;
	rebar_wnd_.SetBarInfo(&ri);

	// turn off UI theme so we can set desired rebar colors
	::SetWindowTheme(rebar_wnd_.m_hWnd, L"", L"");

	COLORSCHEME cs;
	cs.dwSize = sizeof cs;
	COLORREF base= CalcShade(background_, 0.0f);

	// hide separator lines and grip:
	cs.clrBtnHighlight = base;
	cs.clrBtnShadow = base;
//cs.clrBtnHighlight = 0xffffff;
//cs.clrBtnShadow = 0x808080;
	rebar_wnd_.SetColorScheme(&cs);

	rebar_wnd_.SetBkColor(base);
	rebar_wnd_.SetTextColor(text_color_);

	return true;
}


bool HorzReBar::AddBand(CWnd* child_wnd, int id/*= -1*/, bool break_to_new_line/*= false*/)
{
	CRect rect;
	child_wnd->GetWindowRect(rect);

	return AddBand(child_wnd, rect.Size(), 0, id, 50, break_to_new_line);
}

bool HorzReBar::AddBand(CWnd* child_wnd, CSize child_size, const TCHAR* label, int id, int min_width/*= 50*/, bool break_to_new_line/*= false*/)
{
	// create rebar band
	REBARBANDINFO rbi;

	memset(&rbi, 0, sizeof rbi);
	rbi.cbSize = sizeof rbi;
	rbi.fMask = RBBIM_CHILD | /*RBBIM_COLORS |*/ RBBIM_CHILDSIZE | RBBIM_STYLE | RBBIM_ID | RBBIM_LPARAM | RBBIM_IDEALSIZE | RBBIM_SIZE;
	rbi.fStyle = /*RBBS_FIXEDBMP |*/ RBBS_GRIPPERALWAYS; //RBBS_VARIABLEHEIGHT | RBBS_NOGRIPPER;
	if (break_to_new_line)
		rbi.fStyle |= RBBS_BREAK;
	rbi.hwndChild = *child_wnd;
//	rbi.clrBack = ::GetSysColor(COLOR_3DFACE);
//	rbi.clrFore = ::GetSysColor(COLOR_BTNTEXT);
	rbi.cxMinChild = MIN(child_size.cx, min_width);
	rbi.cyMinChild = child_size.cy;
	rbi.cyChild = child_size.cy;
	rbi.cyMaxChild = 99999;
	rbi.cyIntegral = 1;
	rbi.wID = id;
	rbi.lParam = 2;
	rbi.cx = child_size.cx;
	rbi.lpText = const_cast<TCHAR*>(label);
	if (label)
		rbi.fMask |= RBBIM_TEXT;
	rbi.cxIdeal = child_size.cx;
	if (rbi.cxIdeal == 0)
		rbi.fMask &= ~RBBIM_IDEALSIZE;
	if (!rebar_wnd_.InsertBand(-1, &rbi))
		return false;

	return true;
}


bool HorzReBar::AddFixedBand(CWnd* child, int id, const TCHAR* label/*= 0*/)
{
	// create rebar band
	REBARBANDINFO rbi;

	CRect rect(0, 0, 500, 1);
	if (child)
		child->GetWindowRect(rect);
	CSize child_size= rect.Size();

	memset(&rbi, 0, sizeof rbi);
	rbi.cbSize = sizeof rbi;
	rbi.fMask = RBBIM_CHILD | RBBIM_COLORS | RBBIM_CHILDSIZE | RBBIM_STYLE | RBBIM_ID | RBBIM_LPARAM;
	rbi.fStyle = RBBS_FIXEDBMP | /*RBBS_FIXEDSIZE |*/ RBBS_NOGRIPPER;
	rbi.hwndChild = *child;
	rbi.clrBack = background_;
	rbi.clrFore = text_color_;
	rbi.cxMinChild = child_size.cx; //min(child_size.cx, 50);
	rbi.cyMinChild = child_size.cy;
	rbi.cyChild = child_size.cy;
	rbi.cyMaxChild = child_size.cy;
	rbi.cyIntegral = 1;
	rbi.wID = id;
	rbi.lParam = 2;
	rbi.lpText = const_cast<TCHAR*>(label);
	if (label)
		rbi.fMask |= RBBIM_TEXT;
	rbi.cxIdeal = child_size.cx;
	if (rbi.cxIdeal != 0)
		rbi.fMask |= RBBIM_IDEALSIZE;
	if (!rebar_wnd_.InsertBand(-1, &rbi))
		return false;

	return true;
}


void HorzReBar::OnSize(UINT type, int cx, int cy)
{
	CWnd::OnSize(type, cx, cy);

	if (rebar_wnd_.m_hWnd)
	{
		rebar_wnd_.SetWindowPos(0, 0, 0, cx, cy, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

		int height= rebar_wnd_.GetBarHeight() + REBAR_BORDER;
		rebar_wnd_.SetWindowPos(0, 0, 0, cx, height, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
	}
}


void HorzReBar::OnNcCalcSize(BOOL calc_valid_rects, NCCALCSIZE_PARAMS* ncsp)
{
	CWnd::OnNcCalcSize(calc_valid_rects, ncsp);

	if (resizing_border_ && ncsp)
		ncsp->rgrc[0].bottom -= bottom_margin_;
}


LRESULT HorzReBar::OnNcHitTest(CPoint point)
{
	if (resizing_border_)
	{
		CRect rect;
		GetWindowRect(rect);
		rect.top = rect.bottom - bottom_margin_;
		if (rect.PtInRect(point))
			return HTBOTTOM;
	}

	return CWnd::OnNcHitTest(point);
}


void HorzReBar::OnNcPaint()
{
	if (resizing_border_)
	{
		CRect rect;
		GetWindowRect(rect);
		rect.OffsetRect(-rect.TopLeft());

		CWindowDC dc(this);
		dc.FillSolidRect(0, rect.Height() - bottom_margin_, rect.Width(), bottom_margin_, background_);
	}
	else
		Default();
}


BOOL HorzReBar::OnReBarHeightChange(UINT id, NMHDR* nmhdr, LRESULT* result)
{
	if (result)
		*result = 0;

	CWnd* parent= GetParent();
	if (CFrameWnd* frame= dynamic_cast<CFrameWnd*>(parent))
	{
		frame->RecalcLayout();
		return true;
	}
	else if (parent)
	{
		const MSG* msg= GetCurrentMessage();
		parent->SendMessage(msg->message, msg->wParam, msg->lParam);
		return true;
	}

	return false;
}


void HorzReBar::OnGetMinMaxInfo(MINMAXINFO* mmi)
{
	CWnd::OnGetMinMaxInfo(mmi);

	if (mmi)
	{
//TRACE(L"%d %d - %d %d\n", MMI->ptMinTrackSize.x, MMI->ptMinTrackSize.y, MMI->max_track_size.x, MMI->max_track_size.y);
	}
}


void HorzReBar::OnSizing(UINT side, LPRECT rect)
{
	CWnd::OnSizing(side, rect);
}


int HorzReBar::GetHeight() const
{
	return rebar_wnd_.GetBarHeight() + REBAR_BORDER + bottom_margin_;
}


CSize HorzReBar::GetBarSize() const
{
	CRect bound_rect(0,0,0,0);

	int count= rebar_wnd_.GetBandCount();

	for (int i= 0; i < count; ++i)
	{
		REBARBANDINFO rbbi;
		rbbi.cbSize = sizeof rbbi;
		rbbi.fMask = RBBIM_STYLE;
		rebar_wnd_.GetBandInfo(i, &rbbi);
		CRect rect(0,0,0,0);
		if ((rbbi.fStyle & RBBS_HIDDEN) == 0)
		{
			rebar_wnd_.GetRect(i, rect);
			bound_rect |= rect;
		}
	}

	bound_rect.InflateRect(1, 1);

	return bound_rect.Size();
}


void HorzReBar::OnWindowPosChanging(WINDOWPOS* wnd_pos)
{
	CWnd::OnWindowPosChanging(wnd_pos);

//	height_requested_ = wnd_pos->cy;
//TRACE(L"win pos req: %d\n", height_requested_);
	if (rebar_wnd_.m_hWnd)
	{
		CRect rect(0, 0, wnd_pos->cx, wnd_pos->cy - bottom_margin_);
		rebar_wnd_.SizeToRect(rect);
//		rebar_wnd_.SetWindowPos(0, 0, 0, wnd_pos->cx, wnd_pos->cy - MARGIN, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

		int height= rebar_wnd_.GetBarHeight() + REBAR_BORDER;
		rebar_wnd_.SetWindowPos(0, 0, 0, wnd_pos->cx, height, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

		wnd_pos->cy = height + bottom_margin_;

	}
//TRACE(L"win pos chg: %d\n", wnd_pos->cy);
}


void HorzReBar::ShowBand(int band_id, bool show/*= true*/)
{
	int index= rebar_wnd_.IDToIndex(band_id);
	if (index != -1)
		rebar_wnd_.ShowBand(index, show);
}


bool HorzReBar::IsBandVisible(int band_id) const
{
	int index= rebar_wnd_.IDToIndex(band_id);
	if (index != -1)
	{
		REBARBANDINFO rbi;
		rbi.cbSize = sizeof rbi;
		rbi.fMask = RBBIM_STYLE;
		rbi.fStyle = 0;
		rebar_wnd_.GetBandInfo(index, &rbi);
		return !(rbi.fStyle & RBBS_HIDDEN);
	}

	return false;
}


void HorzReBar::MaximizeBand(UINT band, bool ideal_size/*= false*/)
{
	rebar_wnd_.SendMessage(RB_MAXIMIZEBAND, band, ideal_size ? 1 : 0);
}


// registry operation: storing bands' layout
bool HorzReBar::StoreLayout(const TCHAR* section, const TCHAR* key)
{
	if (rebar_wnd_.m_hWnd == 0)
		return false;

	int bands= rebar_wnd_.GetBandCount();

	std::vector<UINT> data;
	data.reserve(bands * 3);

	for (int i= 0; i < bands; ++i)
	{
		REBARBANDINFO rbi;
		rbi.cbSize = sizeof rbi;
		rbi.fMask = RBBIM_STYLE | RBBIM_SIZE | RBBIM_ID;
		rebar_wnd_.GetBandInfo(i, &rbi);

		data.push_back(rbi.wID);
		data.push_back(rbi.cx);
		data.push_back(rbi.fStyle);
	}

	return WriteProfileVector(section, key, data);
}


// registry operation: restoring bands' layout
bool HorzReBar::RestoreLayout(const TCHAR* section, const TCHAR* key)
{
	if (rebar_wnd_.m_hWnd == 0)
		return false;

	std::vector<UINT> data;
	if (!GetProfileVector(section, key, data))
		return false;

	int bands= rebar_wnd_.GetBandCount();
	if (data.size() != bands * 3)
	{
		ASSERT(false);
		return false;
	}

	for (int i= 0, data_idx= 0; i < bands; ++i)
	{
		rebar_wnd_.MoveBand(rebar_wnd_.IDToIndex(data[data_idx++]), i);

		REBARBANDINFO rbi;
		rbi.cbSize = sizeof rbi;
		rbi.fMask = RBBIM_STYLE | RBBIM_SIZE | RBBIM_ID;

		rebar_wnd_.GetBandInfo(i, &rbi);

		rbi.cx = data[data_idx++];
		rbi.fStyle &= ~(RBBS_HIDDEN | RBBS_BREAK);
		rbi.fStyle |= data[data_idx++];
/* doesn't work as expected
		// this is my addition: fixed bands have RBBS_FIXEDBMP style
		if (rbi.style & RBBS_FIXEDBMP)
			rbi.style &= ~RBBS_BREAK;	// remove new line style for fixed bands
*/
		rebar_wnd_.SetBandInfo(i, &rbi);
	}

	return true;
}


void HorzReBar::RebarCustomDraw(NMHDR* nmhdr, LRESULT* result)
{
	NMCUSTOMDRAW* draw= reinterpret_cast<NMCUSTOMDRAW*>(nmhdr);

	*result = CDRF_DODEFAULT;

	if (draw->dwDrawStage == CDDS_PREPAINT)
	{
		*result = CDRF_NOTIFYPOSTPAINT | CDRF_NOTIFYITEMDRAW;
	}
	else if (draw->dwDrawStage == CDDS_ITEMPREPAINT)
	{
		*result = CDRF_NOTIFYPOSTPAINT | CDRF_NOTIFYITEMDRAW;
	}
	else if (draw->dwDrawStage == CDDS_ITEMPOSTPAINT && draw->dwItemSpec != 0)
	{
		CRect rect(0,0,0,0);
		int index= rebar_wnd_.IDToIndex(static_cast<UINT>(draw->dwItemSpec));
		if (rebar_wnd_.GetRect(index, rect))
		{
			//CRect borders(0,0,0,0);
			//rebar_wnd_.GetBandBorders(index, borders);
			//rebar_wnd_.GetBandMargins(
			COLORREF base= background_;
			//if (rect.left > 0)
			//{
			//	COLORREF light= CalcShade(base, 70.0f);
			//	COLORREF dark= CalcShade(base, -10.0f);
			//	FillSolidRect(draw->hdc, rect.left + 0, rect.top, 1, rect.Height(), dark);
			//	FillSolidRect(draw->hdc, rect.left + 1, rect.top, 1, rect.Height(), light);
			//}
//			rect.DeflateRect(3, 3, 0, 1);
//			DrawResizingGrip(draw->hdc, rect, base, false);
			FillSolidRect(draw->hdc, rect.left, rect.top, 4, rect.Height(), base);
		}
	}
}


void HorzReBar::SetBottomMargin(int margin)
{
	bottom_margin_ = margin;
}


void HorzReBar::SetBackgroundColor(COLORREF color)
{
	background_ = color;
	if (m_hWnd)
		Invalidate();
}

void HorzReBar::SetTextColor(COLORREF color)
{
	text_color_ = color;
	if (m_hWnd)
		Invalidate();
}

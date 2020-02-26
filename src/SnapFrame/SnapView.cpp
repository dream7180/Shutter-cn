/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// SnapView.cpp : implementation of the SnapView class
//

#include "stdafx.h"
#include "../Resource.h"
#include "SnapView.h"
#include "SnapFrame.h"
#include "SnapMsg.h"
#include "../Color.h"
#include "../UIElements.h"
#include "../ColorConfiguration.h"
#include "../Config.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern COLORREF GetLighterColor(COLORREF rgb_color, float percent);

#ifndef WM_XBUTTONDOWN
#define WM_XBUTTONDOWN                  0x020B
#endif

/////////////////////////////////////////////////////////////////////////////
// SnapView

IMPLEMENT_DYNCREATE(SnapView, CFrameWnd)

BEGIN_MESSAGE_MAP(SnapView, CFrameWnd)
	//{{AFX_MSG_MAP(SnapView)
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_COMMAND(ID_PANE_CLOSE, OnPaneClose)
	ON_COMMAND(ID_PANE_MAXIMIZE, OnPaneMaximize)
	ON_COMMAND(ID_PANE_RESTORE, OnPaneRestore)
	ON_WM_SETCURSOR()
	ON_WM_MOUSEACTIVATE()
	ON_WM_ACTIVATE()
	ON_WM_DESTROY()
	ON_COMMAND(ID_PANE_CONTEXT_HELP, OnPaneContextHelp)
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND_EX(ID_HELP, OnHelp)
	ON_NOTIFY(TBN_GETINFOTIP, IDB_PANE_TOOLBAR, OnGetInfoTip)
	ON_MESSAGE(WM_XBUTTONDOWN, OnXButtonDown)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// SnapView construction/destruction

HCURSOR SnapView::resize_vert_= 0;
HCURSOR SnapView::resize_horz_= 0;


SnapView::SnapView()
{
//	rgb_color_ = RGB(rand(), rand(), rand());
	moving_ = false;
	frame_ = 0;
	view_next_to_ = 0;
	insert_pos_ = INSERT_NONE;
	marker_ = INSERT_NONE;
	child_view_ = 0;
	current_doc_ = 0;
	active_ = false;
	is_context_help_wnd_ = false;
	separator_base_color_ = ::GetSysColor(COLOR_BTNFACE);// COLOR_INACTIVECAPTION);

	background_color_ = g_Settings.pane_caption_colors_[SnapFrame::C_BACKGROUND].SelectedColor();

	if (resize_vert_ == 0)
		resize_vert_ = AfxGetApp()->LoadCursor(IDC_VERT_RESIZE);
	if (resize_horz_ == 0)
		resize_horz_ = AfxGetApp()->LoadCursor(IDC_HORZ_RESIZE);

	resizing_edge_ = INSERT_NONE;
	resizing_ = false;
}

SnapView::~SnapView()
{
}

BOOL SnapView::PreCreateWindow(CREATESTRUCT& cs)
{
	BOOL ret= CFrameWnd::PreCreateWindow(cs);
	cs.style &= ~WS_BORDER;
	cs.style |= WS_CLIPCHILDREN;
	cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
	return ret;
}

/////////////////////////////////////////////////////////////////////////////
// SnapView diagnostics

#ifdef _DEBUG
void SnapView::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void SnapView::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// SnapView message handlers

static const COLORREF g_rgb_mark= RGB(247, 123, 0);//::GetSysColor(COLOR_HIGHLIGHT);
static const COLORREF g_rgb_back= ::GetSysColor(COLOR_3DFACE);
//static const COLORREF g_rgb_caption= ::GetSysColor(COLOR_INACTIVECAPTION);

//const CSize g_BARS(3, 3);	// resizing bars thickness

static COLORREF CalcShade(COLORREF rgb_color, float shade)
{
	shade /= 120.0f;

	int red= GetRValue(rgb_color);
	int green= GetGValue(rgb_color);
	int blue= GetBValue(rgb_color);

	if (shade > 0.0f)	// lighter
	{
		return RGB(	std::min(255, int(red + shade * (0xff - red))),
					std::min(255, int(green + shade * (0xff - green))),
					std::min(255, int(blue + shade * (0xff - blue))));
	}
	else if (shade < 0.0f)	// darker
	{
		shade = 1.0f + shade;

		return RGB(	std::min(255, int(red * shade)),
					std::min(255, int(green * shade)),
					std::min(255, int(blue * shade)));
	}
	else
		return rgb_color;
}


CSize SnapView::GetBarThickness()
{
	int n = Pixels(1);
	return CSize(n, n);
}

CSize SnapView::GetSeparatorThickness()
{
	CSize size = SnapView::GetBarThickness();
	return CSize(size.cx * 2, size.cy * 2);
}

//static float shades[] = { 0.0f, 20.0f, -20.0f, 40.0f, -40.0f, 60.0f };
// order of shades from left to right: 5, 3, 1, 0, 2, 4
//static float shades[] = { 0.0f, 40.0f, -5.0f, 90.0f, -10.0f, 0.0f };

std::pair<float, float> GetShadeFactors(int i, int N)
{
	return std::make_pair(0.0f, 0.0f);
/*
	if (N < 2)
		return std::make_pair(0.0f, 0.0f);
	float dark = i > 0 ? -10.0f * i / (N - 1) : 0.0f;
	float light = i < N - 1 ? (40.0f * (i + 1) / (N - 1)) : 0.0f;
	return std::make_pair(dark, light); */
}

BOOL SnapView::OnEraseBkgnd(CDC* dc)
{
	CRect rect(0,0,0,0);
	GetClientRect(rect);

	CSize size = GetBarThickness();

	for (int i = 0; i < size.cx; ++i)
	{
		auto shade = GetShadeFactors(i, size.cx);

		COLORREF c1 = CalcShade(separator_base_color_, shade.first);// shades[idx++]);
		COLORREF c2 = CalcShade(separator_base_color_, shade.second);// shades[idx++]);

		dc->Draw3dRect(rect, c1, c2);

		rect.DeflateRect(1, 1);
	}

	dc->FillSolidRect(rect, background_color_);// ::GetSysColor(COLOR_3DFACE));

	return true;
}


void SnapView::DrawHorzSeparator(CDC* dc, COLORREF color, CRect rect)
{
	dc->FillSolidRect(rect, color);

	rect.bottom = rect.top + 1;

	CSize size = GetBarThickness();

	for (int i = size.cx - 1; i >= 0; --i)
	{
		auto shade = GetShadeFactors(i, size.cx);
		dc->FillSolidRect(rect, CalcShade(color, shade.second));
		rect.OffsetRect(0, 1);
	}

	for (int i = 0; i < size.cx; ++i)
	{
		auto shade = GetShadeFactors(i, size.cx);
		dc->FillSolidRect(rect, CalcShade(color, shade.first));
		rect.OffsetRect(0, 1);
	}

	//dc->FillSolidRect(rect, CalcShade(color, shades[5]));
	//rect.OffsetRect(0, 1);
	//dc->FillSolidRect(rect, CalcShade(color, shades[3]));
	//rect.OffsetRect(0, 1);
	//dc->FillSolidRect(rect, CalcShade(color, shades[1]));
	//rect.OffsetRect(0, 1);
	//dc->FillSolidRect(rect, CalcShade(color, shades[0]));
	//rect.OffsetRect(0, 1);
	//dc->FillSolidRect(rect, CalcShade(color, shades[2]));
	//rect.OffsetRect(0, 1);
	//dc->FillSolidRect(rect, CalcShade(color, shades[4]));

	/*
	for (int i= 0, idx= 0; i < 3; ++i)
	{
		COLORREF c1= CalcShade(color, shades[idx++]);
		COLORREF c2= CalcShade(color, shades[idx++]);

		dc->Draw3dRect(rect, c1, c2);

		rect.DeflateRect(0, 1);
	}
*/
}


void SnapView::OnGetInfoTip(NMHDR* nmhdr, LRESULT* result)	// provide tool tip text for toolbar
{
	NMTBGETINFOTIP* info_tip= reinterpret_cast<NMTBGETINFOTIP*>(nmhdr);
	*result = 0;

	CString tip;
	tip.LoadString(info_tip->iItem);
	_tcsncpy(info_tip->pszText, tip, INFOTIPSIZE);
}


void SnapView::InitialUpdate(const TCHAR* title, const TCHAR* ctx_help_topic)
{
	frame_ = dynamic_cast<SnapFrame*>(GetParent());
	ASSERT(frame_);
	//ASSERT(ctx_help_topic);
	ctx_help_topic_ = ctx_help_topic;
	caption_wnd_.SetFrame(frame_);
	caption_wnd_.SetTitle(title);
}


void SnapView::OnLButtonDown(UINT flags, CPoint pos)
{
	start_ = pos;

	Insert edge= HitTest(pos);

	if (edge != INSERT_NONE)
	{
		if (frame_->IsResizingPossible(this, edge))
		{
			resizing_edge_ = edge;
			resizing_ = true;
		}
	}
/*	else
	{
		moving_ = true;
		view_next_to_ = 0;
		insert_pos_ = INSERT_NONE;
		view_with_marker_ = 0;
	} */

	SetCapture();
}


void SnapView::OnLButtonUp(UINT flags, CPoint pos)
{
	ReleaseCapture();
/*	if (moving_)
	{
		if (view_with_marker_)
		{
			view_with_marker_->EraseInsertMarker();
			view_with_marker_ = 0;
		}
		moving_ = false;
		if (view_next_to_ != 0 && insert_pos_ != INSERT_NONE)
			frame_->RepositionWindow(this, view_next_to_, insert_pos_);
		view_next_to_ = 0;
		insert_pos_ = INSERT_NONE;
	}
	else */
	if (resizing_)
	{
		resizing_edge_ = INSERT_NONE;
		resizing_  = false;
	}
}


void SnapView::OnMouseMove(UINT flags, CPoint pos)
{
	if (!(flags & MK_LBUTTON))
		return;

/*	if (moving_)
	{
		CSize delta_size= pos - start_;

		ClientToScreen(&pos);
//		TRACE("%d,%d\n", pos.x, pos.y);
		SnapView* view= dynamic_cast<SnapView*>(CWnd::WindowFromPoint(pos));
		{
			//if (view != this)
			{
				Insert pos= view ? view->FindInsertPos(pos) : INSERT_NONE;
//				TRACE("%d\n", pos);

				if (view_with_marker_)
				{
					if (view != view_with_marker_)
						view_with_marker_->EraseInsertMarker();
					view_with_marker_ = 0;
				}
				if (view && view != this)
				{
					view->DrawInsertMarker(pos);
					view_with_marker_ = view;
				}

				return;
			}
		}

		//frame_->DrawInsertMarker(0, INSERT_NONE);
	}
	else */
	if (resizing_)
	{
		frame_->ResizePane(this, resizing_edge_, pos);
	}
}


bool SnapView::FindQuarter(CPoint first, CPoint second, CPoint test)
{
	double delta_x= second.x - first.x;
	double delta_y= second.y - first.y;

	double result= delta_y / delta_x * (test.x - first.x) + first.y;

	return result > test.y;
}


SnapView::Insert SnapView::FindInsertPos(CPoint pos)
{
	CRect rect;
	GetWindowRect(rect);

	if (rect.PtInRect(pos))
	{
		CPoint first= rect.TopLeft();
		bool right_top= FindQuarter(rect.TopLeft(), rect.BottomRight(), pos);
		bool left_top= FindQuarter(CPoint(rect.left, rect.bottom), CPoint(rect.right, rect.top), pos);

		if (left_top && right_top)
			return INSERT_TOP;
		else if (!left_top && !right_top)
			return INSERT_BOTTOM;
		else if (!left_top && right_top)
			return INSERT_RIGHT;
		else
			return INSERT_LEFT;
	}

	return INSERT_NONE;
}


CRect SnapView::GetMarkerRect(Insert pos, bool full/*= false*/) const
{
	CRect rect;
	GetClientRect(rect);

	CSize frame_size= full ? CSize(0, 0) : CSize(1, 1);

	switch (pos)
	{
	case INSERT_LEFT:
		rect.right = rect.left + GetBarThickness().cx - frame_size.cx;
		break;

	case INSERT_RIGHT:
		rect.left = rect.right - GetBarThickness().cx + frame_size.cx;
		break;

	case INSERT_TOP:
		rect.bottom = rect.top + GetBarThickness().cy - frame_size.cy;
		break;

	case INSERT_BOTTOM:
		rect.top = rect.bottom - GetBarThickness().cy + frame_size.cy;
		break;

	default:
		rect.SetRectEmpty();
		break;
	}

	return rect;
}


void SnapView::EraseInsertMarker()
{
	if (marker_ != INSERT_NONE)
	{
		CClientDC dc(this);
		dc.FillSolidRect(GetMarkerRect(marker_), g_rgb_back);
//		InvalidateRect(GetMarkerRect(marker_));
	}
	marker_ = INSERT_NONE;
}


bool SnapView::IsMarkerChanged(Insert pos) const
{
	return marker_ != pos;
}


void SnapView::DrawInsertMarker(Insert pos)
{
	if (marker_ != pos)
	{
		EraseInsertMarker();
		CClientDC dc(this);
		dc.FillSolidRect(GetMarkerRect(pos), g_rgb_mark);
		marker_ = pos;
	}
}


int SnapView::OnCreate(LPCREATESTRUCT create_struct)
{
	if (CWnd::OnCreate(create_struct) == -1)
		return -1;

//	CString title;
//	title.Format(_T("Window %x"), int(this) & 0xfff);
	caption_wnd_.Create(this, _T(""));

	if (create_struct && create_struct->lpCreateParams)
		current_doc_ = reinterpret_cast<CCreateContext*>(create_struct->lpCreateParams)->m_pCurrentDoc;

	return 0;
}


CString SnapView::GetTitle() const
{
	CString str;
	if (caption_wnd_.m_hWnd)
		caption_wnd_.GetWindowText(str);
	return str;
}


void SnapView::OnSize(UINT type, int cx, int cy)
{
	CWnd::OnSize(type, cx, cy);
	Resize();
}


void SnapView::Resize()
{
	CRect rect;
	GetClientRect(rect);
	rect.DeflateRect(GetBarThickness());
	caption_wnd_.SetPosition(rect);
	if (child_view_ && child_view_->m_hWnd)
		child_view_->MoveWindow(GetViewRect());
}


void SnapView::OnPaneClose()
{
	frame_->PaneClose(this);
}


void SnapView::OnPaneMaximize()
{
	frame_->PaneMaximize(this);
}


void SnapView::OnPaneRestore()
{
	frame_->PaneRestore(this);
}


void SnapView::OnPaneContextHelp()
{
	frame_->PaneContextHelp(this);
}


// hit test: report back the edge below pos
//
SnapView::Insert SnapView::HitTest(CPoint pos) const
{
	CRect rect;
	GetClientRect(rect);

	CRect inside_rect= rect;
	inside_rect.DeflateRect(GetBarThickness());

	// check if cursor is NOT inside working are
	if (!inside_rect.PtInRect(pos))
	{
		Insert veEdges[]= { INSERT_LEFT, INSERT_RIGHT, INSERT_TOP, INSERT_BOTTOM };

		for (int i= 0; i < array_count(veEdges); ++i)
			if (GetMarkerRect(veEdges[i], true).PtInRect(pos))
				return veEdges[i];
	}

	return INSERT_NONE;
}


BOOL SnapView::OnSetCursor(CWnd* wnd, UINT hit_test, UINT message)
{
	CPoint pos;
	::GetCursorPos(&pos);
	ScreenToClient(&pos);

	Insert edge= HitTest(pos);

	switch (edge)
	{
	case INSERT_LEFT:
	case INSERT_RIGHT:
	case INSERT_TOP:
	case INSERT_BOTTOM:
		if (!frame_->IsResizingPossible(this, edge))
			break;
		::SetCursor(edge == INSERT_LEFT || edge == INSERT_RIGHT ? resize_horz_ : resize_vert_);
		return true;
	}

	return CFrameWnd::OnSetCursor(wnd, hit_test, message);
}


void SnapView::ModifyToolbar(bool wnd_maximized)
{
	caption_wnd_.ModifyToolbar(wnd_maximized);
}


int SnapView::OnMouseActivate(CWnd* desktop_wnd, UINT hit_test, UINT message)
{
	int result= CFrameWnd::OnMouseActivate(desktop_wnd, hit_test, message);

	if (result == MA_ACTIVATE && frame_)
		frame_->SetActiveSnapView(this);

	return result;
}


void SnapView::Activate(bool active)
{
//	if (active && frame_)
//		frame_->SetActiveView(child_view_);

	active_ = active;

	//TODO:
//	if (frame_)
//		frame_->SetActiveView(active ? child_view_ : 0);
	// this is a replacement for above lines
	if (child_view_)
		child_view_->SendMessage(SNAP_WND_MSG_ACTIVATE, active);

	caption_wnd_.Activate(active);
}


// showing/hiding pane window
//
void SnapView::Show(bool visible, bool notification/*= false*/)
{
	if (child_view_)
		child_view_->PostMessage(SNAP_WND_MSG_SHOW, visible ? 1 : 0, notification ? 1 : 0);
}


bool SnapView::TabChange()
{
	if (child_view_)
		return child_view_->SendMessage(SNAP_WND_MSG_TAB_CHANGING) != 0;
	return false;
}


LRESULT SnapView::OnXButtonDown(WPARAM wParam, LPARAM lParam)
{
	frame_->PaneZoom(this);
	return 1;
}


bool SnapView::CreateClient(PaneConstruction* construction, UINT pane_flags)
{
	ASSERT(child_view_ == 0);

	if (!construction->CreateWin(this))
		return false;

	child_view_ = construction->wnd_;

	if (pane_flags & PANE_NO_CLOSE)
		caption_wnd_.HideCloseButton();

	if (pane_flags & PANE_NO_MAXIMIZE)
		caption_wnd_.HideMaximizeButton();

/*
	CWnd* view= view_class->CreatePane(frame_); // static_cast<CWnd*>(view_class->CreateObject());
	if (view == NULL)
	{
		TRACE(_T("Warning: Dynamic create of view has failed.\n"));
		//TRACE1("Warning: Dynamic create of view type %hs failed.\n", view_class->class_name_);
		return false;
	}

	ASSERT_KINDOF(CView, view);

	CCreateContext context;
	context.m_pCurrentFrame = frame_;
	context.m_pCurrentDoc = m_pCurrentDoc;
	context.m_pNewViewClass = 0;
	context.m_pNewDocTemplate = 0;

	if (!view->Create(NULL, NULL, WS_CHILD | WS_VISIBLE, GetViewRect(), this, -1, &context))
	{
		TRACE0("Warning: could not create view for frame.\n");
		return false;        // can't continue without a view
	}

	child_view_ = view; //static_cast<CView*>(view);
*/
	// this is a way to find out if our child view is a context help window
	is_context_help_wnd_ = false; //child_view_->SendMessage(SNAP_WND_MSG_DISP_CTX_HELP) == 'ok';

	// display default help topic
	if (is_context_help_wnd_)
		DisplayHelp(GetContextHelpTopic());

	return true;
}


// client view area
//
CRect SnapView::GetViewRect()
{
	CRect rect;
	GetClientRect(rect);
	rect.DeflateRect(GetBarThickness());
	rect.top += caption_wnd_.GetHeight();
	if (rect.bottom < rect.top)
		rect.bottom = rect.top;

	return rect;
}


void SnapView::OnActivate(UINT state, CWnd* wnd_other, BOOL minimized)
{
//	CView::OnActivate(state, wnd_other, minimized);
	CFrameWnd::OnActivate(state, wnd_other, minimized);

	caption_wnd_.Activate(state != WA_INACTIVE);
}


void SnapView::SetTitle(const TCHAR* title)
{
	if (caption_wnd_.m_hWnd)
		caption_wnd_.SetTitle(title);
}


BOOL SnapView::OnCmdMsg(UINT id, int code, void* extra, AFX_CMDHANDLERINFO* handler_info)
{
	if (CFrameWnd::OnCmdMsg(id, code, extra, handler_info))
		return true;

	if (child_view_)
		return child_view_->OnCmdMsg(id, code, extra, handler_info);

	return false;
}


void SnapView::OnDestroy()
{
	if (frame_)
		frame_->RemoveView(this);

	CFrameWnd* frame = GetParentFrame();
	if (frame != NULL && child_view_ != 0 && frame->GetActiveView() == child_view_)
		frame->SetActiveView(NULL);    // deactivate during death

	CFrameWnd::OnDestroy();
}


void SnapView::OnUpdateFrameTitle(BOOL add_to_title)
{
	if (frame_)
		frame_->OnUpdateFrameTitle(add_to_title);
}


BOOL SnapView::OnHelp(UINT)
{
	if (active_ && frame_)
	{
		frame_->PaneContextHelp(this);
///		frame_->DisplayContextHelp(ctx_help_topic_);

		return true;	// help request handled by this view
	}

	return false;	// not an active view, go to the next one
}


// display help in the client help window
//
void SnapView::DisplayHelp(const TCHAR* ctx_help_topic)
{
	ASSERT(is_context_help_wnd_);

	if (child_view_ && child_view_->m_hWnd)
		child_view_->SendMessage(SNAP_WND_MSG_DISP_CTX_HELP, 0, LPARAM(ctx_help_topic));
}


// get help topic (HTML file name) for a view
//
CString SnapView::GetContextHelpTopic() const
{
	const int MAX= 200;
	TCHAR topic[MAX + 1];

	topic[0] = 0;

	// view window has a chance to overwrite default help topic (for instance
	// depending on it's state)

	if (child_view_->SendMessage(SNAP_WND_MSG_GET_CTX_HELP, MAX, LPARAM(topic)) && topic[0] != 0)
		return topic;

	// default help topic

	return ctx_help_topic_;
}


// add a toolbar to the caption free space
void SnapView::AddBand(CWnd* toolbar, CWnd* owner, std::pair<int, int> min_max_width, bool resizable/*= false*/)
{
	caption_wnd_.AddBand(toolbar, owner, min_max_width, resizable);
}


void SnapView::ResetBandsWidth(std::pair<int, int> min_max_width)
{
	caption_wnd_.ResetBandsWidth(min_max_width);
}

void SnapView::ResetBandsWidth()
{
	caption_wnd_.ResetBandsWidth();
}


bool SnapView::IsPaneOpen()
{
	if (frame_)
		return frame_->IsPaneOpen(this);
	return false;
}

/*
void SnapView::ChangeCaptionHeight(bool big)
{
	caption_wnd_.ChangeHeight(big);

	if (CWnd* child= GetChildView())
	{
		child->SendMessage(SNAP_WND_MSG_CAPTION_HEIGHT_CHANGED, big);
		Resize();
	}
}
*/

void SnapView::InvalidateCaption()
{
	caption_wnd_.Invalidate();
}


void SnapView::ResetCaption(const ColorConfiguration& colors)
{
	background_color_ = colors[SnapFrame::C_BACKGROUND].SelectedColor();
	caption_wnd_.SetTabColors(colors);
}


void SnapView::SetSeparatorBaseColor(COLORREF color)
{
	if (separator_base_color_ != color)
	{
		separator_base_color_ = color;
		Invalidate();
	}
}


void SnapView::SetFaintCaptionEdge(bool faint)
{
	caption_wnd_.SetBottomEdge(faint);
}


bool SnapView::CanMaximize() const
{
	return caption_wnd_.HasMaximizeButton();
}

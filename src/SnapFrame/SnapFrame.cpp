/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// SnapFrame.cpp : implementation of the SnapFrame class

#include "stdafx.h"
#include "../Resource.h"
#include "SnapFrame.h"
#include "CaptionWindow.h"
#include "../UniqueLetter.h"
#include "../CatchAll.h"
#include "SnapMsg.h"
#include "../Config.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// pane resizing step
static const CSize g_GRID= CSize(8, 8);
static const TCHAR* g_suffix= sizeof(TCHAR) > 1 ? _T("_l") : _T("_a");

/////////////////////////////////////////////////////////////////////////////
// SnapFrame

//IMPLEMENT_DYNCREATE(SnapFrame, CMDIChildWnd)

BEGIN_MESSAGE_MAP(SnapFrame, CFrameWnd)
	//{{AFX_MSG_MAP(SnapFrame)
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_COMMAND(ID_PANE_ZOOM, OnPaneZoom)
	ON_WM_DESTROY()
	ON_COMMAND(ID_PANES_RESET, OnPanesReset)
	//}}AFX_MSG_MAP
	ON_COMMAND_RANGE(AFX_IDW_PANE_FIRST, AFX_IDW_PANE_LAST, OnPaneToggle)
	ON_UPDATE_COMMAND_UI_RANGE(AFX_IDW_PANE_FIRST, AFX_IDW_PANE_LAST, OnUpdatePaneToggle)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// SnapFrame construction/destruction

SnapFrame::SnapFrame() : border_(2), right_(border_[0]), bottom_(border_[1])
{
	view_with_marker_ = 0;
	wnd_maximized_ = 0;
	wnd_dragged_ = 0;
	insert_pos_ = SnapView::INSERT_NONE;
	layout_initialized_ = false;
	active_container_view_ = 0;
	active_view_ = 0;
	page_id_ = 0;
}


SnapFrame::~SnapFrame()
{}


CString SnapFrame::wnd_class_;

/////////////////////////////////////////////////////////////////////////////
// SnapFrame diagnostics

#ifdef _DEBUG
void SnapFrame::AssertValid() const			{ CWnd::AssertValid(); }
void SnapFrame::Dump(CDumpContext& dc) const	{ CWnd::Dump(dc); }
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// SnapFrame message handlers

// create snapper frame
bool SnapFrame::Create(CWnd* parent, const TCHAR* title, int id)
//	const PaneLayoutInfoArray& PanesInfo, FramePageCreateContext* context)
{
	if (wnd_class_.IsEmpty())
		wnd_class_ = AfxRegisterWndClass(CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS,
						::LoadCursor(NULL,IDC_ARROW), HBRUSH(COLOR_3DFACE + 1), 0);

	icon_index_ = 0;// icon;
	page_id_ = 0; // context ? context->page_id_ : 0;

	if (!CWnd::Create(wnd_class_, title, WS_CHILD | WS_DISABLED | WS_CLIPCHILDREN,
		CRect(0,0,0,0), parent, id))
		return false;

	return true; //CreatePanes(PanesInfo, context);
}


BOOL SnapFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if (!CFrameWnd::PreCreateWindow(cs))
		return FALSE;

	cs.style |= WS_CLIPCHILDREN;
	cs.dwExStyle &= ~WS_EX_CLIENTEDGE;

	return TRUE;
}


CString SnapFrame::GetTabTitle() const
{
	CString title;
	GetWindowText(title);
	return title;
}

/*
BOOL SnapFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* context)
{
//	for (int i= 0; i < 14; ++i)
//		CMDIChildWnd::OnCreateClient(lpcs, context);
	
//	return CMDIChildWnd::OnCreateClient(lpcs, context);

	// collect windows; assign them pane ids
	GetWndList(pane_windows_);

	windows_.Copy(pane_windows_);
	// add border fake windows
	windows_.push_back(right_);
	windows_.push_back(bottom_);

	// resize
	RepositionWindows();

	return true;
}
*/

bool SnapFrame::CreatePanes(const PaneLayoutInfoArray& PanesInfo, FramePageCreateContext* context_ptr)
{
	CCreateContext context;
	context.m_pCurrentFrame = this;
	context.m_pCurrentDoc = context_ptr ? context_ptr->CC_.m_pCurrentDoc : 0;
	context.m_pNewViewClass = RUNTIME_CLASS(SnapView);
	context.m_pNewDocTemplate = 0;

	for (int i= 0; i < PanesInfo.count_; ++i)
	{
		SnapView* view= dynamic_cast<SnapView*>(CreateView(&context));
		if (view == 0)
		{
			ASSERT(false);
			return false;
		}

		// set separator color
		ColorCfg& c= g_Settings.pane_caption_colors_[C_SEPARATOR];
		view->SetSeparatorBaseColor(c.SelectedColor());

		view->InitialUpdate(PanesInfo.pane_layout_[i].pane_title_, PanesInfo.pane_layout_[i].ctx_help_topic_);
	}

	// collect windows; assign them pane ids
	GetWndList(pane_windows_);

	windows_.Copy(pane_windows_);

	// add border fake windows
	windows_.push_back(right_);
	windows_.push_back(bottom_);

	defaut_pane_layout_.reserve(PanesInfo.count_);

	for (int j= 0; j < PanesInfo.count_; ++j)
		defaut_pane_layout_.push_back(PanesInfo.pane_layout_[j]);

	registry_section_ = PanesInfo.registry_section_;
	registry_entry_ = PanesInfo.registry_entry_;

	layout_initialized_ = false;

	layout_list_.RestoreLayouts(registry_section_, registry_entry_ + g_suffix);

	return CreateClientWindows();
}


void SnapFrame::RecalcLayout(BOOL notify)
{}


BOOL SnapFrame::OnEraseBkgnd(CDC* dc)
{
	CRect rect;
	GetClientRect(rect);
	dc->FillSolidRect(rect, ::GetSysColor(COLOR_3DFACE));
	return true;
}


void SnapFrame::SaveVector::Copy(const WndPosVector& src, std::vector<CRect>& dst)
{
	ASSERT(dst.size() == 0);
	const size_t count= src.size();
	for (size_t i= 0; i < count; ++i)
		dst.push_back(src[i].GetRect());
}

void SnapFrame::SaveVector::Copy(const std::vector<CRect>& src, WndPosVector& dst)
{
	ASSERT(dst.size() == src.size());
	const size_t count= dst.size();
	for (size_t i= 0; i < count; ++i)
		dst[i].GetRect() = src[i];
}



CRect SnapFrame::RepositionWindow(CWnd* view, CWnd* view_next_to, SnapView::Insert pos, bool projection)
{
	ASSERT(view);

	WndPos* wnd= windows_.Find(view);
	if (wnd == 0)
	{
		ASSERT(false);
		return CRect(0,0,0,0);
	}

	WndPos* wnd_next_to= 0;
	if (view_next_to)
	{
		wnd_next_to = windows_.Find(view_next_to);
		if (wnd_next_to == 0)
		{
			ASSERT(false);
			return CRect(0,0,0,0);
		}
	}

	CRect rect;
	GetClientRect(rect);
	ClientToGrid(rect);

	CRect pane_rect(0,0,0,0);

	if (projection)
	{
		SaveVector save(windows_);

		if (wnd_next_to)
			windows_.RearrangeWindow(*wnd, *wnd_next_to, pos, rect.Size());
		else
			windows_.RearrangeWindow(*wnd, pos, rect.Size(), right_, bottom_);

		FitIntoFrame();

		pane_rect = wnd->GetRect();
	}
	else
	{
		bool change= false;

		if (wnd_next_to)
			change = windows_.RearrangeWindow(*wnd, *wnd_next_to, pos, rect.Size());
		else
			change = windows_.RearrangeWindow(*wnd, pos, rect.Size(), right_, bottom_);

		if (change)
		{
			FitIntoFrame();
			RepositionWindows();

			// in one pathological case when removed window is inserted in the
			// same place there's no change in layout and reinserted window is
			// not repainted ramaining invisible, thus repaint it now
			if (wnd->wi_.visible_ && wnd->wi_.wnd_)
			{
				wnd->wi_.wnd_->ModifyStyle(WS_VISIBLE, 0);
				wnd->wi_.wnd_->ShowWindow(SW_SHOWNA);
			}
		}

		pane_rect = wnd->GetRect();
	}

	return pane_rect;
}


// frame window resized: check windows' sizes
//
void SnapFrame::RepositionWindows(const std::vector<PaneLayoutInfo>& panes_info)
{
	ASSERT(panes_info.size() == pane_windows_.size());

	CRect frame_rect;
	GetClientRect(frame_rect);

	int width= frame_rect.Width() / g_GRID.cx;
	int height= frame_rect.Height() / g_GRID.cy;

	// default layout is expressed in percents; convert it to grid units

	for (int i= 0; i < pane_windows_.size(); ++i)
	{
		WndInfo& wi= pane_windows_[i];
		const CRect& r= panes_info[i].pane_location_rect_;

		ASSERT(r.left >= 0 && r.left <= 100);
		ASSERT(r.top >= 0 && r.top <= 100);
		ASSERT(r.right  >= 0 && r.right <= 100);
		ASSERT(r.bottom >= 0 && r.bottom <= 100);

		wi.pos_rect_.left = r.left * width / 100;
		wi.pos_rect_.right = r.right * width / 100;
		wi.pos_rect_.top = r.top * height / 100;
		wi.pos_rect_.bottom = r.bottom * height / 100;
	}

	right_.GetRect().SetRect(width,-1, 99999,99999);
	bottom_.GetRect().SetRect(-1,height, width,99999);

	if (!pane_windows_.CheckLayout())
	{
		// supplied pane layout info is invalid
		return;
	}

	// set window visibility based on the parameter from PaneLayoutInfo
	const size_t count= panes_info.size();

	// for pane windows being reset set appropriate visibility flag: this step
	// is crucial for the RemoveWindow() to work properly
	{
		for (size_t i= 0; i < count; ++i)
			pane_windows_[i].visible_ = !(panes_info[i].pane_flags_ & PANE_HIDDEN);
	}

	// now remove panes that are to be hidden
	{
		for (size_t i= 0; i < count; ++i)
			if (panes_info[i].pane_flags_ & PANE_HIDDEN)
			{
				pane_windows_[i].Disable();
				windows_.RemoveWindow(windows_[i]);		// remove the pane to close
				if (SnapView* view= dynamic_cast<SnapView*>(windows_[i].wi_.wnd_))
					view->Show(false);					// inform view
				FitIntoFrame();
			}
	}

	// reposition remaining panes
	RepositionWindows();

	{
		// and show them
		for (size_t i= 0; i < count; ++i)
			if (!(panes_info[i].pane_flags_ & PANE_HIDDEN))
				pane_windows_[i].EnableAndShow();
	}
}


void SnapFrame::RepositionWindows()
{
	pane_windows_.Reposition(this, right_.GetRect().left, bottom_.GetRect().top);
}


void SnapFrame::FitIntoFrame()
{
	windows_.FitIntoFrame(this, right_, bottom_);
}


bool SnapFrame::CreateClientWindows()
{
	ASSERT(defaut_pane_layout_.size() == pane_windows_.size());

	for (int i= 0; i < pane_windows_.size(); ++i)
	{
		SnapView* view= static_cast<SnapView*>(pane_windows_[i].wnd_);
		PaneLayoutInfo& pi= defaut_pane_layout_[i];
		//ASSERT(pi.view_construction_);
		//if (pi.view_construction_)
			if (!view->CreateClient(&pi.view_construction_, pi.pane_flags_))
				return false;
//		if (pane_windows_[i].visible_)
//		view->Show(pane_windows_[i].visible_, true);
	}

	return true;
}


void SnapFrame::ClientToGrid(CRect& rect)
{
	rect.left /= g_GRID.cx;
	rect.right /= g_GRID.cx;
	rect.top /= g_GRID.cy;
	rect.bottom /= g_GRID.cy;
}

void SnapFrame::GridToClient(CRect& rect)
{
	CRect client_rect;
	GetClientRect(client_rect);

	rect.left *= g_GRID.cx;

	if (client_rect.right / g_GRID.cx == rect.right)
		rect.right = client_rect.right;
	else
		rect.right *= g_GRID.cx;

	rect.top *= g_GRID.cy;

	if (client_rect.bottom / g_GRID.cy == rect.bottom)
		rect.bottom = client_rect.bottom;
	else
		rect.bottom *= g_GRID.cy;
}


void SnapFrame::GetWndList(WndInfoVector& wndvect)
{
	wndvect.clear();
	wndvect.reserve(32);

	int id= AFX_IDW_PANE_FIRST;

	for (CWnd* wnd= GetWindow(GW_CHILD); wnd != 0; wnd = wnd->GetNextWindow())
	{
		SnapView* view= dynamic_cast<SnapView*>(wnd);
		if (view == 0)
		{
			ASSERT(false);
			continue;
		}
		wnd->SetDlgCtrlID(id++);
		CRect rect;
		wnd->GetWindowRect(rect);
		ScreenToClient(rect);
		ClientToGrid(rect);
		wndvect.push_back(WndInfo(view, rect));
	}
}


// rearrange (move, reposition) window
//
bool SnapFrame::WndPosVector::RearrangeWindow(WndPos& wp, WndPos& next_to, SnapView::Insert edge_pos, CSize frame_size)
{
	// TODO: check if simple rearrangement is possible


	CRect rect= wp.GetRect();	// original size

	if (!RemoveWindow(wp))
		return false;

	if (edge_pos == SnapView::INSERT_LEFT || edge_pos == SnapView::INSERT_RIGHT)
	{
		int width= rect.Width();
		if (width > frame_size.cx / 2)
			width = frame_size.cx / 2;
		InsertWindow(wp, next_to, width, edge_pos == SnapView::INSERT_LEFT);
	}
	else if (edge_pos == SnapView::INSERT_TOP || edge_pos == SnapView::INSERT_BOTTOM)
	{
		TransposeVector tv(this);
		int height= rect.Height();
		if (height > frame_size.cy / 2)
			height = frame_size.cy / 2;
		InsertWindow(wp, next_to, height, edge_pos == SnapView::INSERT_TOP);
	}

	return true;
}


// rearrange (move, reposition) window by placing it along given edge of frame window
//
bool SnapFrame::WndPosVector::RearrangeWindow(WndPos& wp, SnapView::Insert edge_pos, CSize frame_size, WndPos& rightmost, WndPos& bottommost)
{
	ASSERT(edge_pos != SnapView::INSERT_NONE);

	CRect rect= wp.GetRect();	// original size

	if (!RemoveWindow(wp))
		return false;

	return InsertWindowAtSide(wp, rect, edge_pos, frame_size, rightmost, bottommost);
}


bool SnapFrame::WndPosVector::InsertWindowAtSide(WndPos& wp, CSize frame_size, WndPos& rightmost, WndPos& bottommost)
{
	ASSERT(wp.IsSidePane());

	CRect rect(0,0,0,0); //= wp.GetRect();	// original size

	SnapView::Insert edge_pos= SnapView::INSERT_NONE;

	if (wp.position_flags_ & WndPos::LEFT && wp.position_flags_ & WndPos::RIGHT) // stretched horizontally?
	{
		edge_pos = wp.position_flags_ & WndPos::TOP ? SnapView::INSERT_TOP : SnapView::INSERT_BOTTOM;
		rect.bottom = wp.last_size_;
	}
	else
	{
		edge_pos = wp.position_flags_ & WndPos::LEFT ? SnapView::INSERT_LEFT : SnapView::INSERT_RIGHT;
		rect.right = wp.last_size_;
	}

	return InsertWindowAtSide(wp, rect, edge_pos, frame_size, rightmost, bottommost);
}


bool SnapFrame::WndPosVector::InsertWindowAtSide(WndPos& wp, CRect rect, SnapView::Insert edge_pos, CSize frame_size, WndPos& rightmost, WndPos& bottommost)
{
	switch (edge_pos)
	{
	case SnapView::INSERT_LEFT:
		{
			WndPosVector windows;
			FindWindows(CRect(0, 0, 1, bottommost.GetRect().top), windows);
			if (windows.size() > 0)
			{
				int width= rect.Width();
				if (width > frame_size.cx / 2)
					width = frame_size.cx / 2;
				int max_resize= width;

				for (int i= 0; i < windows.size(); ++i)
				{
					int max= CalcMaxHorzResizeOfWindows(windows[i], false, true);
					if (max < max_resize)
						max_resize = max;
				}

				int space= std::max(wp.GetMinSize(), max_resize);

				for (int j= 0; j < windows.size(); ++j)
				{
					int shift= -HorzResizeWindows(windows[j], -space, max_resize >= wp.GetMinSize());
					ASSERT(shift == space);
				}

				wp.GetRect().SetRect(0, 0, space, bottommost.GetRect().top);
				wp.wi_.Enable();
			}
			else
			{
				ASSERT(false);
				return false;
			}
		}
		break;

	case SnapView::INSERT_RIGHT:
		{
			WndPosVector left_neighbors;
			FindLeftNeighbors(rightmost, left_neighbors);
			if (left_neighbors.IsEmpty())
			{
				ASSERT(false);
				return false;
			}

			int width= rect.Width();
			if (width > frame_size.cx / 2)
				width = frame_size.cx / 2;

			int resized= HorzResizeWindows(left_neighbors[0], width, false);

			int left= rightmost.GetRect().left;

			if (resized >= wp.GetMinSize())
			{
				rightmost.GetRect().left += resized;
				bottommost.GetRect().right += resized;
				wp.GetRect().SetRect(left, 0, left + resized, bottommost.GetRect().top);
			}
			else
			{
				rightmost.GetRect().left += wp.GetMinSize();
				bottommost.GetRect().right += wp.GetMinSize();
				wp.GetRect().SetRect(left, 0, left + wp.GetMinSize(), bottommost.GetRect().top);
			}

			wp.wi_.Enable();
		}
		break;

	case SnapView::INSERT_TOP:
		{
			WndPosVector windows;
			FindWindows(CRect(0, 0, rightmost.GetRect().left, 1), windows);
			if (windows.size() > 0)
			{
				int height= rect.Height();
				if (height > frame_size.cy / 2)
					height = frame_size.cy / 2;
				int max_resize= height;

				TransposeVector tv(this);

				for (int i= 0; i < windows.size(); ++i)
				{
					int max= CalcMaxHorzResizeOfWindows(windows[i], false, true);
					if (max < max_resize)
						max_resize = max;
				}

				int space= std::max(wp.GetMinSize(), max_resize);

				for (int j= 0; j < windows.size(); ++j)
				{
					int shift= -HorzResizeWindows(windows[j], -space, max_resize >= wp.GetMinSize());
					ASSERT(shift == space);
				}

				wp.GetRect().SetRect(0, 0, space, rightmost.GetRect().top);
				wp.wi_.Enable();
			}
			else
			{
				ASSERT(false);
				return false;
			}
		}
		break;

	case SnapView::INSERT_BOTTOM:
		{
			TransposeVector tv(this);

			WndPosVector top_neighbors;
			FindLeftNeighbors(bottommost, top_neighbors);
			if (top_neighbors.IsEmpty())
			{
				ASSERT(false);
				return false;
			}

			int height= rect.Height();
			if (height > frame_size.cy / 2)
				height = frame_size.cy / 2;

			int resized= HorzResizeWindows(top_neighbors[0], height, false);

			int left= bottommost.GetRect().left;

			if (resized >= wp.GetMinSize())
			{
				bottommost.GetRect().left += resized;
				wp.GetRect().SetRect(left, 0, left + resized, rightmost.GetRect().top);
			}
			else
			{
				bottommost.GetRect().left += wp.GetMinSize();
				wp.GetRect().SetRect(left, 0, left + wp.GetMinSize(), rightmost.GetRect().top);
			}

			wp.wi_.Enable();
		}
		break;
	}

	return true;
}


int SnapFrame::WndPosVector::FindWindowsInLine(WndPos& wp, WndPosVector& left_neighbors, WndPosVector& right_neighbors)
{
	bool left= FindWindowsInLine(wp, left_neighbors, true);
	bool right= FindWindowsInLine(wp, right_neighbors, false);

	if (left && right)
		return 3;
	else if (left)
		return 1;
	else if (right)
		return 2;

	return 0;
}


bool SnapFrame::WndPosVector::FindWindowsInLine(WndPos& wp, WndPosVector& neighbors, bool left)
{
	FindNeighbors(wp, left, neighbors);

	if (neighbors.IsEmpty())
		return false;

	for (int i= 0; i < neighbors.size(); ++i)
	{
		if (!neighbors[i].wi_.IsVisible() ||
			neighbors[i].GetRect().top < wp.GetRect().top ||
			neighbors[i].GetRect().bottom > wp.GetRect().bottom)
		{
			return false;
		}
	}

	return true;
}


// resize neighbors of 'wp' covering it's area
//
int SnapFrame::WndPosVector::RemoveWindowAndResize(WndPos& wp)
{
	WndPosVector left_neighbors;
	WndPosVector right_neighbors;

	int space= wp.GetRect().Width();

	if (int in_line= FindWindowsInLine(wp, left_neighbors, right_neighbors))
	{
		if (in_line == 3)	// left AND right neighbors in line with 'wp'?
		{
			// this is an attempt to restore original size of pane that was shrunk to make space
			// for a pane being removed now
			if ((wp.neighbor_edge_ == SnapView::INSERT_RIGHT || wp.neighbor_edge_ == SnapView::INSERT_BOTTOM) &&
				left_neighbors.size() == 1 /* && left_neighbors.front() ==(?) wp.neighbor_index_ */)
			{
				left_neighbors.front().GetRect().right += space;
			}
			else if ((wp.neighbor_edge_ == SnapView::INSERT_LEFT || wp.neighbor_edge_ == SnapView::INSERT_TOP) &&
				right_neighbors.size() == 1 /* && right_neighbors.front() ==(?) wp.neighbor_index_ */)
			{
				right_neighbors.front().GetRect().left -= space;
			}
			else
			{
				int left= space / 2;
				int right= space - left;
				for (int i= 0; i < left_neighbors.size(); ++i)
					left_neighbors[i].GetRect().right += left;
				for (int j= 0; j < right_neighbors.size(); ++j)
					right_neighbors[j].GetRect().left -= right;
			}
		}
		else if (in_line == 1)	// left neighbors in line with 'wp'?
		{
			for (int i= 0; i < left_neighbors.size(); ++i)
				left_neighbors[i].GetRect().right += space;
		}
		else if (in_line == 2)	// right neighbors in line with 'wp'?
		{
			for (int i= 0; i < right_neighbors.size(); ++i)
				right_neighbors[i].GetRect().left -= space;
		}
		else
		{
			ASSERT(false);
		}

		if (in_line & 1)	// left neighbors
			wp.SetNeighbor(Find(left_neighbors[0]), SnapView::INSERT_RIGHT);
		else
			wp.SetNeighbor(Find(right_neighbors[0]), SnapView::INSERT_LEFT);
		wp.last_size_ = space;

		return in_line;
	}
	return 0;
}


bool SnapFrame::WndPosVector::RemoveWindow(WndPos& wp, const WndPosVector& border)
{
	wp.position_flags_ = 0;
	CRect rect= wp.GetRect();

	// check rightmost
	if (wp.GetRect().right == border[0].GetRect().left)
		wp.position_flags_ |= WndPos::RIGHT;

	// check bottommost
	if (wp.GetRect().bottom == border[1].GetRect().top)
		wp.position_flags_ |= WndPos::BOTTOM;

	// check left
	if (wp.GetRect().left == 0)
		wp.position_flags_ |= WndPos::LEFT;

	// check top
	if (wp.GetRect().top == 0)
		wp.position_flags_ |= WndPos::TOP;

	if (!RemoveWindow(wp))
		return false;

//	if (wp.IsSidePane())
//		wp.last_size_ = wp.position_flags_ & WndPos::LEFT && wp.position_flags_ & WndPos::RIGHT ?
//			rect.Height() : rect.Width();

	return true;
}


bool SnapFrame::WndPosVector::RemoveWindow(WndPos& wp)
{
	if (RemoveWindowAndResize(wp))
	{
		wp.wi_.Disable();
		wp.GetRect().SetRectEmpty();
		return true;
	}
	else
	{
		TransposeVector tv(this);
		if (RemoveWindowAndResize(wp))
		{
			wp.wi_.Disable();
			wp.GetRect().SetRectEmpty();
			wp.SetNeighbor(wp.GetNeighborIndex(),
				wp.neighbor_edge_ == SnapView::INSERT_LEFT ? SnapView::INSERT_TOP : SnapView::INSERT_BOTTOM);
			return true;
		}
	}

	WndPosVector left_neighbors;
	WndPosVector right_neighbors;

	FindVertEdgeWindows(wp, left_neighbors, right_neighbors, false);

	if (right_neighbors.size() == 0)
	{
		ASSERT(false);
		return false;
	}

	int width= wp.GetRect().Width();
	wp.wi_.Disable();
	CSize temp_size= wp.minimal_size_;
	wp.minimal_size_ = CSize(0, 0);		// so it can be resized to zero width

	int resized= HorzResizeWindows(right_neighbors[0], -width, false);

	ASSERT(resized == -width);

	wp.last_size_ = width; //wp.GetRect().Width();
	wp.GetRect().SetRectEmpty();
	wp.minimal_size_ = temp_size;
	wp.SetNeighbor(Find(right_neighbors[0]), SnapView::INSERT_RIGHT);

	// FitIntoRect...

	return true;
}


bool SnapFrame::WndPosVector::InsertWindow(WndPos& wp, WndPos& next_to, int width, bool left_edge)
{
	WndPosVector neighbors;

	if (width < wp.GetMinSize())
		width = wp.GetMinSize();

	if (left_edge)
	{
//		if (FindWindowsInLine(next_to, neighbors, false))
//		{
//		}
//		else
		{
			// check how much 'next_to' can be shrunk (while left edge is moved to the right)
			int max_resize= next_to.GetMaxResize(true);

			if (max_resize < width)	// limited space available?
				width = std::max(max_resize, wp.GetMinSize());

			CPoint top_left= next_to.GetRect().TopLeft();
			HorzResize(next_to, -width, false);

			wp.GetRect().SetRect(top_left.x, top_left.y, next_to.GetRect().left, next_to.GetRect().bottom);
			ASSERT(wp.GetRect().Width() >= wp.GetMinSize());
			wp.wi_.Enable();
		}
	}
	else
	{
//		if (FindWindowsInLine(next_to, neighbors, true))
//		{
//		}
//		else
		{
			// check how much 'next_to' can be shrunk (while right edge is moved to the left)
			int max_resize= CalcMaxHorzResize(next_to, true, true);

			if (max_resize < wp.GetMinSize())	// not enough space?
			{
				// panes on the right side of 'next_to' have to be shifted right to make some extra space
				WndPosVector neighbors;
				FindRightNeighbors(next_to, neighbors);
				if (neighbors.size() == 0)
				{
					ASSERT(false);	// at least fake border pane should be on the right side
					return false;
				}
				HorzResizeWindows(neighbors[0], -wp.GetMinSize(), false);

				max_resize = CalcMaxHorzResize(next_to, true, true);
			}

			if (max_resize < width)	// limited space available?
				width = std::max(max_resize, wp.GetMinSize());

			ASSERT(width >= wp.GetMinSize());

			CPoint bottom_right= next_to.GetRect().BottomRight();
			HorzResize(next_to, width, false);

			wp.GetRect().SetRect(next_to.GetRect().right, next_to.GetRect().top, bottom_right.x, bottom_right.y);
			ASSERT(wp.GetRect().Width() >= wp.GetMinSize());
			wp.wi_.Enable();
		}
	}

	return true;
}


// check layout validity
//
bool SnapFrame::WndInfoVector::CheckLayout() const
{
	for (int i= 0; i < size(); ++i)
	{
		const WndInfo& w1= (*this)[i];

		if (w1.IsVisible())
			if (w1.pos_rect_.Width() <= 0 || w1.pos_rect_.Height() <= 0)
			{
				ASSERT(false);	// bogus dimensions
				return false;
			}

		for (int j= i + 1; j < size(); ++j)
		{
			const WndInfo& w2= (*this)[j];

			CRect rect;
			if (rect.IntersectRect(w1.pos_rect_, w2.pos_rect_))
			{
				ASSERT(false);
				return false;	// windows overlap
			}
		}
	}

	return true;
}


// get pane locations and transform them into real windows position
//
void SnapFrame::WndInfoVector::Reposition(CWnd* parent, int rightmost_pos, int bottommost_pos)
{
	if (!CheckLayout())
		return;

	CRect frame_rect;
	parent->GetClientRect(frame_rect);
	int right_border= frame_rect.right / g_GRID.cx;
	int bottom_border= frame_rect.bottom / g_GRID.cx;

	// thanks to this inflation border half separators will be hidden
	CSize bar_size= SnapView::GetBarThickness();
	frame_rect.InflateRect(bar_size.cx, bar_size.cy, bar_size.cx, bar_size.cy);

	HDWP dwp= ::BeginDeferWindowPos(static_cast<int>(size()));
	if (dwp == 0)
	{
		ASSERT(false);
		return;
	}

	for (size_t i= 0; i < size(); ++i)
	{
		WndInfo& w= (*this)[i];

		if (w.wnd_ == 0)		// skip fake windows
			continue;

		CRect rect= w.pos_rect_;
		if (rect.left == 0)
			rect.left = frame_rect.left;
		else
			rect.left *= g_GRID.cx;
		if (rect.right == rightmost_pos && right_border == rightmost_pos)
			rect.right = frame_rect.right;
		else
			rect.right *= g_GRID.cx;

		if (rect.top == 0)
			rect.top = frame_rect.top;
		else
			rect.top *= g_GRID.cy;
		if (rect.bottom == bottommost_pos && bottom_border == bottommost_pos)
			rect.bottom = frame_rect.bottom;
		else
			rect.bottom *= g_GRID.cy;

		CRect wnd_rect;
		if (!::IsWindow(w.wnd_->m_hWnd))
			continue;

		w.wnd_->GetWindowRect(wnd_rect);
		parent->ScreenToClient(wnd_rect);
		if (rect != wnd_rect)
			dwp = ::DeferWindowPos(dwp, *w.wnd_, 0, rect.left, rect.top,
				rect.Width(), rect.Height(), SWP_NOZORDER | SWP_NOACTIVATE);
	}

	::EndDeferWindowPos(dwp);
}

///////////////////////////////////////////////////////////////////////////////

void SnapFrame::WndPosVector::TransposeVect()
{
	for (int i= 0; i < size(); ++i)
		(*this)[i].Transpose();
}

void SnapFrame::WndPos::Transpose()
{
	LONG temp= wi_.pos_rect_.top;
	wi_.pos_rect_.top = wi_.pos_rect_.left;
	wi_.pos_rect_.left = temp;

	temp = wi_.pos_rect_.bottom;
	wi_.pos_rect_.bottom = wi_.pos_rect_.right;
	wi_.pos_rect_.right = temp;

	temp = minimal_size_.cx;
	minimal_size_.cx = minimal_size_.cy;
	minimal_size_.cy = temp;
}

CString SnapFrame::WndPos::GetTitle() const
{
	if (SnapView* view= dynamic_cast<SnapView*>(wi_.wnd_))
		return view->GetTitle();

	return CString();
}


bool SnapFrame::WndPos::IsSidePane() const
{
	// return true if any combination of exactly three flags is set

	switch (~position_flags_ & (LEFT | RIGHT | TOP | BOTTOM))
	{
	case LEFT:
	case TOP:
	case RIGHT:
	case BOTTOM:
		return true;
	default:
		return false;
	}
}


int SnapFrame::WndPos::GetMaxResize(bool limit_to_borders) const
{
	if (limit_to_borders)
	{
		if (wi_.wnd_ == 0)	// border window?
			return 0;
	}

	int max= GetRect().Width() - GetMinSize();

	if (max < 0)
		max = 0;

	return max;
}


bool SnapFrame::WndInfo::Disable() const
{
	bool change= false;
	if (wnd_)
	{
		// 'hide window' will cause other windows obscured by this one to be repainted properly
		wnd_->ShowWindow(SW_HIDE);
		// disable it
		wnd_->ModifyStyle(WS_VISIBLE, WS_DISABLED);
		change = visible_;	// detect change in visibility
		visible_ = false;
	}
	return change;
}


bool SnapFrame::WndInfo::Enable() const
{
	bool change= false;
	if (wnd_)
	{
		wnd_->ModifyStyle(WS_DISABLED, WS_VISIBLE);
		change = !visible_;	// detect change in visibility
		visible_ = true;
	}
	return change;
}


void SnapFrame::WndInfo::EnableAndShow() const
{
	if (wnd_)
	{
		if (SnapView* view= dynamic_cast<SnapView*>(wnd_))
			view->Show(true);
		wnd_->ModifyStyle(WS_DISABLED, 0);
		wnd_->ShowWindow(SW_SHOWNA);
		visible_ = true;
	}
}


bool SnapFrame::WndInfo::IsVisible() const
{
	if (wnd_ == 0)
		return false;
	return visible_;
}


void SnapFrame::WndInfo::Resize(const CRect& pos_rect) const
{
	if (wnd_)
		wnd_->SetWindowPos(0, pos_rect.left, pos_rect.top, pos_rect.Width(), pos_rect.Height(), SWP_NOZORDER);
}


void SnapFrame::WndInfo::SendShowNotification(bool show)
{
	if (SnapView* view= dynamic_cast<SnapView*>(wnd_))
		view->Show(show && visible_, true);
}


bool SnapFrame::WndInfo::SendTabChangeNotification()
{
	if (SnapView* view= dynamic_cast<SnapView*>(wnd_))
		return view->TabChange();
	return true;
}


void SnapFrame::WndInfo::OnUpdate(CView* sender, LPARAM hint, CObject* hint_ptr)
{
	struct CViewSender : CView
	{
		void SendUpdate(CView* sender, LPARAM hint, CObject* hint_ptr)
		{
			OnUpdate(sender, hint, hint_ptr);
		}
	};

//	if (wnd_)
//		if (CView* view= wnd_->GetChildView())
//			static_cast<CViewSender*>(view)->SendUpdate(sender, hint, hint_ptr);
}


// resize given window horizontally taking care of proper resizing of neighbour windows
//
int SnapFrame::WndPosVector::HorzResizeWindows(WndPos& wp, int steps, bool limit_to_borders)
{
	if (steps == 0)
		return 0;

	bool left= steps > 0;		// resizing direction (<- left or -> right)

	// calculate max allowed horizontal shrink
	int max_total_resize= CalcMaxHorzResizeOfWindows(wp, left, limit_to_borders);

	if (max_total_resize == 0)
		return 0;	// windows on the path have minimum size and cannot be shrunk

	if (abs(steps) > max_total_resize)
		steps = steps > 0 ? max_total_resize : -max_total_resize;

	int edge_new_location= left ? wp.GetRect().right - steps : wp.GetRect().left - steps;

	// edge neigboring windows (from left and right side of edge)
	WndPosVector left_neighbors;
	WndPosVector right_neighbors;

	FindVertEdgeWindows(wp, left_neighbors, right_neighbors, left);

	WndPosVector& neighbors= left ? left_neighbors : right_neighbors;

	int max_resize= neighbors.FindMaxWidthShrink();
	if (!left)
		max_resize = -max_resize;

	if (abs(max_resize) >= abs(steps))	// enough space to resize windows (without moving)?
	{
		// apply new layout
		ApplyNewHorzSize(edge_new_location, left_neighbors, right_neighbors);

		// finished
		return steps;
	}

//	if (max_resize > 0)
	int edge= left ? wp.GetRect().right - max_resize : wp.GetRect().left - max_resize;
	ApplyNewHorzSize(edge, left_neighbors, right_neighbors);

	if (neighbors.size() > 0)
	{
		// now move/resize edge neighbors; 'neighbors' also contains 'wp'
		for (int i= 0; i < neighbors.size(); ++i)
			HorzResize(neighbors[i], steps - max_resize, limit_to_borders);
	}
	else
	{
		HorzResize(wp, steps - max_resize, limit_to_borders);
	}

	// resize windows now as left neighbours are moved to make space
//	if (max_resize > 0)
		ApplyNewHorzSize(edge_new_location, left_neighbors, right_neighbors);

	return steps;
}


void SnapFrame::WndPosVector::HorzResize(WndPos& wp, int steps, bool limit_to_borders)
{
	int right_edge_pos= wp.GetRect().right - steps;
	int left_edge_pos= wp.GetRect().left - steps;

	bool left= steps > 0;		// resizing direction (<- left or -> right)

	if (wp.GetMaxResize(limit_to_borders) < abs(steps))		// window is too narrow?
	{
		// find all windows on the left (right) side of 'wp'
		WndPosVector neighbors;
		FindNeighbors(wp, left, neighbors);

		if (neighbors.size() == 0)
		{
			// we are not suppose to be there; max resize value has to be checked
			// before trying to resize windows, so there is a window that can be resized
			ASSERT(false);
		}
		else
		{
			if (steps > 0)
				steps -= wp.GetMaxResize(limit_to_borders);
			else
				steps += wp.GetMaxResize(limit_to_borders);

			// delegate rest of resizing process to first window on the left side;
			// resizing routine will take care of resizing all the windows

			HorzResizeWindows(neighbors[0], steps, limit_to_borders);
		}
	}

	if (left)
		wp.GetRect().right = right_edge_pos;
	else
		wp.GetRect().left = left_edge_pos;
}

// apply new edge horizontal position to pane windows
//
void SnapFrame::WndPosVector::ApplyNewHorzSize(int edge_new_location, WndPosVector& left_neighbors, WndPosVector& right_neighbors)
{
	for (int i= 0; i < left_neighbors.size(); ++i)
		left_neighbors[i].GetRect().right = edge_new_location;

	for (int j= 0; j < right_neighbors.size(); ++j)
		right_neighbors[j].GetRect().left = edge_new_location;
}

// calculate max possible horizontal resize
//
int SnapFrame::WndPosVector::CalcMaxHorzResizeOfWindows(const WndPos& wp, bool left, bool limit_to_borders) const
{
	WndPosVector left_neighbors;
	WndPosVector right_neighbors;

	// find windows sharing right 'wp' edge (on the left and on the right side)
	FindVertEdgeWindows(wp, left_neighbors, right_neighbors, left);

	WndPosVector& neighbors= left ? left_neighbors : right_neighbors;

	// if many windows share same edge check them all
	if (neighbors.size() > 0)
	{
		std::vector<int> min_resize;
		min_resize.reserve(neighbors.size());

		// calc max resize value for all the neighbors
		for (int i= 0; i < neighbors.size(); ++i)
			min_resize.push_back(CalcMaxHorzResize(neighbors[i], left, limit_to_borders));

		// and find the smallest of them
		int minimal_resize= min_resize[0];
		for (int j= 1; j < min_resize.size(); ++j)
			if (minimal_resize > min_resize[j])
				minimal_resize = min_resize[j];

		return minimal_resize;
	}
	else
	{
		// no other window shares right 'wp' window edge (on the left side)
		return CalcMaxHorzResize(wp, left, limit_to_borders);
	}
}


int SnapFrame::WndPosVector::CalcMaxHorzResize(const WndPos& wp, bool left, bool limit_to_borders) const
{
#if _DEBUG
static int counter_= 0;
if (++counter_ > 50)
{
	ASSERT(false);
	--counter_;
	return 0;
}
#endif

	// window 'wp' itself
	int max_resize= wp.GetMaxResize(limit_to_borders);

	// find all windows on the left (right) side of 'wp'
	WndPosVector neighbors;
	FindNeighbors(wp, left, neighbors);

	// if there is any delegate work to 'CalcMaxHorzResizeOfWindows', it will find them all
	if (neighbors.size() > 0)
		max_resize += CalcMaxHorzResizeOfWindows(neighbors[0], left, limit_to_borders);

#if _DEBUG
--counter_;
#endif

	return max_resize;
}


// find all the pane windows sharing same vertical edge
//
void SnapFrame::WndPosVector::FindVertEdgeWindows(const WndPos& wp, WndPosVector& left_neighbors, WndPosVector& right_neighbors, bool left) const
{
	CRect right_rect= wp.GetRect();
	CRect left_rect= wp.GetRect();

	if (left)
	{
		right_rect.left = right_rect.right;
		++right_rect.right;

		left_rect.left = left_rect.right - 1;
	}
	else
	{
		left_rect.right = left_rect.left;
		--left_rect.left;

		right_rect.right = right_rect.left + 1;
	}

	left_neighbors.clear();
	left_neighbors.reserve(32);
	right_neighbors.clear();
	right_neighbors.reserve(32);

	FindWindows(right_rect, right_neighbors);
	FindWindows(left_rect, left_neighbors);

//	if (right_neighbors.size() == 0)
//		return;

	for (;;)
	{
		bool expanded= false;

		for (int i= 0; i < right_neighbors.size(); ++i)
		{
			const WndPos& w= right_neighbors[i];
			if (right_rect.top > w.GetRect().top)
			{
				right_rect.top = w.GetRect().top;
				expanded = true;
			}
			if (right_rect.bottom < w.GetRect().bottom)
			{
				right_rect.bottom = w.GetRect().bottom;
				expanded = true;
			}
		}

		for (int j= 0; j < left_neighbors.size(); ++j)
		{
			const WndPos& w= left_neighbors[j];
			if (right_rect.top > w.GetRect().top)
			{
				right_rect.top = w.GetRect().top;
				expanded = true;
			}
			if (right_rect.bottom < w.GetRect().bottom)
			{
				right_rect.bottom = w.GetRect().bottom;
				expanded = true;
			}
		}

		left_rect.top = right_rect.top;
		left_rect.bottom = right_rect.bottom;

		left_neighbors.clear();
		left_neighbors.reserve(32);
		right_neighbors.clear();
		right_neighbors.reserve(32);

		FindWindows(left_rect, left_neighbors);
		FindWindows(right_rect, right_neighbors);

		if (!expanded)
			break;
	}
}


// find windows intersecting given rect
//
void SnapFrame::WndPosVector::FindWindows(const CRect& rect, WndPosVector& windows) const
{
	for (int i= 0; i < size(); ++i)
	{
		const WndPos& w= (*this)[i];
		CRect r;
		if (r.IntersectRect(w.GetRect(), rect))
			windows.push_back(w);
	}
}

// find neighbor windows on the left side of given pane window
//
void SnapFrame::WndPosVector::FindLeftNeighbors(const WndPos& wp, WndPosVector& neighbors) const
{
	CRect left_rect= wp.GetRect();
	left_rect.right = left_rect.left;
	--left_rect.left;

	neighbors.clear();
	neighbors.reserve(32);
	FindWindows(left_rect, neighbors);
}


// find neighbor windows on the right side of given pane window
//
void SnapFrame::WndPosVector::FindRightNeighbors(const WndPos& wp, WndPosVector& neighbors) const
{
	CRect right_rect= wp.GetRect();
	right_rect.left = right_rect.right;
	++right_rect.right;

	neighbors.clear();
	neighbors.reserve(32);
	FindWindows(right_rect, neighbors);
}

// find neighbor windows on the given side of given pane window
//
void SnapFrame::WndPosVector::FindNeighbors(const WndPos& wp, bool left, WndPosVector& neighbors) const
{
	if (left)
		FindLeftNeighbors(wp, neighbors);
	else
		FindRightNeighbors(wp, neighbors);
}


// find max horizontal resize value for list of windows
//
int SnapFrame::WndPosVector::FindMaxWidthShrink() const
{
	if (size() == 0)
		return 0;

	int max_resize= INT_MAX;

	for (int i= 0; i < size(); ++i)
	{
		const WndPos& w= (*this)[i];
		int resize= w.GetRect().Width() - w.GetMinSize();
		if (max_resize > resize)
			max_resize = resize;
	}

	return max_resize;
}


///////////////////////////////////////////////////////////////////////////////


void SnapFrame::OnSize(UINT type, int cx, int cy)
{
	CWnd::OnSize(type, cx, cy);

	if (type == SIZE_MINIMIZED)
		return;

	CRect rect;
	GetClientRect(rect);
	if (rect.IsRectEmpty())
		return;

	if (!layout_initialized_)
	{
		windows_.Copy(defaut_pane_layout_);

		if (RestoreState(registry_section_, registry_entry_))
		{
			FitIntoFrame();
			RepositionWindows();
		}
		else
			RepositionWindows(defaut_pane_layout_);

		layout_initialized_ = true;
	}
	else
	{
		Resize();
	}

	// update our parent frame - in case we are now maximized or not
//	GetMDIFrame()->OnUpdateFrameTitle(TRUE);

//	CMDIChildWnd::OnSize(type, cx, cy);
}


void SnapFrame::Resize()
{
	if (wnd_maximized_)
	{
		CRect rect= GetFrameRect();
		wnd_maximized_->SetWindowPos(0, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOZORDER | SWP_NOACTIVATE);
	}
	else
	{
		FitIntoFrame();
		RepositionWindows();
	}
}


#if 0
void SnapFrame::PanesMenu(CMenu& menu, int start_index)
{
	int separators= 2;
	for (;;)
	{
		int id= menu.GetMenuItemID(start_index);
		if (id == ID_PANE_RESTORE || id == ID_PANES_RESET ||
			id >= AFX_IDW_PANE_FIRST && id <= AFX_IDW_PANE_LAST)
			menu.RemoveMenu(start_index, MF_BYPOSITION);
		else if (id == 0 && separators > 0)
		{
			menu.RemoveMenu(start_index, MF_BYPOSITION);
			separators--;
		}
		else
			break;
	}

	if (wnd_maximized_ != 0)	// if there is maximized pane only restore cmd is available
	{
		menu.InsertMenu(start_index++, MF_BYPOSITION | MF_STRING, ID_PANE_RESTORE, _T("&Restore Panes"));
		menu.InsertMenu(start_index++, MF_BYPOSITION | MF_SEPARATOR);
	}
	else
	{
		UniqueLetter unique;

		for (int i= 0; i < windows_.size(); ++i)
		{
			const WndPos& w= windows_[i];

			if (w.IsBorderWindow())
				continue;

			String title= w.GetTitle();

			unique.SelectUniqueLetter(title);

			menu.InsertMenu(start_index++, MF_BYPOSITION | MF_STRING, AFX_IDW_PANE_FIRST + i, title.c_str());
		}

		menu.InsertMenu(start_index++, MF_BYPOSITION | MF_SEPARATOR);
		String title= _T("Default Layout");
		unique.SelectUniqueLetter(title);
		menu.InsertMenu(start_index++, MF_BYPOSITION | MF_STRING, ID_PANES_RESET, title.c_str());
		menu.InsertMenu(start_index++, MF_BYPOSITION | MF_SEPARATOR);
	}
}
#endif


void SnapFrame::PanesMenu(CPoint pos, UINT layout_cmd_id)
{
	CMenu menu;
	menu.CreatePopupMenu();

	PanesMenu(menu, layout_cmd_id);

	int cmd= menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON /*| TPM_RETURNCMD*/,
		pos.x, pos.y, GetParent());

	if (cmd == 0)
		return;
}


void SnapFrame::PanesMenu(CMenu& menu, UINT layout_cmd_id)
{
	UniqueLetter unique;

	if (wnd_maximized_ != 0)	// if there is maximized pane only restore cmd is available
	{
		menu.AppendMenu(MF_STRING | MF_BYCOMMAND, ID_PANE_RESTORE, _T("Restore Panes"));
	}
	else
	{
		for (int i= 0; i < windows_.size(); ++i)
		{
			const WndPos& w= windows_[i];

			if (w.IsBorderWindow())
				continue;

			String title= w.GetTitle();

			unique.SelectUniqueLetter(title);

			menu.AppendMenu(MF_STRING | MF_BYCOMMAND, AFX_IDW_PANE_FIRST + i, title.c_str());
		}

		menu.AppendMenu(MF_SEPARATOR);
		String title= _T("Restore Default Layout");
		unique.SelectUniqueLetter(title);
		menu.AppendMenu(MF_STRING | MF_BYCOMMAND, ID_PANES_RESET, title.c_str());
	}

	if (layout_cmd_id > 0)
	{
		menu.AppendMenu(MF_SEPARATOR);

		String title= _T("Store Current Layout...");
		unique.SelectUniqueLetter(title);
		menu.AppendMenu(MF_STRING | MF_BYCOMMAND, ID_PANES_STORE_LAYOUT, title.c_str());

		title = _T("Manage Stored Layouts");
		unique.SelectUniqueLetter(title);
		menu.AppendMenu(MF_STRING | MF_BYCOMMAND, ID_PANES_MANAGE_LAYOUTS, title.c_str());

		if (PaneLayoutsCount() > 0)
		{
			menu.AppendMenu(MF_SEPARATOR);
			PaneLayoutMenu(menu, layout_cmd_id);
		}
	}
}


void SnapFrame::OnPaneToggle(UINT id)
{
	if (WndPos* wnd= windows_.Find(id))
	{
		SnapView* view= dynamic_cast<SnapView*>(wnd->wi_.wnd_);

		if (wnd->wi_.IsVisible())		// pane visible?
			PaneClose(view);
		else
			PaneOpen(view);
	}
}

void SnapFrame::OnUpdatePaneToggle(CCmdUI* cmd_ui)
{
	if (WndPos* wnd= windows_.Find(cmd_ui->m_nID))
		cmd_ui->SetCheck(wnd->wi_.IsVisible() ? 1 : 0);
}


// apply default layout
//
void SnapFrame::OnPanesReset()
{
	RepositionWindows(defaut_pane_layout_);
}


bool SnapFrame::IsPaneOpen(SnapView* view)
{
	if (WndPos* wnd= windows_.Find(view))
		return wnd->wi_.IsVisible();		// pane visible?

	ASSERT(false);
	return false;
}


///////////////////////////////////////////////////////////////////////////////


SnapFrame::WndPosVector::WndPosVector(WndInfoVector& wnd_info)
{
	reserve(wnd_info.size());
	for (int i= 0; i < wnd_info.size(); ++i)
		push_back(wnd_info[i]);
}


void SnapFrame::WndPosVector::Copy(WndInfoVector& wnd_info)
{
	reserve(wnd_info.size());
	for (int i= 0; i < wnd_info.size(); ++i)
		push_back(wnd_info[i]);
}


void SnapFrame::WndPosVector::Copy(const std::vector<PaneLayoutInfo>& PanesInfo)
{
	if (size() >= PanesInfo.size())
	{
		for (int i= 0; i < PanesInfo.size(); ++i)
		{
			WndPos& pos= (*this)[i];
			pos.neighbor_index_ = PanesInfo[i].neighbor_index_;
			pos.neighbor_edge_ = PanesInfo[i].neighbor_edge_;
			pos.last_size_ = PanesInfo[i].last_size_;

			switch (PanesInfo[i].side_pane_position_)
			{
			case SnapView::INSERT_NONE:
				pos.position_flags_ = 0;
				break;
			case SnapView::INSERT_LEFT:
				pos.position_flags_ = WndPos::LEFT | WndPos::TOP | WndPos::BOTTOM;
				break;
			case SnapView::INSERT_RIGHT:
				pos.position_flags_ = WndPos::RIGHT | WndPos::TOP | WndPos::BOTTOM;
				break;
			case SnapView::INSERT_TOP:
				pos.position_flags_ = WndPos::TOP | WndPos::LEFT | WndPos::RIGHT;
				break;
			case SnapView::INSERT_BOTTOM:
				pos.position_flags_ = WndPos::BOTTOM | WndPos::LEFT | WndPos::RIGHT;
				break;
			default:
				ASSERT(false);
				break;
			}
		}
	}
	else
	{ ASSERT(false); }
}


bool SnapFrame::IsResizingPossible(SnapView* view, SnapView::Insert resizing_edge)
{
	ASSERT(view);

	if (wnd_maximized_)
		return false;	// no resizing while there is maximized pane

	WndPosVector& wndvect= windows_;
	WndPos* wnd= wndvect.Find(view);
	if (wnd == 0)
	{
		ASSERT(false);
		return false;
	}

	// edge neigboring windows (from left and right side of edge)
	WndPosVector left_neighbors;
	WndPosVector right_neighbors;

	switch (resizing_edge)
	{
	case SnapView::INSERT_LEFT:
		wndvect.FindLeftNeighbors(*wnd, left_neighbors);
		if (left_neighbors.IsEmpty())	// no windows on the left side, resizing impossible
			return false;				// because there is no windows to shrink on the left side
		break;

	case SnapView::INSERT_RIGHT:
		wndvect.FindRightNeighbors(*wnd, right_neighbors);
		if (right_neighbors.IsEmpty())	// no windows on the right side, resizing impossible
			return false;				// because there is no window to stretch on the right side
		if (right_neighbors[0].wi_.wnd_ == 0)	// border window?
			return false;
		break;

	case SnapView::INSERT_TOP:
		{
			TransposeVector tv(wndvect);
			wndvect.FindLeftNeighbors(*wnd, left_neighbors);
			if (left_neighbors.IsEmpty())
				return false;
		}
		break;

	case SnapView::INSERT_BOTTOM:
		{
			TransposeVector tv(wndvect);
			wndvect.FindRightNeighbors(*wnd, right_neighbors);
			if (right_neighbors.IsEmpty())
				return false;
			if (right_neighbors[0].wi_.wnd_ == 0)	// border window?
				return false;
		}
		break;

	default:
		ASSERT(false);
		return false;
	}

	return true;
}


void SnapFrame::ResizePane(SnapView* view, SnapView::Insert resizing_edge, CPoint pos)
{
	ASSERT(view);

	if (wnd_maximized_)
		return;		// no resizing while there is maximized pane

	WndPosVector& wndvect= windows_; //(pane_windows_);
	WndPos* wnd= wndvect.Find(view);
	if (wnd == 0)
	{
		ASSERT(false);
		return;
	}

	view->MapWindowPoints(this, &pos, 1);

	// determine how much steps to resize
	CRect rect= wnd->GetRect();
	pos.x = (pos.x + g_GRID.cx / 2) / g_GRID.cx;
	pos.y = (pos.y + g_GRID.cy / 2) / g_GRID.cy;

	CSize delta_size(0, 0);

	// edge neigboring windows (from left and right side of edge)
	WndPosVector left_neighbors;
	WndPosVector right_neighbors;

	bool horz_change= false;
	bool vert_change= false;

	switch (resizing_edge)
	{
	case SnapView::INSERT_LEFT:
		delta_size.cx = rect.left - pos.x;
		if (delta_size.cx == 0)
			return;

		wndvect.FindLeftNeighbors(*wnd, left_neighbors);
		if (left_neighbors.IsEmpty())	// no windows on the left side, resizing impossible
			return;						// because there is no windows to shrink on the left side
		if (delta_size.cx > 0)			// delegate resizing to first neighbor window
			wnd = &left_neighbors[0];

		horz_change = wndvect.HorzResizeWindows(*wnd, delta_size.cx, true) != 0;

		break;

	case SnapView::INSERT_RIGHT:
		delta_size.cx = rect.right - pos.x;
		if (delta_size.cx == 0)
			return;

		wndvect.FindRightNeighbors(*wnd, right_neighbors);
		if (right_neighbors.IsEmpty())	// no windows on the right side, resizing impossible
			return;						// because there is no window to stretch on the right side
		if (delta_size.cx < 0)			// delegate resizing to first neighbor window
			wnd = &right_neighbors[0];

		horz_change = wndvect.HorzResizeWindows(*wnd, delta_size.cx, true) != 0;

		break;

	case SnapView::INSERT_TOP:
		{
			delta_size.cy = rect.top - pos.y;
			if (delta_size.cy == 0)
				return;

			TransposeVector tv(wndvect);
			wndvect.FindLeftNeighbors(*wnd, left_neighbors);
			if (left_neighbors.IsEmpty())
				return;
			if (delta_size.cy > 0)
				wnd = &left_neighbors[0];

			vert_change = wndvect.HorzResizeWindows(*wnd, delta_size.cy, true) != 0;
		}
		break;

	case SnapView::INSERT_BOTTOM:
		{
			delta_size.cy = rect.bottom - pos.y;
			if (delta_size.cy == 0)
				return;

			TransposeVector tv(wndvect);
			wndvect.FindRightNeighbors(*wnd, right_neighbors);
			if (right_neighbors.IsEmpty())
				return;
			if (delta_size.cy < 0)
				wnd = &right_neighbors[0];

			vert_change = wndvect.HorzResizeWindows(*wnd, delta_size.cy, true) != 0;
		}
		break;

	default:
		ASSERT(false);
		return;
	}

	// apply new layout if something has changed
	if (horz_change || vert_change)
		RepositionWindows();
}


SnapFrame::WndPos* SnapFrame::WndPosVector::Find(CWnd* wnd)		// find given window on the list
{
	for (int i= 0; i < size(); ++i)
		if ((*this)[i].wi_.wnd_ == wnd)
			return &front() + i;

	return 0;
}

SnapFrame::WndPos* SnapFrame::WndPosVector::Find(int id)		// find given window on the list
{
	for (int i= 0; i < size(); ++i)
		if ((*this)[i].GetId() == id)
			return &front() + i;

	return 0;
}

int SnapFrame::WndPosVector::Find(const WndPos& wp)	// find given window on the list and return it's index
{
	if (wp.wi_.wnd_ == 0)
		return -1;

	for (int i= 0; i < size(); ++i)
		if ((*this)[i].wi_.wnd_ == wp.wi_.wnd_)
			return i;

	ASSERT(false);
	return -1;
}

int SnapFrame::WndPosVector::FindFirstVisible() const	// find index of first visible window
{
	for (int i= 0; i < size(); ++i)
		if ((*this)[i].wi_.IsVisible())
			return i;

	return -1;
}



// make sure windows fit into given area; shrink or stretch them as needed
//
bool SnapFrame::WndPosVector::FitIntoRect(WndPos& rightmost, WndPos& bottommost, const CRect& frame_rect)
{
	bool changed_h= false;

	int delta= rightmost.GetRect().left - frame_rect.right;
	if (delta > 0)
	{
		WndPosVector left_neighbors;
		FindLeftNeighbors(rightmost, left_neighbors);
		if (!left_neighbors.IsEmpty())
			changed_h = HorzResizeWindows(left_neighbors[0], delta, false) != 0;
	}
	else
		changed_h = HorzResizeWindows(rightmost, delta, false) != 0;

	TransposeVector tv(this);

	bool changed_w= false;

	delta = bottommost.GetRect().left - frame_rect.bottom;
	if (delta > 0)
	{
		WndPosVector left_neighbors;
		FindLeftNeighbors(bottommost, left_neighbors);
		if (!left_neighbors.IsEmpty())
			changed_w = HorzResizeWindows(left_neighbors[0], delta, false) != 0;
	}
	else
		changed_w = HorzResizeWindows(bottommost, delta, false) != 0;

	return changed_w || changed_h;
}


// fit into client area of given frame window
//
bool SnapFrame::WndPosVector::FitIntoFrame(CWnd* frame, WndPos& rightmost, WndPos& bottommost)
{
	CRect rect;
	frame->GetClientRect(rect);

	rect.left /= g_GRID.cx;
	rect.right /= g_GRID.cx;
	rect.top /= g_GRID.cy;
	rect.bottom /= g_GRID.cy;

	return FitIntoRect(rightmost, bottommost, rect);
}


// shrink windows if they are too wide/tall to fit into rect borders
//
bool SnapFrame::WndPosVector::ModifyWidth(WndPos& rightmost, int border_pos)
{
	bool changed= false;
	return changed;
}


// return true if there is any window intersecting given rect
//
bool SnapFrame::WndPosVector::FindWindow(const CRect& rect) const
{
	for (int i= 0; i < size(); ++i)
	{
		const WndPos& w= (*this)[i];
		CRect r;
		if (r.IntersectRect(w.GetRect(), rect))
			return true;
	}
	return false;
}


///////////////////////////////////////////////////////////////////////////////

CRect SnapFrame::GetFrameRect() const
{
	CRect rect;
	GetClientRect(rect);

	// thanks to this inflation border half separators will be hidden
	CSize bar_size= SnapView::GetBarThickness();
	rect.InflateRect(bar_size.cx, bar_size.cy, bar_size.cx, bar_size.cy);

	return rect;
}


// maximize pane
//
void SnapFrame::PaneMaximize(SnapView* view)
{
	ASSERT(view);
	WndPos* wnd= windows_.Find(view);
	if (wnd == 0)
	{
		ASSERT(false);
		return;
	}

	CRect rect= GetFrameRect();

	view->ModifyToolbar(true);
	pane_windows_.MaximizeWindow(wnd->wi_, rect);
	wnd_maximized_ = view;
}

// restore maximized pane
//
void SnapFrame::PaneRestore(SnapView* view)
{
	PaneRestoreRemove(view, false);
}


void SnapFrame::PaneRestoreRemove(SnapView* view, bool remove)
{
	ASSERT(view);
	WndPos* wnd= windows_.Find(view);
	if (wnd == 0)
	{
		ASSERT(false);
		return;
	}
	if (wnd_maximized_ == 0 || view != wnd_maximized_)
	{
		ASSERT(false);
		return;
	}

	pane_windows_.RestoreWindows();		// restore panes
	if (remove)
		windows_.RemoveWindow(*wnd, border_);	// remove the pane to close
	view->ModifyToolbar(false);
	wnd_maximized_ = 0;
	if (remove)
		view->Show(false);					// inform view
	FitIntoFrame();
	RepositionWindows();					// reposition windows
}


// close pane
//
void SnapFrame::PaneClose(SnapView* view)
{
	ASSERT(view);
	WndPos* wnd= windows_.Find(view);
	if (wnd == 0)
	{
		ASSERT(false);
		return;
	}

	if (!wnd->wi_.IsVisible())	// pane not visible?
		return;

	// TODO: verify pane has no PANE_NO_CLOSE flag


	// no of the visible panes (including windows covered by maximized pane)
	int vis_count= pane_windows_.CountVisible(true);

	if (vis_count < 2)
		return;	  // not enough pane windows left

	if (view == wnd_maximized_)	// closing maximized pane?
	{
		PaneRestoreRemove(view, true);
	}
	else if (wnd_maximized_ != 0)	// closing pane obscured by maximized pane?
	{
		windows_.RemoveWindow(*wnd, border_);	// remove the pane to close
		view->Show(false);					// inform view
	}
	else							// closing visible pane
	{
		if (windows_.RemoveWindow(*wnd, border_))	// remove the pane to close
		{
			view->Show(false);					// inform view
			RepositionWindows();				// reposition windows
			FitIntoFrame();
		}
	}
}

// open closed pane
//
void SnapFrame::PaneOpen(SnapView* view)
{
	ASSERT(view);
	WndPos* wnd= windows_.Find(view);
	if (wnd == 0)
	{
		ASSERT(false);
		return;
	}

	if (wnd->wi_.IsVisible())		// pane is already visible?
		return;

	int neighbor_index= wnd->GetNeighborIndex();
	SnapView::Insert edge= wnd->neighbor_edge_;
	// find a visible pane view will be inserted next to
	for (int i= 5; i >= 0; --i)
	{
		if (i == 0 || neighbor_index < 0 || neighbor_index >= windows_.size() || edge == SnapView::INSERT_NONE)
		{
			neighbor_index = windows_.FindFirstVisible();
			if (neighbor_index < 0)
			{
				ASSERT(false);	// there is no visible windows?
				return;
			}
			edge = SnapView::INSERT_LEFT;
			break;
		}
		else
		{
			// check neighbor
			if (windows_[neighbor_index].wi_.IsVisible())
			{
				// we have found visible pane
				break;
			}
			else
			{
				// check next neighbor
				neighbor_index = windows_[neighbor_index].neighbor_index_;
				edge= windows_[neighbor_index].neighbor_edge_;
			}
		}
	}

	bool inserted= false;

	if (wnd->IsSidePane())	// side pane?
	{
		CRect rect;
		GetClientRect(rect);
		ClientToGrid(rect);

		inserted = windows_.InsertWindowAtSide(*wnd, rect.Size(), right_, bottom_);
	}
	else
	{
		WndPos& next_to= windows_[neighbor_index];
		if (edge == SnapView::INSERT_LEFT || edge == SnapView::INSERT_RIGHT)
		{
			inserted = windows_.InsertWindow(*wnd, next_to, wnd->last_size_, wnd->neighbor_edge_ == SnapView::INSERT_LEFT);
		}
		else
		{
			TransposeVector tv(windows_);
			inserted = windows_.InsertWindow(*wnd, next_to, wnd->last_size_, wnd->neighbor_edge_ == SnapView::INSERT_TOP);
		}
	}

	if (inserted)
	{
		FitIntoFrame();
		RepositionWindows();
		wnd->wi_.EnableAndShow();
	}
}


void SnapFrame::WndInfoVector::MaximizeWindow(WndInfo& wp, const CRect& frame_rect)
{
	for (int i= 0; i < size(); ++i)
	{
		WndInfo& w= (*this)[i];

		if (&wp == &w)
			w.Enable();
		else
			w.Disable();
	}

	wp.Resize(frame_rect);
}

void SnapFrame::WndInfoVector::RestoreWindows()
{
	for (int i= 0; i < size(); ++i)
	{
		WndInfo& w= (*this)[i];
		if (!w.pos_rect_.IsRectEmpty())
			if (w.wnd_ && ::IsWindow(w.wnd_->m_hWnd))
				w.Enable();
	}

	// If you switch top-level tabs with multiple snap panels, you lose
	// the state of visible_ in each view; reset its value now
	SendShowNotification(true);
}


int SnapFrame::WndInfoVector::CountVisible(bool count_covered/*= false*/) const	// count visible windows
{
	int count= 0;
	for (int i= 0; i < size(); ++i)
	{
		const WndInfo& w= (*this)[i];

		if (count_covered)
		{
			if (!w.pos_rect_.IsRectEmpty() && w.wnd_ && ::IsWindow(w.wnd_->m_hWnd))
				++count;
		}
		else
		{
			if (w.IsVisible())
				++count;
		}
	}
	return count;
}


void SnapFrame::WndInfoVector::ShowAll() const			// show all windows
{
	for (int i= 0; i < size(); ++i)
	{
		const WndInfo& w= (*this)[i];
		if (!w.IsVisible() && w.wnd_)
			w.EnableAndShow();
	}
}


bool SnapFrame::WndInfoVector::OnCmdMsg(UINT id, int code, void* extra, AFX_CMDHANDLERINFO* handler_info)
{
	for (int i= 0; i < size(); ++i)
	{
		const WndInfo& w= (*this)[i];
		if (w.IsVisible() && w.wnd_ && w.wnd_->OnCmdMsg(id, code, extra, handler_info))
			return true;
	}

	return false;
}


void SnapFrame::WndInfoVector::SendShowNotification(bool show)
{
	for (int i= 0; i < size(); ++i)
		(*this)[i].SendShowNotification(show);
}


bool SnapFrame::WndInfoVector::SendTabChangeNotification()
{
	for (int i= 0; i < size(); ++i)
		if ((*this)[i].SendTabChangeNotification())
			return true;

	return false;
}


SnapView* SnapFrame::WndInfoVector::FindHelpView() const	// find context help view window
{
	for (const_iterator it= begin(); it != end(); ++it)
		if (it->wnd_ && it->wnd_->IsContextHelpWnd())
			return it->wnd_;

	return 0;
}


///////////////////////////////////////////////////////////////////////////////


void SnapFrame::OnPaneZoom()
{
	PaneZoom(0);
}

void SnapFrame::PaneZoom(SnapView* view)
{
	if (view == 0)
		view = dynamic_cast<SnapView*>(GetActiveView());

	if (view == 0 || !view->CanMaximize())
		return;

	if (wnd_maximized_ && wnd_maximized_ == view)
		PaneRestore(view);
	else if (wnd_maximized_ == 0)
		PaneMaximize(view);
}

// toggle pane: open if closed, close if opened
void SnapFrame::PaneToggle(SnapView* view)
{
	if (view == 0)
		view = dynamic_cast<SnapView*>(GetActiveView());

	if (view == 0)
		return;

	WndPos* wnd= windows_.Find(view);

	if (wnd == 0)
		return;

	if (wnd_maximized_) // && wnd_maximized_ == view)
		PaneRestore(wnd_maximized_);

	if (wnd_maximized_ == 0)
	{
		if (wnd->wi_.IsVisible())		// pane visible?
			PaneClose(view);
		else
			PaneOpen(view);
	}
}


SnapView* SnapFrame::GetActiveView()
{
	return GetActiveSnapView(); //dynamic_cast<SnapView*>(view_active_);
}

///////////////////////////////////////////////////////////////////////////////
//
// Dragging pane windows to rearrange them


void SnapFrame::CreatePaneImage(CImageList& img_list_dragged_wnd, CSize wnd_size)
{
	if (img_list_dragged_wnd.GetSafeHandle())
		img_list_dragged_wnd.DeleteImageList();

	if (wnd_size.cx == 0 && wnd_size.cy == 0)
	{
		img_list_dragged_wnd.Create(1, 1, ILC_MASK, 1, 0);
		return;
	}

	img_list_dragged_wnd.Create(wnd_size.cx, wnd_size.cy, ILC_MASK, 1, 0);
	ASSERT(img_list_dragged_wnd.GetSafeHandle() != 0);

	// create a bitmap and draw a pane image in it
	CBitmap bmp;
	CDC dc;
	dc.CreateIC(_T("DISPLAY"), NULL, NULL, NULL);
	bmp.CreateCompatibleBitmap(&dc, wnd_size.cx, wnd_size.cy);
	{
		CDC bmp_dc;
		bmp_dc.CreateCompatibleDC(&dc);
		bmp_dc.SelectObject(&bmp);
		CRect rect(CPoint(0, 0), wnd_size);
		bmp_dc.FillSolidRect(rect, ::GetSysColor(COLOR_3DFACE));
		for (int i= 0; i < 3; ++i)
		{
			bmp_dc.Draw3dRect(rect, ::GetSysColor(COLOR_3DSHADOW), ::GetSysColor(COLOR_3DSHADOW));
			rect.DeflateRect(1, 1);
		}
		rect.bottom = rect.top + 16;
		bmp_dc.FillSolidRect(rect, ::GetSysColor(COLOR_ACTIVECAPTION));
	}

	img_list_dragged_wnd.Add(&bmp, RGB(255,255,0));
	img_list_dragged_wnd.SetBkColor(CLR_NONE);
}


// create pane image and display it
//
void SnapFrame::DisplayPaneImage(CImageList& img_list_dragged_wnd, CRect pane_rect)
{
	CreatePaneImage(img_list_dragged_wnd_, pane_rect.Size());

	CRect wnd_rect;
	GetWindowRect(wnd_rect);
	CPoint start(0, 0);
	ClientToScreen(&start);
	start = wnd_rect.TopLeft() - start;

	img_list_dragged_wnd_.BeginDrag(0, start);

	img_list_dragged_wnd_.DragEnter(this, pane_rect.TopLeft());
}


// start rearranging pane windows
//
bool SnapFrame::EnterDragging(SnapView* view)
{
	if (!view)
	{
		ASSERT(false);
		return false;
	}

	if (wnd_maximized_ || pane_windows_.CountVisible() < 2)
		return false;

	wnd_dragged_ = view;

//	CRect rect;
//	view->GetClientRect(rect);
//	view->MapWindowPoints(this, &rect);
//	DisplayPaneImage(img_list_dragged_wnd_, rect);

	StartDrag(wnd_dragged_);

	bool ok= Track();

	ExitDragging(ok);

	::SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));

	return false;
}


// dragging finished
//
void SnapFrame::ExitDragging(bool rearrage)
{
	if (wnd_dragged_ == 0)
	{
		ASSERT(false);
		return;
	}

	EndDrag();

//	img_list_dragged_wnd_.DragLeave(this);
//	img_list_dragged_wnd_.EndDrag();
//	img_list_dragged_wnd_.DeleteImageList();

	if (view_with_marker_)
		view_with_marker_->EraseInsertMarker();

	if (rearrage)
		if (insert_pos_ != SnapView::INSERT_NONE)
			RepositionWindow(wnd_dragged_, view_with_marker_, insert_pos_, false);

	view_with_marker_ = 0;
}


CWnd* SnapFrame::WndInfoVector::FindPaneFromPoint(CPoint pos) const
{
	for (int i= 0; i < size(); ++i)
	{
		const WndInfo& w= (*this)[i];
		if (w.wnd_ == 0)
			continue;
		CRect rect;
		w.wnd_->GetWindowRect(rect);
		if (rect.PtInRect(pos))
			return w.wnd_;
	}
	return 0;
}


void SnapFrame::MouseMoved()
{
	CPoint pos;
	GetCursorPos(&pos);
	bool move_enabled= TrackDraggedPane(pos);

	::SetCursor(AfxGetApp()->LoadCursor(move_enabled ? IDC_MOVE_PANE : IDC_NODROP));
}


bool SnapFrame::TrackDraggedPane(CPoint pos)
{
	if (wnd_dragged_ == 0)
	{
		ASSERT(false);
		return false;
	}

	CPoint client= pos;
	ScreenToClient(&client);

	SnapView* view= 0;

	CRect client_rect;
	GetClientRect(client_rect);

	client_rect.DeflateRect(1, 1);	// facilitate "outside of a window" condition if cursor is on the edge (a must for maximized window)

	if (client_rect.PtInRect(client))
	{
		CWnd* wnd= pane_windows_.FindPaneFromPoint(pos);
		view = dynamic_cast<SnapView*>(wnd);
	}

	ASSERT(view == 0 || view->GetParent() == this);

	SnapView::Insert position= view ? view->FindInsertPos(pos) : SnapView::INSERT_NONE;
	SnapView::Insert original= insert_pos_;

	bool change= false;

	if (view_with_marker_)
	{
		if (view != view_with_marker_)
		{
//			img_list_dragged_wnd_.DragLeave(this);
//			view_with_marker_->EraseInsertMarker();
//			GetMarkerRect
//			ScreenDraw();
			change = true;
		}
		view_with_marker_ = 0;
	}

	if (view && view != wnd_dragged_)
	{
		if (view->IsMarkerChanged(position))
		{
//			if (!change)
//				img_list_dragged_wnd_.DragLeave(this);
//			view->DrawInsertMarker(position);
//			ScreenDraw();
			change = true;
		}
		view_with_marker_ = view;
		insert_pos_ = position;
	}
	else
		insert_pos_ = SnapView::INSERT_NONE;

	if (view == 0)
	{
//		CRect client_rect;
//		GetClientRect(client_rect);

		const int MARGIN= 20;
		CRect rect= client_rect;
		rect.InflateRect(MARGIN, MARGIN);

		if (!client_rect.PtInRect(client) && rect.PtInRect(client))
		{
			// mouse cursor is outside client area on the perimeter of client rectangle;
			// treat such position of dragged window as a request to dock it to the side
			// of the client rectangle

			if (client.x < client_rect.left)
				insert_pos_ = SnapView::INSERT_LEFT;
			else if (client.x >= client_rect.right)
				insert_pos_ = SnapView::INSERT_RIGHT;
			else if (client.y < client_rect.top)
				insert_pos_ = SnapView::INSERT_TOP;
			else if (client.y >= client_rect.bottom)
				insert_pos_ = SnapView::INSERT_BOTTOM;

			ASSERT(insert_pos_ != SnapView::INSERT_NONE);
		}
		else
			insert_pos_ = SnapView::INSERT_NONE;

		if (!change && original != insert_pos_)
		{
//			img_list_dragged_wnd_.DragLeave(this);
			change = true;
		}
	}

	if (change)
	{
//		img_list_dragged_wnd_.EndDrag();

		CRect rect(0,0,0,0);

		if (insert_pos_ != SnapView::INSERT_NONE)
		{
			rect = RepositionWindow(wnd_dragged_, view_with_marker_, insert_pos_, true);
			GridToClient(rect);
		}
		else if (view && view == wnd_dragged_)
		{
			view->GetClientRect(rect);
			view->MapWindowPoints(this, &rect);
//			DisplayPaneImage(img_list_dragged_wnd_, rect);
		}

//		DisplayPaneImage(img_list_dragged_wnd_, rect);
		if (!rect.IsRectEmpty())
		{
			ClientToScreen(&rect);
			MoveTo(rect);
		}
	}

	return insert_pos_ != SnapView::INSERT_NONE;
}


// cancel dragging pane
//
void SnapFrame::CancelDraggingPane()
{
	ExitDragging(false);
}


///////////////////////////////////////////////////////////////////////////////
//
// Saving and restoring pane windows layout and visibility


void SnapFrame::SaveState(std::vector<BYTE>& state)
{
	state.clear();

	if (windows_.size() == 0)
		return;

	state.resize(SavedData::GetSize(static_cast<int>(windows_.size())));
	SavedData* data= reinterpret_cast<SavedData*>(&state.front());

	data->version_ = 0x0001;
	data->count_ = static_cast<WORD>(windows_.size());
	data->CopyFrom(windows_);
}


// save current layout and visibility of pane windows
//
void SnapFrame::SaveState(const TCHAR* section, const TCHAR* entry)
{
	if (windows_.size() == 0)
		return;

	std::vector<BYTE> buffer;
	SaveState(buffer);

	AfxGetApp()->WriteProfileBinary(section, entry, &buffer.front(), static_cast<int>(buffer.size()));
}


bool SnapFrame::RestoreState(const std::vector<BYTE>& state, bool notify_views)
{
	if (windows_.size() == 0)
	{
		ASSERT(false);
		return false;
	}

	if (state.empty())
		return false;

	const SavedData* data= reinterpret_cast<const SavedData*>(&state.front());

	if (state.size() > sizeof(SavedData) && data->count_ == windows_.size() && data->version_ == 0x0001)
	{
		data->CopyTo(windows_, notify_views);
		if (pane_windows_.CheckLayout())
			return true;
	}

	return false;
}


struct MemDisposer
{
	MemDisposer(BYTE* array) : array_(array)
	{}

	~MemDisposer()
	{
		if (array_)
			delete [] array_;
	}

	BYTE* array_;
};


// load pane layout and visibility info
//
bool SnapFrame::RestoreState(const TCHAR* section, const TCHAR* entry)
{
	if (windows_.size() == 0)
	{
		ASSERT(false);
		return false;
	}

	BYTE* data= 0;
	UINT bytes= 0;

	if (!AfxGetApp()->GetProfileBinary(section, entry, &data, &bytes) || data == 0 || bytes == 0)
		return false;

	MemDisposer dispose(data);

	SavedData* saved_data= reinterpret_cast<SavedData*>(data);

	bool result= false;

	if (bytes > sizeof(SavedData) && saved_data->count_ == windows_.size() && saved_data->version_ == 0x0001)
	{
		saved_data->CopyTo(windows_);
		if (pane_windows_.CheckLayout())
			result = true;
	}

	return result;
}


void SnapFrame::SavedData::CopyTo(WndPosVector& dst, bool notify_views/*= false*/) const
{
	if (dst.size() != count_)
		return;

	for (int i= 0; i < dst.size(); ++i)
	{
		WndPos& wp= dst[i];
		wp.GetRect().SetRect(status_[i].left_, status_[i].top_,
			status_[i].right_, status_[i].bottom_);

		wp.neighbor_edge_ = static_cast<SnapView::Insert>(status_[i].neighbor_edge_);
		wp.neighbor_index_ = status_[i].neighbor_index_;
		wp.last_size_ = status_[i].last_size_;
		wp.position_flags_ = status_[i].last_location_flags_;

		if (wp.wi_.wnd_ != 0)
			if (status_[i].visible_)
			{
				if (wp.wi_.Enable() && notify_views)
					wp.wi_.SendShowNotification(true);
			}
			else
			{
				if (wp.wi_.Disable() && notify_views)
					wp.wi_.SendShowNotification(false);
			}
	}
}


void SnapFrame::SavedData::CopyFrom(const WndPosVector& src)
{
	if (src.size() != count_)
		return;

	for (int i= 0; i < src.size(); ++i)
	{
		const WndPos& wp= src[i];
		status_[i].SetRect(wp.GetRect());
		status_[i].visible_				= wp.wi_.visible_ ? 1 : 0;
		status_[i].neighbor_index_		= static_cast<__int16>(wp.GetNeighborIndex());
		status_[i].neighbor_edge_		= static_cast<__int8>(wp.neighbor_edge_);
		status_[i].last_size_			= static_cast<__int16>(wp.last_size_);
		status_[i].last_location_flags_	= static_cast<__int16>(wp.position_flags_);
	}
}

///////////////////////////////////////////////////////////////////////////////

// called by SnapView to set active pane
//
void SnapFrame::SetActiveSnapView(SnapView* view)
{
	if (wnd_maximized_ != 0 && wnd_maximized_ != view)	// is there maximized pane?
		return;

	active_view_ = view;
	pane_windows_.SetActiveView(view);

}


void SnapFrame::WndInfoVector::SetActiveView(SnapView* view)
{
	for (int i= 0; i < size(); ++i)
	{
		WndInfo& w= (*this)[i];
		if (w.wnd_)
			w.wnd_->Activate(view == w.wnd_);
	}
}


void SnapFrame::WndInfoVector::RemoveView(SnapView* view)
{
	for (int i= 0; i < size(); ++i)
	{
		WndInfo& w= (*this)[i];
		if (w.wnd_ == view)
			w.wnd_ = 0;
	}
}


void SnapFrame::OnDestroy()
{
	if (layout_initialized_)
	{
		if (wnd_maximized_ != 0)	// if there is maximized pane restore it
		{
//			pane_windows_.RestoreWindows();
//			wnd_maximized_ = 0;
//			FitIntoFrame();
//			RepositionWindows();
//			SaveState(registry_section_, registry_entry_);
		}
		else
			SaveState(registry_section_, registry_entry_);

		layout_list_.StoreLayouts(registry_section_, registry_entry_ + g_suffix);
	}

	active_view_ = 0;

	CFrameWnd::OnDestroy();
}


BOOL SnapFrame::OnCmdMsg(UINT id, int code, void* extra, AFX_CMDHANDLERINFO* handler_info)
{
	if (CFrameWnd::OnCmdMsg(id, code, extra, handler_info))
		return true;

//	if (CWnd* parent= GetParent())
//		if (parent->OnCmdMsg(id, code, extra, handler_info))
//			return true;

	return pane_windows_.OnCmdMsg(id, code, extra, handler_info);
}


// activate current view
//
void SnapFrame::ActivateView(bool activate)
{
	if (active_view_)
		pane_windows_.SetActiveView(activate ? active_view_ : 0);
}


// remove view window from the list
//
void SnapFrame::RemoveView(SnapView* view)
{
	pane_windows_.RemoveView(view);
}


void SnapFrame::OnUpdateFrameTitle(BOOL add_to_title)
{
	if (CFrameWnd* frame= GetParentFrame())
		frame->OnUpdateFrameTitle(add_to_title);
}


// show/hide this frame window
//
void SnapFrame::ShowFrame(bool show)
{
	if (show)
	{
		// notify view windows that frame is about to be shown
		pane_windows_.SendShowNotification(true);

		// show frame
		ShowWindow(SW_SHOW);

		// restore active view
		if (active_view_)
			SetActiveSnapView(active_view_);
		else if (!pane_windows_.empty())
			SetActiveSnapView(pane_windows_.front().wnd_);
	}
	else
	{
		// hide frame
		ModifyStyle(WS_VISIBLE, WS_DISABLED);

		// notify view windows that frame has been just hidden
		pane_windows_.SendShowNotification(false);
	}
}

SnapView* SnapFrame::GetActiveSnapView()
{
	return active_view_;
}


// display context help in help pane
//
void SnapFrame::DisplayContextHelp(const TCHAR* ctx_help_topic)
{
	if (ctx_help_topic == 0)
		return;


}

// display context help for a given view window
//
void SnapFrame::PaneContextHelp(SnapView* view)
{
#if 0
	ASSERT(view);
	WndPos* wnd= windows_.Find(view);
	if (wnd == 0)
	{
		ASSERT(false);
		return;
	}

	CString topic= view->GetContextHelpTopic();

	if (topic.IsEmpty())
		return;			// there's no help for this view...

	// if there's maximized view window restore it, so help will be visible
	if (wnd_maximized_ != 0)
		PaneRestore(wnd_maximized_);

	SnapView* ctx_help= pane_windows_.FindHelpView();

	if (ctx_help == 0)
	{
		ASSERT(false);
		return;			// there's no help pane window in pane layout!
	}

	// display new topic
	ctx_help->DisplayHelp(topic);

	// make sure help's visible
	PaneOpen(ctx_help);
#endif
}


// send tab (snap frame) change notification
bool SnapFrame::SendTabChangeNotification()
{
	return pane_windows_.SendTabChangeNotification();
}


// add current pane layout to the list of maintained layouts
void SnapFrame::AddCurrentLayout(const TCHAR* name)
{
	if (!CanSaveLayout())
	{
		ASSERT(false);
		return;
	}

	std::vector<BYTE> state;

	// save current layout and visibility of pane windows to the vector
	SaveState(state);

	layout_list_.AddReplaceLayout(this, name, state);
}


// remove a layout
void SnapFrame::RemoveLayout(int index)
{
	layout_list_.RemoveLayout(index);
}


void SnapFrame::CLayoutList::StoreLayouts(const TCHAR* reg_entry, const TCHAR* reg_key)
{
	std::vector<BYTE> buffer;
	SerializeState(buffer);
	AfxGetApp()->WriteProfileBinary(reg_entry, reg_key, &buffer.front(), static_cast<int>(buffer.size()));
}


size_t SnapFrame::LayoutData::SizeOf() const
{
	return sizeof(DWORD) + name_.GetLength() * sizeof(TCHAR) +
		sizeof(frame_rect_) + sizeof(flags_) +
		sizeof(DWORD) + state_.size();
}


void SnapFrame::LayoutData::Store(MemWriter& mem) const
{
	mem << name_;
	mem << frame_rect_;
	mem << flags_;
	mem << state_;
}


void SnapFrame::LayoutData::Restore(MemReader& mem)
{
	mem >> name_;
	mem >> frame_rect_;
	mem >> flags_;
	mem >> state_;
}


void SnapFrame::CLayoutList::SerializeState(std::vector<BYTE>& state) const
{
	const_iterator end= this->end();

	DWORD version= 0x1000;
	DWORD count= static_cast<DWORD>(size());
	size_t size= sizeof version + sizeof count;

	for (const_iterator it= begin(); it != end; ++it)
		size += it->SizeOf();

	state.resize(size);

	MemWriter mem(&state.front());

	mem << version;
	mem << count;

	for (const_iterator it= begin(); it != end; ++it)
		it->Store(mem);
}


void SnapFrame::CLayoutList::RestoreLayouts(const TCHAR* reg_entry, const TCHAR* reg_key)
{
	try
	{
		BYTE* data= 0;
		UINT bytes= 0;

		if (!AfxGetApp()->GetProfileBinary(reg_entry, reg_key, &data, &bytes) || data == 0 || bytes == 0)
			return;

		MemDisposer dispose(data);

		MemReader mem(data);

		DWORD version= 0;
		mem >> version;

		if (version != 0x1000)
		{
			ASSERT(false);
			return;
		}

		DWORD count= 0;
		mem >> count;

		clear();
		resize(count);

		for (int i= 0; i < count; ++i)
			(*this)[i].Restore(mem);
	}
	catch (...)
	{
		AfxMessageBox(_T("Error restoring layout information."), MB_OK);
	}
}


void SnapFrame::CLayoutList::AddLayout(CWnd* frame, const TCHAR* name, std::vector<BYTE>& state)
{
	push_back(LayoutData());
	LayoutData& ld= back();

	WINDOWPLACEMENT wp;
	frame->GetWindowPlacement(&wp);

	ld.name_ = name;
	ld.frame_rect_ = wp.rcNormalPosition;
	ld.flags_ = wp.showCmd == SW_SHOWMAXIMIZED;
	ld.state_.swap(state);
}


void SnapFrame::CLayoutList::AddReplaceLayout(CWnd* frame, const TCHAR* name, std::vector<BYTE>& state)
{
	iterator it= FindLayoutPos(name);
	if (it != end())
	{
		WINDOWPLACEMENT wp;
		frame->GetWindowPlacement(&wp);

		it->name_ = name;
		it->frame_rect_ = wp.rcNormalPosition;
		it->flags_ = wp.showCmd == SW_SHOWMAXIMIZED;
		it->state_.swap(state);
	}
	else
		AddLayout(frame, name, state);
}


SnapFrame::CLayoutList::iterator SnapFrame::CLayoutList::FindLayoutPos(const TCHAR* name)
{
	for (iterator it= begin(); it != end(); ++it)
		if (it->name_ == name)
			return it;
	return end();
}


SnapFrame::LayoutData* SnapFrame::CLayoutList::GetLayout(UINT index)
{
	if (index < size())
		return &(*this)[index];
	else
		return 0;
}


SnapFrame::LayoutData* SnapFrame::CLayoutList::FindLayout(const TCHAR* name)
{
	iterator it= FindLayoutPos(name);
	return it != end() ? &*it : 0;
}


void SnapFrame::CLayoutList::RemoveLayout(int index)
{
	if (index < size() && index >= 0)
		erase(begin() + index);
}


void SnapFrame::CLayoutList::RemoveLayout(const TCHAR* name)
{
	iterator it= FindLayoutPos(name);
	if (it != end())
		erase(it);
}

// append menu items for each layout entry
void SnapFrame::CLayoutList::BuildMenu(CMenu& menu, UINT cmd_id)
{
	const size_t count= size();
	for (size_t i= 0; i < count; ++i)
	{
		CString item= (*this)[i].name_;
		if (i < 12)
		{
			item += _T("\tShift+F");
			if (i >= 9)
				item += _T('1');
			TCHAR accel= i >= 9 ? TCHAR('0' + i - 9) : TCHAR('1' + i);
			item += accel;
		}
		menu.AppendMenu(MF_STRING, static_cast<int>(cmd_id + i), item);
	}
}

void SnapFrame::CLayoutList::BuildList(CListBox& listbox)
{
	listbox.ResetContent();

	const size_t count= size();
	listbox.InitStorage(static_cast<int>(count), 32);

	for (size_t i= 0; i < count; ++i)
		listbox.AddString((*this)[i].name_);
}


void SnapFrame::PaneLayoutMenu(CMenu& menu, UINT cmd_id)
{
	layout_list_.BuildMenu(menu, cmd_id);
}


void SnapFrame::PaneLayoutList(std::vector<std::pair<String, int>>& names)
{
	layout_list_.BuildList(names);
}

// sync internal copy with names
void SnapFrame::PaneLayoutSync(const std::vector<std::pair<String, int>>& names)
{
	layout_list_.Sync(names);
}


void SnapFrame::CLayoutList::BuildList(std::vector<std::pair<String, int>>& names)
{
	const size_t count= size();
	names.clear();
	names.reserve(count);

	for (size_t i= 0; i < count; ++i)
		names.push_back(make_pair(String((*this)[i].name_), static_cast<int>(i)));
}


void SnapFrame::CLayoutList::Sync(const std::vector<std::pair<String, int>>& names)
{
	if (names.empty())
	{
		clear();
		return;
	}

	ASSERT(names.size() <= size());	// cannot be more

	std::vector<std::pair<String, int>>::const_iterator it= names.begin();

	size_t count= size();

	for (size_t i= 0, j= 0; i < count; ++i)
	{
		if (it != names.end())
			if (it->second == i)
			{
				LayoutData& data= (*this)[j];

				// update name
				if (data.name_ != it->first.c_str())
					data.name_ = it->first.c_str();

				++it;
				++j;

				continue;
			}

		erase(begin() + j);
	}
}


void SnapFrame::PaneLayoutList(CListBox& listbox)
{
	layout_list_.BuildList(listbox);
}


// restore pane layout (from layout_list_)
void SnapFrame::RestorePaneLayout(UINT index)
{
	if (LayoutData* layout= layout_list_.GetLayout(index))
	{
		// if there's maximized view window restore it first
		if (wnd_maximized_ != 0)
			PaneRestore(wnd_maximized_);

		if (RestoreState(layout->state_, true))
		{
			FitIntoFrame();
			RepositionWindows();
		}
	}
}


// MFC's UpdateAllViews restricted in scope to 'this' page
//
void SnapFrame::UpdateAllViews(CView* sender, LPARAM hint, CObject* hint_ptr)
{
	const size_t count= pane_windows_.size();
	for (size_t i= 0; i < count; ++i)
		pane_windows_[i].OnUpdate(sender, hint, hint_ptr);
}


void SnapFrame::PostNcDestroy()
{}

/*
void SnapFrame::ChangeCaptionHeight(bool big)
{
	const size_t count= pane_windows_.size();
	for (size_t i= 0; i < count; ++i)
		if (SnapView* view= dynamic_cast<SnapView*>(pane_windows_[i].wnd_))
			view->ChangeCaptionHeight(big);

//	Resize();
}
*/

void SnapFrame::ResetColors(const ColorConfiguration& colors)
{
	ASSERT(colors.size() >= C_MAX_COLORS);

	const size_t count= pane_windows_.size();
	for (size_t i= 0; i < count; ++i)
		if (SnapView* view= dynamic_cast<SnapView*>(pane_windows_[i].wnd_))
		{
			view->ResetCaption(colors);
			view->SetSeparatorBaseColor(colors[C_SEPARATOR].SelectedColor());
		}
}

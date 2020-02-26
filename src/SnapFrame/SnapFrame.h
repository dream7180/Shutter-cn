/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// SnapFrame.h : interface of the SnapFrame class
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_SNAPFRAME_H__0EAED4A2_A124_45FA_B814_AB9E093CBB52__INCLUDED_)
#define AFX_SNAPFRAME_H__0EAED4A2_A124_45FA_B814_AB9E093CBB52__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "SnapView.h"
#include "DockingBar.h"
#include "DragControl.h"
#include "MemPointers.h"
class SnapFrame;
class ColorConfiguration;


struct FramePageCreateContext
{
	FramePageCreateContext(const CCreateContext& cc, int page_id)
	{
		CC_ = cc;
		page_id_ = page_id;
	}

	CCreateContext CC_;
	int page_id_;
};


struct PaneConstruction
{
	PaneConstruction(CWnd* wnd) : wnd_(wnd)
	{}

	virtual bool CreateWin(CWnd* parent) = 0;

	CWnd* wnd_;
};


enum PaneLayoutFlags { PANE_NORMAL= 0, PANE_HIDDEN= 1, PANE_NO_CLOSE= 2, PANE_NO_MAXIMIZE= 4 };
enum PaneEdgeFlag { EDGE_NONE, EDGE_LEFT, EDGE_RIGHT, EDGE_TOP, EDGE_BOTTOM };

struct PaneLayoutInfo
{
	PaneLayoutInfo(PaneConstruction& construction, const CRect& rect, const TCHAR* pane_title, /*const TCHAR* ctx_help_topic,*/
		UINT pane_flags, int neighbor_index, PaneEdgeFlag neighbor_edge, PaneEdgeFlag side_pane_position, int last_size= 0)
		: view_construction_(construction), pane_location_rect_(rect),
		  pane_title_(pane_title), ctx_help_topic_(0), pane_flags_(pane_flags),
		  neighbor_index_(neighbor_index), neighbor_edge_(static_cast<SnapView::Insert>(neighbor_edge)),
		  side_pane_position_(static_cast<SnapView::Insert>(side_pane_position)), last_size_(last_size)
	{}

	PaneConstruction& view_construction_;
	CRect pane_location_rect_;
	const TCHAR* pane_title_;
	const TCHAR* ctx_help_topic_;
	UINT pane_flags_;
	int neighbor_index_;
	SnapView::Insert neighbor_edge_;
	SnapView::Insert side_pane_position_;
	int last_size_;

	PaneLayoutInfo& operator = (const PaneLayoutInfo&)	{ throw 1; }	// so it compiles with VC 7.1's STL
};


struct PaneLayoutInfoArray
{
	PaneLayoutInfoArray() : pane_layout_(0), count_(0),
		registry_section_(0), registry_entry_(0), page_tab_name_(0), icon_index_(0)
	{}

	PaneLayoutInfoArray(const PaneLayoutInfo* pane_layout, int count, const TCHAR* section, const TCHAR* entry, const TCHAR* name, int icon_index)
		: pane_layout_(pane_layout), count_(count),
		  registry_section_(section), registry_entry_(entry), page_tab_name_(name), icon_index_(icon_index)
	{
		ASSERT(count > 0);		// pane layout info expected
		ASSERT(!::IsBadReadPtr(pane_layout_, count * sizeof pane_layout_[0]));
	}

	const PaneLayoutInfo* pane_layout_;
	int count_;
	int icon_index_;	// 0 = hand; 1 = funnel; 2 = folder; 3 = dots; 4 = calculator; 5 = document; 6 = more dots
	const TCHAR* page_tab_name_;
	const TCHAR* registry_section_;
	const TCHAR* registry_entry_;
};



class SnapFrame : public CFrameWnd, DragControl
{
//	DECLARE_DYNCREATE(SnapFrame)
public:
	SnapFrame();

// Attributes
public:
	bool IsResizingPossible(SnapView* view, SnapView::Insert resizing_edge);

	void ResizePane(SnapView* view, SnapView::Insert resizing_edge, CPoint pos);

	// frame title (displayed in tab)
	CString GetTabTitle() const;

	// icon index (one displayed in tab)
	int GetIconIndex() const				{ return icon_index_; }

	// returns true if pane layout can be saved
	bool CanSaveLayout() const				{ return wnd_maximized_ == 0; }

	// user identifier of frame page
	int GetPageId() const					{ return page_id_; }

	// count of pane layouts
	int PaneLayoutsCount() const			{ return layout_list_.GetCount(); }

	// get snap view
	SnapView* GetSnapView(UINT index) const	{ return index < pane_windows_.size() ? pane_windows_[index].wnd_ : 0; }

	// count of snap views
	UINT GetSnapViewCount() const			{ return static_cast<UINT>(pane_windows_.size()); }

	// true if pane is open and visible
	bool IsPaneOpen(SnapView* view);

// Operations
public:
	// create snapper frame and pane windows
	bool Create(CWnd* parent, const TCHAR* title, int id); //,
		//const PaneLayoutInfoArray& PanesInfo, FramePageCreateContext* context);

	// create pane windows
	bool CreatePanes(const PaneLayoutInfoArray& PanesInfo, FramePageCreateContext* context);

	// maximize pane
	void PaneMaximize(SnapView* view);
	// restore maximized pane
	void PaneRestore(SnapView* view);
	// close pane
	void PaneClose(SnapView* view);
	// zoom pane: restore if maximized, maximize otherwise
	void PaneZoom(SnapView* view);
	// available panes
	void PanesMenu(CPoint pos, UINT layout_cmd_id);
	void PanesMenu(CMenu& menu, UINT layout_cmd_id);
//	void PanesMenu(CMenu& menu, int start_index);
	// open closed pane
	void PaneOpen(SnapView* view);
	// display context help
	void PaneContextHelp(SnapView* view);
	// toggle pane: open if closed, close if opened
	void PaneToggle(SnapView* view);

	// start rearranging pane windows
	bool EnterDragging(SnapView* view);
	// dragging finished
	void ExitDragging(bool rearrage);
	// cancel dragging pane
	void CancelDraggingPane();

//	void SetActiveView(SnapView* view_new, BOOL notify);
	// called by SnapView to set active pane
	void SetActiveSnapView(SnapView* view);
	// activate current view
	void ActivateView(bool activate);
	// remove view window from the list
	void RemoveView(SnapView* view);

	// show/hide this frame window
	void ShowFrame(bool show);

	// get the active snap view
	SnapView* GetActiveSnapView();

	// display context help in help pane
	void DisplayContextHelp(const TCHAR* ctx_help_topic);

	// send tab (snap frame) change notification
	bool SendTabChangeNotification();

	// save current layout and visibility of pane windows to the vector
	void SaveState(std::vector<BYTE>& state);

	// load pane layout and visibility info from the state vector
	bool RestoreState(const std::vector<BYTE>& state, bool notify_views);

	// add current pane layout to the list of maintained layouts
	void AddCurrentLayout(const TCHAR* name);

	// remove a layout
	void RemoveLayout(int index);

	// MFC's UpdateAllViews restricted in scope to 'this' page
	void UpdateAllViews(CView* sender, LPARAM hint, CObject* hint_ptr);

	// append items to the 'menu' corresponding to the stored pane layout entries
	void PaneLayoutMenu(CMenu& menu, UINT cmd_id);

	// append items to the 'listbox' corresponding to the stored pane layout entries
	void PaneLayoutList(CListBox& listbox);

	// get list of layout names along with indexes
	void PaneLayoutList(std::vector<std::pair<String, int> >& names);

	// sync internal copy with names
	void PaneLayoutSync(const std::vector<std::pair<String, int> >& names);

	// restore pane layout (from layout_list_)
	void RestorePaneLayout(UINT index);

	// method invoked by caption window
	//void ChangeCaptionHeight(bool big);

	// update caption and separator colors after color configuration change
	void ResetColors(const ColorConfiguration& colors);
	enum Colors { C_BAR, C_ACTIVE_CAPTION, C_INACTIVE_CAPTION, C_ACTIVE_TEXT, C_INACTIVE_TEXT, C_SEPARATOR, C_BACKGROUND, C_MAX_COLORS };

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(SnapFrame)
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void RecalcLayout(BOOL notify = TRUE);
	virtual BOOL OnCmdMsg(UINT id, int code, void* extra, AFX_CMDHANDLERINFO* handler_info);
	//}}AFX_VIRTUAL
	virtual void OnUpdateFrameTitle(BOOL add_to_title);

// Implementation
public:
	virtual ~SnapFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

// Generated message map functions
protected:
	//{{AFX_MSG(SnapFrame)
	afx_msg BOOL OnEraseBkgnd(CDC* dc);
	afx_msg void OnSize(UINT type, int cx, int cy);
	afx_msg void OnPaneZoom();
	afx_msg void OnDestroy();
	afx_msg void OnPanesReset();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	void OnPaneToggle(UINT id);
	void OnUpdatePaneToggle(CCmdUI* cmd_ui);
	virtual void MouseMoved();
	bool TrackDraggedPane(CPoint pos);
	void RepositionWindows();
	void FitIntoFrame();
	void Resize();

	// save current layout and visibility of pane windows
	void SaveState(const TCHAR* section, const TCHAR* entry);
	// load pane layout and visibility info
	bool RestoreState(const TCHAR* section, const TCHAR* entry);
	// get pane window position
	//CRect GetPaneWindowPos(const TCHAR* section, const TCHAR* entry, int wnd_index);

	virtual void PostNcDestroy();

private:
	static CString wnd_class_;
	int icon_index_;
	int page_id_;
	SnapView* view_with_marker_;
	void RepositionWindows(const std::vector<PaneLayoutInfo>& panes_info);
	CRect RepositionWindow(CWnd* view, CWnd* view_next_to, SnapView::Insert pos, bool projection);
	bool CreateClientWindows();
	void PaneRestoreRemove(SnapView* view, bool remove);

	struct WndInfo
	{
		WndInfo() : wnd_(0), visible_(false), pos_rect_(-1, -1, -1, -1)
		{}
		WndInfo(SnapView* wnd, const CRect& rect) : wnd_(wnd), pos_rect_(rect), visible_(false)
		{}
		WndInfo(const WndInfo& wi) : wnd_(wi.wnd_), pos_rect_(wi.pos_rect_), visible_(wi.visible_)
		{}
		WndInfo& operator = (const WndInfo& w)
		{
			wnd_ = w.wnd_;
			pos_rect_ = w.pos_rect_;
			visible_ = w.visible_;
			return *this;
		}
		bool Disable() const;
		bool Enable() const;
		void EnableAndShow() const;
		void Resize(const CRect& pos_rect) const;
		void SendShowNotification(bool show);
		bool SendTabChangeNotification();

		bool IsVisible() const;

		void OnUpdate(CView* sender, LPARAM hint, CObject* hint_ptr);

		SnapView* wnd_;
		mutable bool visible_;
		CRect pos_rect_;

		static WndInfo empty_;
	};

	class WndInfoVector : public std::vector<WndInfo>
	{
	public:
		WndInfoVector() {}

		explicit WndInfoVector(int size) : std::vector<WndInfo>(size) {}

		void Reposition(CWnd* parent, int rightmost_pos, int bottommost_pos);

		bool CheckLayout() const;				// check layout validity

		void MaximizeWindow(WndInfo& wp, const CRect& frame_rect);
		void RestoreWindows();

		int CountVisible(bool count_covered= false) const;	// count visible windows

		void ShowAll() const;					// show all windows

		CWnd* FindPaneFromPoint(CPoint pos) const;	// window below 'pos'

		void SetActiveView(SnapView* view);

		bool OnCmdMsg(UINT id, int code, void* extra, AFX_CMDHANDLERINFO* handler_info);

		void RemoveView(SnapView* view);

		void SendShowNotification(bool show);
		bool SendTabChangeNotification();

		SnapView* FindHelpView() const;		// find context help view window
	};

	class WndPos
	{
	public:
		WndPos(const WndPos& wp)
			: wi_(wp.wi_), minimal_size_(wp.minimal_size_),
			  neighbor_index_(-1), neighbor_edge_(SnapView::INSERT_NONE),
			  last_size_(0), position_flags_(0)
		{}
		WndPos(WndInfo& wi)
			: wi_(wi), minimal_size_(12, 6),
			  neighbor_index_(-1), neighbor_edge_(SnapView::INSERT_NONE),
			  last_size_(0), position_flags_(0)
		{}
		WndPos(WndInfo& wi, bool left);
		WndPos& operator = (const WndPos&)	// for STL
		{ ASSERT(false); return *this; }

		// minimal window size
		int GetMinSize() const				{ return minimal_size_.cx; }
		int GetMaxResize(bool limit_to_borders) const;

		CRect& GetRect()					{ return wi_.pos_rect_; }
		const CRect& GetRect() const		{ return wi_.pos_rect_; }

		void Transpose();

		bool IsBorderWindow() const			{ return wi_.wnd_ == 0; }

		bool IsSidePane() const;

		CString GetTitle() const;
		int GetId() const					{ return wi_.wnd_ ? wi_.wnd_->GetDlgCtrlID() : -1; }

		int GetNeighborIndex() const		{ return neighbor_index_; }

		void SetNeighbor(int index, SnapView::Insert edge_pos)
		{ neighbor_index_ = index;  neighbor_edge_ = edge_pos; }

		WndInfo& wi_;
		CSize minimal_size_;	// minimal pane size (in grid units)
		int neighbor_index_;	// index of the neighbor window (to restore closed pane near former neighbor)
		SnapView::Insert neighbor_edge_;
		int last_size_;		// window size before closing (width or height)
		enum WndPosFlags { NONE= 0, LEFT= 1, RIGHT= 2, TOP= 4, BOTTOM= 8 };
		UINT position_flags_;

		static WndPos pos_top_wnd_;
		static WndPos pos_left_wnd_;
	};

	class WndPosVector : public std::vector<WndPos>
	{
	public:
		WndPosVector(WndInfoVector& wnd_info);

		WndPosVector() {}

		void Copy(WndInfoVector& wnd_info);
		void Copy(const std::vector<PaneLayoutInfo>& PanesInfo);

//		enum WndNeighborPos { LEFT, RIGHT, TOP, BOTTOM };

		bool IsEmpty() const  { return size() == 0; }

		// find all the pane windows sharing same vertical edge
		void FindVertEdgeWindows(const WndPos& w, WndPosVector& left_neighbors, WndPosVector& right_neighbors, bool left) const;

		// find windows intersecting given rect
		void FindWindows(const CRect& rect, WndPosVector& windows) const;
		// return true if there is any window intersecting given rect
		bool FindWindow(const CRect& rect) const;

		// resize given window horizontally taking care of proper resizing of neighbour windows
		int  HorzResizeWindows(WndPos& wp, int steps, bool limit_to_borders);
		void HorzResize(WndPos& wp, int steps, bool limit_to_borders);

		// find neighbor windows on the left side of given pane window
		void FindLeftNeighbors(const WndPos& wp, WndPosVector& neighbors) const;
		// find neighbor windows on the right side of given pane window
		void FindRightNeighbors(const WndPos& wp, WndPosVector& neighbors) const;
		// find neighbor windows on the given side of given pane window
		void FindNeighbors(const WndPos& wp, bool left, WndPosVector& neighbors) const;

		// find max horizontal resize value for list of windows
		int  FindMaxWidthShrink() const;

		// calculate max possible horizontal resize
		int  CalcMaxHorzResizeOfWindows(const WndPos& wp, bool left, bool limit_to_borders) const;
		int  CalcMaxHorzResize(const WndPos& wp, bool left, bool limit_to_borders) const;

		// apply new edge horizontal position to pane windows
		void ApplyNewHorzSize(int edge_new_location, WndPosVector& left_neighbors, WndPosVector& right_neighbors);

		// find given window on the list
		WndPos* Find(CWnd* wnd);
		WndPos* Find(int id);
		int Find(const WndPos& wp);
		int FindFirstVisible() const;		// find index of first visible window

		// make sure windows fit into given area; shrink or stretch them as needed
		bool FitIntoRect(WndPos& rightmost, WndPos& bottommost, const CRect& frame_rect);
		bool ModifyWidth(WndPos& rightmost, int border_pos);

		// fit into client area of given frame window
		bool FitIntoFrame(CWnd* frame, WndPos& rightmost, WndPos& bottommost);

		// rearrange (move, reposition) window
		bool RearrangeWindow(WndPos& wp, WndPos& next_to, SnapView::Insert edge_pos, CSize frame_size);
		bool RearrangeWindow(WndPos& wp, SnapView::Insert edge_pos, CSize frame_size, WndPos& rightmost, WndPos& bottommost);

		bool RemoveWindow(WndPos& wp, const WndPosVector& border);
		bool RemoveWindow(WndPos& wp);
		int  RemoveWindowAndResize(WndPos& wp);
		int  FindWindowsInLine(WndPos& wp, WndPosVector& left_neighbors, WndPosVector& right_neighbors);
		bool FindWindowsInLine(WndPos& wp, WndPosVector& neighbors, bool left);

		bool InsertWindow(WndPos& wp, WndPos& next_to, int width, bool left_edge);
		// insert window at one side stretched horizontally or vertically
		bool InsertWindowAtSide(WndPos& wp, CSize frame_size, WndPos& rightmost, WndPos& bottommost);
		bool InsertWindowAtSide(WndPos& wp, CRect rect, SnapView::Insert edge_pos, CSize frame_size, WndPos& rightmost, WndPos& bottommost);

		// exchange left with top and right with bottom in all window's position rects
		void TransposeVect();
	};

	class TransposeVector	// for transposing WndPos vector
	{
	public:
		TransposeVector(WndPosVector& v) : wnd_pos_(v)		{ wnd_pos_.TransposeVect(); }
		TransposeVector(WndPosVector* pv) : wnd_pos_(*pv)	{ wnd_pos_.TransposeVect(); }
		~TransposeVector()									{ wnd_pos_.TransposeVect(); }
	private:
		WndPosVector& wnd_pos_;
	};

	class SaveVector		// for saving and restoring WndPos vector
	{
	public:
		SaveVector(WndPosVector& v) : wnd_pos_(v)		{ save_.reserve(v.size()); Copy(v, save_); }
		~SaveVector()									{ Copy(save_, wnd_pos_); }
	private:
		WndPosVector& wnd_pos_;
		std::vector<CRect> save_;
		void Copy(const WndPosVector& src, std::vector<CRect>& dst);
		void Copy(const std::vector<CRect>& src, WndPosVector& dst);
	};

	friend class WndPosVector;

	void GetWndList(WndInfoVector& wnd);
	void ClientToGrid(CRect& rect);
	void GridToClient(CRect& rect);

	WndInfoVector pane_windows_;				// pane windows (real windows)
	WndInfoVector border_;					// border (fake) windows
	WndPosVector windows_;					// layout information
	WndPos right_;							// fake right border window
	WndPos bottom_;							// fake bottom border window
	std::vector<PaneLayoutInfo> defaut_pane_layout_;	// default pane layout
	CString registry_section_;
	CString registry_entry_;
	bool layout_initialized_;

	SnapView* wnd_maximized_;
	SnapView* wnd_dragged_;
	CImageList img_list_dragged_wnd_;
	SnapView::Insert insert_pos_;
	void CreatePaneImage(CImageList& img_list_dragged_wnd, CSize wnd_size);
	void DisplayPaneImage(CImageList& img_list_dragged_wnd, CRect pane_rect);

	SnapView* active_container_view_;
	SnapView* active_view_;

	struct SavedData	// for saving/restoring layout info in registry
	{
		WORD count_;
		WORD version_;
		struct Status
		{
			__int32 left_;
			__int32 top_;
			__int32 right_;
			__int32 bottom_;
			__int8  visible_;
			__int8  neighbor_edge_;
			__int16 neighbor_index_;
			__int16 last_size_;
			__int16 last_location_flags_;

			void SetRect(const CRect& rect)
			{
				left_ = static_cast<__int32>(rect.left);
				top_ = static_cast<__int32>(rect.top);
				right_ = static_cast<__int32>(rect.right);
				bottom_ = static_cast<__int32>(rect.bottom);
			}
		} status_[1];

		void CopyTo(WndPosVector& dst, bool notify_views= false) const;
		void CopyFrom(const WndPosVector& src);

		static int GetSize(int entries)	{ return sizeof SavedData + (entries - 1) * sizeof Status; }
	};

	struct LayoutData
	{
		LayoutData()
		{
			frame_rect_.SetRectEmpty();
			flags_ = 0;
		}

		CString name_;
		CRect frame_rect_;
		UINT flags_;
		std::vector<BYTE> state_;

		size_t SizeOf() const;
		void Store(MemWriter& mem) const;
		void Restore(MemReader& mem);
	};

	class CLayoutList : /*public for stupid vc6 */ std::vector<LayoutData>
	{
	public:
		CLayoutList()
		{}

		// find given layout on the list (by name)
		LayoutData* FindLayout(const TCHAR* name);

		LayoutData* GetLayout(UINT index);

		void AddReplaceLayout(CWnd* frame, const TCHAR* name, std::vector<BYTE>& state);

		void AddLayout(CWnd* frame, const TCHAR* name, std::vector<BYTE>& state);

		void RemoveLayout(const TCHAR* name);
		void RemoveLayout(int index);

		int GetCount() const		{ return static_cast<int>(size()); }

		void StoreLayouts(const TCHAR* reg_entry, const TCHAR* reg_key);
		void RestoreLayouts(const TCHAR* reg_entry, const TCHAR* reg_key);

		// append menu items for each layout entry
		void BuildMenu(CMenu& menu, UINT cmd_id);

		void BuildList(CListBox& listbox);

		void BuildList(std::vector<std::pair<String, int>>& names);

		void SerializeState(std::vector<BYTE>& state) const;

		// sync managed vector with names
		void Sync(const std::vector<std::pair<String, int>>& names);

	private:
		iterator FindLayoutPos(const TCHAR* name);
	} layout_list_;

	SnapView* GetActiveView();

	friend class CLayoutList;

	CRect GetFrameRect() const;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SNAPFRAME_H__0EAED4A2_A124_45FA_B814_AB9E093CBB52__INCLUDED_)

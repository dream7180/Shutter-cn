/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// FolderPane.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "FolderPane.h"
#include "ItemIdList.h"
#include "NewFolderDlg.h"
#include "CatchAll.h"
#include "StringConversions.h"
#include "PhotoInfo.h"
#include "PhotoCtrl.h"
#include "Config.h"
#include "GetDefaultGuiFont.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


FolderPane::FolderPane()
{
	timer_id_ = 0;
}

FolderPane::~FolderPane()
{
}


BEGIN_MESSAGE_MAP(FolderPane, PaneWnd)
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_COMMAND(ID_FOLDER_REFRESH, OnFolderRefresh)
	ON_COMMAND(ID_NEW_FOLDER, OnNewFolder)
	ON_COMMAND(ID_OPEN_FOLDER_IN_EXPLORER, OnExploreFolder)
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_NOTIFY(TVN_SELCHANGED, TREE_WINDOW_ID, OnSelChanged)
	ON_UPDATE_COMMAND_UI(ID_RECENT_FOLDERS, OnUpdateRecentFolders)
	ON_COMMAND(ID_FOLDER_UP, OnFolderUp)
	ON_UPDATE_COMMAND_UI(ID_FOLDER_UP, OnUpdateFolderUp)
	ON_NOTIFY(TBN_DROPDOWN, AFX_IDW_TOOLBAR, OnTbDropDown)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// FolderPane message handlers

bool FolderPane::Create(CWnd* parent)
{
	return Create2(parent, 0, FolderPathPtr(), false, 0);
}


class TreeDropTarget : public COleDropTarget
{
public:
	TreeDropTarget(FolderView& tree) : tree_(tree), can_accept_(false), drop_target_(0), copy_operation_(true)
	{}
	virtual ~TreeDropTarget()
	{}

	virtual DROPEFFECT OnDragEnter(CWnd* wnd, COleDataObject* data_object, DWORD key_state, CPoint point);
	virtual DROPEFFECT OnDragOver(CWnd* wnd, COleDataObject* data_object, DWORD key_state, CPoint point);
//	virtual BOOL OnDrop(CWnd* wnd, COleDataObject* data_object, DROPEFFECT dropEffect, CPoint point);
	virtual DROPEFFECT OnDropEx(CWnd* wnd, COleDataObject* data_object, DROPEFFECT dropDefault, DROPEFFECT dropList, CPoint point);
	virtual void OnDragLeave(CWnd* wnd);
	virtual DROPEFFECT OnDragScroll(CWnd* wnd, DWORD key_state, CPoint point);

private:
	FolderView& tree_;
	bool can_accept_;
	bool copy_operation_;
	HTREEITEM drop_target_;
};


DROPEFFECT TreeDropTarget::OnDragEnter(CWnd* wnd, COleDataObject* data_object, DWORD key_state, CPoint point)
{
	COleDropTarget::OnDragEnter(wnd, data_object, key_state, point);
	tree_.SelectDropTarget(0);

	can_accept_ = data_object && data_object->IsDataAvailable(CF_HDROP);

	return DROPEFFECT_NONE;
}


void TreeDropTarget::OnDragLeave(CWnd* wnd)
{
	COleDropTarget::OnDragLeave(wnd);
	tree_.SelectDropTarget(0);
//	return DROPEFFECT_NONE;
}


DROPEFFECT TreeDropTarget::OnDropEx(CWnd* wnd, COleDataObject* data_object, DROPEFFECT dropDefault, DROPEFFECT dropList, CPoint point)
{
	DROPEFFECT effect= DROPEFFECT_NONE;

	tree_.SelectDropTarget(0);

	if (data_object == 0 || !can_accept_ || drop_target_ == 0)
		return effect;

	STGMEDIUM medium;
	memset(&medium, 0, sizeof medium);

	FORMATETC fmt;
	fmt.cfFormat = CF_HDROP;
	fmt.ptd = 0;
	fmt.dwAspect = DVASPECT_CONTENT;
	fmt.lindex = -1;
	fmt.tymed = TYMED_HGLOBAL;

	if (data_object->GetData(CF_HDROP, &medium, &fmt) && medium.tymed == TYMED_HGLOBAL && medium.hGlobal != 0)
	{
		struct lock
		{
			lock(HGLOBAL mem) : mem(mem)
			{
				data = ::GlobalLock(mem);
			}

			~lock()
			{
				::GlobalUnlock(mem);
			}

			void* data;
			HGLOBAL mem;

		} mem(medium.hGlobal);

		if (void* data= mem.data)
		{
			DROPFILES* df= reinterpret_cast<DROPFILES*>(data);
			BYTE* files= reinterpret_cast<BYTE*>(data) + df->pFiles;

			String dest_path= tree_.GetPhysicalPath(drop_target_);

			if (df->fWide)
			{
				SHFILEOPSTRUCTW op;
				memset(&op, 0, sizeof op);
				op.hwnd   = tree_.m_hWnd;
				op.wFunc  = copy_operation_ ? FO_COPY : FO_MOVE;
				op.pFrom  = reinterpret_cast<wchar_t*>(files);
#ifdef _UNICODE
				dest_path.push_back(0);
				op.pTo    = dest_path.data();
#else
				std::wstring dest;
				::MultiByteToWideString(dest_path, dest, CP_ACP);
				dest.push_back(0);
				op.pTo    = dest.data();
#endif
				op.fFlags = FOF_ALLOWUNDO | FOF_WANTNUKEWARNING;
				op.fAnyOperationsAborted = false;
				op.hNameMappings = 0;
				op.lpszProgressTitle = 0;
				::SHFileOperationW(&op);
			}
			else
			{
				SHFILEOPSTRUCTA op;
				memset(&op, 0, sizeof op);
				op.hwnd   = tree_.m_hWnd;
				op.wFunc  = copy_operation_ ? FO_COPY : FO_MOVE;
				op.pFrom  = reinterpret_cast<char*>(files);
#ifdef _UNICODE
				std::string dest;
				::WideStringToMultiByte(dest_path.c_str(), dest, CP_ACP);
				dest.push_back(0);
				op.pTo    = dest.data();
#else
				dest_path.push_back(0);
				op.pTo    = dest_path.data();
#endif
				op.fFlags = FOF_ALLOWUNDO | FOF_WANTNUKEWARNING;
				op.fAnyOperationsAborted = false;
				op.hNameMappings = 0;
				op.lpszProgressTitle = 0;
				::SHFileOperationA(&op);
			}

			effect = copy_operation_ ? DROPEFFECT_COPY : DROPEFFECT_MOVE;
		}
	}

	return effect;
}


// scroll tree content as items dragged are close to the top or bottom of the window
DROPEFFECT TreeDropTarget::OnDragScroll(CWnd* wnd, DWORD key_state, CPoint point)
{
	DROPEFFECT effect= DROPEFFECT_NONE;

	CRect rect(0,0,0,0);
	tree_.GetClientRect(rect);

	CPoint p= point;
//	tree_.ScreenToClient(&p);
	if (!rect.PtInRect(p))
		return effect;

	const int MARGIN= std::max<int>(tree_.GetItemHeight() / 2, 6);
	bool scroll_up= true;

	if (p.y < rect.top + MARGIN)
		scroll_up = true;
	else if (p.y > rect.bottom - MARGIN)
		scroll_up = false;
	else
		return effect;

	if (tree_.GetStyle() & WS_VSCROLL)
	{
		const DWORD SLEEP= 50;

		if (scroll_up)
		{
			if (HTREEITEM first= tree_.GetFirstVisibleItem())
				if (HTREEITEM prev= tree_.GetPrevVisibleItem(first))
				{
					tree_.EnsureVisible(prev);
					tree_.UpdateWindow();
					::Sleep(SLEEP);
					effect = DROPEFFECT_SCROLL;
				}
		}
		else
		{
			if (HTREEITEM first= tree_.GetFirstVisibleItem())
			{
				HTREEITEM item= first;
				UINT count= tree_.GetVisibleCount();
				for (UINT i= 0; i < count; ++i)
				{
					item = tree_.GetNextVisibleItem(item);
					if (item == 0)
						break;
				}

				if (item)
				{
					tree_.EnsureVisible(item);
					tree_.UpdateWindow();
					::Sleep(SLEEP);
					effect = DROPEFFECT_SCROLL;
				}
			}
		}

	}

	return effect;
}


DROPEFFECT TreeDropTarget::OnDragOver(CWnd* wnd, COleDataObject* data_object, DWORD key_state, CPoint point)
{
	COleDropTarget::OnDragOver(wnd, data_object, key_state, point);

	if (wnd != &tree_ || !can_accept_)
		return DROPEFFECT_NONE;

	UINT flags= 0;
	HTREEITEM item= tree_.HitTest(point, &flags);

	if (item != 0 && (TVHT_ONITEM & flags) && tree_.IsPhysicalPath(item))
	{
		drop_target_ = item;

		tree_.SelectDropTarget(item);

		copy_operation_ = !!(key_state & MK_CONTROL);

		return copy_operation_ ? DROPEFFECT_COPY : DROPEFFECT_MOVE;

	}
	else
	{
		drop_target_ = 0;

		tree_.SelectDropTarget(0);
	}

	if (item != 0 && (TVHT_ONITEMBUTTON & flags))
		tree_.Expand(item, TVE_EXPAND);

	return DROPEFFECT_NONE;
}



bool FolderPane::Create2(CWnd* parent, UINT id, FolderPathPtr path, bool root, int width)
{
	if (!CWnd::Create(AfxRegisterWndClass(CS_VREDRAW | CS_HREDRAW, ::LoadCursor(NULL, IDC_ARROW)),
		0, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN, CRect(0,0,0,0), parent, -1))
		return false;

	drop_target_.reset(new TreeDropTarget(tree_wnd_));

	if (!tree_wnd_.Create(this, TREE_WINDOW_ID, path, root))
		return false;

	tree_wnd_.SetFont(&GetDefaultGuiFont());

	drop_target_->Register(&tree_wnd_);

	int cmds[]= { ID_RECENT_FOLDERS, ID_FOLDER_UP };

	tool_bar_wnd_.SetOwnerDraw(true);
	if (!tool_bar_wnd_.Create("vp", cmds, IDB_FOLDER_TB, 0, this, AFX_IDW_TOOLBAR))
		return false;

	tool_bar_wnd_.CWnd::SetOwner(this);

	SetColors();

	AddBand(&tool_bar_wnd_, this);
	//separator_.Create(&tool_bar_wnd_);
	//VERIFY(separator_.SubclassDlgItem(IDC_SEPARATOR, &tree_wnd_));

	return true;
}


void FolderPane::CaptionHeightChanged(bool big)
{
	tool_bar_wnd_.ReplaceImageList(IDB_FOLDER_TB);
	ResetBandsWidth();
}


void FolderPane::OnTbDropDown(NMHDR* nmhdr, LRESULT* result)
{
//	NMTOOLBAR* info_tip= reinterpret_cast<NMTOOLBAR*>(nmhdr);
	*result = TBDDRET_DEFAULT;

	RecentFolders();
}


void FolderPane::RecentFolders()
{
	CRect rect(0,0,0,0);
	tool_bar_wnd_.GetRect(ID_RECENT_FOLDERS, rect);
	CPoint pos(rect.left, rect.bottom);
	tool_bar_wnd_.ClientToScreen(&pos);

	on_recent_folders_(pos);
}


void FolderPane::OnUpdateRecentFolders(CCmdUI* cmd_ui)
{
	cmd_ui->Enable();
}


void FolderPane::OnFolderUp()
{
	on_folder_up_();
}

void FolderPane::OnUpdateFolderUp(CCmdUI* cmd_ui)
{
	cmd_ui->Enable(on_update_folder_up_());
}

BOOL FolderPane::OnEraseBkgnd(CDC* dc)
{
	CRect rect;
	GetClientRect(rect);
	dc->FillSolidRect(rect, g_Settings.AppColors()[AppColors::SecondarySeparator]);
	return true;
}

void FolderPane::OnSize(UINT type, int cx, int cy)
{
	PaneWnd::OnSize(type, cx, cy);
	if (tree_wnd_.m_hWnd)
		tree_wnd_.SetWindowPos(0, 0, 1, cx, cy - 1, SWP_NOZORDER | SWP_NOACTIVATE);
		//if (separator_.m_hWnd)
		//	separator_.SetWindowPos(0, 0, 0, cx, 1, SWP_NOZORDER | SWP_NOACTIVATE);
	//}
}


// current folder in tree view has changed
//
void FolderPane::OnSelChanged(NMHDR* notify_struct, LRESULT*)
{
	NMTREEVIEW* nm_tree_view= reinterpret_cast<NMTREEVIEW*>(notify_struct);

	switch (nm_tree_view->action)
	{
	case TVC_BYKEYBOARD:
		if (timer_id_)
			KillTimer(timer_id_);
		timer_id_ = SetTimer(4, 450, 0);	// 0.45 second delay
		break;

	case TVC_BYMOUSE:
		SelChangeNotify();
		break;

	case TVC_UNKNOWN:
		break;
	}
}

// time has lapsed: inform about selection change
//
void FolderPane::OnTimer(UINT_PTR id_event)
{
	PaneWnd::OnTimer(id_event);

	if (id_event == timer_id_)
	{
		KillTimer(timer_id_);
		timer_id_ = 0;
		SelChangeNotify();
	}
}


// inform view window about selection change
//
void FolderPane::SelChangeNotify()
{
//	ItemIdList idlPath;
	FolderPathPtr path= tree_wnd_.GetSelectedPath();

//	if (!tree_wnd_.GetSelectedPath(idlPath))
//		return;

	if (path && on_folder_changed_)
		on_folder_changed_(path);
}


// return path of currently selected  folder
//
//bool FolderPane::GetCurrentPath(ItemIdList& idlPath) const
//{
//	return tree_wnd_.GetSelectedPath(idlPath);
//}

FolderPathPtr FolderPane::GetCurrentPath() const
{
	return tree_wnd_.GetSelectedPath();
}

#if 0
bool FolderPane::IsPaneVisible(bool folders_pane) const
{
	if (rebar_wnd_.m_hWnd == 0 /*|| separator_wnd_.m_hWnd == 0 || task_rebar_wnd_.m_hWnd == 0*/)
		return false;

	return folders_pane ? rebar_wnd_.IsVisible() : false; //task_rebar_wnd_.IsVisible();
}


bool FolderPane::ToggleVisibility(bool folders_pane)
{
	if (rebar_wnd_.m_hWnd == 0 /*|| separator_wnd_.m_hWnd == 0 || task_rebar_wnd_.m_hWnd == 0*/)
		return false;

	// pane window contains both folders and tasks panes
	bool was_pane_visible= rebar_wnd_.IsVisible(); // || task_rebar_wnd_.IsVisible();

	if (folders_pane)
		rebar_wnd_.ToggleVisibility();
//	else
//		task_rebar_wnd_.ToggleVisibility();

	bool is_pane_visible= rebar_wnd_.IsVisible(); // || task_rebar_wnd_.IsVisible();

	Resize();

	if (was_pane_visible != is_pane_visible)
	{
		DockedPane::Show(is_pane_visible);
		return true;	// whole pane window hidden/shown
	}

	return false;
}


void FolderPane::Show(bool show_folders, bool show_tasks)
{
	rebar_wnd_.Show(show_folders);
	//task_rebar_wnd_.Show(show_tasks);
	Resize();
	bool is_pane_visible= rebar_wnd_.IsVisible(); // || task_rebar_wnd_.IsVisible();
	DockedPane::Show(is_pane_visible);
}

LRESULT FolderPane::OnTaskBarSizeChanged(WPARAM, LPARAM)
{
	Resize();
	return 0;
}
#endif


//void FolderPane::ResetPath(const ITEMIDLIST* folder_list, bool root)
//{
//	tree_wnd_.Reset(folder_list, root);
//}

void FolderPane::ResetPath(FolderPathPtr path, bool root)
{
	tree_wnd_.Reset(path, root);
}


void FolderPane::OnFolderRefresh()
{
	tree_wnd_.Refresh();
}


void FolderPane::OnExploreFolder()
{
	try
	{
		CString path = GetCurrentPhysPath();

		if (path.IsEmpty())
		{
			ItemIdList idl;

			if (!tree_wnd_.GetSelectedPath(idl))
				return;

			path = idl.GetPath();
		}

		if (path.IsEmpty())
			return;

		::ShellExecute(0, _T("explore"), path, 0, 0, SW_SHOWNORMAL);
	}
	CATCH_ALL
}


void FolderPane::OnNewFolder()
{
	try
	{
		ItemIdList idlPath;

		if (!tree_wnd_.GetSelectedPath(idlPath))
			return;

		String path= idlPath.GetPath();
		if (path.empty())
			return;

		NewFolderDlg dlg(this);

		CTime time= CTime::GetCurrentTime();
		dlg.folderName_ = time.Format(_T("%Y-%m-%d"));

		dlg.SetCreateFolderCallback(boost::bind(&CreateFolderHelperFn, _1, _2, &path));

		if (dlg.DoModal() != IDOK)
			return;

		tree_wnd_.Refresh();
	}
	CATCH_ALL
}


void FolderPane::OnDestroy()
{
	if (timer_id_)
		KillTimer(timer_id_);

	PaneWnd::OnDestroy();
}


void FolderPane::ActivatePane(bool active)
{
	if (active)
		if (IsTopParentActive())	// take the focus if this frame/view/pane is now active
			tree_wnd_.SetFocus();
}


BOOL FolderPane::IsFrameWnd() const
{
	return true;	// true so InfoPane can handle UI messages sent by ToolBarWnd
}


CString FolderPane::GetCurrentPhysPath() const
{
	ItemIdList idlPath;

	if (!tree_wnd_.GetRClickedItemPath(idlPath))
		return CString();

	return idlPath.GetPath();
}


void FolderPane::SetColors()
{
	std::vector<COLORREF> colors= g_Settings.main_wnd_colors_.Colors();

	tree_wnd_.SetBkColor(colors[PhotoCtrl::C_EDIT_BACKGND]);//colors[PhotoCtrl::C_BACKGND]);
	tree_wnd_.SetTextColor(colors[PhotoCtrl::C_TEXT]);

	tool_bar_wnd_.Invalidate();
}


void FolderPane::OptionsChanged(OptionsDlg& dlg)
{
	SetColors();
}
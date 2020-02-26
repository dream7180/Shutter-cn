/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// FileListCtrl.cpp : implementation file

#include "stdafx.h"
#include "FileListCtrl.h"
#include "ShellItem.h"
#include "ItemIdList.h"
#include "WhistlerLook.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// FileListCtrl

FileListCtrl::FileListCtrl() : timer_(0), includeFiles_(true)
{
}

FileListCtrl::~FileListCtrl()
{
}


BEGIN_MESSAGE_MAP(FileListCtrl, CListCtrl)
	ON_NOTIFY_REFLECT_EX(LVN_GETDISPINFO, OnGetDispInfo)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnDblClick)
	ON_WM_GETDLGCODE()
	ON_WM_KEYDOWN()
	ON_WM_CHAR()
	ON_WM_DESTROY()
	ON_WM_TIMER()
END_MESSAGE_MAP()


// FileListCtrl message handlers


void FileListCtrl::PreSubclassWindow()
{
	CListCtrl::PreSubclassWindow();

	ModifyStyle(0, LVS_LIST | LVS_SHAREIMAGELISTS | LVS_SINGLESEL | LVS_SHOWSELALWAYS);

	SHFILEINFO info;
	if (DWORD_PTR image_list= ::SHGetFileInfo(_T(""), NULL, &info, sizeof info, SHGFI_SMALLICON | SHGFI_PIDL | SHGFI_SYSICONINDEX))
		SendMessage(LVM_SETIMAGELIST, LVSIL_SMALL, image_list);
	if (DWORD_PTR image_list= ::SHGetFileInfo(_T(""), NULL, &info, sizeof info, SHGFI_LARGEICON | SHGFI_PIDL | SHGFI_SYSICONINDEX))
		SendMessage(LVM_SETIMAGELIST, LVSIL_NORMAL, image_list);

	SetExtendedStyle(LVS_EX_FULLROWSELECT /*| LVS_EX_LABELTIP*/);

	ASSERT(GetStyle() & LVS_OWNERDATA);	// virtual list ctrl required
}


bool FileListCtrl::Create(CWnd* parent, UINT id, DWORD styles/*= ... */)
{
	if (!CListCtrl::Create(styles | WS_CHILD | /*LVS_ICON */LVS_LIST /*LVS_REPORT*/ | LVS_SINGLESEL |
		LVS_SHOWSELALWAYS | LVS_SHAREIMAGELISTS | LVS_OWNERDATA, CRect(0,0,0,0), parent, id))
		return false;

//	InsertColumn(0, _T("Name"), LVCFMT_LEFT, 220);
//	InsertColumn(1, _T("Date"), LVCFMT_LEFT, 140);
	if (parent)
		SendMessage(WM_SETFONT, ::SendMessage(parent->m_hWnd, WM_GETFONT, 0, 0));

	return true;
}


void FileListCtrl::SetIconView()
{
	if (WhistlerLook::IsAvailable())
		SetView(LV_VIEW_ICON);
	else
	{
		ModifyStyle(LVS_TYPEMASK, LVS_ICON);
		Invalidate();
	}

	Arrange(LVA_DEFAULT);

	int count= GetItemCount();
	DeleteAllItems();
	SetItemCount(0);
	SetItemCount(count);
}

void FileListCtrl::SetListView()
{
	if (WhistlerLook::IsAvailable())
		SetView(LV_VIEW_LIST);
	else
	{
		ModifyStyle(LVS_TYPEMASK, LVS_LIST);
		Invalidate();
	}

	//	EnsureVisible(0, false);
	int count= GetItemCount();
	DeleteAllItems();
	SetItemCount(0);
	SetItemCount(count);
}


struct folders_first
{
	bool operator () (const ShellItemPtr& p1, const ShellItemPtr& p2)
	{
		return p1->IsFolder() && !p2->IsFolder();
	}
};


void FileListCtrl::SetPath(const ITEMIDLIST* idl, bool includeFiles,
							const FileFilterCallback& includeFileItem)
{
	includeFiles_ = includeFiles;
	includeFileItem_ = includeFileItem;

	DeleteAllItems();
	SetItemCount(0);

	CWaitCursor wait;

	shell_folder_ = new ShellFolderEx(*this, idl);
	folder_list_.clear();

	shell_folder_->GetSubItemList(folder_list_, includeFiles, includeFileItem);

	if (includeFiles)
		stable_sort(folder_list_.begin(), folder_list_.end(), folders_first());

	int count= static_cast<int>(folder_list_.size());
	SetItemCount(static_cast<int>(folder_list_.size()));
//SetItemCount(0);
//SetItemCount(static_cast<int>(folder_list_.size()));
//	CPoint p;
//	for (int i= 0; i < count; ++i)
//		GetItemPosition(i, &p);
//	CRect rect;
//	for (int i= 0; i < count; ++i)
//		GetItemRect(i, &rect, LVIR_LABEL);
	//TCHAR buf[MAX_PATH+2];
	//for (int i= 0; i < count; ++i)
	//	GetItemText(i, 0, buf, MAX_PATH);
//Arrange(LVA_DEFAULT);
//Invalidate();
//RedrawItems(0, count - 1);
//UpdateWindow();
////SetItemCount(0);
////SetItemCount(count);
//EnsureVisible(0, false);
//Arrange(LVA_DEFAULT);
//SendMessage(LVM_GETITEMSPACING, 0, 0);
////CSize s= GetIconSpacing();
//SetIconSpacing(-1, -1);
//Update(0);

	// select first item
	if (!folder_list_.empty())
	{
		UINT state= LVIS_SELECTED | LVIS_FOCUSED;
		SetItemState(0, state, state);
	}

	if (folder_changed_callback_)
		folder_changed_callback_(shell_folder_);
}


ShellItemPtr GetShellItem(const FolderList& folder_list, size_t index)
{
	if (index < folder_list.size())
	{
		FolderList::const_iterator it= folder_list.begin();
		advance(it, index);
		return *it;
	}
	ASSERT(false);
	return ShellItemPtr();
}


ShellFolderPtr GetShellFolderItem(const FolderList& folder_list, size_t index)
{
	if (index < folder_list.size())
	{
		FolderList::const_iterator it= folder_list.begin();
		advance(it, index);
		return ShellFolderPtr(dynamic_cast<ShellFolderEx*>(it->get()));
	}
	ASSERT(false);
	return ShellFolderPtr();
}


BOOL FileListCtrl::OnGetDispInfo(NMHDR* nmhdr, LRESULT* result)
{
	LV_DISPINFO* disp_info= reinterpret_cast<LV_DISPINFO*>(nmhdr);
	*result = 0;

	size_t line= disp_info->item.iItem;

	ShellItemPtr folder= GetShellItem(folder_list_, line);
	if (folder == 0)
		return false;

	folder->GetFullIDL();

	if ((disp_info->item.mask & LVIF_TEXT))
	{
		disp_info->item.pszText[0] = _T('\0');

		switch (disp_info->item.iSubItem)
		{
		case 0:	// name?
			{
				TCHAR name[MAX_PATH + 1]= { 0 };
				folder->GetName(name);

				_tcsncpy(disp_info->item.pszText, name, disp_info->item.cchTextMax);
			}
			break;

		case 1:	// date/time?
			break;
		}
	}

	if ((disp_info->item.mask & LVIF_IMAGE))
	{
		if (disp_info->item.iSubItem == 0)
			disp_info->item.iImage = folder->GetIconIndex(false);
	}

	return true;
}


void FileListCtrl::OnDblClick(NMHDR* nmhdr, LRESULT* result)
{
	*result = 0;
	NMITEMACTIVATE* item= reinterpret_cast<NMITEMACTIVATE*>(nmhdr);

	OpenItem(item->iItem);
}


void FileListCtrl::OpenItem(size_t item)
{
	ShellItemPtr folder= GetShellFolderItem(folder_list_, item);
	if (folder == 0)
		return;

	ItemIdList path;
	if (folder->GetPath(path))
		SetPath(path, includeFiles_, includeFileItem_);
	else
		;	// message?
}


bool FileListCtrl::GoLevelUp()
{
	if (ShellFolderPtr folder= GetCurrentPath())
	{
		ItemIdList path;
		if (folder->GetPath(path))
		{
			ItemIdList parent;
			if (path.GetParent(parent))
			{
				SetPath(parent, includeFiles_, includeFileItem_);
				return true;
			}
		}
	}
	return false;
}


ShellFolderPtr FileListCtrl::GetCurrentPath()
{
	return shell_folder_;
}


UINT FileListCtrl::OnGetDlgCode()
{
	if (GetItemCount() == 0)
		return static_cast<UINT>(Default());

	return static_cast<UINT>(Default()) | DLGC_WANTALLKEYS; // wants enter key
}


void FileListCtrl::OnKeyDown(UINT chr, UINT rep_cnt, UINT flags)
{
	switch (chr)
	{
	case VK_RETURN:
		ResetSearch();
		{
			POSITION pos= GetFirstSelectedItemPosition();
			int item= GetNextSelectedItem(pos);
			if (item >= 0)
				OpenItem(item);
		}
		return;

	case VK_BACK:
		ResetSearch();
		GoLevelUp();
		return;

	case VK_PRIOR:
		ResetSearch();
		if (::GetKeyState(VK_CONTROL) < 0)
		{
			GoLevelUp();
			return;
		}
		break;

	case VK_TAB:
		ResetSearch();
		if (CWnd* wnd= GetParent())
		{
			bool previous= ::GetKeyState(VK_SHIFT) < 0;
			wnd->SendMessage(WM_NEXTDLGCTL, previous ? 1 : 0, 0L);
			return;
		}
		break;

	case VK_ESCAPE:		// propagate...
		ResetSearch();
		if (CWnd* wnd= GetParent())
		{
			wnd->SendMessage(WM_COMMAND, IDCANCEL, 0);
			return;
		}
		break;

	case VK_LEFT:
	case VK_RIGHT:
	case VK_UP:
	case VK_DOWN:
	case VK_NEXT:
	case VK_HOME:
	case VK_END:
		ResetSearch();
		break;

	default:
		break;
	}

	Default();
}


void FileListCtrl::OnChar(UINT chr, UINT rep_cnt, UINT flags)
{
	if (chr >= '0' && chr <= '9' || chr >= 'A' && chr <= 'Z' ||
		chr >= 'a' && chr <= 'z' || chr >= 0x100)
	{
		bool begin= search_string_.empty();

		search_string_ += static_cast<TCHAR>(chr);

		POSITION pos= GetFirstSelectedItemPosition();
		int item= GetNextSelectedItem(pos);

		int found= FindItem(search_string_, item);
		if (found < 0 && begin)
			found = FindItem(search_string_, -1);
		if (found >= 0)
		{
			UINT state= LVIS_SELECTED | LVIS_FOCUSED;
			SetItemState(found, state, state);
		}
		else
		{
			//TODO:
			// msg beep here to signal no match...
		}

		timer_ = SetTimer(1, 1000, 0);
	}
	else
		Default();
}


int FileListCtrl::FindItem(const String& str, int item)
{
	size_t len= str.length();
	FolderList::const_iterator it= folder_list_.begin();
	if (static_cast<size_t>(item) < folder_list_.size())
		advance(it, item);
	else
		item = 0;

	for (int i= 0; it != folder_list_.end(); ++it, ++i)
	{
		(*it)->GetFullIDL();
		TCHAR name[MAX_PATH + 1]= { 0 };
		(*it)->GetName(name);

		if (_tcsnicmp(str.c_str(), name, len) == 0)
			return item + i;
	}

	return -1;
}


void FileListCtrl::OnDestroy()
{
	if (timer_)
		KillTimer(timer_);
}


void FileListCtrl::OnTimer(UINT_PTR timer)
{
	if (timer_ == timer)
		ResetSearch();
}


void FileListCtrl::ResetSearch()
{
	if (timer_)
	{
		KillTimer(timer_);
		timer_ = 0;
	}
	search_string_.clear();
}


void FileListCtrl::SetFolderChangedCallback(const boost::function<void (ShellFolderPtr)>& callback)
{
	folder_changed_callback_ = callback;
}


//int FileListCtrl::GetItemCountx()
//{
//	return GetItemCount();
//}


void FileListCtrl::GetFiles(std::vector<FileInfo>& files)//, int subDirsDepth/*= 0*/)
{
	files.clear();

	files.reserve(folder_list_.size());

	for (FolderList::const_iterator it= folder_list_.begin(); it != folder_list_.end(); ++it)
	{
		const ShellItemPtr& p= *it;

		if (p->IsFolder() || p->IsLink())
			continue;

		ItemIdList idl(p->FullIdl());
		TCHAR fname[MAX_PATH * 2];
		if (!idl.GetPath(fname))
			continue;

		WIN32_FIND_DATA fnd;
		memset(&fnd, 0, sizeof fnd);
		HANDLE find= ::FindFirstFile(fname, &fnd);
		if (find == INVALID_HANDLE_VALUE)
		{
			ASSERT(false);
			continue;
		}
		VERIFY(FindClose(find));

		uint64 size= (uint64(fnd.nFileSizeHigh) << 32) + uint64(fnd.nFileSizeLow);
		files.push_back(FileInfo(fname, fnd.dwFileAttributes, size, fnd.ftCreationTime, fnd.ftLastWriteTime));
	}
}

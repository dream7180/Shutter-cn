/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "resource.h"
#include "FolderView.h"
#include "Path.h"
#include "intrusive_ptr.h"
typedef mik::intrusive_ptr<ShellFolder> CShellFolderPtr;
#include "CatalogFolder.h"
#include "GetDefaultGuiFont.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


BEGIN_MESSAGE_MAP(FolderView, CTreeCtrl)
	//{{AFX_MSG_MAP(FolderView)
//	ON_WM_KEYDOWN()
//	ON_WM_CREATE()
	ON_NOTIFY_REFLECT(TVN_DELETEITEM, OnDeleteItem)
	ON_NOTIFY_REFLECT(TVN_ITEMEXPANDING, OnExpandItem)
//	ON_NOTIFY_REFLECT(TVN_SELCHANGED, OnSelChanged)
	ON_WM_CONTEXTMENU()
	ON_NOTIFY_REFLECT(NM_RCLICK, OnRightClick)
	ON_WM_CHAR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/********************************************************/
/*						ItemCompare						*/
/********************************************************/
int CALLBACK ItemCompare(LPARAM param1, LPARAM param2, LPARAM param_sort)
{
	return reinterpret_cast<ShellFolder*>(param1)->Compare(reinterpret_cast<ShellFolder*>(param2));
}

/********************************************************/
/*					FolderView							*/
/********************************************************/
FolderView::FolderView()
{
	tree_ = this;
	folder_as_root_ = true;
//	parent_ = 0;
	rclicked_item_ = 0;
}

/********************************************************/
/*					~FolderView						*/
/********************************************************/
FolderView::~FolderView()
{
//	image_list_.Detach();
}

/********************************************************/
/*						Create							*/
/********************************************************/
bool FolderView::Create(CWnd* parent, UINT id, FolderPathPtr path/*= 0*/, bool root/*= true*/)
{
	root_ = path;

	if (!CTreeCtrl::Create(WS_CHILD | WS_VISIBLE | WS_TABSTOP |
		TVS_DISABLEDRAGDROP | TVS_HASBUTTONS | TVS_HASLINES | TVS_SHOWSELALWAYS,
		CRect(0,0,0,0), parent, id))
		return false;

#ifndef TVS_EX_DOUBLEBUFFER
#define TVS_EX_DOUBLEBUFFER 0x0004
#endif

	TreeView_SetExtendedStyle(m_hWnd, TVS_EX_DOUBLEBUFFER, TVS_EX_DOUBLEBUFFER);
	
	LOGFONT lf;
	::GetDefaultGuiFont(lf);
	HFONT hfont = CreateFontIndirectW(&lf);
	SendMessage(WM_SETFONT, WPARAM(hfont));

	//SendMessage(WM_SETFONT, WPARAM(::GetStockObject(DEFAULT_GUI_FONT)));

	if (!SetImageList())
	{
		ASSERT(false);
	//	return false;
	}

	folder_as_root_ = root;

	if (root_)
		if (HTREEITEM item= SetupRoot(root_, folder_as_root_))
		{
			tree_->Expand(item, TVE_EXPAND);
			tree_->SelectItem(item);
		}

	return true;
}


/********************************************************/
/*					OnDeleteItem						*/
/********************************************************/
void FolderView::OnDeleteItem(NMHDR* notify_struct, LRESULT* result)
{
	NMTREEVIEW* nm_tv= reinterpret_cast<NMTREEVIEW*>(notify_struct);
	ASSERT(nm_tv);
	ShellFolder* shl_folder= reinterpret_cast<ShellFolder*>(nm_tv->itemOld.lParam);
	ASSERT(shl_folder);
	delete shl_folder;
}

/********************************************************/
/*					OnExpandItem						*/
/********************************************************/
void FolderView::OnExpandItem(NMHDR* notify_struct, LRESULT* result)
{
	HTREEITEM item = ((NMTREEVIEW*)notify_struct)->itemNew.hItem;
	ShellFolder* shl_folder = (ShellFolder*)((NMTREEVIEW*)notify_struct)->itemNew.lParam;
	ShellFolder* shl_sub_folder;

	if (tree_->GetChildItem(item))
		return;

	CWaitCursor wait;

	if (!shl_folder->GetSubFolderList(&list_))
		return;

	bool has_children= false;

	while (shl_sub_folder = (ShellFolder*)list_.Next())
	{
		if (shl_sub_folder->IsFileSystem() || shl_sub_folder->ContainFileSystemFolder())
			if (InsertItem(shl_sub_folder, item))
			{
				has_children = true;
				continue;
			}

		delete shl_sub_folder;
	}

	list_.DeleteAll();

	if (!has_children)	// if there is no subfolders, then remove 'expand' button
	{
		TVITEM tvi;
		tvi.mask		= TVIF_CHILDREN;
		tvi.hItem		= item;
		tvi.cChildren	= 0;
		tree_->SetItem(&tvi);
	}

	Sort(item);
}


/********************************************************/
/*						OnKeyDown						*/
/********************************************************/
/*
afx_msg void FolderView::OnKeyDown(UINT chr, UINT rep_cnt, UINT flags)
{
//	if (chr == VK_F5)
//		Refresh();
	if (chr == VK_ESCAPE)
		parent_->SendMessage(WM_USER);
	else
		CTreeCtrl::OnKeyDown(chr, rep_cnt, flags);
} */

/********************************************************/
/*					SetImageList						*/
/********************************************************/
BOOL FolderView::SetImageList()
{
	SHFILEINFO info;

	DWORD_PTR image_list= ::SHGetFileInfo(_T("\0\0\0\0"),
		NULL, &info, sizeof info, SHGFI_SMALLICON | SHGFI_PIDL | SHGFI_SYSICONINDEX);

	if (!image_list)
		return FALSE;

	// set item height to ensure space between images
	HIMAGELIST il= reinterpret_cast<HIMAGELIST>(image_list);
	IMAGEINFO ii;
	if (::ImageList_GetImageInfo(il, 0, &ii))
		SetItemHeight(static_cast<SHORT>(ii.rcImage.bottom - ii.rcImage.top + 2));

	SendMessage(TVM_SETIMAGELIST, TVSIL_NORMAL, LPARAM(image_list));

	return TRUE;
}

/********************************************************/
/*					InsertItem							*/
/********************************************************/
HTREEITEM FolderView::InsertItem(ShellFolder* shl_folder, HTREEITEM parent, HTREEITEM insert_after)
{
	if (!shl_folder->GetFullIDL())
		return NULL;

	bool is_link= false;

	TCHAR name[MAX_PATH + MAX_PATH];
	if (!shl_folder->GetName(name))
		*name = NULL;

	int iconIndex= shl_folder->GetIconIndex(FALSE);
	int selectedIconIndex= shl_folder->GetSelectedIconIndex(FALSE);
	shl_folder->ReleaseFullIDL();

	if (iconIndex < 0)
		iconIndex = 0;

	if (selectedIconIndex < 0)
		selectedIconIndex = iconIndex;

	TVINSERTSTRUCT tvis;
	tvis.hParent		= parent;
	tvis.hInsertAfter	= insert_after;
	tvis.item.mask		= TVIF_CHILDREN | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM | TVIF_TEXT | TVIF_STATE;
	tvis.item.pszText	= name;
	tvis.item.iImage	= iconIndex;
	tvis.item.state		= 0;
	tvis.item.iSelectedImage = selectedIconIndex;
	tvis.item.lParam	= reinterpret_cast<LPARAM>(shl_folder);
	tvis.item.cChildren	= /*shl_folder->ContainFileSystemFolder() &&*/ shl_folder->HasSubFolder() ? 1 : 0;

	if (shl_folder->IsLink())
	{
		tvis.item.state |= INDEXTOOVERLAYMASK(2);
		tvis.item.stateMask |= TVIS_OVERLAYMASK;
	}

	return tree_->InsertItem(&tvis);
}

/********************************************************/
/*						SetupRoot						*/
/********************************************************/
HTREEITEM FolderView::SetupRoot(FolderPathPtr path, bool folder_as_root)
{
	if (path == 0)
		return 0;

	ShellFolder* shl_folder= 0;

	try
	{
		if (path)
			shl_folder = path->CreateShellFolder(m_hWnd, folder_as_root);
		else
			shl_folder = new ShellFolder(m_hWnd, 0);

		//Path path= path->GetPath();
		//if (path.MatchExtension(_T("catalog")) && !path.IsFolder())
		//	shl_folder = new CatalogFolder(path, m_hWnd, folder_as_root);
		//else
		//{
		//	const ITEMIDLIST* idl= idlRoot;
		//	shl_folder = new ShellFolder(m_hWnd, folder_as_root ? idl : 0);
		//}
	}
	catch (int)
	{
		return NULL;
	}

	if (shl_folder)
	{
		if (HTREEITEM item= InsertItem(shl_folder))
		{
			if (folder_as_root)
				return item;

			Expand(item, TVE_EXPAND);

	//TODO ======== make it work for catalog paths =================================TODO
			// find item 'path' and expand tree to this node folder
			ItemIdList pidl= path ? path->GetPIDL() : ItemIdList();
			return ExpandTo(item, pidl);
		}

		delete shl_folder;
	}

	return NULL;
}


void FolderView::Reset(FolderPathPtr path, bool root/*= true*/)
{
	DeleteAllItems();
	list_.DeleteAll();

	folder_as_root_ = root;
	//idl_root_.Free();
	//if (root_idl)
		root_ = path;

	if (HTREEITEM item= SetupRoot(root_, folder_as_root_))
	{
		tree_->Expand(item, TVE_EXPAND);
		tree_->SelectItem(item);
	}
}


//void FolderView::Reset(const ITEMIDLIST* root_idl, bool root/*= true*/)
//{
	//DeleteAllItems();
	//list_.DeleteAll();

	//folder_as_root_ = root;
	//idl_root_.Free();
	//if (root_idl)
	//	idl_root_ = root_idl;

	//if (HTREEITEM item= SetupRoot(root_, folder_as_root_))
	//{
	//	tree_->Expand(item, TVE_EXPAND);
	//	tree_->SelectItem(item);
	//}
//}


HTREEITEM FolderView::FindAndExpandFolder(HTREEITEM parent_item, const ITEMIDLIST* idl)
{
	HTREEITEM folder_item= GetChildItem(parent_item);

	while (folder_item != 0)
	{
		if (ShellFolder* shl_folder= reinterpret_cast<ShellFolder*>(GetItemData(folder_item)))
		{
			if (shl_folder->Compare(idl) == 0)
				return folder_item;
		}

		folder_item = GetNextItem(folder_item, TVGN_NEXT);
	}

	return 0;
}


HTREEITEM FolderView::ExpandTo(HTREEITEM item, const ITEMIDLIST* pidl)
{
	ItemIdList idlFolder(pidl);	// make a copy

	ITEMIDLIST* idl= idlFolder;
	uint8* ptr= reinterpret_cast<uint8*>(idl);

	while (idl->mkid.cb)
	{
		ptr += idl->mkid.cb;
		ITEMIDLIST* idl_next = reinterpret_cast<ITEMIDLIST*>(ptr);
		USHORT size= idl_next->mkid.cb;
		idl_next->mkid.cb = 0;		// temporarily

		item = FindAndExpandFolder(item, idl);

		idl_next->mkid.cb = size;	// restore size
		idl = idl_next;

		if (item == 0)				// error, node on path not found
			break;

		if (size == 0)				// end of path?
			break;

		Expand(item, TVE_EXPAND);
	}

	return item;
}

/********************************************************/
/*						Sort							*/
/********************************************************/
BOOL FolderView::Sort(HTREEITEM item)
{
	TVSORTCB sort;

	sort.hParent = item;
	sort.lpfnCompare = ItemCompare;
	sort.lParam = NULL;
	return tree_->SortChildrenCB(&sort);
}
/********************************************************/
/*						Refresh							*/
/********************************************************/
void FolderView::Refresh()
{
	HTREEITEM item;

	if (!(item = tree_->GetRootItem()))
	{
		SetupRoot(root_, folder_as_root_);
		return;
	}

	UpdateItem(item);
}
/********************************************************/
/*						UpdateItem						*/
/********************************************************/
void FolderView::UpdateItem(HTREEITEM hitem)
{
	TVITEM item;
	TCHAR name[MAX_PATH];

	item.mask = TVIF_CHILDREN | TVIF_HANDLE | TVIF_IMAGE | TVIF_PARAM | TVIF_SELECTEDIMAGE | TVIF_STATE | TVIF_TEXT;
	item.stateMask = TVIS_EXPANDED;
	item.hItem = hitem;
	item.pszText = name;
	item.cchTextMax = MAX_PATH;

	if (tree_->GetItem(&item))
	{
		ShellFolder* shl_folder= reinterpret_cast<ShellFolder*>(item.lParam);
		shl_folder->Reset();

		if (!shl_folder->IsExisting())
		{
			tree_->DeleteItem(hitem);
			return;
		}

		UpdateSubItem(item);
		UpdateItemProperty(item);
	}

	if (hitem = tree_->GetChildItem(hitem))
		while (hitem)
		{
			UpdateItem(hitem);
			hitem = tree_->GetNextSiblingItem(hitem);
		}
}
/********************************************************/
/*					UpdateSubItem						*/
/********************************************************/
void FolderView::UpdateSubItem(TVITEM& item)
{
	if (!(item.state & TVIS_EXPANDED))
	{
		if (HTREEITEM hitem= tree_->GetChildItem(item.hItem))
			while (hitem)
			{
				HTREEITEM next_item= tree_->GetNextSiblingItem(hitem);
				tree_->DeleteItem(hitem);
				hitem = next_item;
			}

		return;
	}

	ShellFolder* shl_folder= (ShellFolder*)item.lParam;
	if (!shl_folder->GetSubFolderList(&list_))
		return;

	HTREEITEM hitem= tree_->GetChildItem(item.hItem);

	while (hitem)
	{
		ShellFolder* shl_folder= reinterpret_cast<ShellFolder*>(tree_->GetItemData(hitem));

		while (LINKLISTITEM* ll_item= list_.NextItem())
		{
			ShellFolder* shl_folder1= reinterpret_cast<ShellFolder*>(ll_item->value);

			if (!shl_folder->Compare(shl_folder1))
			{
				delete shl_folder1;
				list_.Delete(ll_item);
			}
		}

		hitem = tree_->GetNextSiblingItem(hitem);
	}

	while (ShellFolder* shl_folder= (ShellFolder*)list_.Next())
	{
		if (shl_folder->IsFileSystem() || shl_folder->ContainFileSystemFolder())
			if (InsertItem(shl_folder, item.hItem))
				continue;

		delete shl_folder;
	}

	list_.DeleteAll();
}

/********************************************************/
/*					UpdateItemProperty					*/
/********************************************************/
void FolderView::UpdateItemProperty(TVITEM& item)
{
	ShellFolder* shl_folder = (ShellFolder*)item.lParam;
	TCHAR name[MAX_PATH];
	int iconIndex;
	int selectedIconIndex;
	int children;
	BOOL need_update = FALSE;

	if (!shl_folder->GetFullIDL())
		return;

	if (!shl_folder->GetName(name))
		*name = NULL;

	iconIndex = shl_folder->GetIconIndex(FALSE);
	selectedIconIndex = shl_folder->GetSelectedIconIndex(FALSE);
	shl_folder->ReleaseFullIDL();
	children = (/*shl_folder->ContainFileSystemFolder() &&*/ shl_folder->HasSubFolder()) ? 1 : 0;

	if (iconIndex < 0)
		iconIndex = 0;

	if (selectedIconIndex < 0)
		selectedIconIndex = iconIndex;

	if (_tcscmp(item.pszText, name))
	{
		item.pszText = name;
		need_update = TRUE;
	}

	if (item.iImage != iconIndex)
	{
		item.iImage = iconIndex;
		need_update = TRUE;
	}
	
	if (item.iSelectedImage != selectedIconIndex)
	{
		item.iSelectedImage = selectedIconIndex;
		need_update = TRUE;
	}

	if (item.cChildren != children)
	{
		item.cChildren = children;
		need_update = TRUE;
	}

	if (need_update)
		tree_->SetItem(&item);
}


bool FolderView::GetSelectedPath(TCHAR* path) const
{
	if (HTREEITEM item= GetSelectedItem())
	{
		if (ShellFolder* shl_folder= reinterpret_cast<ShellFolder*>(GetItemData(item)))
		{
			if (shl_folder->GetPath(path))
				return true;

//			ASSERT(false);	// get path failed
		}
	}

	return false;
}


FolderPathPtr FolderView::GetSelectedPath() const
{
	if (HTREEITEM item= GetSelectedItem())
		if (ShellFolder* shl_folder= reinterpret_cast<ShellFolder*>(GetItemData(item)))
			return shl_folder->GetPath();

	return FolderPathPtr();
}


bool FolderView::GetSelectedPath(ItemIdList& idlPath) const
{
	if (HTREEITEM item= GetSelectedItem())
	{
		if (ShellFolder* shl_folder= reinterpret_cast<ShellFolder*>(GetItemData(item)))
		{
			if (shl_folder->GetPath(idlPath))
				return true;
		}
	}

	return false;
}


bool FolderView::GetRClickedItemPath(ItemIdList& idlPath) const
{
	if (rclicked_item_)
	{
		if (ShellFolder* shl_folder= reinterpret_cast<ShellFolder*>(GetItemData(rclicked_item_)))
		{
			if (shl_folder->GetPath(idlPath))
				return true;
		}
	}

	return false;
}


void ContextMenu(CWnd* view, CPoint pos)
{
	CMenu menu;
	if (!menu.LoadMenu(IDR_FOLDER_CONTEXT))
		return;

	if (CMenu* popup= menu.GetSubMenu(0))
	{
		if (pos.x == -1 && pos.y == -1)
		{
			CRect rect;
			view->GetClientRect(rect);
			view->ClientToScreen(rect);
			pos = rect.CenterPoint();
		}
		popup->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, pos.x, pos.y, AfxGetMainWnd());
	}
}


void FolderView::OnContextMenu(CWnd* wnd, CPoint pos)
{
	rclicked_item_ = 0;
	ContextMenu(this, pos);
}


void FolderView::OnRightClick(NMHDR* notify_struct, LRESULT* result)
{
	*result = 0;
	CPoint pos(0, 0);
	GetCursorPos(&pos);

	TVHITTESTINFO ht;
	ht.flags = 0;
	ht.hItem = 0;
	ht.pt = pos;
	ScreenToClient(&ht.pt);
	HitTest(&ht);

	rclicked_item_ = ht.hItem;

	ContextMenu(this, pos);
}


void FolderView::OnChar(UINT chr, UINT rep_cnt, UINT flags)
{
	if (chr == VK_ESCAPE)
	{
		// no-op
		// eat Esc to prevent it from beeping

		// cheesy: main wnd wants to handle Esc
		if (CWnd* wnd= AfxGetMainWnd())
			wnd->SendMessage(WM_COMMAND, ID_ESCAPE);
	}
	else
		CTreeCtrl::OnChar(chr, rep_cnt, flags);
}


// get physical path represented by the item
String FolderView::GetPhysicalPath(HTREEITEM item) const
{
	TCHAR path[MAX_PATH]= _T("");

	if (ShellFolder* shl_folder= reinterpret_cast<ShellFolder*>(GetItemData(item)))
	{
		if (shl_folder->IsLink())
		{
			TCHAR link[MAX_PATH]= _T("");
			shl_folder->GetPath(link);
			Path path(link);
			ItemIdList linked_obj_idl= path.GetLinkedObject();
			if (linked_obj_idl.IsEmpty())
				return String();

			TCHAR buf[MAX_PATH * 2];
			if (!::SHGetPathFromIDList(linked_obj_idl, buf))
				return String();

			return buf;
		}
		else
			shl_folder->GetPath(path);
	}

	return path;
}

// true if given item represents folder or a drive
bool FolderView::IsPhysicalPath(HTREEITEM item) const
{
	TCHAR path[MAX_PATH];

	if (ShellFolder* shl_folder= reinterpret_cast<ShellFolder*>(GetItemData(item)))
		if (shl_folder->GetPath(path))
			return true;

	return false;
}

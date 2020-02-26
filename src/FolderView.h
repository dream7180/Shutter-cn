/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "ShellFolder.h"
#include "ItemIdList.h"
#include "FolderPath.h"


class FolderView : public CTreeCtrl
{
public:
	FolderView();
	~FolderView();

	bool Create(CWnd* parent, UINT id, FolderPathPtr path= 0, bool root= true);

	void Refresh();

	// get current path (selected folder full path)
	bool GetSelectedPath(TCHAR* path) const;

	// get current path (returned as item id list)
	bool GetSelectedPath(ItemIdList& idlPath) const;

	// get current path
	FolderPathPtr GetSelectedPath() const;

	// get physical path represented by the item
	String GetPhysicalPath(HTREEITEM item) const;

	// true if given item represents folder or a drive
	bool IsPhysicalPath(HTREEITEM item) const;

	// change current path (may reinit tree)
//	void Reset(const ITEMIDLIST* root_idl, bool root= true);
	void Reset(FolderPathPtr path, bool root= true);

	// get a path from the right-clicked item
	bool GetRClickedItemPath(ItemIdList& idlPath) const;

private:
	CTreeCtrl* tree_;
	CLinkList list_;
//	ItemIdList idl_root_;
	FolderPathPtr root_;
	bool folder_as_root_;
	HTREEITEM rclicked_item_;

	DECLARE_MESSAGE_MAP()

	//{{AFX_MSG(BrowserFrame)
	afx_msg void OnDeleteItem(NMHDR* notify_struct, LRESULT* result);
	afx_msg void OnExpandItem(NMHDR* notify_struct, LRESULT* result);
	afx_msg void OnKeyDown(UINT chr, UINT rep_cnt, UINT flags);
	afx_msg void OnSelChanged(NMHDR* notify_struct, LRESULT* result);
	afx_msg void OnTimer(UINT id_event);
	afx_msg void OnContextMenu(CWnd* wnd, CPoint point);
	afx_msg void OnRightClick(NMHDR* notify_struct, LRESULT* result);
	afx_msg void OnChar(UINT chr, UINT rep_cnt, UINT flags);
	//}}AFX_MSG

	BOOL SetImageList();
	HTREEITEM InsertItem(ShellFolder* shl_folder, HTREEITEM parent= TVI_ROOT, HTREEITEM insert_after= TVI_LAST);
	HTREEITEM SetupRoot(FolderPathPtr path, bool folder_as_root);
	BOOL Sort(HTREEITEM item);
	void UpdateItem(HTREEITEM item);
	void UpdateSubItem(TVITEM& item);
	void UpdateItemProperty(TVITEM& item);

	void SelChangeNotify();

	HTREEITEM ExpandTo(HTREEITEM item, const ITEMIDLIST* idl);
	HTREEITEM FindAndExpandFolder(HTREEITEM parent_item, const ITEMIDLIST* idl);
};

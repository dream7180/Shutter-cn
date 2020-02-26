/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
#include "LinkList.h"
#include "intrusive_ptr.h"
#include "FolderPath.h"
class ItemIdList;

#define SF_ERROR_IMALLOC		0
#define SF_ERROR_ISHELLFOLDER	1
#define SF_ERROR_DESKTOP_PIDL	2

class ShellFolder : public mik::counter_base
{
public:
	ShellFolder(HWND wnd, const ITEMIDLIST* idl);
	ShellFolder(HWND wnd, const int cs_idl);
	virtual ~ShellFolder();

	virtual BOOL GetName(TCHAR* name);

	// return folder path
	bool GetPath(TCHAR* path);

	// return folder path as item id list
	bool GetPath(ItemIdList& idlPath) const;

	// get current path
	virtual FolderPathPtr GetPath() const;

	virtual int GetIconIndex(BOOL large);
	virtual int GetSelectedIconIndex(BOOL large);
	virtual int GetOverlayIconIndex(BOOL large);

	virtual BOOL GetFullIDL() const;
	virtual void ReleaseFullIDL();

	virtual BOOL GetSubFolderList(CLinkList* list);

	BOOL GetSubFolderList(std::list<mik::intrusive_ptr<ShellFolder> >& list);

	int Compare(ShellFolder* shl_folder);
	int Compare(const ITEMIDLIST* idl);

	BOOL Reset();
	BOOL IsExisting();
	BOOL IsFileSystem();
	BOOL ContainFileSystemFolder();
	virtual BOOL HasSubFolder();
	bool IsLink() const					{ return link_file_; }

protected:
	HWND m_hWnd;
private:
	IShellFolderPtr shl_folder_;
	IMallocPtr malloc_;
	ITEMIDLIST* idl_;
	mutable ITEMIDLIST* fidl_;
	ShellFolder* shl_parent_folder_;	// parent folder (if passed) or 0
	BOOL is_existing_;
	BOOL is_file_system_;
	BOOL contain_file_system_folder_;
	BOOL has_sub_folder_;
	bool link_file_;
	IShellFolderPtr shl_folder_desk_;	// root folder (if created here)

	ShellFolder(ShellFolder* shl_parent_folder, ITEMIDLIST* idl, IShellFolder* shl_folder = NULL);
	void Free();
	void FreeIDL(ITEMIDLIST* idl) const;
	unsigned long GetIDLLength(const ITEMIDLIST *idl) const;
	ITEMIDLIST* ConcatIDL(const ITEMIDLIST* IDL1, const ITEMIDLIST* IDL2) const;
	ITEMIDLIST* CreateEmptyIDL() const;
	BOOL GetAttribute();

	void Init(HWND wnd);
};

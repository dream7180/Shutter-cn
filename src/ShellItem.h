/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "LinkList.h"
#include "intrusive_ptr.h"
#include <boost/function.hpp>
#include "FileFilterCallback.h"
class ItemIdList;

//_COM_SMARTPTR_TYPEDEF(IShellItem, __uuidof(IShellItem));

enum { SF_ERROR_IMALLOC, SF_ERROR_ISHELLFOLDER, SF_ERROR_DESKTOP_PIDL };
class ShellFolderEx;


class ShellItem : public mik::counter_base
{
public:
	ShellItem(HWND wnd, const ITEMIDLIST* idl);
	ShellItem(HWND wnd, const int cs_idl);
	ShellItem(ShellFolderEx* shl_parent_folder, ITEMIDLIST* idl, SFGAOF status,
		DWORD hi_size, DWORD low_size, DWORD fileAttribs, const FILETIME* timeModified);

	virtual ~ShellItem();

	BOOL GetName(TCHAR* name);

	// return folder path
	bool GetPath(TCHAR* path);

	// return folder path as item id list
	bool GetPath(ItemIdList& idlPath);

	int GetIconIndex(BOOL large);
	int GetSelectedIconIndex(BOOL large);
	int GetOverlayIconIndex(BOOL large);

	BOOL GetFullIDL();
	void ReleaseFullIDL();

	const ITEMIDLIST* FullIdl();

	//BOOL Reset();
	BOOL IsExisting();
	BOOL IsFileSystem();
	BOOL ContainFileSystemFolder();
	BOOL HasSubFolder();
	bool IsLink() const;
	bool IsFolder() const;

	ITEMIDLIST* GetIdl()	{ return idl_; }

	FILETIME GetModifiedTime() const;
	DWORD GetFileAttributes() const;
	uint64 GetFileLength() const;

protected:
	HWND m_hWnd;
	IMallocPtr malloc_;
	ITEMIDLIST* idl_;
	ITEMIDLIST* FIDL_;
	ShellFolderEx* shl_parent_folder_;	// parent folder (if passed) or 0
	struct
	{
		unsigned int is_existing : 1;
		unsigned int is_file_system : 1;
		unsigned int containsFileSystemFolder : 1;
		unsigned int hasSubFolder : 1;
		unsigned int is_link_file : 1;
		unsigned int is_folder : 1;
	};
	uint64 fileSize_;
	DWORD fileAttributes_;
	FILETIME timeModified_;

	IShellFolderPtr shl_folder_desk_;	// root folder (if created here)


	ITEMIDLIST* CreateEmptyIDL();
	ITEMIDLIST* ConcatIDL(const ITEMIDLIST* IDL1, const ITEMIDLIST* IDL2);
	unsigned long GetIDLLength(const ITEMIDLIST* idl);
	void FreeIDL(ITEMIDLIST* idl);

private:

	void Free();
	BOOL GetAttribute(SFGAOF status);

	void Init(HWND wnd);
};


class ShellFolderEx : public ShellItem
{
public:
	ShellFolderEx(HWND wnd, const ITEMIDLIST* idl);
	ShellFolderEx(HWND wnd, const int cs_idl);
	ShellFolderEx(ShellFolderEx* shl_parent_folder, ITEMIDLIST* idl, IShellFolder* shl_folder= 0);

	virtual ~ShellFolderEx();

	int Compare(ShellItem* shl_folder);
	int Compare(const ITEMIDLIST* idl);

	BOOL GetSubItemList(std::vector<mik::intrusive_ptr<ShellItem> >& vect, bool onlyFolders,
		const FileFilterCallback& includeFileItem);
//		const String& includeFileSpec, const String& excludeFileSpec);
	BOOL GetSubFolderList(CLinkList* list);

private:
	IShellFolderPtr shl_folder_;

	BOOL GetAttribute();
};

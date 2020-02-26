/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "ShellFolder.h"
#include "ItemIdList.h"
#include "Path.h"
#include <ShObjIdl.h>
#include "DirectoryPath.h"
#include "CatalogFolder.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/********************************************************/
/*						ShellFolder					*/
/********************************************************/
ShellFolder::ShellFolder(HWND wnd, const ITEMIDLIST* idl)
{
	Init(wnd);

	if (idl && idl->mkid.cb != 0)	// given PIDL to serve as root?
	{
		if (::SHGetDesktopFolder(&shl_folder_desk_) != S_OK)
			throw SF_ERROR_ISHELLFOLDER;

		if (shl_folder_desk_->BindToObject(idl, NULL, IID_IShellFolder, (void**)&shl_folder_) != S_OK)
			throw SF_ERROR_ISHELLFOLDER;

		ITEMIDLIST* idl_copy= CreateEmptyIDL();
		if (idl_copy == 0)
			throw SF_ERROR_IMALLOC;

		idl_ = ConcatIDL(idl_copy, idl);

		if (!idl_)
		{
			FreeIDL(idl_copy);
			throw SF_ERROR_IMALLOC;
		}
	}
	else	// desktop as root
	{
		if (::SHGetDesktopFolder(&shl_folder_) != S_OK)
			throw SF_ERROR_ISHELLFOLDER;

		if (::SHGetSpecialFolderLocation(m_hWnd, CSIDL_DESKTOP, &idl_) != S_OK)
			throw SF_ERROR_DESKTOP_PIDL;
	}

	GetAttribute();
}


ShellFolder::ShellFolder(HWND wnd, const int cs_idl)
{
	Init(wnd);

	if (::SHGetDesktopFolder(&shl_folder_) != S_OK)
		throw SF_ERROR_ISHELLFOLDER;

	if (::SHGetSpecialFolderLocation(m_hWnd, cs_idl, &idl_) != S_OK)
		throw SF_ERROR_DESKTOP_PIDL;

	GetAttribute();
}


void ShellFolder::Init(HWND wnd)
{
	m_hWnd = wnd;
	idl_ = NULL;
	fidl_ = NULL;
	shl_parent_folder_ = NULL;
	is_existing_ = TRUE;
	is_file_system_ = FALSE;
	contain_file_system_folder_ = FALSE;
	has_sub_folder_ = FALSE;
	link_file_ = false;

	if (::SHGetMalloc(&malloc_) != S_OK)
		throw SF_ERROR_IMALLOC;
}

/********************************************************/
/*						ShellFolder					*/
/********************************************************/
ShellFolder::ShellFolder(ShellFolder* shl_parent_folder, ITEMIDLIST* idl, IShellFolder* shl_folder)
{
	m_hWnd = shl_parent_folder->m_hWnd;
	shl_folder_ = shl_folder;
	malloc_ = shl_parent_folder->malloc_;
	idl_ = idl;
	fidl_ = NULL;
	shl_parent_folder_ = shl_parent_folder;
	is_existing_ = TRUE;
	is_file_system_ = FALSE;
	contain_file_system_folder_ = FALSE;
	has_sub_folder_ = FALSE;
	link_file_ = false;

	malloc_->AddRef();
	GetAttribute();
}
/********************************************************/
/*						~ShellFolder					*/
/********************************************************/
ShellFolder::~ShellFolder()
{
	Free();
}
/********************************************************/
/*							Free						*/
/********************************************************/
void ShellFolder::Free()
{
	if (shl_folder_ != 0)
		shl_folder_.Release();

	if (idl_)
		FreeIDL(idl_);

	if (fidl_)
		FreeIDL(fidl_);

	if (malloc_ != 0)
		malloc_.Release();
}

/********************************************************/
/*						GetName							*/
/********************************************************/
BOOL ShellFolder::GetName(TCHAR* name)
{
	if (!fidl_)
		return FALSE;

	SHFILEINFO info;
	if (!::SHGetFileInfo((TCHAR*)fidl_, NULL, &info, sizeof info, SHGFI_DISPLAYNAME | SHGFI_PIDL))
		return FALSE;

	_tcscpy(name, info.szDisplayName);

	return TRUE;
}


bool ShellFolder::GetPath(TCHAR* path)
{
	if (!GetFullIDL())
		return false;

	return !!::SHGetPathFromIDList(fidl_, path);
}


bool ShellFolder::GetPath(ItemIdList& idlPath) const
{
	if (!GetFullIDL())
		return false;

	idlPath = fidl_;

	if (link_file_)
	{
		Path path(static_cast<const TCHAR*>(idlPath.GetPath()));
		ItemIdList linked_obj_idl= path.GetLinkedObject();
		if (linked_obj_idl != 0)
			idlPath = linked_obj_idl;
	}

	return true;
}


int ShellFolder::GetOverlayIconIndex(BOOL large)
{
	return -1;
}


/********************************************************/
/*					GetIconIndex						*/
/********************************************************/
int ShellFolder::GetIconIndex(BOOL large)
{
	if (!fidl_)
		return -1;

	SHFILEINFO info;
	if (!::SHGetFileInfo((TCHAR*)fidl_, NULL, &info, sizeof info,
		(large ? SHGFI_LARGEICON : SHGFI_SMALLICON) | SHGFI_PIDL | SHGFI_SYSICONINDEX))
		return -1;

	return info.iIcon;
}
/********************************************************/
/*					GetSelectedIconIndex				*/
/********************************************************/
int ShellFolder::GetSelectedIconIndex(BOOL large)
{
	if (!fidl_)
		return -1;
	
	SHFILEINFO info;
	if (!::SHGetFileInfo((TCHAR*)fidl_, NULL, &info, sizeof info,
		(large ? SHGFI_LARGEICON : SHGFI_SMALLICON) | SHGFI_PIDL | SHGFI_SYSICONINDEX | SHGFI_OPENICON))
		return -1;

	return info.iIcon;
}
/********************************************************/
/*						FreeIDL							*/
/********************************************************/
void ShellFolder::FreeIDL(ITEMIDLIST* idl) const
{
	malloc_->Free(idl);
}
/********************************************************/
/*						GetFullIDL						*/
/********************************************************/
BOOL ShellFolder::GetFullIDL() const
{
	if (fidl_)
		return TRUE;

	if (!(fidl_ = CreateEmptyIDL()))
		return FALSE;

	const ShellFolder* shl_folder= this;

	while (shl_folder)
	{
		ITEMIDLIST* idl= fidl_;
		fidl_ = ConcatIDL(shl_folder->idl_, fidl_);
		FreeIDL(idl);

		if (!fidl_)
			return FALSE;

		shl_folder = shl_folder->shl_parent_folder_;
	}

	return TRUE;
}
/********************************************************/
/*					ReleaseFullIDL						*/
/********************************************************/
void ShellFolder::ReleaseFullIDL()
{
	if (fidl_)
	{
		FreeIDL(fidl_);
		fidl_ = NULL;
	}
}
/********************************************************/
/*					GetIDLLength						*/
/********************************************************/
unsigned long ShellFolder::GetIDLLength(const ITEMIDLIST* idl) const
{
	unsigned long length= 0;

	while (idl->mkid.cb)
	{
		length += idl->mkid.cb;
		idl = (ITEMIDLIST*)((char*)idl + idl->mkid.cb);
	}

	return length;
}
/********************************************************/
/*						ConcatIDL						*/
/********************************************************/
ITEMIDLIST* ShellFolder::ConcatIDL(const ITEMIDLIST* IDL1, const ITEMIDLIST* IDL2) const
{
	unsigned long size1 = GetIDLLength(IDL1);
	unsigned long size2 = GetIDLLength(IDL2);
	unsigned long newSize = size1 + size2;
	ITEMIDLIST* new_idl = (ITEMIDLIST*)malloc_->Alloc(newSize + sizeof(unsigned short));

	if (!new_idl)
		return NULL;

	CopyMemory((void*)new_idl, IDL1, size1);
	CopyMemory((void*)((char*)new_idl + size1), IDL2, size2);
	((ITEMIDLIST*)((char*)new_idl + newSize))->mkid.cb = NULL;
	return new_idl;
}
/********************************************************/
/*					CreateEmptyIDL						*/
/********************************************************/
ITEMIDLIST* ShellFolder::CreateEmptyIDL() const
{
	ITEMIDLIST* idl = (ITEMIDLIST*)malloc_->Alloc(sizeof(unsigned short));

	if (!idl)
		return NULL;

	idl->mkid.cb = NULL;
	return idl;
}

/********************************************************/
/*					GetSubFolderList					*/
/********************************************************/

BOOL ShellFolder::GetSubFolderList(std::list<mik::intrusive_ptr<ShellFolder> >& list)
{
	list.clear();

	if (shl_folder_ == 0)
		return TRUE;

	IEnumIDListPtr enum_id_list;

	if (shl_folder_->EnumObjects(m_hWnd, SHCONTF_FOLDERS | SHCONTF_NONFOLDERS, &enum_id_list) != S_OK)
	{
		ASSERT(false);
		return false;
	}

	ITEMIDLIST* idl_ptr= 0;
	while (enum_id_list->Next(1, &idl_ptr, NULL) == S_OK)
	{
		ItemIdList idl(idl_ptr);
		SFGAOF status= SFGAO_LINK | SFGAO_FOLDER;
		if (shl_folder_->GetAttributesOf(1, idl.GetConstPIDL(), &status) != S_OK)
			continue;

		if (status & SFGAO_LINK)
		{
			STRRET str;
			memset(&str, 0, sizeof str);
			if (shl_folder_->GetDisplayNameOf(idl, SHGDN_NORMAL | SHGDN_FORPARSING, &str) == S_OK)
			{
				Path path(str, true);
				ItemIdList linked_obj_idl= path.GetLinkedObject();
				if (linked_obj_idl.IsEmpty())
					continue;

				TCHAR buf[MAX_PATH * 2];
				if (!::SHGetPathFromIDList(linked_obj_idl, buf))
					continue;

				path = buf;
				if (path.IsFolder())
					status |= SFGAO_FOLDER;
			}
		}

//		if ((status & SFGAO_STREAM) && (status & SFGAO_FOLDER))
		{
			// filter out fake folders - zip files
//			continue;
		}

		if (status & SFGAO_FOLDER)
		{
			IShellFolderPtr shl_folder;
			if (shl_folder_->BindToObject(idl, NULL, IID_IShellFolder, (void**)&shl_folder) != S_OK)
				break;

			list.push_back(new ShellFolder(this, idl, shl_folder));

			idl.Detach();
			shl_folder.Detach();
		}
	}

	return true;
}


BOOL ShellFolder::GetSubFolderList(CLinkList* list)
{
	bool has_error= false;

	list->DeleteAll();

	if (shl_folder_ == 0)
		return TRUE;

	IEnumIDListPtr enum_id_list;

	if (shl_folder_->EnumObjects(m_hWnd, SHCONTF_FOLDERS | SHCONTF_NONFOLDERS, &enum_id_list) == S_OK)
	{
		ITEMIDLIST* idl_ptr= 0;
		while (enum_id_list->Next(1, &idl_ptr, NULL) == S_OK)
		{
			ItemIdList idl(idl_ptr, false);
			SFGAOF status= SFGAO_LINK | SFGAO_FOLDER;
			if (shl_folder_->GetAttributesOf(1, idl.GetConstPIDL(), &status) != S_OK)
				continue;

			if (status & SFGAO_LINK)
			{
				STRRET str;
				memset(&str, 0, sizeof str);
				if (shl_folder_->GetDisplayNameOf(idl, SHGDN_NORMAL | SHGDN_FORPARSING, &str) == S_OK)
				{
					Path path(str, true);
					ItemIdList linked_obj_idl= path.GetLinkedObject();
					if (linked_obj_idl.IsEmpty())
						continue;

					TCHAR buf[MAX_PATH * 2];
					if (!::SHGetPathFromIDList(linked_obj_idl, buf))
						continue;

					path = buf;
					if (path.IsFolder())
						status |= SFGAO_FOLDER;
				}
			}

			// in W2K local & network folders have status = 0x70400177
			// zip folders in WinXP have no SFGAO_FILESYSANCESTOR flag set, but neither have several other usefull objects

//			if ((status & SFGAO_STREAM) && (status & SFGAO_FOLDER))
			{
				// filter out fake folders - zip files
//				continue;
			}

			// for network files this may be slow
			if ((status & SFGAO_FOLDER) == 0)
			{
				STRRET str;
				memset(&str, 0, sizeof str);
				if (shl_folder_->GetDisplayNameOf(idl, SHGDN_NORMAL | SHGDN_FORPARSING, &str) == S_OK)
				{
					ShellFolder* catalog= 0;
					Path path(str, true);
					if (path.MatchExtension(_T("catalog")))
					{
						try
						{
							if (catalog = new CatalogFolder(path, m_hWnd, false))
							{
								if (list->Add(catalog))
									continue;

								delete catalog;
							}
						}
						catch (...)
						{
						}
						//
						//status |= SFGAO_FOLDER;
						continue;
					}
				}
			}

			if (status & SFGAO_FOLDER)
			{
				IShellFolder* shl_folder= 0;
				if (shl_folder_->BindToObject(idl, NULL, IID_IShellFolder, (void**)&shl_folder) != S_OK)
					shl_folder = NULL;

				if (ShellFolder* C_shl_folder= new ShellFolder(this, idl, shl_folder))
				{
					idl.Detach();
					shl_folder = NULL;

					if (list->Add(C_shl_folder))
						continue;

					delete C_shl_folder;
				}

				if (shl_folder)
					shl_folder->Release();

				//if (idl)
				//	FreeIDL(idl);

				has_error = true;
				break;
			}
		}

//		enum_id_list->Release();
	}

	if (has_error)
	{
		while (ShellFolder* C_shl_folder = (ShellFolder*)list->Next())
			delete C_shl_folder;

		list->DeleteAll();
	}

	return !has_error;
}

/********************************************************/
/*						Compare							*/
/********************************************************/
int ShellFolder::Compare(ShellFolder* shl_folder)
{
	IShellFolder* I_shl_folder= (shl_parent_folder_) ? shl_parent_folder_->shl_folder_ : shl_folder_;
	return short(HRESULT_CODE(I_shl_folder->CompareIDs(0, idl_, shl_folder->idl_)));
}

int ShellFolder::Compare(const ITEMIDLIST* idl)
{
	IShellFolder* I_shl_folder= (shl_parent_folder_) ? shl_parent_folder_->shl_folder_ : shl_folder_;
	return short(HRESULT_CODE(I_shl_folder->CompareIDs(0, idl_, idl)));
}


/********************************************************/
/*					GetAttribute						*/
/********************************************************/
BOOL ShellFolder::GetAttribute()
{
	IShellFolder* shl_folder= shl_folder_;
	if (shl_parent_folder_)
		shl_folder = shl_parent_folder_->shl_folder_;
	else if (shl_folder_desk_)
	{
		// when shl_folder_desk_ is valid idl_ is actually fully qualified PIDL (not relative)
		shl_folder = shl_folder_desk_;
	}

	unsigned long status = SFGAO_FILESYSANCESTOR | SFGAO_FILESYSTEM | SFGAO_HASSUBFOLDER | SFGAO_LINK;

	if (shl_folder->GetAttributesOf(1, (const ITEMIDLIST**)&idl_, &status) != S_OK)
		return FALSE;

	contain_file_system_folder_ = (status & SFGAO_FILESYSANCESTOR) ? TRUE : FALSE;
	is_file_system_ = (status & SFGAO_FILESYSTEM) ? TRUE : FALSE;
	has_sub_folder_ = (status & SFGAO_HASSUBFOLDER) ? TRUE : FALSE;
	link_file_ = !!(status & SFGAO_LINK);
	return TRUE;
}

/********************************************************/
/*						Reset							*/
/********************************************************/
BOOL ShellFolder::Reset()
{
	IShellFolder* shl_folder= shl_parent_folder_ ? shl_parent_folder_->shl_folder_ : shl_folder_;
	unsigned long status= SFGAO_VALIDATE;

	if (shl_folder->GetAttributesOf(1, (const ITEMIDLIST**)&idl_, &status) != S_OK)
	{
		is_existing_ = FALSE;
		contain_file_system_folder_ = FALSE;
		is_file_system_ = FALSE;
		has_sub_folder_ = FALSE;
		return TRUE;
	}

	is_existing_ = TRUE;
	return GetAttribute();
}

/********************************************************/
/*						IsExisting						*/
/********************************************************/
BOOL ShellFolder::IsExisting()
{
	return is_existing_;
}

/********************************************************/
/*						IsFileSystem					*/
/********************************************************/
BOOL ShellFolder::IsFileSystem()
{
	return is_file_system_;
}

/********************************************************/
/*					ContainFileSystemFolder				*/
/********************************************************/
BOOL ShellFolder::ContainFileSystemFolder()
{
	return contain_file_system_folder_;
}

/********************************************************/
/*						HasSubFolder					*/
/********************************************************/
BOOL ShellFolder::HasSubFolder()
{
	return has_sub_folder_;
}

// get current path
//
FolderPathPtr ShellFolder::GetPath() const
{
	ItemIdList path;
	if (!GetPath(path))
		return FolderPathPtr();

	return new DirectoryPath(path);
}

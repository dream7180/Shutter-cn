/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// Those sources were found somewhere on the Internet and heavily modified later

#include "stdafx.h"
#include "ShellItem.h"
#include "ItemIdList.h"
#include "Path.h"
#include <ShObjIdl.h>
//#include <ShLwApi.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



ShellItem::ShellItem(HWND wnd, const ITEMIDLIST* idl)
{
	Init(wnd);

	//GetAttribute();
}


ShellItem::ShellItem(HWND wnd, const int cs_idl)
{
	Init(wnd);

	//if (::SHGetDesktopFolder(&shl_folder_) != S_OK)
	//	throw SF_ERROR_ISHELLFOLDER;

	//if (::SHGetSpecialFolderLocation(m_hWnd, cs_idl, &idl_) != S_OK)
	//	throw SF_ERROR_DESKTOP_PIDL;

	//GetAttribute();
}


void ShellItem::Init(HWND wnd)
{
	m_hWnd = wnd;
	idl_ = NULL;
	FIDL_ = NULL;
	shl_parent_folder_ = NULL;
	is_existing = TRUE;
	is_file_system = FALSE;
	containsFileSystemFolder = FALSE;
	hasSubFolder = FALSE;
	is_link_file = false;
	is_folder = false;
	fileSize_ = 0;
	fileAttributes_ = 0;
	timeModified_.dwHighDateTime = 0;
	timeModified_.dwLowDateTime = 0;

	if (::SHGetMalloc(&malloc_) != S_OK)
		throw SF_ERROR_IMALLOC;
}

/********************************************************/
/*						ShellItem					*/
/********************************************************/
//ShellItem::ShellItem(ShellFolderEx* shl_parent_folder, ITEMIDLIST* idl, SFGAOF status)//, IShellItem* shl_item)
//{
//}

ShellItem::ShellItem(ShellFolderEx* shl_parent_folder, ITEMIDLIST* idl, SFGAOF status,
	DWORD hi_size, DWORD low_size, DWORD fileAttribs, const FILETIME* timeModified)
{
	m_hWnd = shl_parent_folder->m_hWnd;
	//shl_item_ = shl_item;
	malloc_ = shl_parent_folder->malloc_;
	idl_ = idl;
	FIDL_ = NULL;
	shl_parent_folder_ = shl_parent_folder;
	is_existing = TRUE;
	is_file_system = FALSE;
	containsFileSystemFolder = FALSE;
	hasSubFolder = FALSE;
	is_link_file = false;
	is_folder = false;
	fileSize_ = (uint64(hi_size) << 32) + uint64(low_size);
	fileAttributes_ = fileAttribs;
	if (timeModified)
		timeModified_ = *timeModified;
	else
	{
		timeModified_.dwHighDateTime = 0;
		timeModified_.dwLowDateTime = 0;
	}

	malloc_->AddRef();

	GetAttribute(status);
}

/********************************************************/
/*						~ShellItem					*/
/********************************************************/
ShellItem::~ShellItem()
{
	Free();
}
/********************************************************/
/*							Free						*/
/********************************************************/
void ShellItem::Free()
{
//	if (shl_item_ != 0)
//		shl_item_.Release();

	if (idl_)
		FreeIDL(idl_);

	if (FIDL_)
		FreeIDL(FIDL_);

	if (malloc_ != 0)
		malloc_.Release();
}

/********************************************************/
/*						GetName							*/
/********************************************************/
BOOL ShellItem::GetName(TCHAR* name)
{
	if (!FIDL_)
		return FALSE;

	SHFILEINFO info;
	if (!::SHGetFileInfo((TCHAR*)FIDL_, NULL, &info, sizeof info, SHGFI_DISPLAYNAME | SHGFI_PIDL))
		return FALSE;

	_tcscpy(name, info.szDisplayName);

	return TRUE;
}


bool ShellItem::GetPath(TCHAR* path)
{
	if (!GetFullIDL())
		return false;

	return !!::SHGetPathFromIDList(FIDL_, path);
}


bool ShellItem::GetPath(ItemIdList& idlPath)
{
	if (!GetFullIDL())
		return false;

	idlPath = FIDL_;

	if (is_link_file)
	{
		Path path(static_cast<const TCHAR*>(idlPath.GetPath()));
		ItemIdList linked_obj_idl= path.GetLinkedObject();
		if (linked_obj_idl != 0)
			idlPath = linked_obj_idl;
	}

	return true;
}


int ShellItem::GetOverlayIconIndex(BOOL large)
{
	return -1;
}


/********************************************************/
/*					GetIconIndex						*/
/********************************************************/
int ShellItem::GetIconIndex(BOOL large)
{
	if (!FIDL_)
		return -1;
	
	SHFILEINFO info;
	if (!::SHGetFileInfo((TCHAR*)FIDL_, NULL, &info, sizeof info,
		(large ? SHGFI_LARGEICON : SHGFI_SMALLICON) | SHGFI_PIDL | SHGFI_SYSICONINDEX))
		return -1;

	return info.iIcon;
}
/********************************************************/
/*					GetSelectedIconIndex				*/
/********************************************************/
int ShellItem::GetSelectedIconIndex(BOOL large)
{
	if (!FIDL_)
		return -1;
	
	SHFILEINFO info;
	if (!::SHGetFileInfo((TCHAR*)FIDL_, NULL, &info, sizeof info,
		(large ? SHGFI_LARGEICON : SHGFI_SMALLICON) | SHGFI_PIDL | SHGFI_SYSICONINDEX | SHGFI_OPENICON))
		return -1;

	return info.iIcon;
}
/********************************************************/
/*						FreeIDL							*/
/********************************************************/
void ShellItem::FreeIDL(ITEMIDLIST* idl)
{
	malloc_->Free(idl);
}
/********************************************************/
/*						GetFullIDL						*/
/********************************************************/
BOOL ShellItem::GetFullIDL()
{
	if (FIDL_)
		return TRUE;

	if (!(FIDL_ = CreateEmptyIDL()))
		return FALSE;

	ShellItem* shl_folder= this;

	while (shl_folder)
	{
		ITEMIDLIST* idl= FIDL_;
		FIDL_ = ConcatIDL(shl_folder->idl_, FIDL_);
		FreeIDL(idl);

		if (!FIDL_)
			return FALSE;

		shl_folder = shl_folder->shl_parent_folder_;
	}

	return TRUE;
}


const ITEMIDLIST* ShellItem::FullIdl()
{
	GetFullIDL();
	return FIDL_;
}

/********************************************************/
/*					ReleaseFullIDL						*/
/********************************************************/
void ShellItem::ReleaseFullIDL()
{
	if (FIDL_)
	{
		FreeIDL(FIDL_);
		FIDL_ = NULL;
	}
}
/********************************************************/
/*					GetIDLLength						*/
/********************************************************/
unsigned long ShellItem::GetIDLLength(const ITEMIDLIST* idl)
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
ITEMIDLIST* ShellItem::ConcatIDL(const ITEMIDLIST* IDL1, const ITEMIDLIST* IDL2)
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
ITEMIDLIST* ShellItem::CreateEmptyIDL()
{
	ITEMIDLIST* idl = (ITEMIDLIST*)malloc_->Alloc(sizeof(unsigned short));

	if (!idl)
		return NULL;

	idl->mkid.cb = NULL;
	return idl;
}


/********************************************************/
/*					GetAttribute						*/
/********************************************************/
BOOL ShellItem::GetAttribute(SFGAOF status)
{
//	if (shl_item_ == 0)
//		return false;

//	SFGAOF mask= SFGAO_FILESYSANCESTOR | SFGAO_FILESYSTEM | SFGAO_HASSUBFOLDER | SFGAO_LINK;
//	SFGAOF status= 0;

//	if (shl_item_->GetAttributes(mask, &status) != S_OK)
//		return false;

	containsFileSystemFolder = !!(status & SFGAO_FILESYSANCESTOR);
	is_file_system = !!(status & SFGAO_FILESYSTEM);
	hasSubFolder = !!(status & SFGAO_HASSUBFOLDER);
	is_link_file = !!(status & SFGAO_LINK);
	is_folder = !!(status & SFGAO_FOLDER);

	return true;
}

/********************************************************/
/*						Reset							*/
/********************************************************/
/*
BOOL ShellItem::Reset()
{
	IShellFolder* shl_folder= shl_parent_folder_ ? shl_parent_folder_->shl_folder_ : shl_folder_;
	unsigned long status= SFGAO_VALIDATE;

	if (shl_folder->GetAttributesOf(1, (const ITEMIDLIST**)&idl_, &status) != S_OK)
	{
		is_existing = FALSE;
		containsFileSystemFolder = FALSE;
		is_file_system = FALSE;
		hasSubFolder = FALSE;
		is_link_file = false;
		return TRUE;
	}

	is_existing = TRUE;
	return GetAttribute();
} */

/********************************************************/
/*						IsExisting						*/
/********************************************************/
BOOL ShellItem::IsExisting()
{
	return is_existing;
}

/********************************************************/
/*						IsFileSystem					*/
/********************************************************/
BOOL ShellItem::IsFileSystem()
{
	return is_file_system;
}

/********************************************************/
/*					ContainFileSystemFolder				*/
/********************************************************/
BOOL ShellItem::ContainFileSystemFolder()
{
	return containsFileSystemFolder;
}

/********************************************************/
/*						HasSubFolder					*/
/********************************************************/
BOOL ShellItem::HasSubFolder()
{
	return hasSubFolder;
}


bool ShellItem::IsLink() const
{
	return is_link_file;
}


bool ShellItem::IsFolder() const
{
	return is_folder;
}


FILETIME ShellItem::GetModifiedTime() const
{
	return timeModified_;
}


DWORD ShellItem::GetFileAttributes() const
{
	return fileAttributes_;
}


uint64 ShellItem::GetFileLength() const
{
	return fileSize_;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////


ShellFolderEx::ShellFolderEx(HWND wnd, const ITEMIDLIST* idl)
 : ShellItem(wnd, idl)
{
//	Init(wnd);

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


ShellFolderEx::ShellFolderEx(HWND wnd, const int cs_idl)
 : ShellItem(wnd, cs_idl)
{
	if (::SHGetDesktopFolder(&shl_folder_) != S_OK)
		throw SF_ERROR_ISHELLFOLDER;

	if (::SHGetSpecialFolderLocation(m_hWnd, cs_idl, &idl_) != S_OK)
		throw SF_ERROR_DESKTOP_PIDL;

	GetAttribute();
}


ShellFolderEx::~ShellFolderEx()
{
	if (shl_folder_ != 0)
		shl_folder_.Release();
}


ShellFolderEx::ShellFolderEx(ShellFolderEx* shl_parent_folder, ITEMIDLIST* idl, IShellFolder* shl_folder)
 : ShellItem(shl_parent_folder, idl, 0, 0, 0, 0, 0), shl_folder_(shl_folder)
{
	GetAttribute();
}


/********************************************************/
/*					GetSubItemList						*/
/********************************************************/

BOOL ShellFolderEx::GetSubItemList(std::vector<mik::intrusive_ptr<ShellItem> >& vect, bool includeFiles,
								 const FileFilterCallback& includeFileItem)
{
	vect.clear();

	if (shl_folder_ == 0)
		return TRUE;

	IEnumIDListPtr enum_id_list;

	DWORD scanMask= SHCONTF_FOLDERS | SHCONTF_NONFOLDERS /* | SHCONTF_INCLUDEHIDDEN */;

#ifdef _DEBUG
LARGE_INTEGER tm[9];
::QueryPerformanceCounter(&tm[0]);
#endif

	if (shl_folder_->EnumObjects(m_hWnd, scanMask, &enum_id_list) != S_OK)
	{
		ASSERT(false);
		return false;
	}

	if (!GetFullIDL())
	{
		ASSERT(false);
		return false;
	}

	vect.reserve(16);

	ITEMIDLIST* idl_ptr= 0;
	while (enum_id_list->Next(1, &idl_ptr, NULL) == S_OK)
	{
		ItemIdList idl(idl_ptr, false);
		SFGAOF status= SFGAO_LINK | SFGAO_FOLDER;
		if (shl_folder_->GetAttributesOf(1, idl.GetConstPIDL(), &status) != S_OK)
			continue;
//TODO
/*
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
				if (!path.IsFolder())
					continue;			// ignoring links to non-folders! --------------------

				status |= SFGAO_FOLDER;

				idl = linked_obj_idl;
			}
		}
*/
//		if ((status & SFGAO_STREAM) && (status & SFGAO_FOLDER))
		{
			// filter out fake folders - zip files
//			continue;
		}

		//DWORD attrib= 0;
		//DWORD hi_size= 0, low_size= 0;
		//FILETIME modified= { 0, 0 };

		if (status & SFGAO_FOLDER)
		{
			IShellFolderPtr shl_folder;
			if (shl_folder_->BindToObject(idl, NULL, IID_IShellFolder, (void**)&shl_folder) != S_OK)
				continue;

			vect.push_back(new ShellFolderEx(this, idl, shl_folder));

			idl.Detach();
			shl_folder.Detach();
		}
		else if (includeFiles)
		{
			// include files too

			if (includeFileItem)
			{
				TCHAR path[MAX_PATH*2];
				ItemIdList fullIdl(FIDL_, true);
				fullIdl += idl;
				if (!fullIdl.GetPath(path))
					continue;

				//WIN32_FIND_DATA fileInfo;
				//memset(&fileInfo, 0, sizeof fileInfo);

				if (!includeFileItem(path))
					continue;

				//attrib = fileInfo.dwFileAttributes;
				//hi_size = fileInfo.nFileSizeHigh;
				//low_size = fileInfo.nFileSizeLow;
				//modified = fileInfo.ftLastWriteTime;

				//	status.m_ctime = CTime(findFileData.ftCreationTime);
				//	status.atime_ = CTime(findFileData.ftLastAccessTime);
				//	status.m_mtime = CTime(findFileData.ftLastWriteTime);
			}

			//IShellItemPtr shl_item;
			//if (shl_folder_->BindToObject(idl, NULL, IID_IShellItem, (void**)&shl_item) != S_OK)
			//	break;

			vect.push_back(new ShellItem(this, idl, status, 0, 0, 0, 0));

			idl.Detach();
			//shl_item.Detach();
		}
	}

#ifdef _DEBUG
::QueryPerformanceCounter(&tm[1]);
LARGE_INTEGER tt;
tt.QuadPart = tm[1].QuadPart - tm[0].QuadPart;
LARGE_INTEGER frq;
::QueryPerformanceFrequency(&frq);
TCHAR name[MAX_PATH];
GetFullIDL(); GetName(name);
TRACE(L"scan time: %d  for %s \n", int(tt.QuadPart * 1000 / frq.QuadPart), name);
#endif

	return true;
}


BOOL ShellFolderEx::GetSubFolderList(CLinkList* list)
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

			// in W2K local & network folders have status = 0x70400177
			// zip folders in WinXP have no SFGAO_FILESYSANCESTOR flag set, but neither have several other usefull objects

//			if ((status & SFGAO_STREAM) && (status & SFGAO_FOLDER))
			{
				// filter out fake folders - zip files
//				continue;
			}

			if (status & SFGAO_FOLDER)
			{
				IShellFolder* shl_folder= 0;
				if (shl_folder_->BindToObject(idl, NULL, IID_IShellFolder, (void**)&shl_folder) != S_OK)
					shl_folder = NULL;

				if (ShellFolderEx* C_shl_folder= new ShellFolderEx(this, idl, shl_folder))
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
		while (ShellItem* C_shl_folder = (ShellItem*)list->Next())
			delete C_shl_folder;

		list->DeleteAll();
	}

	return !has_error;
}


BOOL ShellFolderEx::GetAttribute()
{
	IShellFolder* shl_folder= shl_folder_;
	if (shl_parent_folder_)
		shl_folder = shl_parent_folder_->shl_folder_;
	else if (shl_folder_desk_)
	{
		// when shl_folder_desk_ is valid idl_ is actually fully qualified PIDL (not relative)
		shl_folder = shl_folder_desk_;
	}

	SFGAOF status = SFGAO_FILESYSANCESTOR | SFGAO_FILESYSTEM | SFGAO_HASSUBFOLDER | SFGAO_LINK;

	if (shl_folder->GetAttributesOf(1, (const ITEMIDLIST**)&idl_, &status) != S_OK)
		return false;

	containsFileSystemFolder = (status & SFGAO_FILESYSANCESTOR) ? TRUE : FALSE;
	is_file_system = (status & SFGAO_FILESYSTEM) ? TRUE : FALSE;
	hasSubFolder = (status & SFGAO_HASSUBFOLDER) ? TRUE : FALSE;
	is_link_file = !!(status & SFGAO_LINK);
	is_folder = true;//!!(status & SFGAO_FOLDER);

	return true;
}

/********************************************************/
/*						Compare							*/
/********************************************************/
int ShellFolderEx::Compare(ShellItem* shl_folder)
{
	IShellFolder* I_shl_folder= (shl_parent_folder_) ? shl_parent_folder_->shl_folder_ : shl_folder_;
	return short(HRESULT_CODE(I_shl_folder->CompareIDs(0, GetIdl(), shl_folder->GetIdl())));
}

int ShellFolderEx::Compare(const ITEMIDLIST* idl)
{
	IShellFolder* I_shl_folder= (shl_parent_folder_) ? shl_parent_folder_->shl_folder_ : shl_folder_;
	return short(HRESULT_CODE(I_shl_folder->CompareIDs(0, idl_, idl)));
}

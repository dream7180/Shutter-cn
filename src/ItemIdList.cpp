/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// ItemIdList.cpp: implementation of the ItemIdList class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ItemIdList.h"
#include "DeleteArray.h"
#include "Exception.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

ItemIdList::ItemIdList()
{
	idl_ = 0;
	GetMalloc();
}


ItemIdList::ItemIdList(const ItemIdList& src)
{
	idl_ = 0;
	GetMalloc();
	*this = src.GetPidl();	// create copy
}


ItemIdList::ItemIdList(const TCHAR* reg_section, const TCHAR* reg_entry)
{
	idl_ = 0;
	GetMalloc();
	Retrieve(reg_section, reg_entry);
}


ItemIdList::ItemIdList(ITEMIDLIST* idl, bool copy)
{
	idl_ = 0;
	GetMalloc();
	if (copy)
		*this = idl;	// create copy
	else
		idl_ = idl;	// assign passed address
}


ItemIdList::ItemIdList(const ITEMIDLIST* idl)
{
	idl_ = 0;
	GetMalloc();
	*this = idl;	// create copy
}


ItemIdList::ItemIdList(const int cs_idl)
{
	idl_ = 0;
	GetMalloc();
	::SHGetSpecialFolderLocation(*AfxGetMainWnd(), cs_idl, &idl_);
}


ItemIdList::~ItemIdList()
{
	if (idl_)
		malloc_->Free(idl_);
}

// free IDL list
//
void ItemIdList::Free()
{
	if (idl_)
	{
		malloc_->Free(idl_);
		idl_ = 0;
	}
}

void ItemIdList::GetMalloc()
{
	if (::SHGetMalloc(&malloc_) != NOERROR)
	{
		THROW_EXCEPTION(L"SHGetMalloc function failed", L"Error obtaining SHGetMalloc result.");
	}
}


UINT GetLastElemLength(const ITEMIDLIST* idl)
{
	if (idl == 0)
		return 0;

	UINT length= 0;

	while (idl->mkid.cb)
	{
		length = idl->mkid.cb;
		idl = (ITEMIDLIST*)((char*)idl + idl->mkid.cb);
	}

	return length;
}


UINT ItemIdList::GetLength(const ITEMIDLIST* idl)
{
	if (idl == 0)
		return 0;

	UINT length= 0;

	while (idl->mkid.cb)
	{
		length += idl->mkid.cb;
		idl = (ITEMIDLIST*)((char*)idl + idl->mkid.cb);
	}

	return length;
}


UINT ItemIdList::GetLength() const
{
	return GetLength(idl_);
}


ITEMIDLIST* ItemIdList::operator += (const ITEMIDLIST* idl)
{
	unsigned long size1= GetLength(idl_);
	unsigned long size2= GetLength(idl);
	unsigned long newSize= size1 + size2;

	ITEMIDLIST* new_idl= (ITEMIDLIST*)malloc_->Alloc(newSize + sizeof(USHORT));

	if (!new_idl)
		return NULL;

	if (idl_)
		CopyMemory(new_idl, idl_, size1);
	CopyMemory(((char*)new_idl + size1), idl, size2);
	((ITEMIDLIST*)((char*)new_idl + newSize))->mkid.cb = NULL;

	if (idl_)
		malloc_->Free(idl_);

	return idl_ = new_idl;
}

// assign IDL
//
ITEMIDLIST* ItemIdList::operator = (const ITEMIDLIST* idl)
{
	if (idl_ == idl)
		return idl_;

	UINT size= GetLength(idl) + sizeof USHORT;

	ITEMIDLIST* new_idl= (ITEMIDLIST*)malloc_->Alloc(size);
	if (new_idl == 0)
		return 0;

	CopyMemory(new_idl, idl, size);

	if (idl_)
		malloc_->Free(idl_);

	idl_ = new_idl;

	return idl_;
}

// assign copy of IDL
//
ItemIdList& ItemIdList::operator = (const ItemIdList& src)
{
	if (this == &src)
		return *this;

	const ITEMIDLIST* idl= src;

	*this = idl;

	return *this;
}


void ItemIdList::CreateEmptyIDL()
{
	if (idl_)
		malloc_->Free(idl_);

	idl_ = 0;

	idl_ = (ITEMIDLIST*)malloc_->Alloc(sizeof(USHORT));

	if (idl_)
		idl_->mkid.cb = NULL;
}


bool ItemIdList::Store(const TCHAR* reg_section, const TCHAR* reg_entry) const
{
	if (!idl_)
		return true;

	const BYTE* data= reinterpret_cast<const BYTE*>(idl_);

	return !!AfxGetApp()->WriteProfileBinary(reg_section, reg_entry,
		const_cast<BYTE*>(data), GetLength() + sizeof USHORT);
}

/*
namespace {
	template<class T> struct DeleteArray
	{
		DeleteArray(T* p) : p_(p)
		{}

		~DeleteArray()
		{
			delete [] p_;
		}

	private:
		T* p_;

		DeleteArray(const DeleteArray&);
		DeleteArray& operator = (const DeleteArray&);
	};
} */


bool ItemIdList::Retrieve(const TCHAR* reg_section, const TCHAR* reg_entry)
{
	BYTE* data= 0;
	UINT bytes= 0;

	if (!AfxGetApp()->GetProfileBinary(reg_section, reg_entry, &data, &bytes))
		return false;

	ITEMIDLIST* idl= reinterpret_cast<ITEMIDLIST*>(data);

	DeleteArray<BYTE> p(data);

	*this = idl;

	return idl_ != 0;
}


// verify whether IDL points to something
//
bool ItemIdList::IsEmpty() const
{
	if (idl_ == 0)
		return true;

	return false;
//	return idl_->mkid.cb == 0;
}

// verify whether IDL is initialize
//
bool ItemIdList::IsInitialized() const
{
	return idl_ != 0;
}


bool ItemIdList::GetPath(TCHAR* path) const
{
	if (idl_ == 0)
		return false;

	return !!::SHGetPathFromIDList(idl_, path);
}

CString ItemIdList::GetPath() const
{
	TCHAR buf[MAX_PATH * 2];
	return GetPath(buf) ? buf : _T("");
}


bool ItemIdList::GetName(TCHAR* name) const
{
	if (idl_ == 0)
		return false;

	SHFILEINFO info;
	if (!::SHGetFileInfo((TCHAR*)idl_, NULL, &info, sizeof info, SHGFI_DISPLAYNAME | SHGFI_PIDL))
		return false;

	_tcscpy(name, info.szDisplayName);

	return true;
}


CString ItemIdList::GetName() const
{
	TCHAR buf[MAX_PATH];
	return GetName(buf) ? buf : _T("");
}


int ItemIdList::GetIconIndex(bool large/*= false*/) const
{
	SHFILEINFO info;
	if (!::SHGetFileInfo((TCHAR*)idl_, NULL, &info, sizeof info,
		(large ? SHGFI_LARGEICON : SHGFI_SMALLICON) | SHGFI_PIDL | SHGFI_SYSICONINDEX))
		return -1;

	return info.iIcon;
}


// assign passed pidl, do not make a copy of it
//
void ItemIdList::AssignNoCopy(ITEMIDLIST* idl)
{
	if (idl_)
		malloc_->Free(idl_);
	idl_ = idl;	// assign passed address
}


ItemIdList::ItemIdList(const TCHAR* abs_path) : idl_(0)
{
	IShellFolderPtr sh_fld;
	if (::SHGetDesktopFolder(&sh_fld) != S_OK)
		THROW_EXCEPTION(L"SHGetDesktopFolder function failed", L"Error obtaining SHGetDesktopFolder result.");

	ULONG ulEaten= 0;
	ITEMIDLIST* idl= 0;
	ULONG ulAttribs= 0;

#ifdef _UNICODE
	LPOLESTR path= const_cast<TCHAR*>(abs_path);
	HRESULT hr= sh_fld->ParseDisplayName(0, 0, path, &ulEaten, &idl, &ulAttribs);
#else
	_bstr_t path= abs_path;
	HRESULT hr= sh_fld->ParseDisplayName(0, 0, path, &ulEaten, &idl, &ulAttribs);
#endif

	GetMalloc();

	if (hr == S_OK)
		AssignNoCopy(idl);
}


ITEMIDLIST* ItemIdList::Detach()
{
	ITEMIDLIST* idl= idl_;
	idl_ = 0;
	return idl;
}


bool ItemIdList::GetParent(ItemIdList& parent) const
{
	if (!IsInitialized())
		return false;

	UINT len= GetLength();
	if (len == 0)
		return false;

	UINT last= GetLastElemLength(idl_);
	ASSERT(last <= len);

	len -= last;

	ITEMIDLIST* new_idl= (ITEMIDLIST*)malloc_->Alloc(len + sizeof(USHORT));

	if (!new_idl)
		return false;

	ASSERT(idl_ != 0);

	if (len > 0)
		CopyMemory(new_idl, idl_, len);
	((ITEMIDLIST*)((char*)new_idl + len))->mkid.cb = NULL;

	parent.AssignNoCopy(new_idl);

	return true;
}


bool ItemIdList::operator == (const ItemIdList& src)
{
	if (idl_ == 0 && src.idl_ == 0)
		return true;

	if (idl_ != 0 && src.idl_ != 0)
	{
		UINT len= GetLength();
		if (src.GetLength() != len)
			return false;

		TCHAR p1[MAX_PATH], p2[MAX_PATH];
		if (GetPath(p1) && src.GetPath(p2))
			return _tcscmp(p1, p2) == 0;

		return false;

		// memcmp is not reliable; same paths created at different time differ
//		return memcmp(idl_, src.idl_, len) == 0;

	}

	return false;
}

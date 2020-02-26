/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// FavoritesFolders.cpp: implementation of the CFavoritesFolders class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "FavoritesFolders.h"
#include "MenuFolders.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


static const TCHAR* REG_FOLDER_NAME= _T("Name");
static const TCHAR* REG_FOLDER_FLAGS= _T("Flag");
static const TCHAR* REG_FOLDERS_COUNT= _T("Count");
static const TCHAR* REG_FOLDER_ENTRY= _T("%02d");

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

FavoriteFolders::FavoriteFolders(int count)
{
}


FavoriteFolders::~FavoriteFolders()
{
}


void FavoriteFolders::AppendMenuItems(CMenuFolders& menu)
{
	int index= 0;
	for (const_iterator it= begin(); it != end(); ++it, ++index)
	{
		CString name= it->GetDisplayName();
		CString item;
		if (index < 10)
			item.Format(_T("%s\tAlt+%c"), static_cast<const TCHAR*>(name), index < 9 ? index + _T('1') : _T('0'));
		else
			item = name;
		menu.InsertItem(it->GetIDL(), ID_BROWSE_FOLDER_1 + index, item);
	}
}


const FavoriteFolder* FavoriteFolders::GetFolder(int index) const
{
	if (index < 0 || index >= size())
	{
		ASSERT(false);
		return 0;
	}

	return &(*element(index));
}

FavoriteFolder* FavoriteFolders::GetFolder(int index)
{
	if (index < 0 || index >= size())
	{
		ASSERT(false);
		return 0;
	}

	return &(*element(index));
}


// store folders in registry
//
void FavoriteFolders::StoreFolders(const TCHAR* reg_section)
{
	int old_count= AfxGetApp()->GetProfileInt(reg_section, REG_FOLDERS_COUNT, 0);

	AfxGetApp()->WriteProfileInt(reg_section, REG_FOLDERS_COUNT, static_cast<int>(size()));

	// write current list
	int index= 0;
	for (const_iterator it= begin(); it != end(); ++it, ++index)
	{
		CString buf;
		buf.Format(REG_FOLDER_ENTRY, index + 1);
		it->Store(reg_section, buf);
	}

	// erase old entries on the end (if there is any)
	for (; index < old_count; ++index)
	{
		CString buf;
		buf.Format(REG_FOLDER_ENTRY, index + 1);
		FavoriteFolder::RemoveEntry(reg_section, buf);
	}
}


// retrieve folders from registry
//
void FavoriteFolders::RetrieveFolders(const TCHAR* reg_section)
{
	int count= AfxGetApp()->GetProfileInt(reg_section, REG_FOLDERS_COUNT, 0);

	for (int i= 0; i < count; ++i)
	{
		CString buf;
		buf.Format(REG_FOLDER_ENTRY, i + 1);
		push_back(FavoriteFolder(reg_section, buf));
	}
}


///////////////////////////////////////////////////////////////////////////////

// store folder in registry
//
void FavoriteFolder::Store(const TCHAR* reg_section, const TCHAR* reg_entry) const
{
	if (!folder_idl_.IsInitialized())
		return;

	folder_idl_.Store(reg_section, reg_entry);
	CString entry= reg_entry;
	AfxGetApp()->WriteProfileInt(reg_section, entry + REG_FOLDER_FLAGS, open_as_root_ ? 1 : 0);
	AfxGetApp()->WriteProfileString(reg_section, entry + REG_FOLDER_NAME, display_name_);
}


void FavoriteFolder::RemoveEntry(const TCHAR* reg_section, const TCHAR* reg_entry)
{
	CString entry= reg_entry;

	if (HKEY sec_key= AfxGetApp()->GetSectionKey(reg_section))
	{
		::RegDeleteValue(sec_key, reg_entry);
		::RegDeleteValue(sec_key, entry + REG_FOLDER_FLAGS);
		::RegDeleteValue(sec_key, entry + REG_FOLDER_NAME);
		::RegCloseKey(sec_key);
	}
}


// remove folder
//
bool FavoriteFolders::Remove(int index)
{
	if (index < 0 || index >= size())
	{
		ASSERT(false);
		return false;
	}

	erase(element(index));

	return true;
}


// add new folder to the list
//
FavoriteFolder* FavoriteFolders::AddNewFolder(const ItemIdList& idl)
{
	push_back(FavoriteFolder(idl, idl.GetName(), true));
	return &back();
}


FavoriteFolder* FavoriteFolders::AddNewFolder(const ItemIdList& idl, const TCHAR* name, bool root)
{
	push_back(FavoriteFolder(idl, name, root));
	return &back();
}


///////////////////////////////////////////////////////////////////////////////

// retrieve folder from registry
//
void FavoriteFolder::Retrieve(const TCHAR* reg_section, const TCHAR* reg_entry)
{
	open_as_root_ = true;
	if (folder_idl_.Retrieve(reg_section, reg_entry))
	{
		CString entry= reg_entry;
		open_as_root_ = AfxGetApp()->GetProfileInt(reg_section, entry + REG_FOLDER_FLAGS, 1) != 0;
		display_name_ = AfxGetApp()->GetProfileString(reg_section, entry + REG_FOLDER_NAME, _T(""));
	}
}

// get folder name
//
CString FavoriteFolder::GetFolderName() const
{
	return folder_idl_.GetName();
}

// get folder path
//
CString FavoriteFolder::GetDisplayPath() const
{
	CString path= folder_idl_.GetPath();
	if (path.IsEmpty())
		path = folder_idl_.GetName();
	return path;
}


// get icon index in system image list
//
int FavoriteFolder::GetIconIndex() const
{
	return folder_idl_.GetIconIndex();
}

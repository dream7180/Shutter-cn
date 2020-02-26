/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// FavoritesFolders.h: interface for the CFavoritesFolders class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FAVORITESFOLDERS_H__801B27C7_8F83_44F0_AFFA_BD1650B1A3F8__INCLUDED_)
#define AFX_FAVORITESFOLDERS_H__801B27C7_8F83_44F0_AFFA_BD1650B1A3F8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "ItemIdList.h"
class CMenuFolders;


class FavoriteFolder
{
public:
	FavoriteFolder() : open_as_root_(true)
	{}

	FavoriteFolder(const FavoriteFolder& fld)
		: folder_idl_(static_cast<const ITEMIDLIST*>(fld.folder_idl_)),
		  display_name_(fld.display_name_), open_as_root_(fld.open_as_root_)
	{}

	FavoriteFolder(const ItemIdList& idl)
		: folder_idl_(static_cast<const ITEMIDLIST*>(idl)), open_as_root_(true)
	{}

	FavoriteFolder(const ItemIdList& idl, const TCHAR* name, bool root)
		: folder_idl_(static_cast<const ITEMIDLIST*>(idl)), display_name_(name), open_as_root_(root)
	{}

	FavoriteFolder(const TCHAR* reg_section, const TCHAR* reg_entry) : open_as_root_(true)
	{
		Retrieve(reg_section, reg_entry);
		if (display_name_.IsEmpty())
			display_name_ = GetFolderName();
	}

	FavoriteFolder& operator = (const FavoriteFolder& fld)
	{
		folder_idl_ = fld.folder_idl_;
		open_as_root_ = fld.open_as_root_;
		display_name_ = fld.display_name_;
		return *this;
	}

	FavoriteFolder& operator = (const ItemIdList& idl)
	{
		folder_idl_ = idl;
		// no change: open_as_root_ & customized name
		return *this;
	}

	// store folder in registry
	void Store(const TCHAR* reg_section, const TCHAR* reg_entry) const;

	// retrieve folder from registry
	void Retrieve(const TCHAR* reg_section, const TCHAR* reg_entry);

	// remove unused entry from registry
	static void RemoveEntry(const TCHAR* reg_section, const TCHAR* reg_entry);

	// get folder name
	CString GetFolderName() const;

	// get folder display path
	CString GetDisplayPath() const;

	// get icon index in system image list
	int GetIconIndex() const;

	const ItemIdList& GetIDL() const		{ return folder_idl_; }
	ItemIdList& GetIDL()					{ return folder_idl_; }

	bool OpenAsRoot() const					{ return open_as_root_; }
	void SetOpenAsRoot(bool open_as_root)	{ open_as_root_ = open_as_root; }

	// get/set folder name for display (customized)
	CString GetDisplayName() const			{ return display_name_; }
	void SetDisplayName(const TCHAR* name)		{ display_name_ = name; }

private:
	ItemIdList folder_idl_;
	CString display_name_;
	bool open_as_root_;
};



class FavoriteFolders : private std::list<FavoriteFolder>	// list used due to the 'strong guarantee'
{
public:
	FavoriteFolders(int count);
	virtual ~FavoriteFolders();

	// build popup menu by appending menu items for each valid folder
	void AppendMenuItems(CMenuFolders& menu);

	// return selected folder
	const FavoriteFolder* GetFolder(int index) const;
	FavoriteFolder* GetFolder(int index);

	// store folders in registry
	void StoreFolders(const TCHAR* reg_section);

	// retrieve folders from registry
	void RetrieveFolders(const TCHAR* reg_section);

	// retrieve number of folders
	int GetCount() const		{ return static_cast<int>(size()); }

	// remove folder
	bool Remove(int index);

	// add new folder to the list
	FavoriteFolder* AddNewFolder(const ItemIdList& idl);
	FavoriteFolder* AddNewFolder(const ItemIdList& idl, const TCHAR* name, bool root);

private:
	const_iterator element(int index) const
	{
		const_iterator it= begin();
		for (; index > 0; ++it, --index)
			;
		return it;
	}

	iterator element(int index)
	{
		iterator it= begin();
		for (; index > 0; ++it, --index)
			;
		return it;
	}
};


#endif // !defined(AFX_FAVORITESFOLDERS_H__801B27C7_8F83_44F0_AFFA_BD1650B1A3F8__INCLUDED_)

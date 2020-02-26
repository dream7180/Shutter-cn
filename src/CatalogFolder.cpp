/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "CatalogFolder.h"
#include "CatalogFile.h"
#include "CatalogHeader.h"
#include <boost/shared_ptr.hpp>
#include "Path.h"
#include "ItemIdList.h"
#include "CatalogPath.h"
#include "StringConversions.h"


struct CatalogFolder::Impl
{
	Impl(const String& path) : catalog_(new CatalogFile(path)) //, path_(path)
	{
//		name_ = Path(path).GetFileNameAndExt();
		dir_ = catalog_->GetRoot();
	}

	Impl(boost::shared_ptr<CatalogFile> catalog, CatalogHeader::DirPtr dir)
		: catalog_(catalog), dir_(dir)
	{
	}

	boost::shared_ptr<CatalogFile> catalog_;
//	String name_;
//	Path path_;
	CatalogHeader::DirPtr dir_;

	bool GetName(TCHAR* name);
	bool GetPath(ItemIdList& idlPath);
	FolderPathPtr GetPath();

	bool HasSubFolders() const
	{
		return dir_ != 0 && !dir_->subdirs_.empty();
	}

	bool IsRoot() const
	{
		return dir_ != 0 && dir_->IsRoot();
	}

	int GetIconIndex(BOOL large, bool open);
};



CatalogFolder::CatalogFolder(const String& path, HWND wnd, bool as_root)
 : ShellFolder(wnd, 0), pImpl_(new Impl(path))
{
}

CatalogFolder::CatalogFolder(boost::shared_ptr<CatalogFile> catalog, CatalogHeader::DirPtr dir, HWND wnd, bool as_root)
 : ShellFolder(wnd, 0), pImpl_(new Impl(catalog, dir))
{
}

CatalogFolder::CatalogFolder(std::auto_ptr<CatalogFolder::Impl> impl)
 : ShellFolder(0, 0), pImpl_(impl)
{
}


BOOL CatalogFolder::GetSubFolderList(CLinkList* list)
{
	if (pImpl_->dir_ == 0)
		return false;

	const size_t count= pImpl_->dir_->subdirs_.size();
	for (size_t i= 0; i < count; ++i)
	{
		CatalogHeader::DirPtr dir= pImpl_->dir_->subdirs_[i];
		std::auto_ptr<Impl> impl(new Impl(pImpl_->catalog_, dir));
		list->Add(new CatalogFolder(impl));
	}

	return true;
}


int CatalogFolder::Impl::GetIconIndex(BOOL large, bool open)
{
	SHFILEINFO info;
	memset(&info, 0, sizeof info);

	UINT flags= SHGFI_USEFILEATTRIBUTES | SHGFI_SYSICONINDEX;

	if (open)
		flags |= SHGFI_OPENICON;

	if (large)
		flags |= SHGFI_LARGEICON;
	else
		flags |= SHGFI_SMALLICON;

	if (IsRoot())
		::SHGetFileInfo(_T("*.catalog"), FILE_ATTRIBUTE_NORMAL, &info, sizeof info, flags);
	else
		::SHGetFileInfo(_T("*"), FILE_ATTRIBUTE_DIRECTORY, &info, sizeof info, flags);

	return info.iIcon;
}


int CatalogFolder::GetIconIndex(BOOL large)
{
	return pImpl_->GetIconIndex(large, false);
}


int CatalogFolder::GetSelectedIconIndex(BOOL large)
{
	return pImpl_->GetIconIndex(large, true);
}


int CatalogFolder::GetOverlayIconIndex(BOOL large)
{
	return 0;
}


BOOL CatalogFolder::GetFullIDL()
{
	return true;
}


void CatalogFolder::ReleaseFullIDL()
{}


BOOL CatalogFolder::GetName(TCHAR* name)
{
	return pImpl_->GetName(name);
}


bool CatalogFolder::Impl::GetName(TCHAR* name)
{
	if (dir_)
	{
		if (dir_->IsRoot())
		{
			String title= catalog_->GetTitle();
			if (title.empty())
				title = Path(catalog_->GetPath()).GetFileNameAndExt();
			_tcsncpy(name, title.c_str(), MAX_PATH);
		}
		else
		{
			_tcsncpy(name, WStr2String(dir_->name_).c_str(), MAX_PATH);
		}
	}
	return true;
}


BOOL CatalogFolder::HasSubFolder()
{
	return pImpl_->HasSubFolders();
}


FolderPathPtr CatalogFolder::GetPath() const
{
	return pImpl_->GetPath();
}


FolderPathPtr CatalogFolder::Impl::GetPath()
{
	return new CatalogPath(catalog_->GetPath(), catalog_, dir_);
}


bool CatalogFolder::Impl::GetPath(ItemIdList& idlPath)
{
	if (IsRoot())
	{
		idlPath = ItemIdList(catalog_->GetPath().c_str());
		return true;
	}
	else
	{
		// to do:
		return false;
	}
}

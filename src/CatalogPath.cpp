/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "CatalogPath.h"
#include "CatalogFile.h"
#include "CatalogFolder.h"
#include "CatalogScanner.h"
#include "ItemIdList.h"
#include "Path.h"
#include "DirectoryPath.h"


struct CatalogPath::Impl
{
	Impl(const String& path, boost::shared_ptr<CatalogFile> catalog, CatalogHeader::DirPtr dir)
		: path_(path), catalog_(catalog), dir_(dir)
	{}

	Impl(const String& path) : path_(path)
	{}

	Path path_;
	boost::shared_ptr<CatalogFile> catalog_;
	CatalogHeader::DirPtr dir_;
};


CatalogPath::CatalogPath(const String& path, boost::shared_ptr<CatalogFile> catalog, CatalogHeader::DirPtr dir)
 : pImpl_(new Impl(path, catalog, dir))
{
}


CatalogPath::CatalogPath(const String& pathToCatalogFile)
 : pImpl_(new Impl(pathToCatalogFile))
{
	pImpl_->catalog_.reset(new CatalogFile(pImpl_->path_));
	pImpl_->dir_ = pImpl_->catalog_->GetRoot();
}


CatalogPath::~CatalogPath()
{}


// returns object that knows how to scan given path to produce image objects
std::auto_ptr<ImageScanner> CatalogPath::GetScanner() const
{
	return std::auto_ptr<ImageScanner>(new CatalogScanner(pImpl_->path_, pImpl_->catalog_, pImpl_->dir_));
}


// go up in the hierarchy of folders
FolderPathPtr CatalogPath::GetParent() const
{
	//TODO:
	ItemIdList folder(pImpl_->path_.GetDir().c_str());
	return new DirectoryPath(folder);
}


// get current path in the form that can be displayed in the address box
String CatalogPath::GetDisplayPath() const
{
	//TODO:
	return pImpl_->path_;
}


bool CatalogPath::TopLevel() const
{
	return false;
}


ShellFolder* CatalogPath::CreateShellFolder(HWND wnd, bool root) const
{
	return new CatalogFolder(pImpl_->catalog_, pImpl_->dir_, wnd, root);
}


ItemIdList CatalogPath::GetPIDL() const
{
	return ItemIdList(pImpl_->path_.c_str());
}


String CatalogPath::GetSelectedFile() const
{
	return String();
}

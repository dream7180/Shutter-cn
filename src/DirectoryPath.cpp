/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "DirectoryPath.h"
#include "ImgScanner.h"
#include "ItemIdList.h"
#include "ShellFolder.h"


struct DirectoryPath::Impl
{
	Impl(const ItemIdList& path) : path_(path)
	{}

//	String path_;
	ItemIdList path_;
	String selected_file_;
};


DirectoryPath::DirectoryPath(const ItemIdList& path) : pImpl_(new Impl(path))
{
}

DirectoryPath::DirectoryPath(const ItemIdList& path, const TCHAR* selectedFile) : pImpl_(new Impl(path))
{
	if (selectedFile && *selectedFile)
		pImpl_->selected_file_ = selectedFile;
}


DirectoryPath::~DirectoryPath()
{}


// returns object that knows how to scan given path to produce image objects
std::auto_ptr<ImageScanner> DirectoryPath::GetScanner() const
{
	TCHAR path[MAX_PATH * 2];
	if (!pImpl_->path_.GetPath(path))
		return std::auto_ptr<ImageScanner>();	// this happens at the "My Computer" level for instance

	return std::auto_ptr<ImageScanner>(new ImgScanner(path, pImpl_->selected_file_.c_str()));
}


// is it top level path or is parent level available?
bool DirectoryPath::TopLevel() const
{
	return pImpl_->path_.GetLength() == 0;
}


// go up in the hierarchy of folders
FolderPathPtr DirectoryPath::GetParent() const
{
	ItemIdList parent;
	if (pImpl_->path_.GetParent(parent))
		return FolderPathPtr(new DirectoryPath(parent));

	return FolderPathPtr();
}


// get current path in the form that can be displayed in the address box
String DirectoryPath::GetDisplayPath() const
{
	String path= pImpl_->path_.GetPath();
	if (path.empty())
		path = String(pImpl_->path_.GetName());

	return path;
}


ShellFolder* DirectoryPath::CreateShellFolder(HWND wnd, bool root) const
{
	return new ShellFolder(wnd, pImpl_->path_);
}


ItemIdList DirectoryPath::GetPIDL() const
{
	return pImpl_->path_;
}


String DirectoryPath::GetSelectedFile() const
{
	if (pImpl_->selected_file_.empty())
		return Path();

	Path path;
	TCHAR buf[MAX_PATH * 2];
	if (pImpl_->path_.GetPath(buf))
	{
		path = buf;
		path.AppendDir(pImpl_->selected_file_.c_str(), false);
	}
	else
		path = pImpl_->selected_file_;

	return path;
}

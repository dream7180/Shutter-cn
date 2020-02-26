/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
#include "FolderPath.h"
#include <boost/shared_ptr.hpp>
#include "CatalogHeader.h"
class CatalogFile;


// Virtual path to the folder within a catalog file


class CatalogPath : public FolderPath
{
public:
	CatalogPath(const String& path, boost::shared_ptr<CatalogFile> catalog, CatalogHeader::DirPtr dir);

	CatalogPath(const String& pathToCatalogFile);

	~CatalogPath();
	//boost::shared_ptr<CatalogFile> catalog_;
	//Path path_;
	//CatalogHeader::DirPtr dir_;

	// returns object that knows how to scan given path to produce image objects
	virtual std::auto_ptr<ImageScanner> GetScanner() const;

	// is it top level path or is parent level available?
	virtual bool TopLevel() const;

	// go up in the hierarchy of folders
	virtual FolderPathPtr GetParent() const;

	// get current path in the form that can be displayed in the address box
	virtual String GetDisplayPath() const;

	virtual ShellFolder* CreateShellFolder(HWND wnd, bool root) const;

	virtual ItemIdList CatalogPath::GetPIDL() const;

	virtual String GetSelectedFile() const;

private:
	struct Impl;
	std::auto_ptr<Impl> pImpl_;
};

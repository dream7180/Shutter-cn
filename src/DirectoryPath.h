/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
#include "FolderPath.h"
class ItemIdList;


class DirectoryPath : public FolderPath
{
public:
	DirectoryPath(const ItemIdList& path);

	// this constructor accepts PIDL of folder as well as a name of selected file (in that folder)
	// that should be used as a starting point for scan procedure
	DirectoryPath(const ItemIdList& path, const TCHAR* selectedFile);

	~DirectoryPath();

	// returns object that knows how to scan given path to produce image objects
	virtual std::auto_ptr<ImageScanner> GetScanner() const;

	// is it top level path or is parent level available?
	virtual bool TopLevel() const;

	// go up in the hierarchy of folders
	virtual FolderPathPtr GetParent() const;

	// get current path in the form that can be displayed in the address box
	virtual String GetDisplayPath() const;

	virtual ShellFolder* CreateShellFolder(HWND wnd, bool root) const;

	virtual ItemIdList GetPIDL() const;

	virtual String GetSelectedFile() const;

private:
	struct Impl;
	std::auto_ptr<Impl> pImpl_;
};

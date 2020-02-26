/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
class ImageScanner;
#include "intrusive_ptr.h"
class FolderPath;
typedef mik::intrusive_ptr<const FolderPath> FolderPathPtr;
class ShellFolder;
class ItemIdList;


// Generalized 'path' to 'folders' that ExifPro scans looking for images

// In particular: path (or PIDL) to the physical location on a disk (folder)


class FolderPath : public mik::counter_base
{
public:
	FolderPath() {}
	virtual ~FolderPath() {}

	// returns object that knows how to scan given path to produce image objects
	virtual std::auto_ptr<ImageScanner> GetScanner() const = 0;

	// is it top level path or is parent level available?
	virtual bool TopLevel() const = 0;

	// go up in the hierarchy of folders
	virtual FolderPathPtr GetParent() const = 0;

	// get current path in the form that can be displayed in the address box
	virtual String GetDisplayPath() const = 0;

	//virtual FolderPathPtr Retrieve(const TCHAR* regSection, const TCHAR* regEntry) const = 0;
	//virtual bool Store(const TCHAR* regSection, const TCHAR* regEntry) const = 0;

	virtual ShellFolder* CreateShellFolder(HWND wnd, bool root) const = 0;

	virtual ItemIdList GetPIDL() const = 0;

	virtual String GetSelectedFile() const = 0;

private:
	FolderPath(const FolderPath&);
	FolderPath& operator = (const FolderPath&);
};

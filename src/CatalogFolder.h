/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once

#include "ShellFolder.h"
//#include "Path.h"
#include "CatalogFile.h"
#include <boost/shared_ptr.hpp>
#include "CatalogHeader.h"
//class CatalogFile;


class CatalogFolder : public ShellFolder
{
public:
	CatalogFolder(const String& path, HWND wnd, bool as_root);
	CatalogFolder(boost::shared_ptr<CatalogFile> catalog, CatalogHeader::DirPtr dir, HWND wnd, bool as_root);

	virtual BOOL GetSubFolderList(CLinkList* list);

	virtual BOOL GetName(TCHAR* name);

	virtual int GetIconIndex(BOOL large);
	virtual int GetSelectedIconIndex(BOOL large);
	virtual int GetOverlayIconIndex(BOOL large);

	virtual BOOL GetFullIDL();
	virtual void ReleaseFullIDL();

	virtual BOOL HasSubFolder();

//	virtual bool GetPath(ItemIdList& idlPath);

	virtual FolderPathPtr GetPath() const;

private:
//	CatalogFolder(const String& name, int id, boost::shared_ptr<CatalogFile> catalog);

	struct Impl;
	std::auto_ptr<Impl> pImpl_;
	CatalogFolder(std::auto_ptr<Impl> impl);

	CatalogFolder(const CatalogFolder&);
	CatalogFolder& operator = (const CatalogFolder&);
};

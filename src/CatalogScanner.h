/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
#include "ImageScanner.h"
#include "CatalogHeader.h"
#include <boost/shared_ptr.hpp>
#include "CatalogFile.h"
class Database;


class CatalogScanner : public ImageScanner
{
public:
	CatalogScanner(const String& path, boost::shared_ptr<CatalogFile> catalog, CatalogHeader::DirPtr dir);

	virtual bool Scan(bool visitSubDirs, uint32& dirVisited, PhotoInfoStorage& store);

	virtual void CancelScan();

private:
	bool break_;
	boost::shared_ptr<CatalogFile> catalog_;
	CatalogHeader::DirPtr dir_;
	String path_;

	bool Scan(bool visitSubDirs, uint32& dirVisited, PhotoInfoStorage& store, boost::shared_ptr<::Database> db, CatalogHeader::DirPtr dir);
};

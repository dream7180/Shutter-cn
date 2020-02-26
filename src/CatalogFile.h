/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

//#include "CatalogFile.h"

#pragma once
#include "CatalogHeader.h"
struct CatalogImgRecord;


class CatalogFile
{
public:
	CatalogFile(String path);
	~CatalogFile();

	String GetTitle() const;
	String GetDescription() const;
	uint32 GetImageCount() const;

	bool ReadRecord(uint64 offset, CatalogImgRecord& img);

	bool NextRecord(uint64& offset);

	//struct const_iterator
	//{
	//	const_iterator();

	//	//String Path() const;
	//	//uint64 FileLength() const;
	//	//int DirNumber();

	//private:
	//	uint32 offset_;
	//};

	//friend struct const_iterator;

	// current image
//	const CatalogImgRecord& Image() const;

	int GetSupportedVersion() const;

	CatalogHeader::DirPtr GetRoot() const;

	String GetPath() const;

private:
	struct Impl;
	std::auto_ptr<Impl> pImpl_;

	CatalogFile(const CatalogFile&);
	CatalogFile& operator = (const CatalogFile&);
};

/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
#include "stdafx.h"
#include "CatalogScanner.h"
#include "CatchAll.h"
#include "CatalogFile.h"
#include "CatalogImgRecord.h"
#include "PhotoInfoCatalog.h"
#include <boost/shared_ptr.hpp>
#include "PhotoInfoStorage.h"
#include "Database/Database.h"

extern void PostProcessTime(PhotoInfoPtr info);
extern void SetAutoRotationFlag(PhotoInfoPtr photo);

//PhotoInfoStorage& info_;



CatalogScanner::CatalogScanner(const String& path, boost::shared_ptr<CatalogFile> catalog, CatalogHeader::DirPtr dir)
 : break_(false), catalog_(catalog), dir_(dir), path_(path)
{
}


bool CatalogScanner::Scan(bool visitSubDirs, uint32& dirVisited, PhotoInfoStorage& store)
{
	try
	{
		boost::shared_ptr<::Database> db(new ::Database(false));
		if (!(db->Open(path_, catalog_->GetSupportedVersion(), true)))
			throw std::exception("Cannot open catalog database");

		if (break_)	// scan interrupted?
			return false;

		return Scan(visitSubDirs, dirVisited, store, db, dir_);
	}
	CATCH_ALL_W(AfxGetMainWnd())

	return false;
}


bool CatalogScanner::Scan(bool visitSubDirs, uint32& dirVisited, PhotoInfoStorage& store,
						  boost::shared_ptr<::Database> db, CatalogHeader::DirPtr dir)
{
	if (dir == 0)
	{
		ASSERT(false);
		return false;
	}

//###### lousy hack #######
if (dirVisited < 0x40000000)
	dirVisited = 0x40000000;
//#########################

	int start_id= dirVisited++;

	if (visitSubDirs)
	{
		const size_t count= dir->subdirs_.size();

		for (size_t i= 0; i < count; ++i)
		{
			bool ret= Scan(visitSubDirs, dirVisited, store, db, dir->subdirs_[i]);
			if (!ret)
				return false;
		}

	}

	if (dir->records_.empty())
		return true;

	const size_t max_count= dir->records_.size();

	for (size_t i= 0; i < max_count; ++i)
	{
		uint64 offset= dir->records_[i];

		if (offset == 0)	// no more images here?
			break;

		CatalogImgRecord img;
		catalog_->ReadRecord(offset, img);

		SmartPhotoPtr photo(new PhotoInfoCatalog(img, db, offset));

		photo->SetVisitedDirId(start_id);

		// maintain unique directory ids
//		if (photo->dir_visited_ >= dirVisited)
//			dirVisited = photo->dir_visited_ + 1;

		SetAutoRotationFlag(photo.get());

		PostProcessTime(photo.get());

		store.Append(photo);

		if (break_)	// scan interrupted?
			return false;

		// refresh
		Notify();
	}

	return true;

/*
//	try
	{
//		CatalogFile cat(path);
		// this is db object shared by catalog photos; they will use it to read exif block
		// on request (in CompleteInfo method)
//		boost::shared_ptr<Database> db(new Database(false));
//		VERIFY(db->Open(path_, catalog_->GetSupportedVersion(), true));

		if (break_)	// scan interrupted?
			return false;

		int start_id= dirVisited;
		uint64 offset= 0;

		while (catalog_->NextRecord(offset))
		{
			const CatalogImgRecord& img= catalog_->Image();

			AutoPtr<PhotoInfo> photo= new PhotoInfoCatalog(img, db, offset);

			photo->dir_visited_ += start_id;

			// maintain unique directory ids
			if (photo->dir_visited_ >= dirVisited)
				dirVisited = photo->dir_visited_ + 1;

			SetAutoRotationFlag(photo.get());

			PostProcessTime(photo.get());

			store.Append(photo);

			if (break_)	// scan interrupted?
				return false;

			// refresh
			Notify();
		}
	}
//	CATCH_ALL
	return true; */
}


void CatalogScanner::CancelScan()
{
	break_ = true;
}

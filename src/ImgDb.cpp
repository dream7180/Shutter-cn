/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "Database/ImageDatabase.h"
#include "ImgDb.h"
#include "Config.h"
#include "ItemIdList.h"

static ImageDatabase dbImages;
static ImageDatabase dbImagesRO;
static bool delete_cache_file= false;

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


String GetDefaultDbFolder()
{
	Path dir= _T("c:\\");

	ITEMIDLIST* pidl= 0;
	if (::SHGetSpecialFolderLocation(0, CSIDL_COMMON_APPDATA, &pidl) == NOERROR && pidl != 0)
	{
		ItemIdList folder(pidl, false);
		dir.assign(folder.GetPath());
	}
	else if (::SHGetSpecialFolderLocation(0, CSIDL_APPDATA, &pidl) == NOERROR && pidl != 0)
	{
		ItemIdList folder(pidl, false);
		dir.assign(folder.GetPath());
	}

	dir.AppendDir(_T("Shutter"), false);
	if (!dir.CreateFolders())
		return _T("");

	return dir;
}


String GetConfiguredDbFileAndPath()
{
	String path= g_Settings.img_cache_db_path_;
	if (path.empty())
		path = GetDefaultDbFolder();

	String db= ImageDatabase::CreateDbFolderIfNeeded(path);

	return db;
}


extern ImageDatabase& GetImageDataBase(bool read_only, bool open)
{
	if (read_only)
	{
		if (open && !dbImagesRO.IsOpen())
		{
			String db= GetConfiguredDbFileAndPath();

			if (!db.empty())
				dbImagesRO.OpenDb(db, true);
		}
		return dbImagesRO;
	}
	else
	{
		if (open && !dbImages.IsOpen())
		{
			String db= GetConfiguredDbFileAndPath();

			if (!db.empty())
				dbImages.OpenDb(db, false);
		}

		return dbImages;
	}
}


extern void SetDeleteFlagForImageDataBase(bool del)
{
	delete_cache_file = del;
}

extern bool GetDeleteFlagForImageDataBase()
{
	return delete_cache_file;
}


extern void DeleteImageDataBase()
{
	if (dbImages.IsOpen()) dbImages.Close();
	if (dbImagesRO.IsOpen()) dbImagesRO.Close();

	ImageDatabase::RemoveDbFiles(dbImages.GetDbFileAndPath());
}

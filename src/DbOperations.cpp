/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "ImgDb.h"
#include "Database\ImageDatabase.h"
#include "DbOperations.h"


bool MarkRecordAsStale(const TCHAR* file_path)
{
	ImageDatabase& db= GetImageDataBase(false, true);

	if (!db.IsOpen())
		throw String(_T("Cannot open cache database to modify stale image record."));

	ImgDataRecord record;
	if (uint64 offset= db.FindImage(file_path, record))
	{
		// clear time stamp to trigger db record regeneration during next scan
		record.time_stamp_.dwHighDateTime = 0;
		record.time_stamp_.dwLowDateTime = 1;	// non-zero to make it different from defaults
		db.AddModify(record);
		return true;
	}

	return false;
}

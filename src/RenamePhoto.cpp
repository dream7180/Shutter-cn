/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "PhotoInfo.h"
#include "PhotoInfoPtr.h"
#include "Database/ImageDatabase.h"

// helper function: rename given photograph's file name:
// - modify file name
// - if 'replace_name_and_ext' then replace name and extension; otherwise only name
// - change path & name in the PhotoInfo
// - update image record in the database

void RenamePhoto(PhotoInfoPtr photo, const String& file_name, bool replace_name_and_ext, ImageDatabase& db)
{
	if (file_name.empty())
		throw std::exception("不允许空文件名");

	if (photo == 0 || !photo->CanRename())
		throw std::exception("重命名参数错误");

	// new path
	Path file= photo->GetPhysicalPath();
	Path renamed= file;
	if (replace_name_and_ext)
		renamed.ReplaceFileNameExt(file_name.c_str());
	else
		renamed.RenameFileName(file_name.c_str());

	if (_tcsicmp(file.c_str(), renamed.c_str()) == 0)
		return;	// no change in name

	if (renamed.FileExists())
		throw std::exception("请选择另外的名称.\n指定的文件名称已存在File with a specified name already exists.");

	// new name
	String name= replace_name_and_ext ? renamed.GetFileName() : file_name;

	// record in the db (if any)
	ImgDataRecord record;
	uint64 offset= db.FindImage(file, record);
	if (offset != 0)
		record.path_ = renamed;

	if (!::MoveFile(file.c_str(), renamed.c_str()))
	{
		String msg(_T("重命名文件失败."));

		DWORD er= ::GetLastError();
		if (er != 0)
		{
			TCHAR buf[500];
			::FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, er, 0, buf, array_count(buf), 0);
			msg += _T('\n');
			msg += buf;
		}

		throw msg;
	}

	// no-fail rename
	photo->Rename(renamed, name);

	// database update
	if (offset != 0)
	{
		photo->SetRecordOffset(0);
		try
		{
			//TODO: lock database?
			// low level operations are already locked, should entire sequence be locked?

			db.DeleteImage(offset);
			photo->SetRecordOffset(db.AddModify(record));
		}
		catch (...)
		{
			//TODO:
			//log it
		}
	}
}

/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "RenameFileThread.h"
#include "PhotoInfo.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

extern void RenamePhoto(PhotoInfoPtr photo, const String& fileName, bool replace_name_and_ext, ImageDatabase& db);


RenameFileThread::RenameFileThread(const VectPhotoInfo& files, const std::vector<String>& new_names, ImageDatabase& db, bool verification)
  : ImgProcessingThread(files.size()), files_(files), new_names_(new_names), db_(db)
{
	verification_ = verification;
}


String RenameFileThread::GetSourceFileName(size_t index) const
{
	ASSERT(index < files_.size());
	return files_[index]->GetPhysicalPath();
}


String RenameFileThread::GetDestFileName(size_t index) const
{
	ASSERT(index < files_.size());
	return files_[index]->GetPhysicalPath();
}


void RenameFileThread::Process(size_t index)
{
	ASSERT(index < files_.size());

	PhotoInfoPtr photo= files_[index];

	//ASSERT(photo->CanRename());

	Path path= photo->GetPhysicalPath();

	String name= path.GetFileNameAndExt();

	const String& new_name= new_names_[index];

	if (verification_)
	{
		// verify destination path/file name

		if (_tcsicmp(name.c_str(), new_name.c_str()) == 0)
		{
			// no name change apart from letter case perhaps
			if (name != new_name)	// latter case changed?
			{
				// this change is OK if file is not read-only
				DWORD attrib= ::GetFileAttributes(path.c_str());

				if (attrib == INVALID_FILE_ATTRIBUTES)
				{
					SendPhotoStatus(NoFile);
					return;
				}

				if (attrib & FILE_ATTRIBUTE_READONLY)
				{
					SendPhotoStatus(ReadOnlyFile);
					return;
				}
			}
		}
		else
		{
			// name changed

			DWORD attrib= ::GetFileAttributes(path.c_str());

			if (attrib == INVALID_FILE_ATTRIBUTES)
			{
				SendPhotoStatus(NoFile);
				return;
			}
			if (attrib & FILE_ATTRIBUTE_READONLY)
			{
				SendPhotoStatus(ReadOnlyFile);
				return;
			}

			Path new_path= path;
			new_path.ReplaceFileNameExt(new_name.c_str());

			if (new_path.size() >= MAX_PATH)
			{
				SendPhotoStatus(InvalidPath);
				return;
			}

			DWORD renamed_attrib= ::GetFileAttributes(new_path.c_str());

			if (renamed_attrib == INVALID_FILE_ATTRIBUTES)
				return;	// good, this name is available

			// not good--name taken
			if (renamed_attrib & FILE_ATTRIBUTE_DIRECTORY)
				SendPhotoStatus(DirNameConflict);
			else
				SendPhotoStatus(FileNameConflict);
		}
	}
	else
	{
		// rename/move
		RenamePhoto(photo, new_names_[index], true, db_);
	}
}


ImgProcessingThread* RenameFileThread::Clone()
{
	return new RenameFileThread(files_, new_names_, db_, verification_);
}

/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// This is worker thread for renaming (image) files

#pragma once
#include "ImgProcessingThread.h"
#include "VectPhotoInfo.h"
class ImageDatabase;


class RenameFileThread : public ImgProcessingThread
{
public:
	// rename photos
	RenameFileThread(const VectPhotoInfo& files, const std::vector<String>& new_names, ImageDatabase& db, bool verification);

	enum Errors { OK, NoFile, ReadOnlyFile, FileNameConflict, DirNameConflict, InvalidPath, EmptyName };

	virtual ImgProcessingThread* Clone();

private:
	// process one file
	virtual void Process(size_t index);

	virtual String GetSourceFileName(size_t index) const;
	virtual String GetDestFileName(size_t index) const;

	const VectPhotoInfo& files_;
	const std::vector<String>& new_names_;
	ImageDatabase& db_;
	bool verification_;
};

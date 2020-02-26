/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// This is worker thread for applying metadata to several images at once;
// It's also useful for applying/removing tags and setting rating (stars),
// as they too involve changing and re-writing metadata.


#pragma once
#include "ImgProcessingThread.h"
#include "PhotoInfo.h"
#include "VectPhotoInfo.h"
class FileStream;
#include "XmpData.h"


class ApplyMetadataThread : public ImgProcessingThread
{
public:
	// write xmp into all files
	ApplyMetadataThread(const VectPhotoInfo& files, const XmpData& xmp, CWnd* view);
	// apply/remove tag to all files
	ApplyMetadataThread(const VectPhotoInfo& files, const String& tag, bool apply, CWnd* view);
	// apply rating
	ApplyMetadataThread(const VectPhotoInfo& files, int stars, CWnd* view);

	virtual ImgProcessingThread* Clone();

private:
	// process one file
	virtual void Process(size_t index);

	virtual String GetSourceFileName(size_t index) const;
	virtual String GetDestFileName(size_t index) const;

	const VectPhotoInfo& files_;
	XmpData xmp_;
	String tag_;
	bool write_metadata_;
	bool apply_tag_;
	CWnd* wnd_to_report_changes_;
	bool persist_tags_;
	int stars_;
	bool apply_rating_;
};


extern void ApplyTagToPhoto(PhotoInfoPtr photo, const String& tag, bool apply, bool persist, const PhotoInfo::WriteAccessFn& 
get_write_access);

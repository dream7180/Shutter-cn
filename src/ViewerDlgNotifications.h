/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once

class ViewerDlgNotifications
{
public:
	virtual void CurrentChanged(PhotoInfoPtr photo) = 0;
	virtual void PhotoDeleted(PhotoInfoPtr photo) = 0;
	virtual void PhotoOrientationChanged(PhotoInfoPtr photo) = 0;
	virtual void SelectionRequest(const VectPhotoInfo& selected) = 0;
	enum FileOper { Copy, Move, Rename };
	virtual void FileOperationRequest(const VectPhotoInfo& photos, FileOper oper, CWnd* parent) = 0;
	virtual void SelectAndDelete(const VectPhotoInfo& photos) = 0;
	// animation is about to start; update dirty windows so they don't block main thread
	virtual void AnimationStarts() = 0;
};

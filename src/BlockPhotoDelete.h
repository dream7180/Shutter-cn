/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once

// This helper class is to be used in a PhotoInfoXxx constructors that call other functions
// expecting PhotoInfoPtr smart pointer.
// The reason why this is needed is because in a constructor reference count is zero, so
// passing paras by smart pointer will delete *this when function parameter goes out of scope.
// This class artificially increments ref counter for the file time of the photo info constructor,
// by keeping extra smart pointer. In the end, this smart pointer is released without deleting
// photo, and photo's ref counter is restored to it's original value (zero)

class BlockPhotoDelete
{
public:
	BlockPhotoDelete(PhotoInfo* ptr) : photo_(ptr)
	{}

	~BlockPhotoDelete()
	{
#ifdef PHOTO_INFO_SMART_PTR
		photo_->xadd_ref();
		PhotoInfo* ptr= photo_.get();
		photo_ = 0;
		ptr->xrelease_ref();
#endif
	}

	PhotoInfoPtr photo_;
};

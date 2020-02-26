/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// PhotoInfoStorage.h: interface for the PhotoInfoStorage class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PHOTOINFOVECTOR_H__A26B4CD6_D213_4E6D_AFE4_E2DBA083AD0B__INCLUDED_)
#define AFX_PHOTOINFOVECTOR_H__A26B4CD6_D213_4E6D_AFE4_E2DBA083AD0B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "PhotoInfo.h"
#include "VectPhotoInfo.h"


class PhotoInfoStorage;

class PhotoInfoStorageObserver
{
public:
	// photo delete notification
	virtual void Deleting(PhotoInfoStorage& storage, const VectPhotoInfo& selected);
	virtual void Deleting(PhotoInfoStorage& storage, PhotoInfoPtr photo);

	virtual ~PhotoInfoStorageObserver();

protected:
	PhotoInfoStorageObserver(PhotoInfoStorage& storage);
	PhotoInfoStorage& storage_;
};



class PhotoInfoStorage
{
public:
	PhotoInfoStorage();
	virtual ~PhotoInfoStorage();

	// add new element (and take it's ownership)
	void Append(SmartPhotoPtr photo);

	// remove elements (expensive)
	void Remove(const VectPhotoInfo& selected);	// O(n^2)
	void Remove(PhotoInfoPtr photo);	// O(n)

	// get count of elements
	size_t size() const;

	// remove elements
	void clear();

	// copy list to given vector
	void Copy(VectPhotoInfo& photos) const;
	void Copy(size_t from, size_t to, VectPhotoInfo& photos) const;

	bool empty() const;

	void reserve(size_t size);

	// get n-th item
	PhotoInfoPtr GetNthItem(size_t index);
	PhotoInfoPtr operator [] (size_t index);

	// find by name
//	PhotoInfoPtr FindItem(const TCHAR* path) const;

	void Attach(PhotoInfoStorageObserver& observer);
	void Detach(PhotoInfoStorageObserver& observer);

private:
	struct Impl;
	std::auto_ptr<Impl> impl_;

	PhotoInfoStorage(const PhotoInfoStorage&);
	PhotoInfoStorage& operator = (const PhotoInfoStorage&);

	void DeleteNotify(const VectPhotoInfo& selected);
	void DeleteNotify(PhotoInfoPtr photo);
};


//typedef VectPhotoInfo VectPhotoInfo;


// sort images by similarity to given image
void SortBySimilarity(const VectPhotoInfo& input, PhotoInfo& img, VectPhotoInfo& sorted, float colorVsShapeWeight);


#endif // !defined(AFX_PHOTOINFOVECTOR_H__A26B4CD6_D213_4E6D_AFE4_E2DBA083AD0B__INCLUDED_)

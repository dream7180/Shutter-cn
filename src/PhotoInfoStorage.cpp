/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// PhotoInfoStorage.cpp: implementation of the PhotoInfoStorage class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PhotoInfoStorage.h"
#include <float.h>
#include <boost/bind.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <map>
#include <vector>
#include <ppl.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////

// define USE_MAP to use std::map for fast search based on physical path
// note that PhotoInfoCatalog doesn't return unique/meaningful path

#ifdef PHOTO_INFO_SMART_PTR
	typedef VectPhotoInfo Storage;
	PhotoInfoPtr* FirstElement(Storage& s)	{ return s.empty() ? 0 : &s.front(); }
#else
	typedef boost::ptr_vector<PhotoInfo> Storage;
	PhotoInfoPtr* FirstElement(Storage& s)	{ return s.c_array(); }
#endif


struct PhotoInfoStorage::Impl
{
	mutable CCriticalSection access_ctrl_;
#ifdef USE_MAP
	std::map<String, PhotoInfoPtr> map_;
#endif
	Storage storage_;
	LONG count_;
	std::vector<PhotoInfoStorageObserver*> observers_;
};


PhotoInfoStorage::PhotoInfoStorage() : impl_(new Impl())
{
	impl_->count_ = 0;
	impl_->storage_.reserve(100);
}

PhotoInfoStorage::~PhotoInfoStorage()
{}


size_t PhotoInfoStorage::size() const
{
	CSingleLock lock(&impl_->access_ctrl_, true);
	return impl_->storage_.size();
}

/*
const PhotoInfo& PhotoInfoStorage::operator [] (int index) const
{
	CSingleLock lock(&impl_->access_ctrl_, true);
	ASSERT(index >= 0 && index < size());
	return vector<PhotoInfo>::operator [] (index);
}


PhotoInfo& PhotoInfoStorage::operator [] (int index)
{
	CSingleLock lock(&impl_->access_ctrl_, true);
	ASSERT(index >= 0 && index < size());
	return vector<PhotoInfo>::operator [] (index);
}
*/


void PhotoInfoStorage::Append(SmartPhotoPtr photo)
{
	CSingleLock lock(&impl_->access_ctrl_, true);

	PhotoInfoPtr last= photo.get();

	impl_->storage_.push_back(ReleaseSmartPhotoPtr(photo));

#ifdef USE_MAP
	impl_->map_[last->GetPhysicalPath()] = last;
#endif

//	::InterlockedExchange(&impl_->count_, impl_->storage_.size());
}


void PhotoInfoStorage::clear()
{
	CSingleLock lock(&impl_->access_ctrl_, true);
	impl_->storage_.clear();
#ifdef USE_MAP
	impl_->map_.clear();
#endif
}


bool PhotoInfoStorage::empty() const
{
	CSingleLock lock(&impl_->access_ctrl_, true);
	return impl_->storage_.empty();
}

void PhotoInfoStorage::reserve(size_t size)
{
	CSingleLock lock(&impl_->access_ctrl_, true);
	impl_->storage_.reserve(size);
}


// copy list to given vector
void PhotoInfoStorage::Copy(VectPhotoInfo& photos) const
{
	CSingleLock lock(&impl_->access_ctrl_, true);

	photos.assign(FirstElement(impl_->storage_), FirstElement(impl_->storage_) + impl_->storage_.size());
}


void PhotoInfoStorage::Copy(size_t from, size_t to, VectPhotoInfo& photos) const
{
	CSingleLock lock(&impl_->access_ctrl_, true);

	if (from < size() && to <= size() && from < to)
		photos.assign(FirstElement(impl_->storage_) + from, FirstElement(impl_->storage_) + to);
	else
	{
		ASSERT(false);
		photos.clear();
	}
}


// get n-th item
PhotoInfoPtr PhotoInfoStorage::GetNthItem(size_t index)
{
	CSingleLock lock(&impl_->access_ctrl_, true);

	ASSERT(index >= 0 && index < impl_->storage_.size());

	if (index < impl_->storage_.size())
		return
#ifdef PHOTO_INFO_SMART_PTR
			impl_->storage_[index];
#else
			&impl_->storage_[index];
#endif

	return 0;
}


PhotoInfoPtr PhotoInfoStorage::operator [] (size_t index)
{
	return GetNthItem(index);
}


struct ThisPhoto
{
	ThisPhoto(PhotoInfoPtr arg) : arg_(arg)
	{}

#ifdef PHOTO_INFO_SMART_PTR
	bool operator () (const PhotoInfoPtr& p) const
	{
		return p == arg_;
	}
#else
	bool operator () (const PhotoInfo& p) const
	{
		return &p == arg_;
	}
#endif
private:
	PhotoInfoPtr arg_;
};


void Erase(Storage& s, PhotoInfoPtr& photo)	// O(n)
{
#ifdef PHOTO_INFO_SMART_PTR
	Storage::iterator it= std::find_if(s.begin(), s.end(), ThisPhoto(photo));
	if (it != s.end())
		s.erase(it);
	else
	{ ASSERT(false); }
#else
	s.erase_if(ThisPhoto(photo));
#endif
}


void PhotoInfoStorage::Remove(const VectPhotoInfo& selected)
{
	DeleteNotify(selected);

	CSingleLock lock(&impl_->access_ctrl_, true);

	for (VectPhotoInfo::const_iterator it= selected.begin(); it != selected.end(); ++it)
	{
		PhotoInfoPtr photo= *it;
		Path path= photo->GetPhysicalPath();

		//TODO: this is expensive!
		Erase(impl_->storage_, photo);

#ifdef USE_MAP
		impl_->map_.erase(path);
#endif
	}
}


void PhotoInfoStorage::Remove(PhotoInfoPtr photo)
{
	DeleteNotify(photo);

	CSingleLock lock(&impl_->access_ctrl_, true);

	Path path= photo->GetPhysicalPath();

	Erase(impl_->storage_, photo);

#ifdef USE_MAP
	impl_->map_.erase(path);
#endif
}


// sort images by similarity to given image
//
struct Photo // local struct for sorting purposes
{
	Photo(PhotoInfoPtr photo) : photo(photo), distance(0.0f) {}
	Photo(PhotoInfo& photo) : photo(&photo), distance(0.0f) {}

	Photo(const Photo& photo)  { *this = photo; }

	Photo& operator = (const Photo& photo)
	{
		this->photo = photo.photo;
		distance = photo.distance;
		return *this;
	}

	bool operator < (const Photo& photo) const
	{
		return distance < photo.distance;
	}

	void UpdateIndex()
	{
		if (photo->index_.IsInitialized())
			return;

		Dib img;
		if (photo->CreateThumbnail(img, 0) == IS_OK)
			photo->index_.CalcHistogram(img);
	}

	PhotoInfoPtr photo;
	float distance;
};


void SortBySimilarity(const VectPhotoInfo& input, PhotoInfo& img, VectPhotoInfo& sorted, float colorVsShapeWeight)
{
	sorted.clear();

	ASSERT(!input.empty());
	if (input.empty())
		return;

	std::vector<Photo> photos;
	{
//		CSingleLock lock(&impl_->access_ctrl_, true);
		photos.assign(input.begin(), input.end());
	}

	// extremely expensive operation... provide progress and cancel btn
//	for_each(photos.begin(), photos.end(), boost::bind(&Photo::UpdateIndex, _1));
	Concurrency::parallel_for_each(photos.begin(), photos.end(), boost::bind(&Photo::UpdateIndex, _1));

	sorted.reserve(photos.size());

	for (std::vector<Photo>::iterator it= photos.begin(); it != photos.end(); ++it)
		it->distance = img.index_.Distance(it->photo->index_, colorVsShapeWeight);

	std::sort(photos.begin(), photos.end());

	{
		for (std::vector<Photo>::iterator it= photos.begin(); it != photos.end(); ++it)
		{
#ifdef _DEBUG
			it->photo->index_.distance_ = it->distance;
#endif
			sorted.push_back(it->photo);
		}
	}
}

/*
void PhotoInfoStorage::SortBySimilarity(PhotoInfo& img, VectPhotoInfo& sorted, float color_vs_shape_weight)
{
	ASSERT(!empty());
	if (empty())
		return;

	std::vector<Photo> photos;
	{
		CSingleLock lock(&impl_->access_ctrl_, true);
		photos.assign(impl_->storage_.begin(), impl_->storage_.end());
	}

	// extremely expensive operation... provide progress and cancel btn
	for_each(photos.begin(), photos.end(), boost::bind(&Photo::UpdateIndex, _1));

	sorted.clear();
	sorted.reserve(size());

	for (vector<Photo>::iterator it= photos.begin(); it != photos.end(); ++it)
		it->distance = img.index_.Distance(it->photo->index_, color_vs_shape_weight);

	std::sort(photos.begin(), photos.end());

	{
		for (vector<Photo>::iterator it= photos.begin(); it != photos.end(); ++it)
		{
#ifdef _DEBUG
			it->photo->index_.distance_ = it->distance;
#endif
			sorted.push_back(it->photo);
		}
	}
} */


void PhotoInfoStorage::Attach(PhotoInfoStorageObserver& observer)
{
	if (impl_->observers_.capacity() == 0)
		impl_->observers_.reserve(4);

	impl_->observers_.push_back(&observer);
}


void PhotoInfoStorage::Detach(PhotoInfoStorageObserver& observer)
{
	std::vector<PhotoInfoStorageObserver*>::iterator it= find(impl_->observers_.begin(), impl_->observers_.end(), &observer);
	if (it != impl_->observers_.end())
		impl_->observers_.erase(it);
	else
	{ ASSERT(false); }
}


PhotoInfoStorageObserver::PhotoInfoStorageObserver(PhotoInfoStorage& storage) : storage_(storage)
{
	storage_.Attach(*this);
}

PhotoInfoStorageObserver::~PhotoInfoStorageObserver()
{
	storage_.Detach(*this);
}

void PhotoInfoStorageObserver::Deleting(PhotoInfoStorage& storage, const VectPhotoInfo& selected) {}
void PhotoInfoStorageObserver::Deleting(PhotoInfoStorage& storage, PhotoInfoPtr photo) {}


void PhotoInfoStorage::DeleteNotify(const VectPhotoInfo& selected)
{
	std::vector<PhotoInfoStorageObserver*>::iterator end= impl_->observers_.end();

	for (std::vector<PhotoInfoStorageObserver*>::iterator it= impl_->observers_.begin(); it != end; ++it)
		(*it)->Deleting(*this, selected);
}

void PhotoInfoStorage::DeleteNotify(PhotoInfoPtr photo)
{
	std::vector<PhotoInfoStorageObserver*>::iterator end= impl_->observers_.end();

	for (std::vector<PhotoInfoStorageObserver*>::iterator it= impl_->observers_.begin(); it != end; ++it)
		(*it)->Deleting(*this, photo);
}

#ifdef USE_MAP
PhotoInfoPtr PhotoInfoStorage::FindItem(const TCHAR* path) const
{
	CSingleLock lock(&impl_->access_ctrl_, true);

	std::map<String, PhotoInfoPtr>::const_iterator it= impl_->map_.find(path);
	if (it != impl_->map_.end())
		return it->second;

	return 0;
}
#endif

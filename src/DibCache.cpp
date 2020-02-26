/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "DibCache.h"
#include "PhotoInfo.h"


void RemoveAllPhotoDibs(ConstPhotoInfoPtr photo, DibCache& cache)
{
	const size_t count= cache.Size();	// this size doesn't change when removing elements

//	DibCache::iterator it= cache.GetFirstPos();

	// this loop is travesing storage part of a cache
	for (size_t i= 0; i < count; ++i)
	{
		const DibImgKey& key= cache.GetKey(i);

		if (key.key == photo)
		{
			// remove item from cache: map in a cache is updated, but storage doesn't
			// shrink (item pointed to is freed though); it's safe to continue the loop
			cache.Remove(key);
		}

//		it = cache.GetNextPos(it);
	}
}


std::pair<DibImg*, bool> GetCacheEntry(ConstPhotoInfoPtr key, CSize size, DibCache& cache)
{
	return cache.GetEntry(DibImgKey(key, size));
}

/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once

#include "Cache.h"
#include "Dib.h"
#include "PhotoInfoPtr.h"

struct DibImg
{
	DibImg() : original_size(0, 0)
	{}

	Dib dib;
	CSize original_size;
};


class PhotoInfo;


struct DibImgKey
{
	DibImgKey(ConstPhotoInfoPtr key, CSize size) : key(key), size(size)
	{}
	DibImgKey() : key(0), size(0, 0)
	{}

	ConstPhotoInfoPtr key;
	CSize size;

	size_t operator () () const	// hash value
	{
		size_t n= GetHashValue(key);
		return (n >> 4 ^ size.cx ^ size.cy);
	}

	bool operator < (const DibImgKey& k) const
	{
		if (key != k.key)
			return key < k.key;
		return (size.cx << 16) + size.cy < (k.size.cx << 16) + k.size.cy;
	}

	bool operator == (const DibImgKey& k) const
	{
		return key == k.key && size ==  k.size;
	}
};


inline size_t GetHashValue(const DibImgKey& key)	{ return key(); }


typedef Cache<DibImg, DibImgKey> DibCache;



extern void RemoveAllPhotoDibs(ConstPhotoInfoPtr key, DibCache& cache);

extern std::pair<DibImg*, bool> GetCacheEntry(ConstPhotoInfoPtr key, CSize size, DibCache& cache);

/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "DibCache.h"
#include "PhotoCache.h"
#include "PhotoInfo.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


AutoPtr<DibCache> global_dib_cache;			// one central img cache to draw thumbnails
AutoPtr<PhotoCache> global_photo_cache;		// one central photo cache

extern DWORDLONG GetPhysicalMemoryStatus();


void SetImageCache(int image_cache_size)
{
	if (image_cache_size < 0 || image_cache_size > 100)
		image_cache_size = 20;

	// check physical memory amount
	DWORDLONG total_phys_mem= GetPhysicalMemoryStatus();

	// hopefully this won't overflow any time soon
	DWORDLONG memory= total_phys_mem * image_cache_size / 100;

#ifndef _WIN64
	// limit used memory to 1 GB on an x86 system to leave some free virtual memory space
	const DWORDLONG one_gig= 1 * 1024 * 1024 * 1024;

	if (memory > one_gig)
		memory = one_gig;
#endif

	// amount of memory to be used by cache in KB
	auto RAM_KB= memory / 1024;

	auto thumbnails_ram= RAM_KB / 3;	// 33.3% of RAM to be used by thumbnails; the rest by big images

	// one thumbnail image needs 120x160x3 bytes (56 KB), plus Dib overhead; note: this is very conservative number
	// because thumbnails of requested size are kept in cache, frequently smaller than 120x160
	auto images= thumbnails_ram / 56;

	if (images < 1)
		images = 50;	// minimum cache size; it may be too small a number,
						// at least screen-worth of thumbnails should be supported

	// here only no of images is a limiting factor
	global_dib_cache = new DibCache(static_cast<size_t>(images));

	auto mem_limit= RAM_KB - thumbnails_ram;
	size_t photos_in_cache= static_cast<size_t>(mem_limit / 256);	// it's four small photos per 1 MB of buffer space

	if (photos_in_cache < 10)
		photos_in_cache = 10;

	// no of photos is not critical, but both this and amount of mem are limiting factors
	// so it's best to increase no of photos and let memory decide how many images fit in cache
	global_photo_cache = new PhotoCache(photos_in_cache, static_cast<size_t>(mem_limit) * 1024);
}


//void SetPhotoCache(int images, size_t memory_limit)
//{
//	global_photo_cache = new PhotoCache(images, memory_limit);
//}

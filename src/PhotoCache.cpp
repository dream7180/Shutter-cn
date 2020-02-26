/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "PhotoCache.h"
#include "DecoderJob.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


void CacheImg::SetDib(DibPtr dib, ConstPhotoInfoPtr photo)
{
	dib_ = dib;
//TRACE(L"cached dib %s: %d x %d\n", photo ? photo->name_.c_str() : L"self", dib_->GetWidth(), dib_->GetHeight());
}

void CacheImg::SetReductionFactor(int f)
{ reduction_factor_ = f; }

void CacheImg::SetOriginalSize(CSize size)
{
	ASSERT(dib_.get());
	if (dib_.get())
	{
		// make sure original size is in agreement with possibly rotated image
		if (dib_->GetWidth() < dib_->GetHeight())	//portrait?
		{
			if (size.cx > size.cy)
				std::swap(size.cx, size.cy);
		}
		else	// landscape
		{
			if (size.cx < size.cy)
				std::swap(size.cx, size.cy);
		}
	}

	original_size_ = size;
}

void CacheImg::SetColorProfile(ColorProfilePtr p)
{ pICCProfile_ = p; }

//void CacheImg::SetRotation(bool rotated)
//{ rotated_ = rotated; }


void PhotoCache::CacheDecodedImage(DecoderJobInterface* decoder, ImageStat status)
{
	if (decoder == 0)
		return;

	PhotoInfoPtr photo= ConstCast(decoder->GetPhoto());
	photo->photo_stat_ = status;

	DibPtr dib= decoder->GetBitmap();
	if (dib.get() == 0)
		return;

	std::pair<CacheImg*, bool> entry= GetEntry(photo);
	// by the time image is decoded another window might have already updated cache;
	// check this cache entry and update it if it's empty or newly decoded bmp is larger than the one in cache

	bool update= true;

	CacheImg* img= entry.first;

	if (entry.second && img->Dib())	// bmp already there?
		if (img->ReductionFactor() <= decoder->ReductionFactor())	// bigger or same size?
		{
			// take care of non-permanent rotation: cached img may be rotated;
			// normalize sizes so width is GTEq height
			CSize cached= img->Dib()->GetSize();
			CSize decoded= dib->GetSize();

			if (cached.cy > cached.cx)
				std::swap(cached.cx, cached.cy);
			if (decoded.cy > decoded.cx)
				std::swap(decoded.cx, decoded.cy);

			if (cached.cx >= decoded.cx && cached.cy >= decoded.cy)
				update = false;
		}

	if (update)
	{
		// take over the bitmap
		img->SetDib(decoder->GetBitmap(), photo);
		// remember its reduction factor
		img->SetReductionFactor(decoder->ReductionFactor());
		// and image dimensions
		img->SetOriginalSize(decoder->GetOriginalSize());
		// ICC profile
		img->SetColorProfile(photo->GetColorProfile());
		// rotation
		//img->SetRotation(decoder->Rotated() != 0);

		// check memory consumption
		size_t memory= GetAllocatedMemorySize();

		// if excessive free some old elements
		if (memory > memory_limit_)
		{
			size_t count= Size();
			iterator iter= GetLastPos();		// start from last (oldest) entry
			while (--count > 0)
			{
				CacheImg* cimg= GetItem(iter);

				if (cimg == img)
					break;

				if (cimg != 0 && cimg->Dib() != 0 && cimg->Dib()->IsValid())
				{
					memory -= cimg->Dib()->GetBufferSize();

					iterator prev= GetPrevPos(iter);

					Remove(iter);

					iter = prev;

					if (memory <= memory_limit_)
						break;
				}
				else
					iter = GetPrevPos(iter);
			}
		}
	}

	//TODO: better clean up
	decoder->Quit();

	// delete decoder
//	decoder = 0;
}


size_t PhotoCache::GetAllocatedMemorySize() const
{
	size_t count= Size();
	size_t memory= 0;
	iterator iter= GetFirstPos();		// start from first (newest) entry
	for (size_t i= 0; i < count; ++i)
	{
		CacheImg* img= GetItem(iter);

		if (img == 0 || img->Dib() == 0)	// do not traverse over empty entries (they follow)
			break;

		if (img->Dib()->IsValid())
		{
			memory += img->Dib()->GetBufferSize();
			if (memory > memory_limit_)	// if more than the limit stop counting
				break;
		}

		iter = GetNextPos(iter);
	}

	return memory;
}


size_t PhotoCache::GetFreeMemorySize() const
{
	size_t memory= GetAllocatedMemorySize();

	if (memory > memory_limit_)
		return 0;

	return memory_limit_ - memory;
}

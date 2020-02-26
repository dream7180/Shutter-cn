/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
#include "DibCache.h"
#include "ColorProfileForward.h"
#include "ImageStat.h"
class DecoderJobInterface;


class CacheImg
{
public:
	CacheImg()
	{
		reduction_factor_ = 0;
		original_size_ = CSize(0, 0);
	}

	DibPtr Dib() const				{ return dib_; }
	int ReductionFactor() const		{ return reduction_factor_; }
	CSize OriginalSize() const		{ return original_size_; }
	ColorProfilePtr ColorProfile()	{ return pICCProfile_; }

	void SetDib(DibPtr dib, ConstPhotoInfoPtr photo);
	void SetReductionFactor(int f);
	void SetOriginalSize(CSize size);
	void SetColorProfile(ColorProfilePtr p);

private:
	DibPtr dib_;					// decoded bitmap is here
	int reduction_factor_;			// reduction factor used by decoder to speed up loading and reduce image size
	CSize original_size_;			// physical dimensions of an image (in case EXIF info differs)
	ColorProfilePtr pICCProfile_;	// if photo had its own profile it's kept here
};


class PhotoInfo;

class PhotoCache : public Cache<CacheImg, ConstPhotoInfoPtr>
{
public:
	PhotoCache(size_t size, size_t memory_limit) : Cache<CacheImg, ConstPhotoInfoPtr>(size), memory_limit_(memory_limit)
	{}

	void CacheDecodedImage(DecoderJobInterface* decoder, ImageStat status);

	size_t GetFreeMemorySize() const;
	size_t GetMemoryLimit() const			{ return memory_limit_; }

private:
	size_t GetAllocatedMemorySize() const;
	size_t memory_limit_;
};

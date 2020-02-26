/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
#include <boost/function.hpp>
//class PhotoCache;
class CacheImg;
class DecoderJobInterface;
#include "ImageStat.h"
#include "VectPhotoInfo.h"

// ImageLoader service
//
// Load queued images in a worker thread and announce their availability via call back
//

class ImageLoader
{
public:
	ImageLoader(const boost::function<void (DecoderJobInterface* decoder, ImageStat)>& img_ready,
		const boost::function<DecoderJobInterface* (PhotoInfo& photo, CSize size, CWnd* receiver)>& create_job,
		const boost::function<bool (ConstPhotoInfoPtr photo)>& can_decode);
	~ImageLoader();

	void SetQueuePopulateCallback(const boost::function<void (void)>& populate_queue);
	void SetPhotoLoadedCallback(const boost::function<void (PhotoInfoPtr)>& photo_loaded);

	void RemovePhoto(PhotoInfoPtr photo, bool start_decoding_next);
	void RemovePhotos(const VectPhotoInfo& photos);
	void RemoveAll();

	bool AddPhoto(PhotoInfoPtr photo, CSize size, bool speculative_loading= false);
	void AddPhotos(const VectPhotoInfo& photos, CSize size, bool speculative_loading= false);

	void MarkPhotoAsRequested(PhotoInfoPtr photo);

	bool IsQueueEmpty() const;

	void StartDecoding();

	void RemoveSpeculativeLoadingItems();
	bool IsSpeculativeLoadingItem(size_t index) const;

//	PhotoCache* Cache() const;

	size_t ItemCount() const;
	PhotoInfoPtr GetItem(size_t index) const;

private:
	struct Impl;
	std::auto_ptr<Impl> pImpl_;
};


// helper functions:

// is cached image big enough?
extern bool IsCachedImageAvailable(CacheImg* img, CSize requested_size);

// can this photo be decoded (or has decoding failed already)?
extern bool CanPhotoBeDecoded(ConstPhotoInfoPtr photo);
extern bool CanThumbnailBeDecoded(ConstPhotoInfoPtr photo);

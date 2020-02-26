/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "ImageLoader.h"
#include "PhotoCache.h"
#include <deque>
#include "PhotoInfo.h"
#include "DecoderJobInterface.h"
#include <boost/shared_ptr.hpp>
#include <boost/ptr_container/ptr_map.hpp>
#include "CatchAll.h"


// queue of photos waiting for a decoder thread to process


template <class T> class Queue
{
public:
	void Clear()							{ queue_.clear(); }

	void Add(T item, const CSize& size, bool speculativeLoad= false)
	{ queue_.push_back(Item(item, size, speculativeLoad)); }

	size_t Size() const						{ return queue_.size(); }
	bool Empty() const						{ return queue_.empty(); }

	T FirstItem()							{ return queue_.front().item; }
	CSize FirstSize()						{ return queue_.front().size; }

	void EraseAt(size_t index)				{ queue_.erase(queue_.begin() + index); }

	bool IsSpeculative(size_t index) const	{ return queue_[index].speculative; }

	struct Item
	{
		T item;
		CSize size;
		bool speculative;
		bool pending;	// if true, decoding is in progress

		Item() : item(0), size(0, 0), speculative(false), pending(false)
		{}
		Item(T item, const CSize& size, bool speculative)
			: item(item), size(size), speculative(speculative), pending(false)
		{}

		bool Match(const T item) const		{ return item == this->item; }
		bool Ready() const					{ return !pending; }
	};

	Item& operator [] (size_t index)		{ return queue_[index]; }

	// find first item in a queue that's not being decoded
	Item* FindFirstItem()
	{
		std::deque<Item>::iterator it= find_if(queue_.begin(), queue_.end(), boost::bind(&Item::Ready, _1));
		return it == queue_.end() ? 0 : &*it;
	}

	bool Find(const T& item)
	{
		std::deque<Item>::iterator it= find_if(queue_.begin(), queue_.end(), boost::bind(&Item::Match, _1, item));
		return it != queue_.end();
	}

	void Erase(const T& item)
	{
		std::deque<Item>::iterator it= find_if(queue_.begin(), queue_.end(), boost::bind(&Item::Match, _1, item));
		if (it != queue_.end())
			queue_.erase(it);
	}

	// clear 'speculative' flag
	void RequestedItem(const T& item)
	{
		std::deque<Item>::iterator it= find_if(queue_.begin(), queue_.end(), boost::bind(&Item::Match, _1, item));
		if (it != queue_.end())
			it->speculative = false;
	}

private:
	std::deque<Item> queue_;
};


class ReceiverWnd : public CWnd
{
public:
	ReceiverWnd() : call(0)
	{}

	bool Create();

	LRESULT OnImgReloadingDone(WPARAM jobId, LPARAM status);

	boost::function<LRESULT (WPARAM, LPARAM)> call;

	DECLARE_MESSAGE_MAP()
};


BEGIN_MESSAGE_MAP(ReceiverWnd, CWnd)
	ON_MESSAGE(WM_USER + 1000, OnImgReloadingDone)
END_MESSAGE_MAP()


// Implementation of image loader service

typedef boost::ptr_map<ConstPhotoInfoPtr, DecoderJobInterface> Map;
typedef Queue<PhotoInfoPtr> QueueT;


struct ImageLoader::Impl
{
	Impl()
	{
		// window message from a decoder to be directed here:
		receiver_.call = boost::bind(&Impl::OnImgReloadingDone, this, _1, _2);

		extern int GetLogicalProcessorInfo();

		int cores= GetLogicalProcessorInfo();

		max_decoders_ = cores;
	}

	// queue of images waiting for decoding
	QueueT queue_;
	// receiver of messages sent by decoder
	ReceiverWnd receiver_;
	// working decoders
	Map decoders_;
	// maximum allowed number of decoders running concurrently
	int max_decoders_;

	Map::iterator FindDecoder(ConstPhotoInfoPtr photo);

	// fn with a signature matching message sent by decoder job on completion
	LRESULT OnImgReloadingDone(WPARAM jobId, LPARAM status);

	void StartDecodingNextPhoto();

	bool StartNext(QueueT::Item* item);

	//void PopulateQueue(bool scrolled_down);

	boost::function<void (void)> populate_queue_;
	boost::function<void (PhotoInfoPtr)> photo_loaded_;
	boost::function<DecoderJobInterface* (PhotoInfo& photo, CSize size, CWnd* receiver)> create_job_;
	// notification: image is ready
	boost::function<void (DecoderJobInterface* decoder, ImageStat)> image_ready_;
	// callback to decide whether item can be decoded
	boost::function<bool (ConstPhotoInfoPtr photo)> can_decode_;
};


bool CanThumbnailBeDecoded(ConstPhotoInfoPtr photo)
{
	if (photo == 0)
		return false;

	// if photo's thumbnail cannot be opened then do not retry opening it
	if (photo->thumbnail_stat_ != IS_NO_IMAGE && photo->thumbnail_stat_ != IS_OK)
		return false;

	return true;
}

bool CanPhotoBeDecoded(ConstPhotoInfoPtr photo)
{
	if (photo == 0)
		return false;

	// if photo cannot be opened then do not retry opening it
	if (photo->photo_stat_ != IS_NO_IMAGE && photo->photo_stat_ != IS_OK)
		return false;

	return true;
}


LRESULT ReceiverWnd::OnImgReloadingDone(WPARAM jobId, LPARAM status)
{
	return call(jobId, status);
}


Map::iterator ImageLoader::Impl::FindDecoder(ConstPhotoInfoPtr photo)
{
	return decoders_.find(photo);
}


void ImageLoader::Impl::StartDecodingNextPhoto()
{
	if (decoders_.size() >= max_decoders_)
		return;		// all busy

	QueueT::Item* item= queue_.FindFirstItem();

	if (item == 0)
	{
		// queue is empty or all items are pending; attempt speculative loading
		if (populate_queue_)
			populate_queue_();

		item = queue_.FindFirstItem();
	}

	while (item)
	{
		if (!StartNext(item))
		{
			PhotoInfoPtr photo= item->item;
			photo->photo_stat_ = IS_DECODING_FAILED;
			queue_.Erase(photo);	// remove item that cannot be decoded
		}

		if (decoders_.size() >= max_decoders_)
			break;		// all busy

		item = queue_.FindFirstItem();

		//TODO: limit speclative load to one thread?
	}
}


bool ImageLoader::Impl::StartNext(QueueT::Item* item)
{
	ASSERT(item);

	PhotoInfoPtr photo= item->item;
	CSize image_size= item->size;

	{
#if _DEBUG == 999
		TRACE(_T("-------------------------------\n"));
		for (size_t i= 0; i < queue_.Size(); ++i)
		{
			TRACE(_T("que: %c  %s\n"), queue_.IsSpeculative(i) ? 'x' : '-' , queue_[i]->name_.c_str());
		}
#endif
	}

	if (photo)
	{
		try
		{
			std::auto_ptr<DecoderJobInterface> decoder(create_job_(*photo, image_size, &receiver_));
			decoders_.insert(photo, decoder);
			item->pending = true;
			return true;
		}
		CATCH_ALL_W(AfxGetMainWnd())
		return false;
	}

	return true;
}


LRESULT ImageLoader::Impl::OnImgReloadingDone(WPARAM jobId, LPARAM status)
{
	DecoderJobInterface* decoder= 0;

	Map::iterator it= decoders_.begin();

	for (; it != decoders_.end(); ++it)
		if (it->second->GetUniqueId() == jobId)
		{
			decoder = it->second;
			break;
		}

	if (decoder == 0)
	{
//		ASSERT(false);
		return 0;
	}

	PhotoInfoPtr photo= ConstCast(decoder->GetPhoto());

	queue_.Erase(photo);

	image_ready_(decoder, static_cast<ImageStat>(status));

	decoders_.erase(it);

	// decoding thread finished; next photo
	StartDecodingNextPhoto();

	if (photo && photo_loaded_)
		photo_loaded_(photo);

	return 0;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


ImageLoader::ImageLoader(const boost::function<void (DecoderJobInterface* decoder, ImageStat)>& img_ready,
						 const boost::function<DecoderJobInterface* (PhotoInfo& photo, CSize size, CWnd* receiver)>& create_job,
						 const boost::function<bool (ConstPhotoInfoPtr photo)>& can_decode)
 : pImpl_(new Impl)
{
	pImpl_->image_ready_ = img_ready;
	pImpl_->create_job_ = create_job;
	pImpl_->can_decode_ = can_decode;

	// msg window: WinNT only?
	if (!pImpl_->receiver_.CreateEx(0, _T("STATIC"), 0, WS_POPUP, 0,0,0,0, HWND_MESSAGE, 0))
		pImpl_->receiver_.CreateEx(0, _T("STATIC"), 0, WS_POPUP, 0,0,0,0, HWND_DESKTOP, 0);
}


ImageLoader::~ImageLoader()
{}


void ImageLoader::RemovePhoto(PhotoInfoPtr photo, bool start_decoding_next)
{
	pImpl_->queue_.Erase(photo);

	Map::iterator it= pImpl_->FindDecoder(photo);
	if (it != pImpl_->decoders_.end())
	{
		(*it)->second->Quit();
		pImpl_->decoders_.erase(it);
		if (start_decoding_next)
			pImpl_->StartDecodingNextPhoto();
	}
}


void ImageLoader::RemovePhotos(const VectPhotoInfo& photos)
{
	bool restart= false;

	for (VectPhotoInfo::const_iterator it= photos.begin(); it != photos.end(); ++it)
	{
		pImpl_->queue_.Erase(*it);

		Map::iterator dec= pImpl_->FindDecoder(*it);
		if (dec != pImpl_->decoders_.end())
		{
			(*dec)->second->Quit();
			pImpl_->decoders_.erase(dec);
			restart = true;
		}
	}

	if (restart)
		pImpl_->StartDecodingNextPhoto();
}


void ImageLoader::RemoveAll()
{
	pImpl_->queue_.Clear();

	for (Map::iterator it= pImpl_->decoders_.begin(); it != pImpl_->decoders_.end(); ++it)
		(*it)->second->Quit();

	pImpl_->decoders_.clear();
}


void ImageLoader::MarkPhotoAsRequested(PhotoInfoPtr photo)
{
	pImpl_->queue_.RequestedItem(photo);
}


bool ImageLoader::AddPhoto(PhotoInfoPtr photo, CSize size, bool speculative_loading)
{
	if (!pImpl_->can_decode_(photo))
		return false;

	if (!pImpl_->queue_.Find(photo))
	{
//TRACE(L"Thumbnail in queue: %s", photo->GetName().c_str());
		pImpl_->queue_.Add(photo, size, speculative_loading);
		return true;
	}

	return false;	// photo already queued
}


void ImageLoader::AddPhotos(const VectPhotoInfo& photos, CSize size, bool speculative_loading)
{
	// add photos to decode (speculative loading)
	for (size_t i= 0; i < photos.size(); ++i)
		AddPhoto(photos[i], size, speculative_loading);
}


bool ImageLoader::IsQueueEmpty() const
{
	return pImpl_->queue_.Empty();
}


void ImageLoader::StartDecoding()
{
	pImpl_->StartDecodingNextPhoto();
}


void ImageLoader::RemoveSpeculativeLoadingItems()
{
	for (size_t i= 0; i < pImpl_->queue_.Size(); )
	{
		// get rid of existing speculative loading items
		if (pImpl_->queue_.IsSpeculative(i))
			pImpl_->queue_.EraseAt(i);
		else
			++i;
	}
}


void ImageLoader::SetQueuePopulateCallback(const boost::function<void (void)>& populate_queue)
{
	pImpl_->populate_queue_ = populate_queue;
}


void ImageLoader::SetPhotoLoadedCallback(const boost::function<void (PhotoInfoPtr)>& photo_loaded)
{
	pImpl_->photo_loaded_ = photo_loaded;
}


bool ImageLoader::IsSpeculativeLoadingItem(size_t index) const
{
	if (index < pImpl_->queue_.Size())
		return pImpl_->queue_.IsSpeculative(index);

	ASSERT(false);
	return false;
}


size_t ImageLoader::ItemCount() const
{
	return pImpl_->queue_.Size();
}


PhotoInfoPtr ImageLoader::GetItem(size_t index) const
{
	if (index < pImpl_->queue_.Size())
		return pImpl_->queue_[index].item;

	ASSERT(false);
	return 0;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////


bool IsCachedImageAvailable(CacheImg* image, CSize requested_size)
{
	if (image->Dib() && image->Dib()->IsValid())
	{
		CSize size= requested_size;
		CSize img= Dib::SizeToFit(size, image->OriginalSize());
		if (img.cx > image->OriginalSize().cx || img.cy > image->OriginalSize().cy)
			img = image->OriginalSize();

		if (image->Dib()->GetWidth() < image->Dib()->GetHeight() && img.cx > img.cy)
			std::swap(img.cx, img.cy);

		// check if decoded img is big enough

		if (image->Dib()->GetWidth() >= img.cx && image->Dib()->GetHeight() >= img.cy)
			return true;
	}

	return false;
}

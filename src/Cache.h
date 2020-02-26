/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// Cache.h: Cache LRU template

#pragma once


struct ptr_hash_fn
{
	template<class K> size_t operator()(K p) const
	{
		return p();
//		size_t n= reinterpret_cast<size_t>(p);
//		return n >> 4;
//		return (n & 0xffff) ^ (n >> 16);
	}

	template<class K> size_t operator()(const K* p) const
	{
//		return p();
		size_t n= reinterpret_cast<size_t>(p);
		return n >> 4;
//		return (n & 0xffff) ^ (n >> 16);
	}

	template<class K> size_t operator () (K p, K q) const
	{
		return p < q;
	}

	enum
	{
		bucket_size = 4,
		min_buckets = 8
	};
};


// This is LRU cache implemented using a list (tail is the oldest element, head the youngest)
// T is a type of an element stored in a cache
// K is a key type that supports ordering by 'operator <', hash value by 'operator ()', and comparison by 'operator =='

template<class T, class K> class Cache
{
public:
	Cache<T, K>(size_t size);
	~Cache<T, K>();

	// check if photo is in a cache and return it; if it's not present find next available slot;
	// bool param says whether photo was found
	std::pair<T*, bool> GetEntry(const K& key);

	// check if photo is in a cache already; if it is promote it to the front (if 'promote' is true)
	// marking it as recently used and return a dib pointer; if it's not present return 0
	T* FindEntry(const K& key, bool promote= true);

	// remove photos from the cache
	void RemoveAll();

	// remove photo from the cache, move corresponding cache entry (if it exists) to the tail position
	void Remove(const K& key);

	// retrieve an element (enumeration in no particular order)
	T* GetItem(size_t index) const;

	// max amount of elements in cache
	size_t Size() const					{ return cache_.size(); }

	// get last (oldest) element
	T* GetLast(size_t& iter) const;

	struct iterator
	{
		size_t iter;
	};

	// iterator to the oldest element
	iterator GetLastPos() const			{ iterator it; it.iter = tail_ - &cache_.front(); return it; }

	// get iterator to previous (younger) element
	iterator GetPrevPos(iterator iter) const
	{
		const Image* img= &cache_[iter.iter];
		if (img != head_)
			iter.iter = cache_[iter.iter].prev;
		return iter;
	}

	// iterator to the newest element
	iterator GetFirstPos() const		{ iterator it; it.iter = head_ - &cache_.front(); return it; }

	// get iterator to previous (younger) element
	iterator GetNextPos(iterator iter) const
	{
		const Image* img= &cache_[iter.iter];
		if (img != tail_)
			iter.iter = cache_[iter.iter].next;
		return iter;
	}

	T* GetItem(iterator iter) const			{ return cache_[iter.iter].image.get(); }
	const K& GetKey(iterator iter) const	{ return cache_[iter.iter].key; }

	const K& GetKey(size_t index) const		{ ASSERT(index < cache_.size()); return cache_[index].key; }

//	T* GetPrevious(size_t& iter) const;

	// remove entry from cache
	void Remove(iterator iter);

private:
	void ResetSize(size_t size);
	void ReinitializeCache();

	struct Image
	{
		K key;
		// NOTE: danger!, AutoPtr in a cache_ vector, no sorting or manipulating allowed
		AutoPtr<T> image;
		uint16 next;
		uint16 prev;
	};

	Image* NextAvailable();
	void MoveToFront(Image* img);
	void MoveToTail(Image* img);

	struct hash_fn
	{
		template<class Key> size_t operator()(const Key& p) const				{ return ::GetHashValue(p) >> 4; }

		template<class Key> size_t operator()(const Key* p) const
		{
			size_t n= ::GetHashValue(p); //reinterpret_cast<size_t>(p);
			return n >> 4;
		}

		template<class Key> size_t operator () (Key p, Key q) const		{ return p < q; }

		enum
		{
			bucket_size = 4,
			min_buckets = 8
		};
	};

	typedef std::unordered_map<K, Image*, hash_fn> hash_table;
	hash_table image_map_;
	// this is storage only (no vector manipulation is allowed due to the AutoPtr used inside Image struct)
	// (Note: as crazy as it seems it actually works, because vector is sized to the requested size, and does
	// not grow/shrink any more; changing cache size requires reinitialization--nuke all and resize)
	std::vector<Image> cache_; // boost::scoped_array<Image> can be considered instead, but it has no size()
	Image* head_;
	Image* tail_;
	static const uint16 NIL= 0xffff;

public:
#ifdef _DEBUG
	DWORD thread_id_;
	void Dump();
#endif
};


/////////////////////////////////////////////////////////////////////////////////////////////////////////////



template<class T, class K> Cache<T, K>::Cache(size_t size)
{
#ifdef _DEBUG
	thread_id_ = ::GetCurrentThreadId();
#endif
	head_ = 0;
	tail_ = 0;

	ResetSize(size);
}


template<class T, class K> Cache<T, K>::~Cache()
{
}


template<class T, class K> void Cache<T, K>::ResetSize(size_t size)
{
	if (size < 2)
	{
		ASSERT(false);
		return;
	}

	if (size >= 0xffff)
		size = 0xfffe;

	if (cache_.size() != size)
	{
		cache_.clear();		// nuke all (use auto ptrs)

		ASSERT(thread_id_ == ::GetCurrentThreadId());
		image_map_.clear();
#if _MSC_VER < 1310	// not vc 7.1?
		image_map_.resize(size);
#endif

		cache_.resize(size);	// re-allocate
	}

	ReinitializeCache();
}


template<class T, class K> T* Cache<T, K>::FindEntry(const K& key, bool promote/*= true*/)
{
	ASSERT(thread_id_ == ::GetCurrentThreadId());
	hash_table::const_iterator it= image_map_.find(key);
	if (it == image_map_.end())
		return 0;

	if (Image* img= it->second)
	{
		if (promote)
			MoveToFront(img);

		return img->image.get();
	}

	return 0;
}


template<class T, class K> std::pair<T*, bool> Cache<T, K>::GetEntry(const K& key)
{
	ASSERT(thread_id_ == ::GetCurrentThreadId());
	Image*& img= image_map_[key];

	bool found= img != 0;

	if (found)
	{
		MoveToFront(img);
	}
	else
	{
		img = NextAvailable();

		// entry was used?
		if (!(img->key == K()))
		{
			image_map_[img->key] = 0;	// clear map info
			// entry in a hash map is left (not erased); it may be used again later
		}
	}

	//Dump();

	if (img->image.empty())
		img->image = new T();

	img->key = key;

	return std::make_pair(img->image.get(), found);
}


template<class T, class K> typename Cache<T, K>::Image* Cache<T, K>::NextAvailable()
{
	// get last entry--tail
	Image* img= tail_;

	MoveToFront(img);

	return img;
}


template<class T, class K> void Cache<T, K>::MoveToFront(Image* img)
{
	// move from current place to the head of the cache list

	if (img == head_)
		return;

	// remove img from the list

	if (img->prev != NIL)
		cache_[img->prev].next = img->next;

	if (img->next != NIL)
		cache_[img->next].prev = img->prev;
	else
	{
		// since img is the last element tail ptr has to be reset
		ASSERT(img->prev != NIL);
		tail_ = &cache_[img->prev];
	}

	// now img is removed

	img->prev = NIL;

	// insert it at the head position

	img->next = static_cast<uint16>(head_ - &cache_.front());

	ASSERT(head_->prev == NIL);
	head_->prev = static_cast<uint16>(img - &cache_.front());

	// reset head

	head_ = img;
}


#ifdef _DEBUG

template<class T, class K> void Cache<T, K>::Dump()
{
	int n= cache_.size();

	TRACE(_T("\n--------------------\n"));

	TRACE(_T("head: %d    tail: %d\n"), head_ - &cache_.front(), tail_ - &cache_.front());

	for (int i= 0; i < n; ++i)
	{
		TRACE(_T("%d  p: %d,  n: %d\n"), i, int(cache_[i].prev), int(cache_[i].next));
	}
}

#endif


template<class T, class K> void Cache<T, K>::MoveToTail(Image* img)
{
	// move from current place to the tail of the cache list

	if (img == tail_)
		return;

	// remove img from the list

	if (img->prev != NIL)
		cache_[img->prev].next = img->next;

	if (img->next != NIL)
		cache_[img->next].prev = img->prev;

	// if img is at the head position reset head pointer
	if (img == head_)
	{
		ASSERT(img->next != NIL);
		head_ = &cache_[img->next];
	}

	// now img is removed

	img->next = NIL;

	// insert it at the tail position

	img->prev = static_cast<uint16>(tail_ - &cache_.front());

	ASSERT(tail_->next == NIL);
	tail_->next = static_cast<uint16>(img - &cache_.front());

	// reset tail

	tail_ = img;
}


template<class T, class K> void Cache<T, K>::Remove(const K& key)
{
	ASSERT(thread_id_ == ::GetCurrentThreadId());
	Image* img= image_map_[key];

	if (img == 0)
		return;

	// remove map entry
	image_map_.erase(key);

	img->key = K();
	img->image.free();

	MoveToTail(img);
}


template<class T, class K> void Cache<T, K>::Remove(iterator iter)
{
	Remove(cache_[iter.iter].key);
}


template<class T, class K> void Cache<T, K>::RemoveAll()
{
	ReinitializeCache();
}


template<class T, class K> void Cache<T, K>::ReinitializeCache()
{
	if (cache_.empty())
		return;

	const size_t size= cache_.size();
	ASSERT(size > 1);

	for (size_t i= 0; i < size; ++i)
	{
		Image& img= cache_[i];
		img.key = K();
		img.next = static_cast<uint16>(i + 1);
		img.prev = static_cast<uint16>(i - 1);
		img.image.free();
	}

	cache_[0].prev = NIL;
	cache_[size - 1].next = NIL;

	head_ = &cache_.front();
	tail_ = &cache_.back();

/*
{
int n= image_map_.bucket_count();
int b= 0;
for (int i= 0; i < n; ++i)
{
	int elems= image_map_.elems_in_bucket(i);
	if (elems > 0)
	{
		b++;
		if (elems > 1)
		{
			int a= 0;
		}
	}
}
TRACE(L"used buckets: %d\n", b);
}
*/

	ASSERT(thread_id_ == ::GetCurrentThreadId());
	image_map_.erase(image_map_.begin(), image_map_.end());
}


template<class T, class K> T* Cache<T, K>::GetItem(size_t index) const
{
	ASSERT(index < cache_.size());
	return cache_[index].image.get();
}


/*
template<class T> T* Cache<T, K>::GetLast(size_t& iter) const
{
	iter = tail_->prev;
	return tail_->image.get();
}


template<class T> size_t Cache<T, K>::GetLastPos() const
{
	return tail_ - &cache_.front();
}


template<class T> T* Cache<T, K>::GetPrevious(size_t& iter) const
{
	const Image* img= &cache_[iter];

	if (img != head_)
		iter = cache_[iter].prev;

	return img->image.get();
}
*/

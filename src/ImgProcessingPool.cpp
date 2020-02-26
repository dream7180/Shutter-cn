/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "ImgProcessingPool.h"
#include "ImgProcessingThread.h"
#include <boost/ptr_container/ptr_vector.hpp>
#include "PhotoQueue.h"
extern int GetLogicalProcessorCores();


struct ImgProcessingPool::Impl
{
	Impl(size_t file_count) : queue_(file_count), file_count_(file_count), max_threads_(0)
	{}

	boost::ptr_vector<ImgProcessingThread> pool_;
	size_t file_count_;
	PhotoQueue queue_;
	size_t max_threads_;
};


ImgProcessingPool::ImgProcessingPool(std::auto_ptr<ImgProcessingThread> worker_thread, size_t max_threads/*= 0*/)
 : impl_(*(new Impl(worker_thread->GetFileCount())))
{
	impl_.max_threads_ = max_threads;
	impl_.pool_.push_back(worker_thread.release());
//	impl_.file_count_ = impl_.pool_.front().GetFileCount();

	if (impl_.file_count_ > 1)
	{
		size_t threads= GetLogicalProcessorCores();

		if (impl_.max_threads_ > 0 && threads > impl_.max_threads_)
			threads = impl_.max_threads_;

		if (threads > 1)
		{
			// divide work between cores

			if (impl_.file_count_ < threads)
				threads = static_cast<int>(impl_.file_count_);

			// create more threads (all suspended)
			for (int thread= 1; thread < threads; ++thread)
				impl_.pool_.push_back(impl_.pool_[0].Clone());
		}
	}
}


ImgProcessingPool::~ImgProcessingPool()
{
	delete &impl_;
}


void ImgProcessingPool::Start()
{
	const size_t threads= impl_.pool_.size();

	for (size_t i= 0; i < threads; ++i)
	{
		// distribute work among threads
		impl_.pool_[i].Start(&impl_.queue_);//i, threads);
	}
}


void ImgProcessingPool::Quit()
{
	const size_t threads= impl_.pool_.size();

	for (size_t i= 0; i < threads; ++i)
		impl_.pool_[i].Break();

	for (size_t i= 0; i < threads; ++i)
		impl_.pool_[i].Quit();
}


size_t ImgProcessingPool::ThreadCount() const
{
	return impl_.pool_.size();
}


size_t ImgProcessingPool::GetFileCount() const
{
	return impl_.file_count_;
}


void ImgProcessingPool::SetStatusWnd(CWnd* wnd)
{
	const size_t threads= impl_.pool_.size();

	for (size_t i= 0; i < threads; ++i)
		impl_.pool_[i].SetStatusWnd(wnd);
}

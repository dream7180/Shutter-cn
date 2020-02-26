/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "PhotoQueue.h"


PhotoQueue::PhotoQueue(size_t count) : count_(count), index_(-1)
{}


LONG PhotoQueue::GetNext()
{
	LONG i= index_;
	if (i >= 0 && static_cast<size_t>(i) >= count_)
		return -1;

	i = ::InterlockedIncrement(&index_);
	ASSERT(i >= 0);
	if (static_cast<size_t>(i) >= count_)
		return -1;

	return i;
}


LONG PhotoQueue::CurrentIndex() const
{
	return index_;
}

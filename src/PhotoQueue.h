/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once


class PhotoQueue
{
public:
	PhotoQueue(size_t count);

	LONG GetNext();

	LONG CurrentIndex() const;

private:
	size_t count_;
	LONG index_;
};

/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "TaggedValues.h"

static const size_t NOT_FOUND= ~size_t(0);


size_t FindTagByValue(uint32 val, const TaggedValue tags[], std::size_t size)
{
	for (size_t i= 0; i < size; ++i)
		if (tags[i].value_ == val)
			return i;

	return NOT_FOUND;
}


const TCHAR* GetTagNameByValue(uint32 val, const TaggedValue tags[], std::size_t size, const TCHAR* not_found_value)
{
	size_t index= FindTagByValue(val, tags, size);
	return index == NOT_FOUND ? not_found_value : tags[index].name_;
}

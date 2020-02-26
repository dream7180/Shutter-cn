/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once

struct TaggedValue
{
	TaggedValue(uint32 value, const TCHAR* name) : value_(value), name_(name)
	{}

	uint32 value_;
	const TCHAR* name_;
};


size_t FindTagByValue(uint32 val, const TaggedValue tags[], std::size_t size);

const TCHAR* GetTagNameByValue(uint32 val, const TaggedValue tags[], std::size_t size, const TCHAR* not_found_value);

template <std::size_t SIZE>
const TCHAR* GetTag(uint32 val, const TaggedValue (&tags)[SIZE], const TCHAR* not_found_value= _T("?"))
{ return GetTagNameByValue(val, tags, SIZE, not_found_value); }

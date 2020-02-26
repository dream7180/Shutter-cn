/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// PhotoTags.cpp: implementation of the PhotoTags class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PhotoTags.h"
#include "File.h"
#include <boost/algorithm/string/trim.hpp>
using namespace boost::algorithm;

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////

struct AccStringLength
{
	size_t operator () (size_t size, const String& str) const
	{
		return size + str.length();
	}
};

struct AccStrings
{
	String& operator () (String& accum, const String& str) const
	{
		accum += str;
		accum += _T('\n');
		return accum;
	}
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////

void PhotoTags::LoadTags(FileStream& fs)
{
}


void PhotoTags::SaveTags(FileStream& fs)
{
}


void PhotoTags::ApplyTag(const String& name)
{
	std::pair<iterator, iterator> i= equal_range(begin(), end(), name);

	if (i.first == i.second)	// doesn't yet exist?
		insert(i.first, name);
}


void PhotoTags::RemoveTag(const String& name)
{
	std::pair<iterator, iterator> i= equal_range(begin(), end(), name);

	if (i.first != i.second)	// tag found?
		erase(i.first, i.second);
}


void PhotoTags::CopyTagsToArray(vector<String>& tags) const
{
	// for the time being it's merely a copy
	tags = *this;
}


void PhotoTags::AssignKeywords(const vector<String>& keys)
{
	vector<String>::operator = (keys);
	sort(begin(), end());
}


namespace {

struct LessThanNoCase
{
	bool operator () (const String& s1, const String& s2)
	{
		return _tcsicmp(s1.c_str(), s2.c_str()) < 0;
	}
};

}


//	Return a pointer to the start of the search string
//	If source does not contain search then returns NULL
//
extern const TCHAR* x_stristr(const TCHAR* source, const TCHAR* search)
{
	const size_t length= _tcslen(search);

	while (*source)
	{
		if (!_tcsnicmp(source, search, length))
			break;
		source++;
	}

	if (!*source)
		source = 0;

	return source;
}


bool PhotoTags::FindTagNoCase(const String& str, bool partial_match) const
{
	if (partial_match)
	{
		for (const_iterator it= begin(); it != end(); ++it)
			if (x_stristr(it->c_str(), str.c_str()) != 0)
				return true;

		return false;
	}
	else
		return binary_search(begin(), end(), str, LessThanNoCase()); //less<String>());
}


String PhotoTags::AsString() const
{
	size_t length= 0;
	length = accumulate(begin(), end(), length, AccStringLength());

	String tags;
	tags.reserve(length + size());	// space for all strings and their separators

	tags = accumulate(begin(), end(), tags, AccStrings());

	return tags;
}


String PhotoTags::CommaSeparated() const
{
	size_t length= 0;
	length = accumulate(begin(), end(), length, AccStringLength());

	String str;
	str.reserve(length + 2 * size());

	const size_t count= size();
	for (size_t i= 0; i < count; ++i)
	{
		str += (*this)[i];
		if (i < count - 1)
			str += _T(", ");
	}

	return str;
}

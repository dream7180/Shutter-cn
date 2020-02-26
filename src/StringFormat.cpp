/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "StringFormat.h"


string_format::string_format()
{
	radix_ = 10;
	fill_ = 0;
	str_.reserve(16);
}


string_format& string_format::operator << (int n)
{
	TCHAR buf[64];
	_itot_s(n, buf, radix_);
	str_ += buf;
	return *this;
}

string_format& string_format::operator << (unsigned int n)
{
	TCHAR buf[64];
	_ultot_s(n, buf, radix_);
	str_ += buf;
	return *this;
}

string_format& string_format::operator << (float f)
{
	TCHAR buf[64];
	wsprintf(buf, L"%f", double(f));
	str_ += buf;
	return *this;
}


string_format& string_format::operator << (const char* s)
{
	if (s)
	{
		auto size= strlen(s);

		auto len= str_.size();
		str_.resize(str_.size() + size);
		for (size_t i= 0; i < size; ++i)
			str_[len + i] = s[i];	// utf8 to unicode?
	}

	return *this;
}

string_format& string_format::operator << (const wchar_t* s)
{
	if (s)
		str_.append(s, s + wcslen(s));
	return *this;
}


const String& string_format::str() const
{
	return str_;
}

void string_format::hex()
{
	radix_ = 16;
}

void string_format::dec()
{
	radix_ = 10;
}

void string_format::reserve(size_t size)
{
	str_.reserve(str_.size() + size);
}

void string_format::clear()
{
	str_.clear();
}

void string_format::fill(wchar_t f)
{
	fill_ = f;
}

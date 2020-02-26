/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once

// simple stringstream replacement for speed critical sections of code

class string_format
{
public:
	string_format();

	string_format& operator << (int n);
	string_format& operator << (unsigned int n);
	string_format& operator << (float f);
	string_format& operator << (const char* s);
	string_format& operator << (const wchar_t* s);

	void reserve(size_t size);
	void clear();
	const String& str() const;

	void fill(wchar_t f);

	void hex();
	void dec();

private:
	int radix_;
	wchar_t fill_;
	String str_;
};

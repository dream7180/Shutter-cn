/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once

#include <sstream>
#include <iosfwd>
#include <vector>


// Formatting strings in-line

class StrFormat
{
public:
	StrFormat()
	{}

	StrFormat& operator << (long l)					{ ost_ << l; return *this; }
	StrFormat& operator << (int i)					{ ost_ << i; return *this; }
	StrFormat& operator << (short i)				{ ost_ << i; return *this; }
	StrFormat& operator << (unsigned int u)			{ ost_ << u; return *this; }
	StrFormat& operator << (unsigned long u)		{ ost_ << u; return *this; }
	StrFormat& operator << (unsigned short u)		{ ost_ << u; return *this; }
	StrFormat& operator << (float f)				{ ost_ << f; return *this; }
	StrFormat& operator << (double d)				{ ost_ << d; return *this; }
	StrFormat& operator << (const wchar_t* s)		{ ost_ << s; return *this; }
	StrFormat& operator << (wchar_t c)				{ wchar_t buf[2]= { c, 0 }; ost_ << buf; return *this; }
	StrFormat& operator << (const std::wstring& s)	{ ost_ << s; return *this; }

	StrFormat& operator << (const char* s)
	{
		std::vector<wchar_t> buf(strlen(s) + 1, 0);
		if (::MultiByteToWideChar(CP_ACP, 0, s, -1, &buf.front(), int(buf.size())) > 0)
			ost_ << &buf.front();
		return *this;
	}

	StrFormat& operator << (char c)				{ char buf[2]= { c, 0 }; return *this << buf; }
	StrFormat& operator << (const CString& s)	{ ost_ << static_cast<const TCHAR*>(s); return *this; }

	std::wstring string() const					{ return ost_.str(); }

	StrFormat& precision(int prec)				{ ost_.precision(prec); return *this; }
	StrFormat& fill(wchar_t c)					{ ost_.fill(c); return *this; }
	StrFormat& width(int w)						{ ost_.width(w); return *this; }

	//template <class Manipulator>
	//StrFormat& operator << (const Manipulator& m)		{ m(*this); return *this; }

private:
	std::wstringstream ost_;
};


struct Precision
{
	Precision(int prec) : prec_(prec)
	{}

	void operator () (StrFormat& fmt) const	{ fmt.precision(prec_); }

private:
	int prec_;
};


struct Width
{
	Width(int w) : w_(w)
	{}

	void operator () (StrFormat& fmt) const	{ fmt.width(w_); }

private:
	int w_;
};


struct Fill
{
	Fill(int f) : f_(f)
	{}

	void operator () (StrFormat& fmt) const	{ fmt.width(f_); }

private:
	int f_;
};

/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
#include "GlobalLock.h"


template<class T> class GlobalFixAlloc
{
public:
	GlobalFixAlloc(unsigned int count)
	{
		t_ = reinterpret_cast<T*>(::GlobalAlloc(GPTR, count * sizeof(T)));
	}

	~GlobalFixAlloc()
	{
		if (t_)
			::GlobalFree(reinterpret_cast<HGLOBAL>(t_));
	}

	T* operator -> () const throw ()	{ return t_; }

	operator T* () const throw ()		{ return t_; }

	BYTE* AsBytePtr()					{ return reinterpret_cast<BYTE*>(t_); }

	TCHAR* AsTCharPtr()					{ return reinterpret_cast<TCHAR*>(t_); }

private:
	T* t_;
};


template<class T> class GlobalMemAlloc
{
public:
	GlobalMemAlloc(size_t count, UINT flags= GMEM_MOVEABLE)
	{
		mem_ = ::GlobalAlloc(flags, count);
	}

	~GlobalMemAlloc()
	{
		if (mem_)
			::GlobalFree(mem_);
	}

	GlobalMemLock<T> get () const throw ()	{ return GlobalMemLock<T>(mem_); }

private:
	HGLOBAL mem_;
};

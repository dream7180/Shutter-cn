/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once

template<class T> class GlobalMemLock
{
public:
	GlobalMemLock(HGLOBAL mem) : mem_(mem)
	{
		t_ = reinterpret_cast<T*>(::GlobalLock(mem));
	}

	GlobalMemLock(const GlobalMemLock& g) : mem_(g.mem_)
	{
		t_ = reinterpret_cast<T*>(::GlobalLock(mem));
	}

	~GlobalMemLock()
	{
		if (mem_)
			::GlobalUnlock(mem_);
	}

	T* operator -> () const throw ()	{ return t_; }

	operator T* () const throw ()		{ return t_; }

	BYTE* AsBytePtr()					{ return reinterpret_cast<BYTE*>(t_); }

	TCHAR* AsTCharPtr()					{ return reinterpret_cast<TCHAR*>(t_); }

private:
	T* t_;
	HGLOBAL mem_;
};

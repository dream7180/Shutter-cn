/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#ifndef mkBOOST_INTRUSIVE_PTR_HPP_INCLUDED
#define mkBOOST_INTRUSIVE_PTR_HPP_INCLUDED

//
//  intrusive_ptr.hpp
//
//  Copyright (c) 2001, 2002 Peter Dimov
//
//  Permission to copy, use, modify, sell and distribute this software
//  is granted provided this copyright notice appears in all copies.
//  This software is provided "as is" without express or implied
//  warranty, and with no claim as to its suitability for any purpose.
//
//  See http://www.boost.org/libs/smart_ptr/intrusive_ptr.html for documentation.
//

// modifications MK

//#define PRINT_ADD_RELEASE_MESSAGES


#ifdef BOOST_MSVC  // moved here to work around VC++ compiler crash
# pragma warning(push)
# pragma warning(disable:4284) // odd return type for operator ->
#endif

//#include <boost/assert.hpp>
//#include <boost/detail/workaround.hpp>

#include <functional>           // for std::less
#include <iosfwd>               // for std::basic_ostream

namespace mik {
//  intrusive_ptr
//
//  A smart pointer that uses intrusive reference counting.
//

// base class to be included in T (provides counter)

struct counter_base
{
	counter_base() : reference_count_(0) {}
	virtual ~counter_base() {}

	long xrelease_ref() const		{ return ::InterlockedDecrement(&reference_count_); }
	long xadd_ref() const			{ return ::InterlockedIncrement(&reference_count_); }

private:
	mutable LONG reference_count_;
};


template<class T> inline bool IsPointerValid(T)		{ return true; }


#ifdef _DEBUG
#	define		VALIDATE(p)		ASSERT(IsPointerValid(p));
#else
#	define		VALIDATE(p)
#endif


template<class T> class intrusive_ptr
{
private:

	typedef intrusive_ptr this_type;

public:

	typedef T element_type;

	intrusive_ptr() : p_(0)
	{
	}

	intrusive_ptr(T* p, bool add_ref = true): p_(p)
	{
		if (p_ != 0 && add_ref) intrusive_ptr_add_ref(p_);
	}

#if !defined(BOOST_NO_MEMBER_TEMPLATES) || defined(BOOST_MSVC6_MEMBER_TEMPLATES)

	template<class U> intrusive_ptr(intrusive_ptr<U> const& rhs): p_(rhs.get())
	{
		if (p_ != 0) intrusive_ptr_add_ref(p_);
	}

#endif

	intrusive_ptr(intrusive_ptr const& rhs): p_(rhs.p_)
	{
		if (p_ != 0) intrusive_ptr_add_ref(p_);
	}

	~intrusive_ptr()
	{
		if (p_ != 0) intrusive_ptr_release(p_);
	}

#if !defined(BOOST_NO_MEMBER_TEMPLATES) || defined(BOOST_MSVC6_MEMBER_TEMPLATES)

	template<class U> intrusive_ptr& operator = (intrusive_ptr<U> const& rhs)
	{
		this_type(rhs).swap(*this);
		return *this;
	}

#endif

	intrusive_ptr & operator = (intrusive_ptr const& rhs)
	{
		this_type(rhs).swap(*this);
		return *this;
	}

	intrusive_ptr & operator = (T* rhs)
	{
		this_type(rhs).swap(*this);
		return *this;
	}

	T* get() const
	{
		VALIDATE(p_);
		return p_;
	}

	T & operator * () const
	{
		VALIDATE(p_);
		return *p_;
	}

	T* operator -> () const
	{
		VALIDATE(p_);
		return p_;
	}

	typedef T* (intrusive_ptr::*unspecified_bool_type) () const;

	operator unspecified_bool_type () const
	{
		VALIDATE(p_);
		return p_ == 0 ? 0 : &intrusive_ptr::get;
	}

	// operator ! is a Borland-specific workaround
	bool operator ! () const
	{
		return p_ == 0;
	}

	void swap(intrusive_ptr& rhs)
	{
		VALIDATE(p_);
		T* tmp = p_;
		p_ = rhs.p_;
		rhs.p_ = tmp;
	}

private:
	T* p_;

	static void intrusive_ptr_add_ref(T* p)
	{
		VALIDATE(p);
		long count= p->xadd_ref();
#ifdef PRINT_ADD_RELEASE_MESSAGES
		wchar_t buf[200];
		wsprintf(buf, L"add ref %p: %d\n", p, count);
		::OutputDebugString(buf);
#endif
	}

	static void intrusive_ptr_release(T* p)
	{
		long count= p->xrelease_ref();
#ifdef PRINT_ADD_RELEASE_MESSAGES
		wchar_t buf[200];
		wsprintf(buf, count != 0 ? _T("release ref %p: %d\n") : _T("release & delete %p: %d\n"), p, count);
		::OutputDebugString(buf);
#endif
		if (count == 0)
			delete p;
	}
};



template<class T, class U> inline bool operator == (intrusive_ptr<T> const& a, intrusive_ptr<U> const& b)
{
	return a.get() == b.get();
}

template<class T, class U> inline bool operator != (intrusive_ptr<T> const& a, intrusive_ptr<U> const& b)
{
	return a.get() != b.get();
}

template<class T> inline bool operator == (intrusive_ptr<T> const& a, T* b)
{
	return a.get() == b;
}

template<class T> inline bool operator != (intrusive_ptr<T> const& a, T* b)
{
	return a.get() != b;
}

template<class T> inline bool operator == (T* a, intrusive_ptr<T> const& b)
{
	return a == b.get();
}

template<class T> inline bool operator != (T* a, intrusive_ptr<T> const& b)
{
	return a != b.get();
}

#if __GNUC__ == 2 && __GNUC_MINOR__ <= 96

// Resolve the ambiguity between our op!= and the one in rel_ops

template<class T> inline bool operator != (intrusive_ptr<T> const& a, intrusive_ptr<T> const& b)
{
	return a.get() != b.get();
}

#endif

template<class T> inline bool operator < (intrusive_ptr<T> const& a, intrusive_ptr<T> const& b)
{
	return std::less<T*>()(a.get(), b.get());
}

template<class T> void swap(intrusive_ptr<T> & lhs, intrusive_ptr<T> & rhs)
{
	lhs.swap(rhs);
}

// mem_fn support

template<class T> T* get_pointer(intrusive_ptr<T> const& p)
{
	return p.get();
}

template<class T, class U> intrusive_ptr<T> static_pointer_cast(intrusive_ptr<U> const& p)
{
	return static_cast<T*>(p.get());
}

template<class T, class U> intrusive_ptr<T> dynamic_pointer_cast(intrusive_ptr<U> const& p)
{
	return dynamic_cast<T*>(p.get());
}

// operator <<
/*
#if defined(__GNUC__) &&  (__GNUC__ < 3)

template<class Y> std::ostream & operator << (std::ostream & os, intrusive_ptr<Y> const& p)
{
	os << p.get();
	return os;
}

#else

# if BOOST_WORKAROUND(BOOST_MSVC, <= 1200 && __SGI_STL_PORT)
// MSVC6 has problems finding std::basic_ostream through the using declaration in namespace _STL
using std::basic_ostream;
template<class E, class T, class Y> basic_ostream<E, T> & operator << (basic_ostream<E, T> & os, intrusive_ptr<Y> const& p)
# else
template<class E, class T, class Y> std::basic_ostream<E, T> & operator << (std::basic_ostream<E, T> & os, intrusive_ptr<Y> const& p)
# endif 
{
	os << p.get();
	return os;
}

#endif
*/
//} // namespace boost

#ifdef BOOST_MSVC
# pragma warning(pop)
#endif

} // namespace

#endif  // #ifndef mkBOOST_INTRUSIVE_PTR_HPP_INCLUDED

/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once

//#include <boost/range/???.hpp>


template<>
struct boost::range_const_iterator<CString>
{
	typedef const TCHAR* type;
};

template<>
struct boost::range_iterator<CString>
{
	typedef TCHAR* type;
};

template<>
struct boost::range_size<CString>
{
public:
	typedef int type;
};

template<>
struct boost::range_size<CString const>
{
public:
	typedef int type;
};


inline boost::range_iterator<CString>::type begin(CString& str)
{
	return str.GetBuffer(0);
}

inline boost::range_const_iterator<CString>::type begin(CString const& str)
{
	return str;
}

inline boost::range_size<CString>::type size(CString const& str)
{
	return str.GetLength();
}

inline boost::range_iterator<CString>::type end(CString& str)
{
	return begin(str) + size(str);
}

inline boost::range_const_iterator<CString>::type end(CString const& str)
{
	return begin(str) + size(str);
}

inline boost::range_iterator<CString>::type range_begin(CString& str)
{
        return begin(str);
}

inline boost::range_iterator<CString const>::type range_begin(CString const& str)
{
        return begin(str);
}

inline boost::range_iterator<CString>::type range_end(CString& str)
{
        return end(str);
}

inline boost::range_iterator<CString const>::type range_end(CString const& str)
{
        return end(str);
}

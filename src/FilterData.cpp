/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "FilterData.h"
#include <boost/algorithm/string/replace.hpp>
#include "DateTimeUtils.h"
using namespace boost::algorithm;


#if _UNICODE
	#define LINE_SEP L"\x2028"
#else
	#define LINE_SEP "\x01"
#endif

template<class T> void write(oStringstream& ost, const T& v)
{
	ost << v << std::endl;
}


void write(oStringstream& ost, const String& v)
{
	String temp= replace_all_copy(v, _T("\n"), LINE_SEP);
	ost << temp << std::endl;
}


//void write(oStringstream& ost, const CTime& t)
//{
//	ost << t.GetTime() << std::endl;
//}


void write(oStringstream& ost, const DateTime& t)
{
	ost << ToISOString(t) << std::endl;
}


template<class T> void write(oStringstream& ost, const std::vector<T>& v)
{
	const size_t count= v.size();
	ost << count << std::endl;
	for (size_t i= 0; i < count; ++i)
		ost << v[i] << std::endl;
}


template<class T> void read(iStringstream& ist, T& v)
{
	String line;
	if (getline(ist, line, _T('\n')))
	{
		iStringstream i(line);
		i >> v;
	}
	else
		throw int(2);
}


void read(iStringstream& ist, String& v)
{
	String temp;
	if (!getline(ist, temp, _T('\n')))
		throw int(1);
	v = replace_all_copy(temp, LINE_SEP, _T("\n"));
}


void read(iStringstream& ist, CTime& v)
{
	__time64_t t;
	read(ist, t);
	v = CTime(t);
}


void read(iStringstream& ist, DateTime& v)
{
	std::wstring t;
	read(ist, t);
	if (t == L"not-a-date-time")
		v = DateTime();
	else
		v = FromISOString(t);
}


template<class T> void read(iStringstream& ist, std::vector<T>& v)
{
	v.clear();
	size_t count= 0;
	read(ist, count);
	v.reserve(count);
	for (size_t i= 0; i < count; ++i)
	{
		T t;
		read(ist, t);
		v.push_back(t);
	}
}


String FilterDataToString(const FilterData& filter, int version)
{
	oStringstream ost;

	write(ost, version);
	write(ost, filter.selected_tags_.include);
	write(ost, filter.selected_tags_.exclude);
	write(ost, filter.selected_tags_.match_all);
	write(ost, filter.text_.include);
	write(ost, filter.text_.exclude);
	write(ost, filter.stars_.stars);
	write(ost, filter.expression_.rule);
	write(ost, filter.time_.from);
	write(ost, filter.time_.to);
	write(ost, filter.name_);

	return ost.str();
}


bool FilterDataFromString(const String& str, FilterData& filter)
{
	try
	{
		iStringstream ist(str);

		int version= 0;
		read(ist, version);

		read(ist, filter.selected_tags_.include);
		read(ist, filter.selected_tags_.exclude);
		read(ist, filter.selected_tags_.match_all);
		read(ist, filter.text_.include);
		read(ist, filter.text_.exclude);
		read(ist, filter.stars_.stars);
		read(ist, filter.expression_.rule);
		if (version > 1)
		{
			read(ist, filter.time_.from);
			read(ist, filter.time_.to);
		}
		else
		{
			CTime time;
			read(ist, time);
			if (auto t= time.GetTime())
				filter.time_.from = DateTime(boost::posix_time::from_time_t(t));
			read(ist, time);
			if (auto t= time.GetTime())
				filter.time_.to = DateTime(boost::posix_time::from_time_t(t));
		}

		read(ist, filter.name_);

		return true;
	}
	catch (int)
	{
		return false;
	}
}


void FilterData::ClearAll()
{
	selected_tags_.ClearAll();
	text_.ClearAll();
	stars_.ClearAll();
	expression_.ClearAll();
	time_.ClearAll();
}


bool FilterData::IsActive() const
{
	return selected_tags_.IsActive() || text_.IsActive() || stars_.IsActive() || expression_.IsActive() || time_.IsActive();
}

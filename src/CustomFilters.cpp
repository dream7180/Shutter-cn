/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "CustomFilters.h"


CustomFilters::CustomFilters()
{
}

//CustomFilters::~CustomFilters()
//{
//}


size_t CustomFilters::AddFilter(const FilterData& filter)
{
	filters_.push_back(filter);
	return filters_.size() - 1;
}


const FilterData* CustomFilters::FindFilter(const String& name) const
{
	return const_cast<CustomFilters*>(this)->FindFilter(name);
}


FilterData* CustomFilters::FindFilter(const String& name)
{
	const size_t count= filters_.size();
	for (size_t i= 0; i < count; ++i)
		if (filters_[i].name_ == name)
			return &filters_[i];

	return 0;
}


const FilterData* CustomFilters::GetFilter(size_t index) const
{
	return const_cast<CustomFilters*>(this)->GetFilter(index);
}


FilterData* CustomFilters::GetFilter(size_t index)
{
	if (index < filters_.size())
		return &filters_[index];
	else
		return 0;
}


void CustomFilters::DeleteFilter(size_t index)
{
	if (index < filters_.size())
		filters_.erase(filters_.begin() + index);
	else
	{ ASSERT(false); }
}


void CustomFilters::SerializeTo(std::vector<String>& filters, int version)
{
	filters.clear();

	const size_t count= filters_.size();

	filters.reserve(count);

	for (size_t i= 0; i < count; ++i)
	{
		String str= FilterDataToString(filters_[i], version);
		filters.push_back(str);
	}
}


void CustomFilters::SerializeFrom(const std::vector<String>& filters, int version)
{
	filters_.clear();
	filters_.reserve(filters.size());

	const size_t count= filters.size();
	for (size_t i= 0; i < count; ++i)
	{
		FilterData filter;
		if (FilterDataFromString(filters[i], filter))
			filters_.push_back(filter);
	}
}

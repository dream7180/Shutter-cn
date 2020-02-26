/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
#include "FilterData.h"


class CustomFilters
{
public:
	CustomFilters();

	size_t GetCount() const;

	size_t AddFilter(const FilterData& filter);
	void DeleteFilter(size_t index);

	const FilterData* FindFilter(const String& name) const;
	FilterData* FindFilter(const String& name);

	const FilterData* GetFilter(size_t index) const;
	FilterData* GetFilter(size_t index);

	//
	void SerializeTo(std::vector<String>& filters, int version);
	void SerializeFrom(const std::vector<String>& filters, int version);

private:
	std::vector<FilterData> filters_;
};

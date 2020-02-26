/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once


struct FilterTags
{
	FilterTags() : match_all(true)
	{}

	std::vector<String> include;
	std::vector<String> exclude;
	bool match_all;

	bool IsActive() const	{ return !include.empty() || !exclude.empty(); }
	void ClearAll()			{ include.clear(); exclude.clear(); match_all = true; }
};


struct FilterText
{
	String include;
	String exclude;

	bool IsActive() const	{ return !include.empty() || !exclude.empty(); }
	void ClearAll()			{ include.clear(); exclude.clear(); }
};


struct FilterExpression
{
	String rule;

	bool IsActive() const	{ return !rule.empty(); }
	void ClearAll()			{ rule.clear(); }
};


struct FilterStars
{
	FilterStars() : stars(0)
	{}

	int stars;

	bool IsActive() const	{ return stars > 0; }
	void ClearAll()			{ stars = 0; }
};


struct FilterTimeSpan
{
	DateTime from;
	DateTime to;

	bool IsActive() const	{ return !from.is_not_a_date_time() && !to.is_not_a_date_time(); }
	void ClearAll()			{ from = to = DateTime(); }
};


struct FilterData
{
	FilterTags selected_tags_;
	FilterText text_;
	FilterStars stars_;
	FilterExpression expression_;
	FilterTimeSpan time_;
	String name_;

	bool IsActive() const;
	void ClearAll();
};


// serializing support

extern String FilterDataToString(const FilterData& filter, int version);

extern bool FilterDataFromString(const String& str, FilterData& filter);

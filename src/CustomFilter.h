/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
#include "Lua.h"
class PhotoInfo;


class CustomFilter
{
public:
//	CustomFilter();
	CustomFilter(const String& rule);

	// calculate filter rule for given 'photo'; return true for 'filter in', and false otherwise
	bool CalcResult(const PhotoInfo& photo) const;


private:
	void Init() const;
	String rule_;
	mutable std::auto_ptr<Lua> lua_;
};

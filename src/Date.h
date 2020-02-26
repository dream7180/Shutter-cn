/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once


class Date
{
public:
	Date();

	unsigned int Day() const				{ return day_; }
	unsigned int Month() const				{ return month_; }
	unsigned int Year() const				{ return year_; }

	unsigned int AsInt() const				{ return (year_ * 12 + month_) * 31 + day_; }

	int Difference(const Date& date) const	{ return AsInt() - date.AsInt(); }

private:
	uint16	year_;
	uint8	month_;
	uint8	day_;
};

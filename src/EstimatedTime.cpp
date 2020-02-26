/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "EstimatedTime.h"

EstimatedTime::EstimatedTime()
{
	Start();
}

void EstimatedTime::Start()
{
	::GetSystemTimeAsFileTime(&start_time_);
}


uint64 EstimatedTime::SecondsLeft(size_t processed, size_t total) const
{
	return MillisecondsLeft(processed, total) / 1000;
}


uint64 EstimatedTime::MillisecondsLeft(size_t processed, size_t total) const
{
	if (processed > 0)
	{
		FILETIME ft;
		::GetSystemTimeAsFileTime(&ft);
		uint64 lapse= (ft.dwLowDateTime + (uint64(ft.dwHighDateTime) << 32)) -
			(start_time_.dwLowDateTime + (uint64(start_time_.dwHighDateTime) << 32));
		const uint64 TO_MS= 10000;
		uint64 est= (lapse * total / processed - lapse) / TO_MS;
		return est;
	}
	else
		return ~uint64(0);
}


CString EstimatedTime::TimeLeft(size_t processed, size_t total) const
{
	uint64 est= MillisecondsLeft(processed, total);
	CString time= CTimeSpan((est + 999) / 1000).Format(_T("%H:%M:%S"));
	return time;
}

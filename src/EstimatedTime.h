/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once

class EstimatedTime
{
public:
	EstimatedTime();

	void Start();

	uint64 SecondsLeft(size_t processed, size_t total) const;
	uint64 MillisecondsLeft(size_t processed, size_t total) const;
	CString TimeLeft(size_t processed, size_t total) const;

private:
	FILETIME start_time_;
};

/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once

class ImgLogger
{
public:
	ImgLogger();
	~ImgLogger();

	void AddEntry(const TCHAR* msg, const TCHAR* path);

	size_t GetCount() const;

	typedef std::pair<String, String> LogEntry;

	LogEntry GetItem(size_t index) const;

private:
	std::vector<LogEntry> logs_;
	mutable CCriticalSection sentry_;
};

/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
#include "boost/noncopyable.hpp"
#include "boost/function.hpp"


class DirectoryChangeWatcher : boost::noncopyable
{
public:
	DirectoryChangeWatcher();

	// start watch this folder
	void WatchFolder(const TCHAR* path, bool including_subdirs, const TCHAR* include_mask);

	// stop directory waching and sit idle
	void Stop();

	enum Change { NewFile, FileRemoved, FileRenamed, FileModified, NewDirectory, DirectoryRemoved, DirectoryRenamed };

	//
	void SetCallBacks(
		const boost::function<void (const TCHAR* path, Change change, const TCHAR* new_name)>& file_changed,
		const boost::function<void (const TCHAR* path, Change change, const TCHAR* new_name)>& dir_changed);

	~DirectoryChangeWatcher();

private:
	struct Impl;
	std::auto_ptr<Impl> impl_;
};

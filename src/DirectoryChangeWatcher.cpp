/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "DirectoryChangeWatcher.h"
#include "DirectoryChanges.h"


struct DirectoryChangeWatcher::Impl
#ifdef DIR_WATCH
	: CDirectoryChangeHandler
#endif
{
	Impl()
	{
//		watcher_.WatchDirectory(L"c:\\", FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME, this, true, L"*.jpg;*.jpeg", 0);


	}

#ifdef DIR_WATCH
	virtual void On_FileAdded(const CString& file_name)
	{
		if (file_changed_)
			file_changed_(file_name, DirectoryChangeWatcher::NewFile, 0);
	}

	virtual void On_FileRemoved(const CString& file_name)
	{
		if (file_changed_)
			file_changed_(file_name, DirectoryChangeWatcher::FileRemoved, 0);
	}

	virtual void On_FileNameChanged(const CString& old_file_name, const CString& new_file_name)
	{
		if (file_changed_)
			file_changed_(old_file_name, DirectoryChangeWatcher::FileRenamed, new_file_name);
	}

	virtual void On_FileModified(const CString& file_name)
	{
		if (file_changed_)
			file_changed_(file_name, DirectoryChangeWatcher::FileModified, 0);
	}

	CDirectoryChangeWatcher watcher_;
#endif
	boost::function<void (const TCHAR* path, DirectoryChangeWatcher::Change change, const TCHAR* new_name)> file_changed_;
	boost::function<void (const TCHAR* path, DirectoryChangeWatcher::Change change, const TCHAR* new_name)> dir_changed_;
};


DirectoryChangeWatcher::DirectoryChangeWatcher() : impl_(new Impl())
{
}


DirectoryChangeWatcher::~DirectoryChangeWatcher()
{}


void DirectoryChangeWatcher::SetCallBacks(
	const boost::function<void (const TCHAR* path, Change change, const TCHAR* new_path)>& file_changed,
	const boost::function<void (const TCHAR* path, Change change, const TCHAR* new_name)>& dir_changed)
{
	impl_->file_changed_ = file_changed;
	impl_->dir_changed_ = dir_changed;
}


// start watch this folder
void DirectoryChangeWatcher::WatchFolder(const TCHAR* path, bool including_subdirs, const TCHAR* include_mask)
{
#ifdef DIR_WATCH

	DWORD flags= FILE_NOTIFY_CHANGE_ATTRIBUTES |
				FILE_NOTIFY_CHANGE_SIZE |
				FILE_NOTIFY_CHANGE_LAST_WRITE |
				FILE_NOTIFY_CHANGE_LAST_ACCESS |
				FILE_NOTIFY_CHANGE_CREATION |
				FILE_NOTIFY_CHANGE_SECURITY;

	DWORD stat= impl_->watcher_.WatchDirectory(path, FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | flags, impl_.get(),
		including_subdirs, include_mask, 0);
#endif
}


void DirectoryChangeWatcher::Stop()
{
#ifdef DIR_WATCH

	impl_->watcher_.UnwatchAllDirectories();

#endif
}

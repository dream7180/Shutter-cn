/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
class ShellItem;
class ShellFolderEx;
#include "intrusive_ptr.h"
#include <boost/function.hpp>
typedef mik::intrusive_ptr<ShellFolderEx> ShellFolderPtr;
typedef mik::intrusive_ptr<ShellItem> ShellItemPtr;
typedef std::vector<ShellItemPtr> FolderList;
#include "FileFilterCallback.h"
#include "FileInfo.h"


// FileListCtrl

class FileListCtrl : public CListCtrl
{
public:
	FileListCtrl();
	virtual ~FileListCtrl();

	bool Create(CWnd* parent, UINT id, DWORD styles= WS_BORDER | WS_VISIBLE);

	void SetPath(const ITEMIDLIST* idl, bool includeFiles,
		const FileFilterCallback& includeFileItem);
		//const String& inclFileSpec, const String& exclFileSpec);

	bool GoLevelUp();

	ShellFolderPtr GetCurrentPath();

	void SetFolderChangedCallback(const boost::function<void (ShellFolderPtr)>& callback);

	void SetIconView();
	void SetListView();

	void GetFiles(std::vector<FileInfo>& files);//, int subDirsDepth= 0);

protected:
	DECLARE_MESSAGE_MAP()

private:
	ShellFolderPtr shell_folder_;
	FolderList folder_list_;
	UINT_PTR timer_;
	String search_string_;
	bool includeFiles_;
	FileFilterCallback includeFileItem_;
	boost::function<void (ShellFolderPtr)> folder_changed_callback_;

	BOOL OnGetDispInfo(NMHDR* nmhdr, LRESULT* result);
	void OnDblClick(NMHDR* nmhdr, LRESULT* result);
	UINT OnGetDlgCode();
	void OnKeyDown(UINT chr, UINT rep_cnt, UINT flags);
	void OpenItem(size_t item);
	int FindItem(const String& str, int item);
	void OnChar(UINT chr, UINT rep_cnt, UINT flags);
	void OnTimer(UINT_PTR timer);
	void OnDestroy();
	void ResetSearch();
	virtual void PreSubclassWindow();
};

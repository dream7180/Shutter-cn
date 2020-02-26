/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
class ShellFolder;
#include "intrusive_ptr.h"
#include <boost/function.hpp>
typedef mik::intrusive_ptr<ShellFolder> CShellFolderPtr;
typedef std::list<CShellFolderPtr> FolderList;


// FolderListCtrl

class FolderListCtrl : public CListCtrl
{
public:
	FolderListCtrl();
	virtual ~FolderListCtrl();

	bool Create(CWnd* parent, UINT id, DWORD styles= WS_BORDER | WS_VISIBLE);

	void SetPath(const ITEMIDLIST* idl);

	bool GoLevelUp();

	CShellFolderPtr GetCurrentPath();

	void SetFolderChangedCallback(const boost::function<void (CShellFolderPtr)>& callback);

	void SetIconView();
	void SetListView();

protected:
	DECLARE_MESSAGE_MAP()

private:
	CShellFolderPtr shell_folder_;
	FolderList folder_list_;
	UINT_PTR timer_;
	String search_string_;
	boost::function<void (CShellFolderPtr)> folder_changed_callback_;

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

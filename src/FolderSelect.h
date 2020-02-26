/*____________________________________________________________________________

   EXIF Image Viewer

   Copyright (C) 2000 Michael Kowalski
____________________________________________________________________________*/

// FolderSelect.h: interface for the CFolderSelect class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FOLDERSELECT_H__D3858CA3_D671_11D3_B62D_000000000000__INCLUDED_)
#define AFX_FOLDERSELECT_H__D3858CA3_D671_11D3_B62D_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
class ItemIdList;


class CFolderSelect
{
public:
	CFolderSelect(CWnd* parent) : parent_(parent), pidl_init_dir_(0), mask_(0) {}
	virtual ~CFolderSelect() {}

//	String DoSelect(const TCHAR* title, const TCHAR* folder);

	ITEMIDLIST* DoSelect(const TCHAR* title, const TCHAR* folder);
	ITEMIDLIST* DoSelect(const TCHAR* title, ITEMIDLIST* pidlInitial);
	bool DoSelect(const TCHAR* title, ItemIdList& idlFolder);

	CString DoSelectPath(const TCHAR* title, const TCHAR* folder);

	CString DoSelectFile(const TCHAR* title, int CSIDL, const TCHAR* mask);

private:
	static int CALLBACK BrowseCallbackProc(HWND wnd, UINT msg, LPARAM lParam, LPARAM data);
	String init_dir_;
	const ITEMIDLIST* pidl_init_dir_;
	CWnd* parent_;
	const TCHAR* mask_;

	ITEMIDLIST* DoSelectFileHelper(const TCHAR* title, const TCHAR* folder, const TCHAR* mask);
};

#endif // !defined(AFX_FOLDERSELECT_H__D3858CA3_D671_11D3_B62D_000000000000__INCLUDED_)

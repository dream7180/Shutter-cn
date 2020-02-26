/*____________________________________________________________________________

   EXIF Image Viewer

   Copyright (C) 2000 Michael Kowalski
____________________________________________________________________________*/

// FolderSelect.cpp: implementation of the CFolderSelect class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FolderSelect.h"
#include "ItemIdList.h"
#include "Path.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////

int CALLBACK CFolderSelect::BrowseCallbackProc(HWND wnd, UINT msg, LPARAM lParam, LPARAM data)
{
	CFolderSelect* SD= reinterpret_cast<CFolderSelect*>(data);

	switch (msg)
	{
	case BFFM_INITIALIZED:
		{
			if (SD->pidl_init_dir_)
				::SendMessage(wnd, BFFM_SETSELECTION, false, reinterpret_cast<LPARAM>(SD->pidl_init_dir_));
			else
				::SendMessage(wnd, BFFM_SETSELECTION, true, reinterpret_cast<LPARAM>(SD->init_dir_.c_str()));
		}
		break;

	case BFFM_VALIDATEFAILED:
		{
			//      PCSTR dir= reinterpret_cast<PCSTR>(data);
			//      AfxMessageBox(dir, MB_OK);
			return 1;
		}
		break;

	case BFFM_SELCHANGED:
		if (SD->mask_ != 0 && lParam != 0)
		{
			ITEMIDLIST* items= reinterpret_cast<ITEMIDLIST*>(lParam);
			TCHAR buf[MAX_PATH];
			::SHGetPathFromIDList(items, buf);
			try
			{
				Path path= buf;
				::SendMessage(wnd, BFFM_ENABLEOK, 0, path.MatchExtension(SD->mask_));
			}
			catch (...)
			{
			}
		}

/*	case BFFM_IUNKNOWN:
		if (SD->mask_ != 0 && lParam != 0)
		{
			IUnknown* unknown= reinterpret_cast<IUnknown*>(lParam);

			// XP only:
//			IFolderFilterSite* filter= unknown->QueryInterface(
		}
		break; */
	}
	return 0;
}

/*
String CFolderSelect::DoSelect(const TCHAR* title, const TCHAR* folder)
{
	TCHAR buf[MAX_PATH + 4];
	BROWSEINFO bi;
	bi.hwndOwner      = *parent_;
	bi.pidlRoot       = NULL;
	bi.pszDisplayName = buf;
	bi.title      = title;
	bi.ulFlags        = BIF_RETURNONLYFSDIRS | BIF_VALIDATE | BIF_EDITBOX | 0x40; // | BIF_VALIDATE | BIF_STATUSTEXT;
	bi.lpfn           = BrowseCallbackProc;
	bi.lParam         = reinterpret_cast<LPARAM>(this);
	bi.iImage         = 0;

	init_dir_ = folder;

	ITEMIDLIST* item_list= ::SHBrowseForFolder(&bi);

	if (item_list == NULL)
		return String();

	TCHAR path[MAX_PATH * 2];
	::SHGetPathFromIDList(item_list, path);

	LPMALLOC malloc;
	if (::SHGetMalloc(&malloc) == NOERROR)
		malloc->Free(item_list);

	return String(path);
}
*/


ITEMIDLIST* CFolderSelect::DoSelect(const TCHAR* title, const TCHAR* folder)
{
	TCHAR buf[MAX_PATH + 4];
	BROWSEINFO bi;
	bi.hwndOwner      = *parent_;
	bi.pidlRoot       = NULL;
	bi.pszDisplayName = buf;
	bi.lpszTitle      = title;
	bi.ulFlags        = BIF_RETURNONLYFSDIRS | BIF_VALIDATE | BIF_EDITBOX | BIF_NEWDIALOGSTYLE; // | BIF_STATUSTEXT;
	bi.lpfn           = BrowseCallbackProc;
	bi.lParam         = reinterpret_cast<LPARAM>(this);
	bi.iImage         = 0;

	init_dir_ = folder;

	return ::SHBrowseForFolder(&bi);
}


ITEMIDLIST* CFolderSelect::DoSelect(const TCHAR* title, ITEMIDLIST* pidlInitial)
{
	TCHAR buf[MAX_PATH + 4];
	BROWSEINFO bi;
	bi.hwndOwner      = *parent_;
	bi.pidlRoot       = NULL;
	bi.pszDisplayName = buf;
	bi.lpszTitle      = title;
	bi.ulFlags        = BIF_RETURNONLYFSDIRS | BIF_VALIDATE | BIF_EDITBOX | BIF_NEWDIALOGSTYLE; // | BIF_STATUSTEXT;
	bi.lpfn           = BrowseCallbackProc;
	bi.lParam         = reinterpret_cast<LPARAM>(this);
	bi.iImage         = 0;

	pidl_init_dir_ = pidlInitial;

	return ::SHBrowseForFolder(&bi);
}


bool CFolderSelect::DoSelect(const TCHAR* title, ItemIdList& idlFolder)
{
	TCHAR buf[MAX_PATH + 4];
	BROWSEINFO bi;
	bi.hwndOwner      = *parent_;
	bi.pidlRoot       = NULL;
	bi.pszDisplayName = buf;
	bi.lpszTitle      = title;
	bi.ulFlags        = BIF_RETURNONLYFSDIRS | BIF_VALIDATE | BIF_EDITBOX | BIF_NEWDIALOGSTYLE; // | BIF_STATUSTEXT;
	bi.lpfn           = BrowseCallbackProc;
	bi.lParam         = reinterpret_cast<LPARAM>(this);
	bi.iImage         = 0;

	pidl_init_dir_ = idlFolder;

	ITEMIDLIST* item_list= ::SHBrowseForFolder(&bi);

	if (item_list == NULL)
		return false;

	idlFolder.AssignNoCopy(item_list);

	return true;
}


CString CFolderSelect::DoSelectPath(const TCHAR* title, const TCHAR* folder)
{
	ITEMIDLIST* item_list= DoSelect(title, folder);

	if (item_list == NULL)
		return CString();

	ItemIdList idlFolder(item_list, false);

	return idlFolder.GetPath();
}


//---------------------------------------------------------------------------------------


ITEMIDLIST* CFolderSelect::DoSelectFileHelper(const TCHAR* title, const TCHAR* folder, const TCHAR* mask)
{
	TCHAR buf[MAX_PATH + 4];
	BROWSEINFO bi;
	bi.hwndOwner      = *parent_;
	bi.pidlRoot       = NULL;
	bi.pszDisplayName = buf;
	bi.lpszTitle      = title;
	bi.ulFlags        = BIF_RETURNONLYFSDIRS | BIF_VALIDATE | BIF_EDITBOX | BIF_NEWDIALOGSTYLE | BIF_BROWSEINCLUDEFILES | BIF_NONEWFOLDERBUTTON; // | BIF_STATUSTEXT;
	bi.lpfn           = BrowseCallbackProc;
	bi.lParam         = reinterpret_cast<LPARAM>(this);
	bi.iImage         = 0;

	mask_ = mask;
	init_dir_ = folder;

	return ::SHBrowseForFolder(&bi);
}


CString CFolderSelect::DoSelectFile(const TCHAR* title, int CSIDL, const TCHAR* mask)
{
	CString initial_dir("");

	ITEMIDLIST* pidl= 0;
	if (::SHGetSpecialFolderLocation(*parent_, CSIDL, &pidl) == NOERROR && pidl != 0)
	{
		ItemIdList folder(pidl, false);
		initial_dir = folder.GetPath();
	}

	ITEMIDLIST* item_list= DoSelectFileHelper(title, initial_dir, mask);

	if (item_list == NULL)
		return CString();

	ItemIdList idlFolder(item_list, false);

	return idlFolder.GetPath();
}

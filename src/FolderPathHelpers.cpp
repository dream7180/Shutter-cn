/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "FolderPathHelpers.h"
#include "DirectoryPath.h"
#include "ItemIdList.h"
#include "CatalogPath.h"
#include "Path.h"


FolderPathPtr CreateFolderPath(const TCHAR* path)
{
	DWORD attrib= ::GetFileAttributes(path);

	if (attrib == INVALID_FILE_ATTRIBUTES)
		return 0;

	if ((attrib & FILE_ATTRIBUTE_DIRECTORY) == 0)	// a file?
	{
		// determine if it's a link

		IShellLinkWPtr SL;
		if (::CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (void**)&SL) == S_OK)
		{
			IPersistFilePtr PF;
			if (SL->QueryInterface(IID_IPersistFile, (void**)&PF) == S_OK)
			{
#ifdef _UNICODE
				LPCOLESTR a_path= path;
#else
				_bstr_t path_str= path;
				LPCOLESTR a_path= path_str;
#endif
				if (PF->Load(a_path, SLR_ANY_MATCH | SLR_NOSEARCH | SLR_NO_UI | SLR_UPDATE) == S_OK)
				{
					ITEMIDLIST* idl= 0;
					if (SL->GetIDList(&idl) == S_OK)
					{
						return ::CreateFolderPath(idl);
						//ItemIdList idlPath(idl);
//						Browser(a_path, false);
					}
				}
				else	// not a link
				{
					// check if this is catalog file
					{
						Path p(path);
						if (p.MatchExtension(_T("catalog")) && p.FileExists())
							return new CatalogPath(p);
					}

					const TCHAR* sep= _tcsrchr(path, _T('\\'));
					if (sep == 0)
						sep = _tcsrchr(path, _T('/'));
					if (sep != 0)
					{
						CString dir(path, static_cast<int>(sep - path + 1));	// include '\' at the end or else c: won't be parsed properly
						//FolderSelected(dir);
//						return ::CreateFolderPath(dir);

						// create directory path object passing name of selected file
						ItemIdList pidl(dir);
						return new DirectoryPath(pidl, sep + 1);
					}
				}
			}
		}
	}
	else if (attrib & FILE_ATTRIBUTE_DIRECTORY)
	{
		//ItemIdList idlPath(path);
		//FolderPathPtr path= ::CreateFolderPath(path);
//		Browser(path, true);
		ItemIdList pidl(path);
		return new DirectoryPath(pidl);
	}

	//Path p(path);
	//if (p.MatchExtension(_T("catalog")) && p.FileExists())
	//	return new CatalogPath(p,
	//ItemIdList pidl(path);
	//return new DirectoryPath(pidl);

	return 0;
}


FolderPathPtr CreateFolderPath(const String& path)
{
	return ::CreateFolderPath(path.c_str());
}


FolderPathPtr CreateFolderPath(const ItemIdList& path)
{
	TCHAR buf[MAX_PATH * 2];
	buf[0] = 0;
	path.GetPath(buf);
	if (buf[0] == 0)
		return new DirectoryPath(path);
	else
		return ::CreateFolderPath(buf);
}


//bool StoreFolderPath(const TCHAR* regSection, const TCHAR* regEntry)
//{
//}


FolderPathPtr RestoreFolderPath(const TCHAR* regSection, const TCHAR* regEntry)
{
	ItemIdList path;
	if (path.Retrieve(regSection, regEntry))
		return CreateFolderPath(path);
	else
		return 0;
}

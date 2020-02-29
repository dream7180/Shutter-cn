/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// Path.cpp: implementation of the Path class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Path.h"
#include "BalloonMsg.h"
#include <shtypes.h>	// for STRRET
#include <shlwapi.h>
#include "ItemIdList.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

static const TCHAR DIRSEP= _T('\\');
static const TCHAR DIRSEP_2= _T('/');
static const TCHAR* ALLMASK= _T("*.*");


String::size_type find_last_dir_sep(const String& str, String::size_type from= String::npos)
{
	if (str.empty())
		return String::npos;

	size_t n= from == String::npos ? str.length() : from + 1;

	while (n > 0)
	{
		--n;
		if (str[n] == '\\' || str[n] == '/')
			return n;
	}

	return String::npos;
}


void Path::AppendDirSeparator()
{
	if (!empty() && *(end() - 1) != DIRSEP && *(end() - 1) != _T('/'))
		append(1, DIRSEP);
}


void Path::AppendAllMask()
{
	if (!empty())
	{
		AppendDirSeparator();
		append(ALLMASK);
	}
}

void Path::AppendMask(const TCHAR* mask)
{
	ASSERT(mask && *mask && *mask != _T('\\'));

	if (!empty())
	{
		AppendDirSeparator();
		append(mask);
	}
}


bool Path::MatchExtension(const TCHAR* ext) const
{
	if (empty())
		return false;

	size_t len= length();
	size_t ext_len= _tcslen(ext);
	if (ext_len >= len)
		return false;

	ptrdiff_t n= ext_len;
	while (--n >= 0)
	{
		--len;
		if (tolower(at(len)) != tolower(ext[n]))
			return false;
	}

	return at(--len) == _T('.');
}

// replace extension
bool Path::ReplaceExtension(const TCHAR* new_ext)
{
	size_type end= rfind(_T('.'));
	if (end == npos)
		return false;

	end++;		// skip dot

	replace(end, length() - end, new_ext);

	return true;
}


String Path::GetFileName() const		// extract filename
{
	size_type start= find_last_dir_sep(*this);
	if (start == npos)
		start = 0;
	else
		++start;

	size_type end= rfind(_T('.'));
	if (end == npos || end < start)
		end = npos;
	else
		--end;

	return String(*this, start, end != npos ? end - start + 1 : npos);
}


String Path::GetFileNameAndExt() const		// extract filename with extension
{
	size_type start= find_last_dir_sep(*this);
	if (start == npos)
		start = 0;
	else
		++start;

	return String(*this, start, npos);
}


String Path::GetDir() const	// extract directory name
{
	size_type end= find_last_dir_sep(*this);
	if (end == npos)
		return String();

	return String(*this, 0, end);
}


String Path::GetParentDir() const	// extract parent directory name
{
	size_type end= find_last_dir_sep(*this); //rfind(DIRSEP);
	if (end == npos)
		return String();
	else
		--end;

	size_type start= rfind(DIRSEP, end - 1);
	if (start == npos)
		start = 0;
	else
		++start;

	return String(*this, start, end != npos ? end - start + 1 : npos);
}


String Path::GetParentParentDir() const
{
	size_type end= find_last_dir_sep(*this);
	if (end == npos)
		return String();
	else
		end= find_last_dir_sep(*this, end - 1);//rfind(DIRSEP, end - 1);
	if (end == npos)
		return String();
	else
		--end;

	size_type start= find_last_dir_sep(*this, end - 1); //rfind(DIRSEP, end - 1);
	if (start == npos)
		start = 0;
	else
		++start;

	return String(*this, start, end != npos ? end - start + 1 : npos);
}


String Path::GetExtension() const
{
	size_type sep= find_last_dir_sep(*this);
	size_type end= rfind(_T('.'));
	if (end != npos && (sep == npos || (sep != npos && end > sep)))
		return String(*this, end + 1, size() - end);
	else
		return String();
}


// replace existing file name (leaving path and file extension intact)
//
void Path::RenameFileName(const TCHAR* new_name)
{
	ASSERT(new_name && *new_name);

	size_type start= find_last_dir_sep(*this);// rfind(DIRSEP);
	//if (start == npos)
	//	rfind(_T('/'));
	if (start == npos)
		start = 0;
	else
		++start;

	size_type end= rfind(_T('.'));
	if (end == npos || end < start)
		end = npos;
	else
		--end;

	size_type count= end == npos ? npos : end - start + 1;
	replace(start, count, new_name, _tcslen(new_name));
}


// replace existing file name and extension with a new one
//
void Path::ReplaceFileNameExt(const TCHAR* new_name)
{
	ASSERT(new_name && *new_name);

	size_type start= find_last_dir_sep(*this);//rfind(DIRSEP);
	//if (start == npos)
	//	rfind(_T('/'));
	if (start == npos)
		start = 0;
	else
		++start;

	replace(start, npos, new_name, _tcslen(new_name));
}


extern void AppendToFileName(String& fname, const TCHAR* name_suffix)
{
	ASSERT(name_suffix && *name_suffix);

	String::size_type start= find_last_dir_sep(fname); // fname.rfind(DIRSEP);
	if (start == String::npos)
		start = 0;
	else
		++start;

	String::size_type end= fname.rfind(_T('.'));
	if (end == String::npos || end < start)
		end = String::npos;

	fname.insert(end, name_suffix);
}

// append suffix to file name
//
void Path::AppendToFileName(const TCHAR* name_suffix)
{
	::AppendToFileName(*this, name_suffix);
}


// append dir
//
void Path::AppendDir(const TCHAR* dir, bool add_separator/*= true*/)
{
	ASSERT(dir);

	if (dir != 0 && *dir != 0)	// not an empty string?
	{
		if (*dir != DIRSEP)
			AppendDirSeparator();

		*this += dir;
	}

	if (add_separator)
		AppendDirSeparator();
}


bool Path::FileExists() const
{
	DWORD attrib= ::GetFileAttributes(c_str());

	if (attrib == INVALID_FILE_ATTRIBUTES)
		return false;

	if (attrib & FILE_ATTRIBUTE_DIRECTORY)
		return false;

	return true;
}


// create directory if it doesn't exist
//
bool Path::CreateIfDoesntExist(CWnd* msg_parent) const
{
	if (size() == 0)
	{
		new BalloonMsg(msg_parent, _T("要求路径"),
			_T("请输入目录路径."), BalloonMsg::IERROR);
		return false;
	}

	DWORD attrib= ::GetFileAttributes(c_str());

	if (attrib == 0-1)
	{
		// create dir
		if (CreateFolders())
			return true;

		new BalloonMsg(msg_parent, _T("路径错误"),
			_T("目录路径未能创建."), BalloonMsg::IERROR);

		return false;
	}

	if (attrib & FILE_ATTRIBUTE_DIRECTORY)
		return true;

	if (msg_parent)
	{
		new BalloonMsg(msg_parent, _T("路径错误"),
			_T("请选择存在的文件夹路径或\na 创建新路径."), BalloonMsg::IERROR);
		return false;
	}

	return false;
}


// create folders along the path
bool Path::CreateFolders() const
{
	DWORD attrib= ::GetFileAttributes(c_str());

	if (attrib == 0-1)
	{
		Path parent= GetDir();
		if (parent.size() == 0)
			return false;

		if (!parent.CreateFolders())
			return false;

		if (::CreateDirectory(c_str(), 0))
			return true;

		return false;
	}
	else if (attrib & FILE_ATTRIBUTE_DIRECTORY)
		return true;
	else
		return false;
}


uint64 Path::GetFileLength() const
{
	WIN32_FIND_DATA findFileData;
	HANDLE find= ::FindFirstFile(c_str(), &findFileData);
	if (find == INVALID_HANDLE_VALUE)
		return 0;
	VERIFY(::FindClose(find));

	return uint64(findFileData.nFileSizeLow) + (uint64(findFileData.nFileSizeHigh) << 32);
}


bool Path::IsFolder() const
{
	DWORD attrib= ::GetFileAttributes(c_str());
	if (attrib == INVALID_FILE_ATTRIBUTES)
		return false;

	return !!(attrib & FILE_ATTRIBUTE_DIRECTORY);
}


bool Path::IsLink() const
{
	return MatchExtension(_T("lnk"));
}


// resolve link file
ITEMIDLIST* Path::GetLinkedObject()
{
	IShellLinkWPtr SL;
	if (::CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (void**)&SL) == S_OK)
	{
		IPersistFilePtr PF;
		if (SL->QueryInterface(IID_IPersistFile, (void**)&PF) == S_OK)
		{
			// Load() accepts only Unicode path
#ifdef _UNICODE
			if (PF->Load(c_str(),
#else
			_bstr_t path= c_str();
			LPCOLESTR path= path;
			if (PF->Load(path,
#endif
					STGM_READ | STGM_SHARE_DENY_WRITE) == S_OK)
			{
				if (SL->Resolve(0, SLR_ANY_MATCH | SLR_NOSEARCH | SLR_NO_UI | SLR_UPDATE) == S_OK)
				{
					ITEMIDLIST* idl= 0;
					if (SL->GetIDList(&idl) == S_OK)
						return idl;
				}
			}
		}
	}

	return 0;
}


Path::Path(STRRET& str, bool release_str_ret)
{
//TODO: revise on win98
#ifdef _UNICODE
	if (str.uType == STRRET_WSTR)
	{
		*this = str.pOleStr;
		if (release_str_ret)
		{
			IMallocPtr malloc;
			::SHGetMalloc(&malloc);
			if (malloc != 0)
				malloc->Free(str.pOleStr);
		}
	}
#else
	if (str.type == STRRET_CSTR)
	{
		*this = str.cStr;
	}
#endif
}


void Path::BackslashToForwardslash()
{
	size_type pos= find(DIRSEP);

	while (pos != npos)
	{
		replace(pos, 1, 1, _T('/'));
		pos = find(DIRSEP, pos + 1);
	}
}


String Path::GetRoot() const
{
	if (size() < 2)
		return *this;

	size_type drv= find(_T(':'));
	if (drv != npos)
	{
		TCHAR c= (*this)[drv + 1];
		return String(*this, 0, drv + 1 + (c == _T('\\') || c == _T('/') ? 1 : 0));
	}

	//TODO

//	if ((*this)[0] == _T('/') && (*this)[0] == _T('/')

	return *this;
}


bool Path::MatchFileSpec(const TCHAR* spec) const
{
	return !!::PathMatchSpec(c_str(), spec);
}


extern Path GetDocumentsFolder(const TCHAR* fallback_dir)
{
	Path docs;

	ITEMIDLIST* pidl= 0;
	if (::SHGetSpecialFolderLocation(AfxGetMainWnd()->GetSafeHwnd(), CSIDL_PERSONAL, &pidl) == NOERROR && pidl != 0)
	{
		ItemIdList folder(pidl, false);
		docs.assign(folder.GetPath());
	}
	else if (fallback_dir)
		docs = fallback_dir;

	return docs;
}

/*
extern Path GetApplicationDataFolder(const TCHAR* fallback_dir)
{
	Path dir;

	ITEMIDLIST* pidl= 0;
	if (::SHGetSpecialFolderLocation(0, CSIDL_APPDATA, &pidl) == NOERROR && pidl != 0)
	{
		ItemIdList folder(pidl, false);
		dir.assign(folder.GetPath());
	}
	else if (fallback_dir)
		dir = fallback_dir;

	if (!dir.empty())
	{
		dir.AppendDir(_T("MiK\\ExifPro"), false);

		dir.CreateFolders();
	}

	return dir;
}
*/
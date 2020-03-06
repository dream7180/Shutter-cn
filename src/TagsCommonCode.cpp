/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// TagsBarCommon.cpp : implementation file
//

#include "stdafx.h"
#include "TagsCommonCode.h"
#include "PhotoTags.h"
#include "Path.h"
#include "BalloonMsg.h"
#include <boost/algorithm/string/replace.hpp>
#include "PhotoTagsCollection.h"
#include "Exception.h"

using namespace boost::algorithm;

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

namespace Tags {


String GetTagsPathName()
{
	TCHAR prog_path[_MAX_PATH];
	VERIFY(::GetModuleFileName(AfxGetInstanceHandle(), prog_path, _MAX_PATH));
	Path path= Path(prog_path).GetDir();
	//Path path= GetApplicationDataFolder(L"C:\\");
	path.AppendDir(_T("PhotoTags.dat"), false);
	return path;
}


bool OpenTags(const TCHAR* filename, PhotoTagsCollection& collection)
{
	// reload tags
	bool loaded= false;
	try
	{
		loaded = LoadTags(filename, collection, 0);
	}
	catch (...)
	{
	}

	if (!loaded)
	{
		// no tags loaded--add some so tag bar doesn't show up empty
		collection.FromString(_T("Good\n候选\n\n打印\n已打印\n待打印"));
	}

	return loaded;
}

///////////////////////////////////////////////////////////////////////////////

static TCHAR* g_ext= _T(".txt");
static TCHAR* g_name= _T("Photo Tags.txt");
static TCHAR* g_filter= _T("标记文本文件 (*.txt)|*.txt|所有文件 (*.*)|*.*||");


bool LoadTags(const TCHAR* filename, PhotoTagsCollection& collection, CWnd* parent)
{
	Path path(filename);
	if (!path.FileExists())
		return false;

	CFile file(filename, CFile::modeRead);

	ULONGLONG len= file.GetLength();
	if (len > 0x100000)
	{
		if (parent)
			new BalloonMsg(parent, _T("标记文件太大"), _T("尝试载入的文件过大.\n文件应不大于 1 MB."), BalloonMsg::IERROR);

		return false;
	}

	DWORD length= static_cast<DWORD>(len);

	std::vector<TCHAR> buf(1 + length / sizeof TCHAR);

	//TODO: handle non-Unicode

	file.Read(&buf.front(), length);

	// skip Unicode signature
	if (length > 2 && buf.front() == 0xfeff)		// Unicode marker?
		buf.erase(buf.begin(), buf.begin() + 1);

	replace_all(buf, _T("\xd\xa"), _T("\xa"));

	collection.FromString(String(&buf.front()));

	collection.SetModified(false);

	return true;
}


void SaveTags(const TCHAR* filename, const PhotoTagsCollection& collection)
{
	// save tags
	CFile file(filename, CFile::modeWrite | CFile::modeCreate);

	String tags= collection.AsString();

	if (tags.empty())
	{
		ASSERT(false);
		return;
	}

#ifdef _UNICODE
	// insert unicode signature, so Notepad will handle this file properly
	tags.insert(0, 1, wchar_t(0xfeff));
//	tags.insert(0, 1, L'\xfeff');
#endif

	// DOS end of lines
	replace_all(tags, _T("\xa"), _T("\xd\xa"));

	size_t size= tags.length() * sizeof TCHAR;
	if (size > 1 << 31)	// 2 GB
		THROW_EXCEPTION(L"保存标记出错", L"标记文件 2 GB 大小限制.");

	file.Write(&tags[0], static_cast<UINT>(size));
}


void ResetPopupMenuTags(CMenu& menu, int first_id, const PhotoTagsCollection& tags)
{
	// delete all items first
	{
		const UINT count= menu.GetMenuItemCount();
		for (UINT i= count; i > 0; --i)
			menu.DeleteMenu(i - 1, MF_BYPOSITION);
	}

	// limit amount of tags to show
	const int count= std::min<int>(static_cast<int>(tags.GetCount()), MAX_TAGS);

	if (count == 0)	// this menu cannot be empty
		menu.AppendMenu(MF_STRING, first_id, _T("<No Tags Defined>"));
	else
	{
		for (int i= 0; i < count; ++i)
		{
			int id= first_id + i;

			if (i < 9)
			{
				oStringstream ost;
				ost << _T('&') << i + 1 << _T(". ") << tags.Get(i).c_str();

				menu.AppendMenu(MF_STRING, id, ost.str().c_str());
			}
			else
				menu.AppendMenu(MF_STRING, id, tags.Get(i).c_str());
		}
	}
}


// common tags collection
static PhotoTagsCollection* global_tags= new PhotoTagsCollection();

PhotoTagsCollection& GetTagCollection()
{
//	static PhotoTagsCollection* global= new PhotoTagsCollection();
	return *global_tags;
}

// this isn't really necessary, but MFC's leak reporting kicks in too early, and reports
// function local static objects as not freed...
extern void FreeTagCollection()
{
	if (global_tags)
	{
		delete global_tags;
		global_tags = 0;
	}
}


} // namespace

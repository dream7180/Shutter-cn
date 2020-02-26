/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "RecentPaths.h"
#include "ItemIdList.h"


namespace {
	struct same_path
	{
		same_path(const ItemIdList& path) : path_(path)
		{}

		bool operator () (const boost::shared_ptr<ItemIdList>& elem) const
		{
			return *elem == path_;
		}

	private:
		const ItemIdList& path_;
	};
}


extern void InsertUniquePath(PathVector& paths, const ItemIdList& path, bool at_end)
{
	PathVector::iterator it= find_if(paths.begin(), paths.end(), same_path(path));
	if (it == paths.end())
	{
		const ITEMIDLIST* p= path;
		boost::shared_ptr<ItemIdList> ptr(new ItemIdList(p));
		if (at_end)
			paths.push_back(ptr);
		else
			paths.insert(paths.begin(), ptr);
	}
	else
	{
		// since path being inserted is already present, promote it
		// (if 'at_end' is true it will be promoted to the 'last recently used' category)
		ItemIdListPtr p= *it;
		paths.erase(it);
		if (at_end)
			paths.push_back(p);
		else
			paths.insert(paths.begin(), p);
	}
}

/*
void PromotePath(PathVector& paths, const ItemIdList& path)
{
	PathVector::iterator it= find_if(paths.begin(), paths.end(), same_path(path));
	if (it == paths.end())
	{
		const ITEMIDLIST* p= path;
		paths.push_back(boost::shared_ptr<ItemIdList>(new ItemIdList(p)));
	}
	else
	{
		// move to front
		boost::shared_ptr<ItemIdList> p= *it;
		paths.erase(it);
		paths.insert(paths.begin(), p);
	}
}  */

extern void RecentPaths(PathVector& paths, bool store, const TCHAR* section, int max_count)
{
	if (!store)
		paths.clear();

	const int MAX_RECENT_PATHS= max_count;

	// when storing, store up to MAX_RECENT_PATHS most recent paths

	// yes, this is correct, most recent are at the back
	int start= 0;
	if (paths.size() > MAX_RECENT_PATHS)
		start = static_cast<int>(paths.size()) - MAX_RECENT_PATHS;

	for (int i= 0; i < MAX_RECENT_PATHS; ++i)
	{
		oStringstream ost;
		ost << _T("path_") << i;

		if (store)
		{
			int index= start + i;

			if (index >= paths.size())
			{
				// clear remaining
				if (HKEY sec_key= AfxGetApp()->GetSectionKey(section))
				{
					::RegDeleteValue(sec_key, ost.str().c_str());
					::RegCloseKey(sec_key);
				}
			}
			else
			{
				paths[index]->Store(section, ost.str().c_str());
			}
		}
		else
		{
			ItemIdList path;
			if (path.Retrieve(section, ost.str().c_str()))
				InsertUniquePath(paths, path, true);
		}
	}
}


extern void AddCommonPaths(PathVector& paths, ItemIdList* extra_path)
{
	InsertUniquePath(paths, ItemIdList(CSIDL_DESKTOP), true);
	InsertUniquePath(paths, ItemIdList(CSIDL_MYPICTURES), true);

	if (extra_path)
		InsertUniquePath(paths, *extra_path, true);

	if (DWORD len= ::GetLogicalDriveStrings(0, 0))
	{
		std::vector<TCHAR> drv(len + 1, 0);
		::GetLogicalDriveStrings(len, &drv[0]);

		TCHAR* p= &drv[0];
		while (*p)
		{
			switch (::GetDriveType(p))
			{
			case DRIVE_CDROM:
			case DRIVE_NO_ROOT_DIR:
				break;
			default:
				InsertUniquePath(paths, ItemIdList(p), true);
				break;
			}

			p += _tcslen(p) + 1;
		}
	}
}


extern void AppendPaths(PathVector& paths, const PathVector& more_paths)
{
	const size_t count= more_paths.size();
	for (size_t i= 0; i < count; ++i)
	{
		ItemIdListPtr path= more_paths[i];
		PathVector::iterator it= find_if(paths.begin(), paths.end(), same_path(*path));
		if (it == paths.end())
			paths.push_back(path);
	}
}

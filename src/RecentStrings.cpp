/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"


void AddRecentString(std::vector<String>& strings, const TCHAR* text)
{
	String str(text);

	std::vector<String>::iterator it= find(strings.begin(), strings.end(), str);
	if (it == strings.end())
	{
		// not found--add it
		strings.push_back(str);
	}
	else
	{
		// since string being inserted is already present, promote it to LRU area (at the back)
		strings.erase(it);
		strings.push_back(str);
	}
}


void RecentStrings(std::vector<String>& strings, size_t max_count, bool store, const TCHAR* section)
{
	if (!store)
		strings.clear();

	// when storing, store up to 'max_count' most recent strings

	int start= 0;
	if (strings.size() > max_count)
		start = static_cast<int>(strings.size()) - max_count;

	for (int i= 0; i < max_count; ++i)
	{
		oStringstream ost;
		ost << _T("str-") << i;

		if (store)
		{
			int index= start + i;

			if (index >= strings.size())
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
				AfxGetApp()->WriteProfileString(section, ost.str().c_str(), strings[index].c_str());
			}
		}
		else
		{
			CString str= AfxGetApp()->GetProfileString(section, ost.str().c_str());
			if (str.IsEmpty())
				break;	// no more strings

			AddRecentString(strings, str);
		}
	}
}

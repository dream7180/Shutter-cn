/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// PhotoTags.h: interface for the PhotoTags class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PHOTOTAGS_H__B26822C7_F685_46ED_9AE0_FCEBBAD7F25A__INCLUDED_)
#define AFX_PHOTOTAGS_H__B26822C7_F685_46ED_9AE0_FCEBBAD7F25A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class FileStream;


// photo tags stored by particular photos (usually empty)
//
class PhotoTags : std::vector<String>
{
public:
	PhotoTags() {}

	bool FindTag(const String& name) const
	{
		return binary_search(begin(), end(), name);
	}

	bool FindTagNoCase(const String& str, bool partial_match) const;

	// load tags from photo's file
	void LoadTags(FileStream& fs);

	// save tags in photo's file
	void SaveTags(FileStream& fs);

	void ApplyTag(const String& name);
	void RemoveTag(const String& name);

	void AssignKeywords(const std::vector<String>& keys);

	void CopyTagsToArray(std::vector<String>& tags) const;

	// all tags '\n' separated
	String AsString() const;

	// all tags comma separated, in one string
	String CommaSeparated() const;

	using std::vector<String>::empty;
	using std::vector<String>::size;
	using std::vector<String>::push_back;
	using std::vector<String>::operator [];
	using std::vector<String>::const_iterator;
	using std::vector<String>::begin;
	using std::vector<String>::end;
	using std::vector<String>::clear;
};


#endif // !defined(AFX_PHOTOTAGS_H__B26822C7_F685_46ED_9AE0_FCEBBAD7F25A__INCLUDED_)

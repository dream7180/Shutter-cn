/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once

// TagsCommonCode

class PhotoTagsCollection;


namespace Tags
{

enum { MAX_TAGS= 10000 };

void ResetPopupMenuTags(CMenu& menu, int first_id, const PhotoTagsCollection& tags);

// global tag collection
PhotoTagsCollection& GetTagCollection();

// load tags from text file
bool LoadTags(const TCHAR* filename, PhotoTagsCollection& collection, CWnd* parent);

// save
void SaveTags(const TCHAR* filename, const PhotoTagsCollection& collection);

// default path
String GetTagsPathName();

// initialize collection using given file or default tags if file is not available
bool OpenTags(const TCHAR* filename, PhotoTagsCollection& collection);


} // namespace

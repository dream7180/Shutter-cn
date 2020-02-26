/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once

void AddRecentString(std::vector<String>& strings, const TCHAR* text);

void RecentStrings(std::vector<String>& strings, size_t max_count, bool store, const TCHAR* section);

/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once

#include <boost/shared_ptr.hpp>

class ItemIdList;
typedef boost::shared_ptr<ItemIdList> ItemIdListPtr;
typedef std::vector<ItemIdListPtr> PathVector;

// add 'path' to a vector of 'paths' if it isn't there yet; if it is present
// already, it will be promoted to the end/front of a vector
// path is added to the front or end of a vector depending on 'at_end' param
extern void InsertUniquePath(PathVector& paths, const ItemIdList& path, bool at_end);

// function to store or retrieve vector of paths to/from the registry;
// if more than 'max_count' paths is present in a vector, only *last* max_count paths
// will be stored, most recent paths are expected to be at the end
extern void RecentPaths(PathVector& paths, bool store, const TCHAR* section, int max_count);

// handy function to add most common path to the 'paths' vector; those include
// documents folder, pictures folder, and logical drives (apart from CD-ROMS)
extern void AddCommonPaths(PathVector& paths, ItemIdList* extra_path);

// append 'more_paths' to the 'paths' taking only those that are not yet in a 'paths' vector
extern void AppendPaths(PathVector& paths, const PathVector& more_paths);

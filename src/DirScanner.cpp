/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// DirScanner.cpp: implementation of the DirScanner class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DirScanner.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

DirScanner::DirScanner() : break_(false), dir_visited_(0)
{}

DirScanner::~DirScanner()
{}


bool DirScanner::ScanDir(Path path, bool scan_subdirs)
{
	ASSERT(!path.empty());

	CFileFind find;
	path.AppendAllMask();
	BOOL working= find.FindFile(path.c_str());
	if (!working)
		return true;	// just continue

	int dir_visited= ++dir_visited_;

	EnteringDir(path /*Path(find.GetFilePath()), find*/, dir_visited);

	while (working)
	{
		if (break_)
			return false;

		working = find.FindNextFile();

		if (find.IsDots())
			continue;

		if (find.IsDirectory())
		{
			if (scan_subdirs)
			{
//				EnteringDir(Path(find.GetFilePath()), find, dir_visited);

				if (!ScanDir(Path(find.GetFilePath()), scan_subdirs))
					return false;

//				ExitingDir(dir_visited);
			}
			continue;
		}

#if _MSC_VER < 1310	// not vc 7.1?
		int64 len= find.GetLength64();
#else
		int64 len= find.GetLength();
#endif

		if (!FileScan(Path(find.GetFilePath()), len, find, dir_visited, scan_subdirs))
			return true;
	}

	ExitingDir(dir_visited);

	return true;
}


void DirScanner::EnteringDir(const Path& path, int id)
{}

void DirScanner::ExitingDir(int id)
{}

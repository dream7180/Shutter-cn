/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
#include "DirScanner.h"
#include "ColorProfileForward.h"


struct ICCProfileInfo
{
	ICCProfileInfo(ColorProfilePtr profile, Path file) : file_(file), profile_(profile)
	{}

	Path file_;
	ColorProfilePtr profile_;
};


class ICCScanner : public DirScanner
{
public:
	ICCScanner(std::vector<ICCProfileInfo>& icc_files);
	virtual ~ICCScanner();

	virtual bool FileScan(Path path, int64 file_length, const CFileFind& find, int dir_visited, bool scanSubdirs);

	std::vector<ICCProfileInfo>& icc_files_;
};

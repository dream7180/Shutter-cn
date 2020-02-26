/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
#include "ImageStat.h"
class Dib;


class BMPReader
{
public:
	BMPReader();
	~BMPReader();

	ImageStat Open(const TCHAR* filePath);

	bool IsSupported() const;

	CSize GetSize() const;

	ImageStat ReadImage(Dib& dib);

private:
	struct Impl;
	std::auto_ptr<Impl> pImpl_;
};

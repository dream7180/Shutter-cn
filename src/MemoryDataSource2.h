/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// MemoryDataSource.h: interface for the CMemoryDataSource class.

#pragma once
#include "JPEGDataSource.h"


class CMemoryDataSource2 : public JPEGDataSource
{
public:
	CMemoryDataSource2(const BYTE* data, size_t size);

	virtual ~CMemoryDataSource2();

	virtual bool FillInputBuffer();
	virtual void SkipInputData(long num_bytes);

private:
	const BYTE* end_of_data_;
	std::vector<BYTE> data_;
};


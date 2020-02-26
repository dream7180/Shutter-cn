/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// MemoryDataSource.h: interface for the CMemoryDataSource class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MEMORYDATASOURCE_H__9BE09AEA_BB48_44B7_813F_6EA9151C4608__INCLUDED_)
#define AFX_MEMORYDATASOURCE_H__9BE09AEA_BB48_44B7_813F_6EA9151C4608__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "JPEGDataSource.h"


class CMemoryDataSource : public JPEGDataSource
{
public:
	CMemoryDataSource(const BYTE* data, size_t size);
	CMemoryDataSource(const char* data, size_t size);
	virtual ~CMemoryDataSource();

	virtual bool FillInputBuffer();
	virtual void SkipInputData(long num_bytes);

private:
	const BYTE* end_of_data_;
};

#endif // !defined(AFX_MEMORYDATASOURCE_H__9BE09AEA_BB48_44B7_813F_6EA9151C4608__INCLUDED_)

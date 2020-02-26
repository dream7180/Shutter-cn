/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// FileDataSource.h: interface for the CFileDataSource class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FILEDATASOURCE_H__463A96A6_C4A3_4F4B_BEE7_E475CC4401FF__INCLUDED_)
#define AFX_FILEDATASOURCE_H__463A96A6_C4A3_4F4B_BEE7_E475CC4401FF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "JPEGDataSource.h"


class CFileDataSource : public JPEGDataSource
{
public:
	CFileDataSource(const TCHAR* file_name, uint32 offset);
	virtual ~CFileDataSource();

	virtual bool FillInputBuffer();
	virtual void SkipInputData(long num_bytes);

	virtual void InitSource();
	virtual void TermSource();
	virtual void Abort();

private:
	CStdioFile in_file_;
	std::vector<uint8> buffer_;
	uint32 offset_;
	String file_name_;
	enum { BUFFER_SIZE= 64 * 1024 };
};

#endif // !defined(AFX_FILEDATASOURCE_H__463A96A6_C4A3_4F4B_BEE7_E475CC4401FF__INCLUDED_)

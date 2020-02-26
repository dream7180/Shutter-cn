/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// FileDataDestination.h: interface for the CFileDataDestination class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FILEDATADESTINATION_H__F1162469_7DBC_4BCB_B9FA_05806CADDD83__INCLUDED_)
#define AFX_FILEDATADESTINATION_H__F1162469_7DBC_4BCB_B9FA_05806CADDD83__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "JPEGDataDestination.h"


class CFileDataDestination : public JPEGDataDestination
{
public:
	CFileDataDestination(const TCHAR* file_name);
	virtual ~CFileDataDestination();

	virtual bool EmptyOutputBuffer();

	virtual void InitDestination();
	virtual void TermDestination();

	virtual void Abort();

private:
	CFile out_file_;
	std::vector<uint8> buffer_;
};

#endif // !defined(AFX_FILEDATADESTINATION_H__F1162469_7DBC_4BCB_B9FA_05806CADDD83__INCLUDED_)

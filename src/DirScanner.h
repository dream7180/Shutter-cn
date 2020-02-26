/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/


// DirScanner.h: interface for the DirScanner class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SCANNER_H__F5546631_CC6A_11D3_B61E_000000000000__INCLUDED_)
#define AFX_SCANNER_H__F5546631_CC6A_11D3_B61E_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Path.h"


class DirScanner
{
public:
	DirScanner();
	virtual ~DirScanner();

	bool ScanDir(Path path, bool scan_subdirs);

	virtual bool FileScan(Path path, int64 file_length, const CFileFind& find, int dir_visited, bool scanSubdirs)= 0;

	virtual void EnteringDir(const Path& path, int id);
	virtual void ExitingDir(int id);

	void Break()				{ break_ = true; }
	bool Stop() const			{ return break_; }

private:
	bool break_;
	int dir_visited_;
};

#endif // !defined(AFX_SCANNER_H__F5546631_CC6A_11D3_B61E_000000000000__INCLUDED_)

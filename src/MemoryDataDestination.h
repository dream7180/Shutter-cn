/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// MemoryDataDestination.h: interface for the CMemoryDataDestination class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MEMORYDATADESTINATION_H__088734DD_8DE7_4FD5_815E_7905020EF346__INCLUDED_)
#define AFX_MEMORYDATADESTINATION_H__088734DD_8DE7_4FD5_815E_7905020EF346__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "JPEGDataDestination.h"


class CMemoryDataDestination : public JPEGDataDestination
{
public:
	CMemoryDataDestination();
	virtual ~CMemoryDataDestination();

	virtual bool EmptyOutputBuffer();

	virtual void InitDestination();
	virtual void TermDestination();

	virtual void Abort();

	const BYTE* GetJPEG() const			{ return jpeg_.empty() ? 0 : &jpeg_.front(); }
	const int GetSize() const			{ return static_cast<int>(jpeg_.size()); }

	void SwapJPEG(std::vector<uint8>& jpg)	{ jpeg_.swap(jpg); }

private:
	void WriteOut(const BYTE* data, size_t len);
	std::vector<uint8> buffer_;
	std::vector<uint8> jpeg_;
};

#endif // !defined(AFX_MEMORYDATADESTINATION_H__088734DD_8DE7_4FD5_815E_7905020EF346__INCLUDED_)

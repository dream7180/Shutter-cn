// MemoryDataSource.h: interface for the CMemoryDataSource class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MEMORYDATASOURCE_H__9BE09AEA_BB48_44B7_813F_6EA9151C4608__INCLUDED_)
#define AFX_MEMORYDATASOURCE_H__9BE09AEA_BB48_44B7_813F_6EA9151C4608__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "JPEGDataSource.h"


class CMemoryDataSource : public CJPEGDataSource
{
public:
	CMemoryDataSource(const BYTE* pcData, int nSize);
	CMemoryDataSource(const char* pcData, int nSize);
	virtual ~CMemoryDataSource();

	virtual bool FillInputBuffer();
	virtual void SkipInputData(long lNumBytes);

private:
	const BYTE* m_pcEndOfData;
	static const BYTE s_vEndOfImg[];
};

#endif // !defined(AFX_MEMORYDATASOURCE_H__9BE09AEA_BB48_44B7_813F_6EA9151C4608__INCLUDED_)

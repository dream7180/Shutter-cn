// MemoryDataSource.cpp: implementation of the CMemoryDataSource class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MemoryDataSource.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMemoryDataSource::CMemoryDataSource(const BYTE* pcData, int nSize)
{
	next_input_byte = pcData;
	bytes_in_buffer = nSize;
	m_pcEndOfData = pcData + nSize;
}

CMemoryDataSource::CMemoryDataSource(const char* pcData, int nSize)
{
	next_input_byte = reinterpret_cast<const BYTE*>(pcData);
	bytes_in_buffer = nSize;
	m_pcEndOfData = next_input_byte + nSize;
}


CMemoryDataSource::~CMemoryDataSource()
{}


const BYTE CMemoryDataSource::s_vEndOfImg[]= {	0xff, 0xd9, 0xff, 0xff };	// end of image mark


void CMemoryDataSource::SkipInputData(long lNumBytes)
{
	next_input_byte += lNumBytes;
	if (next_input_byte >= m_pcEndOfData)
		next_input_byte = m_pcEndOfData - 1;
}


bool CMemoryDataSource::FillInputBuffer()
{
	next_input_byte = s_vEndOfImg;
	bytes_in_buffer = array_count(s_vEndOfImg);
	return true;
}

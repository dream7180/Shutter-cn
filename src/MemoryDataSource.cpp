/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

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

CMemoryDataSource::CMemoryDataSource(const BYTE* data, size_t size)
{
	next_input_byte = data;
	bytes_in_buffer = size;
	end_of_data_ = data + size;
}

CMemoryDataSource::CMemoryDataSource(const char* data, size_t size)
{
	next_input_byte = reinterpret_cast<const BYTE*>(data);
	bytes_in_buffer = size;
	end_of_data_ = next_input_byte + size;
}


CMemoryDataSource::~CMemoryDataSource()
{}


const BYTE EndOfJpeg[]= { 0xff, 0xd9, 0xff, 0xff };	// end of image mark


void CMemoryDataSource::SkipInputData(long num_bytes)
{
	next_input_byte += num_bytes;
	if (next_input_byte >= end_of_data_)
		next_input_byte = end_of_data_ - 1;
}


bool CMemoryDataSource::FillInputBuffer()
{
	next_input_byte = EndOfJpeg;
	bytes_in_buffer = array_count(EndOfJpeg);
	return true;
}

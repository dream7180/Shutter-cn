/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "MemoryDataSource2.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////

CMemoryDataSource2::CMemoryDataSource2(const BYTE* data, size_t size)
 : data_(data, data + size)
{
	data = &data_.front();
	next_input_byte = data;
	bytes_in_buffer = size;
	end_of_data_ = data + size;
}


CMemoryDataSource2::~CMemoryDataSource2()
{}


const BYTE EndOfJpeg[]= { 0xff, 0xd9, 0xff, 0xff };	// end of image mark


void CMemoryDataSource2::SkipInputData(long num_bytes)
{
	next_input_byte += num_bytes;
	if (next_input_byte >= end_of_data_)
		next_input_byte = end_of_data_ - 1;
}


bool CMemoryDataSource2::FillInputBuffer()
{
	next_input_byte = EndOfJpeg;
	bytes_in_buffer = array_count(EndOfJpeg);
	return true;
}

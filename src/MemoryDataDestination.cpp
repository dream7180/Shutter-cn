/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// MemoryDataDestination.cpp: implementation of the CMemoryDataDestination class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
//#include "exifpro.h"
#include "MemoryDataDestination.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMemoryDataDestination::CMemoryDataDestination()
 : buffer_(64 * 1024)
{
	next_output_byte = &buffer_.front();
	free_in_buffer = buffer_.size();
}

CMemoryDataDestination::~CMemoryDataDestination()
{}


bool CMemoryDataDestination::EmptyOutputBuffer()
{
	WriteOut(&buffer_.front(), buffer_.size());

	next_output_byte = &buffer_.front();
	free_in_buffer = buffer_.size();

	return true;
}


void CMemoryDataDestination::InitDestination()
{
}

void CMemoryDataDestination::TermDestination()
{
	WriteOut(&buffer_.front(), buffer_.size() - free_in_buffer);
}


void CMemoryDataDestination::Abort()
{
	jpeg_.clear();
}


void CMemoryDataDestination::WriteOut(const BYTE* data, size_t len)
{
	if (data && len)
	{
		size_t size= jpeg_.size();
		jpeg_.resize(size + len);
		memcpy(&jpeg_.front() + size, data, len);
	}
}

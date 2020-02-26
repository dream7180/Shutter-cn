/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// FileDataSource.cpp: implementation of the CFileDataSource class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FileDataSource.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
static int file_counter_= 0;
#endif

//////////////////////////////////////////////////////////////////////

CFileDataSource::CFileDataSource(const TCHAR* file_name, uint32 offset)
 : /*in_file_(file_name, CFile::modeRead | CFile::shareDenyWrite),*/
   file_name_(file_name), offset_(offset), buffer_(BUFFER_SIZE)
{
	next_input_byte = &buffer_.front();
	bytes_in_buffer = 0;
//	in_file_.Seek(offset, CFile::begin);
#ifdef _DEBUG
	++file_counter_;
//	TRACE(L"new file data src: %d\n", file_counter_);
#endif;
}


CFileDataSource::~CFileDataSource()
{
#ifdef _DEBUG
	--file_counter_;
//	TRACE(L"file data src left: %d\n", file_counter_);
#endif;
}


bool CFileDataSource::FillInputBuffer()
{
	if (buffer_.empty())
	{
		ASSERT(false);
		return false;
	}

	UINT count= in_file_.Read(&buffer_.front(), static_cast<UINT>(buffer_.size()));
	next_input_byte = &buffer_.front();
	if (count == 0)  // end of file?
	{
		// return fake EndOfImg
		buffer_[0] = 0xff;
		buffer_[1] = JPEG_EOI;
		bytes_in_buffer = 2;
	}
	else
		bytes_in_buffer = count;
	return true;
}


void CFileDataSource::SkipInputData(long num_bytes)
{
	while (num_bytes > bytes_in_buffer)
	{
		num_bytes -= static_cast<long>(bytes_in_buffer);
		bytes_in_buffer = 0;
		FillInputBuffer();
	}

	bytes_in_buffer -= num_bytes;
	next_input_byte += num_bytes;

	ASSERT(bytes_in_buffer < buffer_.size());
	ASSERT(next_input_byte >= &buffer_.front());
	ASSERT(next_input_byte <= &buffer_.back());
}


void CFileDataSource::InitSource()
{
	if (buffer_.empty())
		buffer_.resize(BUFFER_SIZE);

	next_input_byte = &buffer_.front();
	bytes_in_buffer = 0;

	if (in_file_ == CFile::hFileNull)	// open if not already opened
	{
		// note: in principle file should be opened in a shareDenyWrite mode, but main thread
		// uses CanEditIPTCRecord that opens file in a write mode; this causes share violations
		if (!in_file_.Open(file_name_.c_str(), CFile::modeRead | CFile::shareDenyNone | CFile::typeBinary))
		{
	//TRACE(L"open file %s failed\n", file_name_.c_str());
			// do not give up too easily
			::Sleep(20);

			if (!in_file_.Open(file_name_.c_str(), CFile::modeRead | CFile::shareDenyWrite | CFile::typeBinary))
				CFileException::ThrowOsError(::GetLastError(), file_name_.c_str());
		}

	//	TRACE(L"file src handle: %x - %s\n", in_file_.m_hFile, file_name_.c_str());
	}

	in_file_.Seek(offset_, CFile::begin);
}


void CFileDataSource::TermSource()
{
	in_file_.Close();
	buffer_.swap(std::vector<uint8>());	// release buffer's memory
	next_input_byte = 0;
	bytes_in_buffer = 0;
}


void CFileDataSource::Abort()
{
	TermSource();
}

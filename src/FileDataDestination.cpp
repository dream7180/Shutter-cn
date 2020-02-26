/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// FileDataDestination.cpp: implementation of the CFileDataDestination class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FileDataDestination.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFileDataDestination::CFileDataDestination(const TCHAR* file_name)
 : out_file_(file_name, CFile::modeWrite | CFile::modeCreate), buffer_(64 * 1024)
{
	next_output_byte = &buffer_.front();
	free_in_buffer = buffer_.size();
}


CFileDataDestination::~CFileDataDestination()
{}


bool CFileDataDestination::EmptyOutputBuffer()
{
	out_file_.Write(&buffer_.front(), static_cast<UINT>(buffer_.size()));

	next_output_byte = &buffer_.front();
	free_in_buffer = buffer_.size();

	return true;
}


void CFileDataDestination::InitDestination()
{
}

void CFileDataDestination::TermDestination()
{
	out_file_.Write(&buffer_.front(), static_cast<UINT>(buffer_.size() - free_in_buffer));
	out_file_.Close();
}


void CFileDataDestination::Abort()
{
	CString file= out_file_.GetFilePath();
	out_file_.Close();
	::DeleteFile(file);
}

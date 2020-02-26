/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// Logger.cpp: implementation of the Logger class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Logger.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


std::ofstream* Logger::ofs_log_= 0;

//////////////////////////////////////////////////////////////////////

void Logger::Open(const TCHAR* file_name)
{
	ASSERT(file_name && *file_name);

	if (ofs_log_)
		return;

#ifdef _UNICODE
	USES_CONVERSION;
	ofs_log_ = new std::ofstream(W2A(file_name));
//	ofs_log_->open(W2A(file_name));
#else
	ofs_log_ = new ofstream(file_name);
//	ofs_log_->open(file_name);
#endif
}


void Logger::WriteFileName(const char* str, int line)
{
	if (ofs_log_)
	{
		*ofs_log_ << str << '\t' << line << '\t';
		SYSTEMTIME st;
		::GetLocalTime(&st);
		CString s;
		s.Format(_T("%02hd:%02hd:%02hd.%03hd"), st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
#ifdef _UNICODE
		USES_CONVERSION;
		*ofs_log_ << W2A(s) << '\t';
#else
		*ofs_log_ << static_cast<const char*>(s) << '\t';
#endif
	}
}

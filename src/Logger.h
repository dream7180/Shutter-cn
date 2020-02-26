/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// Logger.h: interface for the Logger class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LOGGER_H__D145E4AA_91E5_461F_9499_A50EF6BBA017__INCLUDED_)
#define AFX_LOGGER_H__D145E4AA_91E5_461F_9499_A50EF6BBA017__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class Logger
{
public:
	Logger() {}

	static void Open(const TCHAR* file_name);

	Logger& operator << (int n)					{ if (ofs_log_) { *ofs_log_ << n; ofs_log_->flush(); } return *this; }
	Logger& operator << (unsigned int u)		{ if (ofs_log_) { *ofs_log_ << u; ofs_log_->flush(); } return *this; }
	Logger& operator << (short s)				{ if (ofs_log_) { *ofs_log_ << s; ofs_log_->flush(); } return *this; }
//	Logger& operator << (unsigned short u)		{ if (ofs_log_) { *ofs_log_ << u; ofs_log_->flush(); } return *this; }
	Logger& operator << (float f)				{ if (ofs_log_) { *ofs_log_ << f; ofs_log_->flush(); } return *this; }
	Logger& operator << (double d)				{ if (ofs_log_) { *ofs_log_ << d; ofs_log_->flush(); } return *this; }

	Logger& operator << (char c)				{ if (ofs_log_) { *ofs_log_ << c; ofs_log_->flush(); } return *this; }
	Logger& operator << (wchar_t c)				{ if (ofs_log_) { *ofs_log_ << static_cast<char>(c); ofs_log_->flush(); } return *this; }

	Logger& operator << (const char* str)		{ if (ofs_log_) { *ofs_log_ << str; ofs_log_->flush(); } return *this; }
	Logger& operator << (const wchar_t* str)	{ if (ofs_log_) { USES_CONVERSION; *this << W2A(str); } return *this; }

	void WriteFileName(const char* str, int line);

private:
	static std::ofstream* ofs_log_;
};


#define	LOG_FILENAME(log)	log.WriteFileName(__FILE__, __LINE__);


#endif // !defined(AFX_LOGGER_H__D145E4AA_91E5_461F_9499_A50EF6BBA017__INCLUDED_)

/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// JPEGException.h: interface for the JPEGException class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_JPEGEXCEPTION_H__2660834B_8AA1_4E77_A578_0DAA55AED906__INCLUDED_)
#define AFX_JPEGEXCEPTION_H__2660834B_8AA1_4E77_A578_0DAA55AED906__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
struct jpeg_common_struct;


class JPEGException
{
public:
	JPEGException(jpeg_common_struct* cinfo);
	JPEGException(const TCHAR* msg);
	virtual ~JPEGException();

	const TCHAR* GetMessage() const		{ return message_; }
	int GetMessageCode() const			{ return msg_code_; }

private:
	CString message_;
	int msg_code_;
};

#endif // !defined(AFX_JPEGEXCEPTION_H__2660834B_8AA1_4E77_A578_0DAA55AED906__INCLUDED_)

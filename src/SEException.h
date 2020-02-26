/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// SEException.h: interface for the SEException class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SEEXCEPTION_H__F5442794_4FEE_4D42_BB93_F2FDA4E2F3A2__INCLUDED_)
#define AFX_SEEXCEPTION_H__F5442794_4FEE_4D42_BB93_F2FDA4E2F3A2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class SEException
{
public:
	SEException(unsigned int code) : exception_code_(code) {}
	~SEException() {}

    unsigned int exception_code_;

	static void Install();

private:
	static void SETransFunction(unsigned int code, EXCEPTION_POINTERS* exp);
};

#endif // !defined(AFX_SEEXCEPTION_H__F5442794_4FEE_4D42_BB93_F2FDA4E2F3A2__INCLUDED_)

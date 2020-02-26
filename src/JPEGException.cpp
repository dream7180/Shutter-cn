/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// JPEGException.cpp: implementation of the JPEGException class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "JPEGException.h"
#include "jpeglib.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

JPEGException::JPEGException(jpeg_common_struct* cinfo)
{
	char buffer[JMSG_LENGTH_MAX];

	/* Create the message */
	(*cinfo->err->format_message) (cinfo, buffer);

	message_ = buffer;

	jpeg_error_mgr* err= cinfo->err;
	msg_code_ = err->msg_code;
}


JPEGException::JPEGException(const TCHAR* msg) : message_(msg)
{}


JPEGException::~JPEGException()
{}

// JPEGException.cpp: implementation of the CJPEGException class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "JPEGException.h"
extern "C" {
	#include "../jpeglib/jpeglib.h"
}

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CJPEGException::CJPEGException(jpeg_common_struct* cinfo)
{
	char buffer[JMSG_LENGTH_MAX];

	/* Create the message */
	(*cinfo->err->format_message) (cinfo, buffer);

	strcpy(m_strMessage, buffer);
}


CJPEGException::CJPEGException(const char* pcszMsg)
{
	strcpy(m_strMessage, pcszMsg);
}


CJPEGException::~CJPEGException()
{}

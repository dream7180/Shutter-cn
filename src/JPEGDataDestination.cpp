/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// JPEGDataDestination.cpp: implementation of the JPEGDataDestination class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "JPEGDataDestination.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

JPEGDataDestination::JPEGDataDestination()
{
	next_output_byte = 0;
	free_in_buffer = 0;
	init_destination = _InitDestination;
	empty_output_buffer = _EmptyOutputBuffer;
	term_destination = _TermDestination;
}

JPEGDataDestination::~JPEGDataDestination()
{}


void JPEGDataDestination::_InitDestination(j_compress_ptr cinfo)
{ ASSERT(cinfo);	static_cast<JPEGDataDestination*>(cinfo->dest)->InitDestination(); }

boolean JPEGDataDestination::_EmptyOutputBuffer(j_compress_ptr cinfo)
{ ASSERT(cinfo);	return static_cast<JPEGDataDestination*>(cinfo->dest)->EmptyOutputBuffer(); }

void JPEGDataDestination::_TermDestination(j_compress_ptr cinfo)
{ ASSERT(cinfo);	static_cast<JPEGDataDestination*>(cinfo->dest)->TermDestination(); }


void JPEGDataDestination::InitDestination()
{}

void JPEGDataDestination::TermDestination()
{}

void JPEGDataDestination::Abort()
{}

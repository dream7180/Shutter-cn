/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// JPEGDataSource.cpp: implementation of the JPEGDataSource class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "JPEGDataSource.h"
#include "JpegDecoder.h"
#include "ColorProfile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

JPEGDataSource::JPEGDataSource()
{
	next_input_byte = 0;
	bytes_in_buffer = 0;

	init_source = _InitSource;
	fill_input_buffer = _FillInputBuffer;
	skip_input_data = _SkipInputData;
	resync_to_restart = _ResyncToRestart;
	term_source = _TermSource;
}

JPEGDataSource::~JPEGDataSource()
{}


///////////////////////////////////////////////////////////////////////////////

void JPEGDataSource::_InitSource(j_decompress_ptr cinfo)
{ ASSERT(cinfo);	static_cast<JPEGDataSource*>(cinfo->src)->InitSource(); }


boolean JPEGDataSource::_FillInputBuffer(j_decompress_ptr cinfo)
{ ASSERT(cinfo);	return static_cast<JPEGDataSource*>(cinfo->src)->FillInputBuffer(); }


void JPEGDataSource::_SkipInputData(j_decompress_ptr cinfo, long num_bytes)
{
	ASSERT(cinfo);
	if (num_bytes > 0)
		static_cast<JPEGDataSource*>(cinfo->src)->SkipInputData(num_bytes);
}

boolean JPEGDataSource::_ResyncToRestart(j_decompress_ptr cinfo, int desired)
{
	ASSERT(cinfo);
	return static_cast<JPEGDataSource*>(cinfo->src)->ResyncToRestart(
		static_cast<jpeg_decompress_struct*>(cinfo), desired);
}


void JPEGDataSource::_TermSource(j_decompress_ptr cinfo)
{ ASSERT(cinfo);	static_cast<JPEGDataSource*>(cinfo->src)->TermSource(); }

///////////////////////////////////////////////////////////////////////////////

bool JPEGDataSource::ResyncToRestart(jpeg_decompress_struct* decoder, int desired)
{
	return !!jpeg_resync_to_restart(decoder, desired);
}

void JPEGDataSource::InitSource()
{}

void JPEGDataSource::TermSource()
{}

void JPEGDataSource::Abort()
{}

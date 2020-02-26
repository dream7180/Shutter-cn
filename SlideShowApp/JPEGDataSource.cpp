// JPEGDataSource.cpp: implementation of the CJPEGDataSource class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "JPEGDataSource.h"
#include "JpegDecoder.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CJPEGDataSource::CJPEGDataSource()
{
	next_input_byte = 0;
	bytes_in_buffer = 0;

	init_source = _InitSource;
	fill_input_buffer = _FillInputBuffer;
	skip_input_data = _SkipInputData;
	resync_to_restart = _ResyncToRestart;
	term_source = _TermSource;
}

CJPEGDataSource::~CJPEGDataSource()
{}


///////////////////////////////////////////////////////////////////////////////

void CJPEGDataSource::_InitSource(j_decompress_ptr cinfo)
{ ASSERT(cinfo);	static_cast<CJPEGDataSource*>(cinfo->src)->InitSource(); }


boolean CJPEGDataSource::_FillInputBuffer(j_decompress_ptr cinfo)
{ ASSERT(cinfo);	return static_cast<CJPEGDataSource*>(cinfo->src)->FillInputBuffer(); }


void CJPEGDataSource::_SkipInputData(j_decompress_ptr cinfo, long lNumBytes)
{
	ASSERT(cinfo);
	if (lNumBytes > 0)
		static_cast<CJPEGDataSource*>(cinfo->src)->SkipInputData(lNumBytes);
}

boolean CJPEGDataSource::_ResyncToRestart(j_decompress_ptr cinfo, int nDesired)
{ ASSERT(cinfo);	return static_cast<CJPEGDataSource*>(cinfo->src)->ResyncToRestart(static_cast<CJPEGDecoder*>(cinfo), nDesired); }


void CJPEGDataSource::_TermSource(j_decompress_ptr cinfo)
{ ASSERT(cinfo);	static_cast<CJPEGDataSource*>(cinfo->src)->TermSource(); }

///////////////////////////////////////////////////////////////////////////////

bool CJPEGDataSource::ResyncToRestart(CJPEGDecoder* pDecoder, int nDesired)
{
	return !!jpeg_resync_to_restart(pDecoder, nDesired);
}

void CJPEGDataSource::InitSource()
{}

void CJPEGDataSource::TermSource()
{}

void CJPEGDataSource::Abort()
{}

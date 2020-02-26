// JPEGDataSource.h: interface for the CJPEGDataSource class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_JPEGDATASOURCE_H__7619BE7C_B21D_49BE_95C0_5418C4817D10__INCLUDED_)
#define AFX_JPEGDATASOURCE_H__7619BE7C_B21D_49BE_95C0_5418C4817D10__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
extern "C" {
	#include "../jpeglib/jpeglib.h"
}
class CJPEGDecoder;


class CJPEGDataSource : public jpeg_source_mgr
{
public:
	CJPEGDataSource();
	virtual ~CJPEGDataSource();

	virtual bool FillInputBuffer()= 0;
	virtual void SkipInputData(long lNumBytes)= 0;

	virtual void InitSource();
	virtual bool ResyncToRestart(CJPEGDecoder* pDecoder, int nDesired);
	virtual void TermSource();
	virtual void Abort();

private:
	static void _InitSource(j_decompress_ptr cinfo);
	static boolean _FillInputBuffer(j_decompress_ptr cinfo);
	static void _SkipInputData(j_decompress_ptr cinfo, long num_bytes);
	static boolean _ResyncToRestart(j_decompress_ptr cinfo, int desired);
	static void _TermSource(j_decompress_ptr cinfo);
};

#endif // !defined(AFX_JPEGDATASOURCE_H__7619BE7C_B21D_49BE_95C0_5418C4817D10__INCLUDED_)

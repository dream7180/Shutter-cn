// JPEGDecoder.h: interface for the CJpegDecoder class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_JPEGDECODER_H__8305CA8C_F114_4D29_AF9B_3B92CEB163FE__INCLUDED_)
#define AFX_JPEGDECODER_H__8305CA8C_F114_4D29_AF9B_3B92CEB163FE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
extern "C" {
	#include "../jpeglib/jpeglib.h"
}
#include "JPEGDataSource.h"
//#include "AutoPtr.h"
class CDib;


class CJPEGDecoder : public jpeg_decompress_struct
{
public:
	CJPEGDecoder(int nDCTMethod= 0);
	CJPEGDecoder(CJPEGDataSource& src);
	virtual ~CJPEGDecoder();

	virtual bool LinesDecoded(int nLinesReady, bool bFinished);
	int ScanLines() const				{ return output_height; }

	AutoPtr<CDib> Decode(int nScaleDenominator= 1);
	bool Decode(CJPEGDataSource& src, CDib* pBmp, MSize sizeImg= MSize(0, 0), COLORREF rgbBack= RGB(255,255,255));

	// set fast decoding (for 160x120 thumbnail image)
	void SetFast(bool bFastDecoding)	{ m_bFast = bFastDecoding; }

	// select Discrete Cosine Transform implementation
	void SetDCTMethod(int nMethod)		{ m_nDCTMethod = nMethod; }

	MSize GetOriginalSize() const		{ return m_sizeOriginal; }

private:
	void SetParams(MSize& sizeImg, bool bFast, bool bRescaling);
	struct jpeg_error_mgr m_jerr;
	bool m_bFast;
	int m_nDCTMethod;
	MSize m_sizeOriginal;
};

#endif // !defined(AFX_JPEGDECODER_H__8305CA8C_F114_4D29_AF9B_3B92CEB163FE__INCLUDED_)

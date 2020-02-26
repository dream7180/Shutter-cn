// JPEGException.h: interface for the CJPEGException class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_JPEGEXCEPTION_H__2660834B_8AA1_4E77_A578_0DAA55AED906__INCLUDED_)
#define AFX_JPEGEXCEPTION_H__2660834B_8AA1_4E77_A578_0DAA55AED906__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
struct jpeg_common_struct;


class CJPEGException
{
public:
	CJPEGException(jpeg_common_struct* cinfo);
	CJPEGException(const char* pcszMsg);
	virtual ~CJPEGException();

	const TCHAR* GetMessage() const		{ return m_strMessage; }

private:
	TCHAR* m_strMessage;
};

#endif // !defined(AFX_JPEGEXCEPTION_H__2660834B_8AA1_4E77_A578_0DAA55AED906__INCLUDED_)

// DC.h: interface for the MDC class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DC_H__5445E0F5_F2BB_4C56_9594_E88EFE0D4A2F__INCLUDED_)
#define AFX_DC_H__5445E0F5_F2BB_4C56_9594_E88EFE0D4A2F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class MDC
{
public:
	MDC();
	~MDC();

	bool CreateDC(const TCHAR* pcszDriver, LPCTSTR, LPCTSTR, CONST DEVMODE*);
	bool CreateIC(const TCHAR* pcszDriver, LPCTSTR, LPCTSTR, CONST DEVMODE*);

	operator HDC ()		{ return m_hDC; }

	HDC m_hDC;
};

#endif // !defined(AFX_DC_H__5445E0F5_F2BB_4C56_9594_E88EFE0D4A2F__INCLUDED_)

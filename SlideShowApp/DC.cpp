// DC.cpp: implementation of the MDC class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DC.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

MDC::MDC()
{
	m_hDC = 0;
}

MDC::~MDC()
{
	if (m_hDC)
		::DeleteDC(m_hDC);
}


bool MDC::CreateDC(const TCHAR* pcszDriver, LPCTSTR, LPCTSTR, CONST DEVMODE*)
{
	m_hDC = ::CreateDC(pcszDriver, 0, 0, 0);
	return m_hDC != 0;
}


bool MDC::CreateIC(const TCHAR* pcszDriver, LPCTSTR, LPCTSTR, CONST DEVMODE*)
{
	m_hDC = ::CreateIC(pcszDriver, 0, 0, 0);
	return m_hDC != 0;
}

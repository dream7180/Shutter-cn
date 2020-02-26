// PhotoInfo.h: interface for the PhotoInfo class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PHOTOINFO_H__61C8903A_F65E_415A_9E65_88EDC667CF8C__INCLUDED_)
#define AFX_PHOTOINFO_H__61C8903A_F65E_415A_9E65_88EDC667CF8C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

struct CPhotoInfo
{
	DWORD m_dwJPEGSize;
	DWORD m_dwIndex;
	DWORD m_dwReserved1;
	DWORD m_dwReserved2;
	DWORD m_dwReserved3;
	const BYTE m_vJPEGData[4];
};


typedef const CPhotoInfo* PhotoInfoPtr;


#endif // !defined(AFX_PHOTOINFO_H__61C8903A_F65E_415A_9E65_88EDC667CF8C__INCLUDED_)

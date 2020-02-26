// Bitmap.h: interface for the MBitmap class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_BITMAP_H__39134F0E_40DA_4E9D_B32B_7F6B8568FEC3__INCLUDED_)
#define AFX_BITMAP_H__39134F0E_40DA_4E9D_B32B_7F6B8568FEC3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class MBitmap  
{
public:
	MBitmap() : m_hBmp(0) {}
	~MBitmap();


	HBITMAP m_hBmp;

	void Attach(HBITMAP hBmp)	{ m_hBmp = hBmp; }

};

#endif // !defined(AFX_BITMAP_H__39134F0E_40DA_4E9D_B32B_7F6B8568FEC3__INCLUDED_)

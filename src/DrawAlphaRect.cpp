/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"


typedef BOOL (WINAPI ALPHABLEND)(HDC,int, int, int, int,HDC, int, int, int, int, BLENDFUNCTION);

static ALPHABLEND* AlphaBlendFn= (ALPHABLEND*) ::GetProcAddress(::LoadLibrary(_T("msimg32.dll")), "AlphaBlend");
static CBitmap source;


bool DrawAlphaRect(CDC& dc, const CRect& rect, int alpha)
{
	if (AlphaBlendFn == 0)
		return false;

	const int SIZE= 1;

	if (source.m_hObject == 0)
	{
		BYTE data[]= { 110, 210, 60, 50 };

		source.CreateBitmap(SIZE, SIZE, 1, 32, data);
	}
 
	BLENDFUNCTION bf;
	bf.BlendOp = AC_SRC_OVER;
	bf.BlendFlags = 0;
	bf.SourceConstantAlpha = alpha;
	bf.AlphaFormat = 0 /*AC_SRC_ALPHA*/;

	CDC src;
	src.CreateCompatibleDC(&dc);
	src.SelectObject(&source);

	return AlphaBlendFn(dc, rect.left, rect.top, rect.Width(), rect.Height(), src, 0, 0, SIZE, SIZE, bf) != 0;
}

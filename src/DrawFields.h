/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// DrawFields.h: interface for the CDrawFields class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DRAWFIELDS_H__C5EC1CD9_5F6C_4F4E_B810_4A3605F4A998__INCLUDED_)
#define AFX_DRAWFIELDS_H__C5EC1CD9_5F6C_4F4E_B810_4A3605F4A998__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

namespace DrawFields
{
	extern const TCHAR* LABEL;
	extern const TCHAR* VALUE;
	extern const TCHAR* NORMAL;
	extern const TCHAR* BOLD;

	void Draw(CDC& dc, const TCHAR* text, CRect rect, COLORREF rgb_text,
		COLORREF rgb_label, COLORREF rgb_default, bool ellipsis= true);

	void Draw(CDC& dc, const TCHAR* text, CRect rect, COLORREF rgb_text,
		COLORREF rgb_label, COLORREF rgb_default, CFont& normal, CFont& bold, bool ellipsis= true);
};

#endif // !defined(AFX_DRAWFIELDS_H__C5EC1CD9_5F6C_4F4E_B810_4A3605F4A998__INCLUDED_)

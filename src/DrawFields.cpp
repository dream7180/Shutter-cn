/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// DrawFields.cpp: implementation of the CDrawFields class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DrawFields.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

namespace DrawFields
{

const TCHAR* LABEL=  _T("\x01");	// should be wchars, but that confuses oStringstream
const TCHAR* VALUE=  _T("\x02");	// due to the wchar_t being unsigned short rather than distinct type
const TCHAR* NORMAL= _T("\x03");	//TODO: consider defining wchar_t as distinct type
const TCHAR* BOLD=   _T("\x04");


void DrawFields(CDC& dc, const TCHAR* text, CRect rect, COLORREF rgb_text,
				COLORREF rgb_label, COLORREF rgb_default, CFont* normal, CFont* bold, bool ellipsis)
{
	if (text == 0 || *text == 0)
		return;

	COLORREF rgb_color= rgb_default;
	const TCHAR* start= text;
	CFont* font= 0;

	dc.SetTextColor(rgb_color);
	if (normal)
		dc.SelectObject(normal);

	for (const TCHAR* text_ptr= text; ; ++text_ptr)
	{
		if (*text_ptr == *LABEL)		// label follows?
		{
			rgb_color = rgb_label;
			font = normal;
		}
		else if (*text_ptr == *VALUE)	// text follows?
		{
			rgb_color = rgb_text;
			font = normal;
		}
		else if (*text_ptr == *NORMAL)
		{
			font = normal;
		}
		else if (*text_ptr == *BOLD)
		{
			font = bold;
		}
		else if (*text_ptr != 0)		// not EOS?
			continue;

		if (text_ptr > start)
		{
			int length= static_cast<int>(text_ptr - start);
			bool tab= false;

			if (start[length - 1] == '\x9')	// tab?
				length--, tab = true;

			//TODO: GetTabbedTextExtent
			int width= dc.GetTextExtent(start, length).cx;
			bool last_one= width >= rect.Width();

			if (tab && !last_one)
			{
				int space= dc.GetTextExtent(_T(" "), 1).cx;
				width += 5 * space;
				last_one = width >= rect.Width();
			}

			UINT format= DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX;
			if (last_one && ellipsis)
				format |= DT_END_ELLIPSIS;

			dc.DrawText(start, length, rect, format);

			if (last_one)
				break;

			rect.left += width;
		}

		dc.SetTextColor(rgb_color);
		if (font)
			dc.SelectObject(font);

		if (*text_ptr == 0)
			break;

		start = text_ptr + 1;
	}
}


void Draw(CDC& dc, const TCHAR* text, CRect rect, COLORREF rgb_text,
		COLORREF rgb_label, COLORREF rgb_default, CFont& normal, CFont& bold, bool ellipsis/*= true*/)
{
	DrawFields(dc, text, rect, rgb_text, rgb_label, rgb_default, &normal, &bold, ellipsis);
}


void Draw(CDC& dc, const TCHAR* text, CRect rect, COLORREF rgb_text,
		   COLORREF rgb_label, COLORREF rgb_default, bool ellipsis/*= true*/)
{
	DrawFields(dc, text, rect, rgb_text, rgb_label, rgb_default, 0, 0, ellipsis);
}

}	// namespace

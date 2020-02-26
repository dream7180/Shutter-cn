/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "StdAfx.h"
#include "GetDefaultGuiFont.h"


HFONT GetDefaultGuiHFont()
{
	auto& font = GetDefaultGuiFont();
	return font;
}


CFont& GetDefaultGuiFont()
{
	static CFont default_font;

	if (default_font.m_hObject == nullptr)
	{
		// create default UI font
		HFONT font = static_cast<HFONT>(::GetStockObject(DEFAULT_GUI_FONT));
		LOGFONT lf;
		::GetObject(font, sizeof(lf), &lf);
		lf.lfWeight = FW_NORMAL;
		lf.lfHeight += 1;
		_tcscpy(lf.lfFaceName, _T("Tahoma"));
		default_font.CreateFontIndirect(&lf);
	}

	return default_font;
}


CFont& GetDefaultGuiBoldFont()
{
	static CFont default_bold_font;

	if (default_bold_font.m_hObject == nullptr)
	{
		// create default UI bold font
		HFONT font = static_cast<HFONT>(::GetStockObject(DEFAULT_GUI_FONT));
		LOGFONT lf;
		::GetObject(font, sizeof(lf), &lf);
		lf.lfWeight = FW_NORMAL;
		lf.lfHeight += 1;
		_tcscpy(lf.lfFaceName, _T("Tahoma"));
		default_bold_font.CreateFontIndirect(&lf);
	}

	return default_bold_font;
}


void GetDefaultGuiFont(LOGFONT& lf)
{
	auto& font = GetDefaultGuiFont();
	font.GetLogFont(&lf);
}


void GetDefaultGuiBoldFont(LOGFONT& lf)
{
	auto& font = GetDefaultGuiBoldFont();
	font.GetLogFont(&lf);
}

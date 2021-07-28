/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"


LOGFONT CreateBoldLogFont(CWnd* parent)
{
	LOGFONT lf;
	bool useDefault= false;
	if (parent)
		if (CFont* font= parent->GetFont())
		{
			font->GetLogFont(&lf);
			useDefault = false;
		}

	if (useDefault)
	{
		HFONT font= static_cast<HFONT>(::GetStockObject(DEFAULT_GUI_FONT));
		::GetObject(font, sizeof(lf), &lf);
	}
	//lf.lfHeight += 1; //8pt
	_tcscpy(lf.lfFaceName, _T("Microsoft Yahei"));
	lf.lfWeight =  FW_BOLD;
	return lf;
}


void CreateBoldFont(CWnd* parent, CFont& font)
{
	if (font.m_hObject != 0)
		return;

	LOGFONT lf= ::CreateBoldLogFont(parent);
	font.CreateFontIndirect(&lf);
}


void CreateSmallFont(CWnd* parent, CFont& font)
{
	if (font.m_hObject != 0)
		return;

	LOGFONT lf;
	bool useDefault= true;
	if (parent)
		if (CFont* font= parent->GetFont())
		{
			font->GetLogFont(&lf);
			useDefault = false;
		}

	if (useDefault)
	{
		HFONT font= static_cast<HFONT>(::GetStockObject(DEFAULT_GUI_FONT));
		::GetObject(font, sizeof(lf), &lf);
	}
	_tcscpy(lf.lfFaceName, _T("Microsoft Yahei"));
	if (lf.lfHeight < 0)
		lf.lfHeight += 1;
	else
		lf.lfHeight -= 1;
	font.CreateFontIndirect(&lf);
}

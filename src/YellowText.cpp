/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// YellowText.cpp: implementation of the YellowText class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Dib.h"
#include "YellowText.h"
#include "Config.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

YellowText::YellowText()
{
//	font_.CreateFont(-40, 0, 0, 0, 400, true, false, false, DEFAULT_CHARSET,
//		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH, "Verdana");
	empty_text_ = true;
	draw_text_format_ = DT_CENTER | DT_TOP | DT_EXPANDTABS | DT_NOPREFIX;

	CDC dc;
	dc.CreateIC(_T("DISPLAY"), NULL, NULL, NULL);
	mem_dc_.CreateCompatibleDC(&dc);
//	mem_dc_.SelectObject(&font_);

	ResetFont();

	old_bmp_ = 0;

	rgb_text_ = RGB(255, 255, 255);
	rgb_back_ = RGB(0,0,0);
}


YellowText::~YellowText()
{
	// destroy dc before bitmap selected into it
	mem_dc_.DeleteDC();
}


void YellowText::Prepare(CWnd* wnd)
{
	if (old_bmp_)
	{
		::SelectObject(mem_dc_, old_bmp_);
		old_bmp_ = 0;
		buff_bmp_.DeleteObject();
	}

	if (empty_text_ || wnd == 0)
		return;

	// re-scale text to the size of window
	{
		CRect rect(0,0,0,0);
		wnd->GetClientRect(rect);
		int w= ::GetSystemMetrics(SM_CXFULLSCREEN);
		int h= ::GetSystemMetrics(SM_CYFULLSCREEN);
		double dw= rect.Width() / double(w);
		double dh= rect.Height() / double (h);
		double scale= std::min(dw, dh);
		ASSERT(log_font_.lfHeight < 0);
		if (scale < 1.0 && scale > 0.0)
			scale = pow(scale, 0.6);	// non-linear scaling down: slower than linear
		int font_height= static_cast<int>(scale * log_font_.lfHeight - 0.5);
		if (font_height > -14)
			font_height = -14;
		ResetFont(log_font_, font_height);
	}

	CRect client_rect;
	wnd->GetClientRect(client_rect);
	CRect rect= client_rect;
	CClientDC dc(wnd);
	CFont* old_font= dc.SelectObject(&font_);
#ifdef _UNICODE
	::DrawTextW(dc, text_.c_str(), static_cast<int>(text_.size()), rect, draw_text_format_ | DT_CALCRECT);
#else
	{
		USES_CONVERSION;
		const char* text= W2A(text_.c_str());
		::DrawTextA(dc, text, strlen(text), rect, draw_text_format_ | DT_CALCRECT);
	}
#endif
	dc.SelectObject(old_font);

	text_size_ = rect.Size() + CSize(4, 2);
	buff_bmp_.CreateCompatibleBitmap(&dc, text_size_.cx, text_size_.cy);

	pos_.x = (client_rect.Width() - text_size_.cx) / 2;
	pos_.y = MAX(client_rect.Height() - text_size_.cy, 0);

	if (CBitmap* old_bmp= mem_dc_.SelectObject(&buff_bmp_))
		old_bmp_ = *old_bmp;

	mem_dc_.SetWindowOrg(pos_);

	mem_dc_.FillSolidRect(CRect(pos_, text_size_), rgb_back_);
}


void YellowText::Prepare(CWnd* wnd, const std::wstring& text)
{
	empty_text_ = text.empty();

	if (!empty_text_)
	{
		// check if there is something other then spaces and nuls
		//
		if (text.find_first_not_of(L' ') == std::wstring::npos &&
			text.find_first_not_of(L'\0') == std::wstring::npos)
			empty_text_ = true;
	}

	//TODO: replace single '\n' and '\r' chars by EOL "\r\n"
	text_ = text;

	Prepare(wnd);
}


void YellowText::Empty()
{
	text_.clear();
	Prepare(0);
}


void YellowText::SetFont(const LOGFONT& lf)
{
	log_font_ = lf;
}


void YellowText::ResetFont()
{
	log_font_ = g_Settings.description_font_;
	if (log_font_.lfHeight > 0)
		log_font_.lfHeight = -log_font_.lfHeight;
	ResetFont(log_font_, 0);
}


void YellowText::ResetFont(const LOGFONT& lf, int height)
{
	mem_dc_.SelectStockObject(SYSTEM_FONT);
	font_.DeleteObject();
	LOGFONT logfnt= lf;
	if (height)
		logfnt.lfHeight = height;
	font_.CreateFontIndirect(&logfnt);
	mem_dc_.SelectObject(&font_);
}


void YellowText::PrintText(CDC& dc)
{
	if (empty_text_)
		return;

	// draw black anti-aliased text outline
	//
	mem_dc_.SetBkMode(TRANSPARENT);
	mem_dc_.SetBkColor(RGB(255,255,255));
	mem_dc_.SetTextColor(RGB(0,0,0));

	static const CSize shift[]=
	{
		CSize(0, 1), CSize(0, -1), CSize(1, 0), CSize(-1, 0),
		CSize(1, 1), CSize(-1, 1), CSize(1, -1), CSize(-1, -1)
	};

#ifdef _UNICODE
	if (const wchar_t* text= text_.c_str())
	{
		int size= static_cast<int>(text_.size());
#else
	USES_CONVERSION;
	if (const char* text= W2A(text_.c_str()))
	{
		int size= strlen(text);
#endif

		CRect rect(pos_, text_size_);
		for (int i= 0; i < array_count(shift); ++i)
			::DrawText(mem_dc_, text, size, rect + shift[i], draw_text_format_);

		// draw text itself
		//
		mem_dc_.SetBkColor(rgb_back_);
		mem_dc_.SetTextColor(rgb_text_);
		::DrawText(mem_dc_, text, size, rect, draw_text_format_);
	}

	// blt to screen
	dc.BitBlt(pos_.x, pos_.y, text_size_.cx, text_size_.cy, &mem_dc_, pos_.x, pos_.y, SRCCOPY);
}


void YellowText::SetFont(const char* font_name, int height)
{
}

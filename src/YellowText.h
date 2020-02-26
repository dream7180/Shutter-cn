/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// YellowText.h: interface for the YellowText class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_YELLOWTEXT_H__35ED13A4_60C4_44F7_87DD_F8AD1EEC2A5E__INCLUDED_)
#define AFX_YELLOWTEXT_H__35ED13A4_60C4_44F7_87DD_F8AD1EEC2A5E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class YellowText
{
public:
	YellowText();
	virtual ~YellowText();

	void ResetFont();

	void SetFont(const LOGFONT& lf);

	void Empty();

	void Prepare(CWnd* wnd, const std::wstring& text);
	void Prepare(CWnd* wnd);

	CDC* GetDC()	{ return &mem_dc_; }

	void PrintText(CDC& dc);
//	void PrintText(CWnd* wnd);

	void SetFont(const char* font_name, int height);

	void SetBackgndColor(COLORREF rgb_back)  { rgb_back_ = rgb_back; }
	void SetTextColor(COLORREF rgb_text)		{ rgb_text_ = rgb_text; }

private:
	void ResetFont(const LOGFONT& lf, int height);

	LOGFONT	 log_font_;
	CFont	 font_;
	COLORREF rgb_text_;
	COLORREF rgb_back_;
	CDC		 mem_dc_;
	CBitmap	 buff_bmp_;
	CPoint	 pos_;
	CSize	 text_size_;
	bool	 empty_text_;
	std::wstring	 text_;
	UINT	 draw_text_format_;
	HBITMAP	 old_bmp_;
};

#endif // !defined(AFX_YELLOWTEXT_H__35ED13A4_60C4_44F7_87DD_F8AD1EEC2A5E__INCLUDED_)

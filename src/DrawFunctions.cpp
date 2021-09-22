/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "PhotoInfo.h"
#include "ImageDraw.h"
#include "DrawFunctions.h"
#include "PNGImage.h"
#include "resource.h"
#include "BoldFont.h"
#include "UIElements.h"
#include "GetDefaultGuiFont.h"

extern std::auto_ptr<Gdiplus::Bitmap> LoadPng(int rsrc_id, const wchar_t* rsrc_type, HMODULE instance);


void DrawPhoto(CDC& dc, CRect rect, PhotoInfoPtr photo)
{
	photo->Draw(&dc, rect, RGB(50,50,50), 0, 0, 0, 0, /*ImageDraw::DRAW_DIBDRAW |*/ ImageDraw::NO_RECT_RESCALING | ImageDraw::DRAW_HALFTONE | ImageDraw::DRAW_BACKGND | ImageDraw::FLIP_SIZE_IF_NEEDED);
}


void DrawPhotoTags(CDC& dc, const CRect& rect, const PhotoTags& tags, int rating, COLORREF text, COLORREF backgnd)
{
	if (tags.empty() && rating == 0)
		return;

	CRect img_rect= rect;

	const int MARGIN= 0;
	int tag_w= MARGIN + img_rect.Width();// * 2 / 3;
	
	CFont font;
	::CreateBoldFont(0, font);
	dc.SelectObject(&font);

	//LOGFONT lf;//= logfont;
	//HFONT hfont= static_cast<HFONT>(::GetStockObject(DEFAULT_GUI_FONT));
	//::GetObject(hfont, sizeof(lf), &lf);
	//lf.lfWeight = FW_BOLD;
	//lf.lfQuality = ANTIALIASED_QUALITY;
	//_tcscpy(lf.lfFaceName, _T("Segoe UI"));
	//CFont font;
	//font.CreateFontIndirect(&lf);

	//CFont* old_font= dc.SelectObject(&font);
	CSize text_size= dc.GetTextExtent(_T("X"), 1);	// TODO: Msize here

	dc.SetTextColor(text);
	dc.SelectStockObject(NULL_PEN);
	dc.SetBkColor(backgnd);
	CBrush br(backgnd);
	CBrush* old_brush= dc.SelectObject(&br);

	//int tag_h= min<int>(text_size.cy * tags_count + MARGIN, img_rect.Height());

	CPoint tags_pt(img_rect.right - tag_w, img_rect.top);

	tags_pt.x += MARGIN;
	tag_w -= MARGIN;

	CRect text_rect(tags_pt, CSize(tag_w, text_size.cy));

	const int tag_backgnd_alpha= 200;

	// draw stars ----------------------------------------------------------------------

	if (rating > 0 && rating <= 5)	// draw stars for ratings > 0
	{
		CFont wingdings;
		//LOGFONT lf= logfont;
//		ctrl.default_fnt_.GetLogFont(&lf);
		LOGFONT lf;
		HFONT hfont = static_cast<HFONT>(::GetStockObject(DEFAULT_GUI_FONT));
		::GetObject(hfont, sizeof(lf), &lf);
		_tcscpy(lf.lfFaceName, _T("WingDings"));
		lf.lfCharSet = SYMBOL_CHARSET;
		if (lf.lfHeight < 0)
			lf.lfHeight -= 4; // larger stars
		else
			lf.lfHeight += 4;
		lf.lfPitchAndFamily = DEFAULT_PITCH;
		lf.lfWeight = FW_DONTCARE;
		wingdings.CreateFontIndirect(&lf);
		TCHAR stars[]=
#ifdef _UNICODE
		{ 0xf0ab, 0xf0ab, 0xf0ab, 0xf0ab, 0xf0ab, 0 };	// stars
#else
		{ 0xab, 0xab, 0xab, 0xab, 0xab, 0 };	// stars
#endif
		int len= rating;
		stars[len] = 0;
		int h= dc.GetTextExtent(_T("X"), 1).cy;
		CFont* old= dc.SelectObject(&wingdings);
		int shift= -1;
		int round= 0;
		CSize size(0, 0);
		int tag_width= 0;
		int right_margin= 2;
		for (;;)
		{
			dc.SetTextCharacterExtra(shift);
			size = dc.GetTextExtent(stars, len);
			size.cy = h + 1;
			round = size.cy;
			tag_width = size.cx + round / 2 + (shift < -1 ? 3 : 1);
			if (img_rect.Width() >= tag_width)
				break;
			// shrink stars (overlap them a bit) if they don't fit
			shift--;
			right_margin = 1;
			if (shift < -6)
				break;
		}
		int width= std::min<long>(img_rect.Width(), tag_width);
		int xPos= text_rect.right - width;
		::DrawTagBackground(dc, CRect(xPos, text_rect.top, text_rect.right, text_rect.top + size.cy - 1), backgnd, tag_backgnd_alpha);
		CRect rect(xPos + round / 2, text_rect.top, text_rect.right, text_rect.top + size.cy - 1);
		rect.right -= right_margin;
		rect.left -= 4;
		dc.SetBkMode(TRANSPARENT);
		dc.DrawText(stars, len, rect, DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
		dc.SelectObject(old);
		dc.SetTextCharacterExtra(0);
		text_rect.OffsetRect(0, size.cy);
	}

	dc.SetBkMode(OPAQUE);

	// draw tags ------------------------------------------------------------

	dc.SetBkColor(backgnd);
	dc.SetBkMode(OPAQUE);
	//dc.SelectObject(&font);
	{
		const size_t tagsCount= tags.size();
		for (size_t i= 0; i < tagsCount; ++i)
		{
			if (text_rect.bottom > img_rect.bottom)	// no partial tags...
				break;

			const String& str= tags[i];
			CSize size= dc.GetTextExtent(str.c_str(), static_cast<int>(str.length()));
			size.cy++;
			int round= size.cy;
			int width= std::min<int>(text_rect.Width(), size.cx + round / 2 + 3);
			int x_pos= text_rect.right - width;
			::DrawTagBackground(dc, CRect(x_pos, text_rect.top, text_rect.right, text_rect.top + size.cy - 1), backgnd, tag_backgnd_alpha);
			CRect rect(x_pos + round / 2, text_rect.top, text_rect.right, text_rect.top + size.cy - 1);
			rect.right -= 3;
			dc.DrawText(str.c_str(), static_cast<int>(str.size()), rect, DT_RIGHT | DT_VCENTER | DT_SINGLELINE | DT_EXPANDTABS | DT_END_ELLIPSIS);
			text_rect.OffsetRect(0, size.cy);
		}

	}

	dc.SelectObject(old_brush);
	//dc.SelectObject(old_font);
}



void DrawNoExifIndicator(CDC& dc, const CRect& rect)
{
	static CImageList noExif;
	if (noExif.m_hImageList == 0)
		VERIFY(PNGImage().LoadImageList(IDB_NO_EXIF, 8, RGB(255,0,0), noExif));

	CPoint pos= rect.BottomRight();
	pos.Offset(-9, -9);
	if (pos.x < rect.left)
		pos.x = rect.left;
	if (pos.y < rect.top)
		pos.y = rect.top;

	noExif.Draw(&dc, 0, pos, ILD_TRANSPARENT);
}

void DrawPaneIndicator(CDC& dc, const CRect& rect, COLORREF text_color, COLORREF back_color, size_t id)
{
	dc.SetTextColor(text_color);
	LOGFONT lf;
	::GetDefaultGuiBoldFont(lf);
	CFont _font;
	_font.CreateFontIndirect(&lf);
	int size= sizeof(&_font)*2 + 4;
	CRect r(CPoint(rect.right - size, rect.bottom - size), CSize(size, size));
	dc.FillSolidRect(r,back_color);
	dc.SetBkMode(TRANSPARENT);
	TCHAR buf[128];
	_itot(static_cast<int>(id), buf, 10);
	dc.DrawText(buf, static_cast<int>(_tcslen(buf)), r, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	dc.SelectObject(&_font);
}


void DrawLightTableIndicator(CDC& dc, const CRect& rect, COLORREF color)
{
	static std::auto_ptr<Gdiplus::Bitmap> light_bulb;

	if (light_bulb.get() == 0)
		light_bulb = LoadPng(IDB_LIGHT_BULB, L"PNG", AfxGetResourceHandle());

    UINT w= light_bulb->GetWidth();
    UINT h= light_bulb->GetHeight();

	Gdiplus::Graphics g(dc);
	Gdiplus::Rect r(rect.left - 3, rect.bottom - h + 3, w, h);
	g.DrawImage(light_bulb.get(), r);

/*
//	dc.SetBkColor(back_color);
	int size= 10;

	CRect r(CPoint(rect.left - 2, rect.bottom - size + 1), CSize(size, size));

//	for (int i= 0; i < 2; ++i)
	{
//		COLORREF c= i == 0 ? RGB(0,0,0) : color;
		int half= size / 2;
		for (int y= 0; y <= size; ++y)
		{
			int x= 0;
			int cx= 0;
			if (y <= half)
			{
				x = r.left + half - y;
				cx = 1 + 2 * y;
			}
			else
			{
				x = r.left + y - half;
				cx = 1 + 2 * (size - y);
			}

			dc.FillSolidRect(x, r.top + y, cx, 1, color);
			dc.FillSolidRect(x + cx, r.top + y, 1, 1, RGB(0,0,0));
		}
//		r.OffsetRect(-1, -1);
	}
	//dc.FillSolidRect(r, RGB(0,0,0));
	//r.OffsetRect(-1, -1);
	//dc.FillSolidRect(r, color);
*/
}

/*
void DrawPhotoTagsAndStars(CDC& dc, const CRect& rect, const PhotoTags& tags, int rating, COLORREF text, COLORREF backgnd)
{
	if (tags.empty() && rating == 0)
		return;

	// tags & stars

	CRect img_rect= ctrl.image_rect_ + rect.TopLeft();

	const int MARGIN= 3;
	int tag_w= MARGIN;

	tag_w += img_rect.Width() * 2 / 3;

	// in preview mode draw tag on the photo
	if (preview_mode)
	{
		const int PROTRUDE= ctrl.M_size_size_.cx / 2;
		CRect rect(photo_rect.left, photo_rect.top - PROTRUDE, photo_rect.right + PROTRUDE + 1, img_rect.bottom);
		img_rect = rect & img_rect;
	}

	dc.SelectObject(&ctrl.tags_fnt_);

	CSize text_size= dc.GetTextExtent(_T("X"), 1);	// TODO: Msize here

	const int tags_count= photo_->GetTags().size();

	//int tag_h= min<long>(text_size.cy * tags_count + MARGIN, img_rect.Height());

	CPoint tags(img_rect.right - tag_w, img_rect.top);

	CBrush br(ctrl.rgb_tag_bkgnd_);
	CBrush* old= dc.SelectObject(&br);
	dc.SelectStockObject(NULL_PEN);
	//dc.FillSolidRect(tags.x, tags.y, tag_w, tag_h, ctrl.rgb_tag_bkgnd_);

	tags.x += MARGIN;
	tag_w -= MARGIN;

	dc.SetTextColor(ctrl.rgb_tag_text_);
	dc.SetBkColor(ctrl.rgb_tag_bkgnd_);

	//		dc.SetBkMode(OPAQUE);

	CRect text_rect(tags, CSize(tag_w - 2, text_size.cy));
	text_rect.top++;

	// draw stars
	int rating= photo_->GetRating();	// draw stars for ratings > 0
	if (rating > 0 && rating <= 5)
	{
		CFont wingdings;
		LOGFONT lf;
		ctrl.default_fnt_.GetLogFont(&lf);
		_tcscpy(lf.lfFaceName, _T("WingDings"));
		lf.lfCharSet = SYMBOL_CHARSET;
		lf.lfHeight -= 4; // larger stars
		lf.lfPitchAndFamily = DEFAULT_PITCH;
		lf.lfWeight = FW_DONTCARE;
		wingdings.CreateFontIndirect(&lf);
		TCHAR stars[]=
#ifdef _UNICODE
		{ 0xf0ab, 0xf0ab, 0xf0ab, 0xf0ab, 0xf0ab, 0 };	// stars
#else
		{ 0xab, 0xab, 0xab, 0xab, 0xab, 0 };	// stars
#endif
		int len= rating;
		stars[len] = 0;
		int h= dc.GetTextExtent(_T("X"), 1).cy;
		CFont* old= dc.SelectObject(&wingdings);
		int shift= -1;
		int round= 0;
		CSize size(0, 0);
		int tag_width= 0;
		int right_margin= 2;
		for (;;)
		{
			dc.SetTextCharacterExtra(shift);
			size = dc.GetTextExtent(stars, len);
			size.cy = h + 1;
			round = size.cy;
			tag_width = size.cx + round / 2 + (shift < -1 ? 3 : 1);
			if (img_rect.Width() >= tag_width)
				break;
			// shrink stars (overlap them a bit) if they don't fit
			shift--;
			right_margin = 1;
			if (shift < -6)
				break;
		}
		int width= min<long>(img_rect.Width(), tag_width);
		int xPos= text_rect.right - width;
		dc.RoundRect(xPos, text_rect.top, xPos + size.cy, text_rect.top + size.cy, round, size.cy);
		CRect rect(xPos + round / 2, text_rect.top, text_rect.right, text_rect.top + size.cy - 1);
		dc.FillSolidRect(rect, ctrl.rgb_tag_bkgnd_);
		rect.right -= right_margin;
		rect.left -= 4;
		dc.SetBkMode(TRANSPARENT);
		dc.DrawText(stars, len, rect, DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
		dc.SelectObject(old);
		dc.SetTextCharacterExtra(0);
		text_rect.OffsetRect(0, size.cy);
	}

	dc.SetBkMode(OPAQUE);

	// draw tags
	for (int i= 0; i < tags_count; ++i)
	{
		bool fit= text_rect.bottom < photo_rect.bottom;
		const String& str= photo_->GetTags()[i];
		const TCHAR* text= fit ? str.c_str() : _T("...");
		size_t length= fit ? str.size() : _tcslen(text);
		CSize size= dc.GetTextExtent(text, length);
		size.cy++;
		int round= size.cy;// * 3 / 4;
		int width= min<long>(text_rect.Width(), size.cx + round / 2 + 3);
		int x_pos= text_rect.right - width;
		dc.RoundRect(x_pos, text_rect.top, x_pos + size.cy, text_rect.top + size.cy, round, size.cy);
		CRect rect(x_pos + round / 2, text_rect.top, text_rect.right, text_rect.top + size.cy - 1);
		dc.FillSolidRect(rect, ctrl.rgb_tag_bkgnd_);
		rect.right -= 3;
		dc.DrawText(text, length, rect, DT_RIGHT | DT_VCENTER | DT_SINGLELINE | DT_EXPANDTABS | DT_END_ELLIPSIS);
		text_rect.OffsetRect(0, size.cy);
		if (!fit)
			break;
	}

	dc.SelectObject(old);
}
*/

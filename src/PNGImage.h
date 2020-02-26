/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// PNGImage.h: Helper methods to load PNG image form resources and
// return it as a DIB or image list object.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PNGIMAGE_H__500D514D_C788_4A45_AE82_AFA81AFE25EE__INCLUDED_)
#define AFX_PNGIMAGE_H__500D514D_C788_4A45_AE82_AFA81AFE25EE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
class Dib;

class PNGImage
{
public:
	PNGImage();
	~PNGImage();

	// blend images with background color instead of preserving transparency
	void AlphaToColor(COLORREF rgb_backgnd)		{ rgb_backgnd_ = rgb_backgnd; }

	bool Load(int resource_id, CBitmap& bmp, BITMAPINFOHEADER* bih= 0, float saturation= 0.0f, float lightness= 0.0f, float alpha= 1.0f);
	bool Load(LPCTSTR resource_id, CBitmap& bmp, BITMAPINFOHEADER* bih= 0, float saturation= 0.0f, float lightness= 0.0f, float alpha= 1.0f);
	bool Load(int resource_id, Dib& dib, BITMAPINFOHEADER* bih = 0, float saturation = 0.0f, float lightness = 0.0f, float alpha = 1.0f);
	bool Load(LPCTSTR resource_id, Dib& dib, BITMAPINFOHEADER* bmpih, float saturation, float lightness, float alpha);

	bool LoadMask(int resource_id, CBitmap& bmp);
	bool LoadMask(LPCTSTR resource_id, CBitmap& bmp);

	bool LoadImageList(int bmp_id, int single_bmp_width, COLORREF rgb_backgnd, CImageList& image_list);

	bool LoadImageList(int bmp_id, int single_bmp_width, COLORREF rgb_backgnd, CImageList& image_list,
		float saturation, float lightness, float alpha, bool has_transparency);

	// load PNG image and create ImageList control with 'img_count' items;
	// backgnd_color is used when transparency is not requested (has_transparency == false)
	// saturation and lightness (if != 0.0f) are used to modify pixel colors
	bool LoadImageList(int bmp_id, COLORREF backgnd_color, CImageList& image_list, float saturation, float lightness, bool has_transparency, int img_count);

	//	const TCHAR* const RES_TYPE_PNG= _T("PNG");

	BYTE* data_;
	BYTE* end_;
private:
	COLORREF rgb_backgnd_;
};


extern std::auto_ptr<Gdiplus::Bitmap> LoadPng(int rsrc_id, const wchar_t* rsrc_type, HMODULE instance);

#endif // !defined(AFX_PNGIMAGE_H__500D514D_C788_4A45_AE82_AFA81AFE25EE__INCLUDED_)

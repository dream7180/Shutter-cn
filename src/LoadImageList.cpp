/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "PNGImage.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


// those two functions are supposed to load a bitmap and assign it to the image list object
// taking care of mask creation (if needed)


bool LoadImageList(CImageList& il, UINT id, int img_width)
{
	ASSERT(il.m_hImageList == 0);

	HINSTANCE inst= AfxFindResourceHandle(MAKEINTRESOURCE(id), RT_BITMAP);

	if (HIMAGELIST img_list= ImageList_LoadImage(inst, MAKEINTRESOURCE(id), img_width, 0, RGB(255,0,255), IMAGE_BITMAP, LR_CREATEDIBSECTION))
	{
		il.Attach(img_list);
		return true;
	}
	return false;
}


// this version attempts to find PNG image first and regular BMP in the absence of the former

bool LoadImageList(CImageList& il, UINT id, int img_width, COLORREF rgb_backgnd, bool has_transparency)
{
	ASSERT(il.m_hImageList == 0);

	if (PNGImage().LoadImageList(id, img_width, rgb_backgnd, il, 0.0f, 0.0f, 1.0f, has_transparency))
		return true;

	return ::LoadImageList(il, id, img_width);
}


bool LoadPngImageList(CImageList& il, UINT id, COLORREF backgnd_color, bool has_transparency, int img_count)
{
	ASSERT(il.m_hImageList == 0);

	return PNGImage().LoadImageList(id, backgnd_color, il, 0.0f, 0.0f, has_transparency, img_count);
}

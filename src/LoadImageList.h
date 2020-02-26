/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once

bool LoadImageList(CImageList& il, UINT uId, int img_width);
bool LoadImageList(CImageList& il, UINT uId, int img_width, COLORREF rgb_backgnd, bool has_transparency= true);

// load PNG image and create ImageList control with 'img_count' items;
// backgnd_color is used when transparency is not requested (has_transparency == false)
bool LoadPngImageList(CImageList& il, UINT id, COLORREF backgnd_color, bool has_transparency, int img_count);

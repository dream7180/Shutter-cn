/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once

#include "PhotoInfoPtr.h"
class PhotoTags;

extern void DrawPhoto(CDC& dc, CRect rect, PhotoInfoPtr photo);
extern void DrawPhotoTags(CDC& dc, const CRect& rect, const PhotoTags& tags, int rating, COLORREF text, COLORREF backgnd);
extern void DrawNoExifIndicator(CDC& dc, const CRect& rect);
extern void DrawPaneIndicator(CDC& dc, const CRect& rect, COLORREF text_color, COLORREF back_color, size_t id);
extern void DrawLightTableIndicator(CDC& dc, const CRect& rect, COLORREF color);

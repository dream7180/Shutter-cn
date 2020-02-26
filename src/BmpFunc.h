/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "Dib.h"
#include "PhotoInfo.h"
#include "AutoPtr.h"

int CopyBitmap(Dib* src_bmp, CBitmap& dest_bmp, PhotoInfo::ImgOrientation rotation= PhotoInfo::ORIENT_NORMAL);

AutoPtr<Dib> ApplyGamma(Dib* bmp, double gamma);

void ApplyGammaInPlace(Dib* bmp, double gamma, int line_from, int line_to);

// low level rotation routine that expects matching (already rotated) dest bitmap; it'll fill it with pixels form src
void Rotate(const Dib* src, Dib* dest, bool clockwise, int lines_from, int lines_to);
bool Rotate180(const Dib* bmp, Dib* copy, int lines_from, int lines_to);

void MagnifyBitmap(Dib& dibSrc, double zoom_x, double zoom_y, Dib& dibDest, bool cubic);

void MagnifyBitmap(Dib& dibSrc, CSize dest_size, Dib& dibDest, bool cubic);

// given image size, try to uniformly (preserving aspect ratio) resize it to fill 'dest' area;
// filling means that entire 'dest' area shall be covered with image, at the expense of cropping the image
CRect SizeToFill(CSize image, CSize dest, bool magnify_if_needed);

// given image size, try to uniformly (preserving aspect ratio) resize it to fit in 'dest' area;
// fitting means that entire image will be visible (no cropping), but parts of 'dest' area may not be covered by image
// 'tolerance' is used to avoid reporting different sizes, when aspect ratio are close
CSize SizeToFit(CSize image, CSize dest, double tolerance= 0.02);

// pass decoded image and rotation_flag_ from the PhotoInfo to create appropriately rotated copy image
AutoPtr<Dib> RotateBitmap(Dib& img, int rotation_flag);

// flip bitmap (in-place)
void FlipBitmap(Dib& bmp, bool horizontally, bool vertically);

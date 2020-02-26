/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once

enum DibDispMethod
{
	DIB_DRAW_DIB= 0,	// use ::DrawDibDraw()
	DIB_FAST_DRAW,		// COLORONCOLOR
	DIB_SMOOTH_DRAW,	// HALFTONE
	DIB_DIRECT_2D		// GPU-based
};

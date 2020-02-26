/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once


enum FileTypeIndex
{
	FT_JPEG,
	FT_PSD,
	FT_TIFF,
	FT_PNG,
	FT_CRW,
	FT_NEF,
	FT_ORF,
	FT_DNG,
	FT_RAF,
	FT_GIF,
	FT_BMP,
	FT_PEF,
	FT_ARW,
	FT_RW2,
	FT_X3F,
	FT_SRW,
	//new type to be inserted here...

	FT_CATALOG,
	FT_LAST= FT_CATALOG

	// BrowserFrame::OnLoadRawOnly() depends on raw types...
};

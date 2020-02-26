/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// PrintPhotos.h: interface for the PrintPhotos class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PRINTPHOTOS_H__E7CC5B0C_FBFF_4ADC_B571_A227C167D450__INCLUDED_)
#define AFX_PRINTPHOTOS_H__E7CC5B0C_FBFF_4ADC_B571_A227C167D450__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "PrintEngine.h"
#include "DibCache.h"


class PrintPhotos : public PrintEngine
{
public:
	PrintPhotos();
	virtual ~PrintPhotos();

	virtual int GetPageCount(int items_count) const;

	virtual CSize GetImageSize() const;

private:
	virtual bool PrintFn(CDC& dc, CRect page_rect, VectPhotoInfo& photos, int page);

	bool PrintFunction(CDC& dc, CRect page_rect, VectPhotoInfo& photos, int page);
	void PrintPhoto(PhotoInfo& photo, CDC& dc, const CRect& dest_rect, bool halftone);

	// zoom - this is magnification multiplier, with 1 being zoom to fit, 2 twice that big, 0.5 half of it, ect.
	void PrintDib(Dib& dib, CDC& dc, const CRect& dest_rect, bool halftone, double zoom);

	DibCache dib_cache_;
	int image_gap_;	// space between adjacent images (in LOMETRIC)

	// preview: decode small preview img and draw it; img will be kept in a cache
	void PreviewPhotograph(PhotoInfo& photo, CDC& dc, const CRect& dest_rect, bool halftone, double zoom);

	// print: decode whole scale image, draw it and discard it
	void PrintPhotograph(PhotoInfo& photo, CDC& dc, const CRect& dest_rect, bool halftone, double zoom);

};

#endif // !defined(AFX_PRINTPHOTOS_H__E7CC5B0C_FBFF_4ADC_B571_A227C167D450__INCLUDED_)

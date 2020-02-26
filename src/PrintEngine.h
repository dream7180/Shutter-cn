/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// PrintEngine.h: interface for the PrintEngine class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PRINTENGINE_H__2CDEBC49_C9E2_45BC_8DB3_7946A6D32262__INCLUDED_)
#define AFX_PRINTENGINE_H__2CDEBC49_C9E2_45BC_8DB3_7946A6D32262__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "PhotoInfoStorage.h"
#include "ICMProfile.h"


class PrintEngine
{
public:
	PrintEngine();
	virtual ~PrintEngine();

	void Print(CDC& dc, const CRect& area_rect, VectPhotoInfo& photos, int page);

	virtual int GetPageCount(int items_count) const = 0;

	int GetCurPage() const						{ return cur_page_; }

	void SetCurPage(int page);

	void SetZoom(double zoom);
	double GetZoom() const;

	void SetItemsAcross(int count)				{ columns_ = count; }

	void SetPageSize(CSize size, const CRect& margins_rect);

	void SetPrintableArea(const CRect& rect)	{ printable_area_rect_ = rect; }

	CSize GetPageSize() const					{ return page_size_; }

	virtual CSize GetImageSize() const = 0;

	void SetMargins(const CRect& margins_rect)	{ margins_rect_ = margins_rect; }

	void SetNoOfPhotoCopies(int copies)			{ image_copies_ = copies; }

	void SetPrinterProfile(ICMProfilePtr icc)	{ printer_profile_ = icc; }
	void SetPhotoProfile(ICMProfilePtr icc)		{ photo_profile_ = icc; }

	virtual void GetDefaultFont(LOGFONT& lf) const;
	virtual void SetDefaultFont(const LOGFONT& lf);

	enum PrintingOptions { None, PrintFileNames, PrintDateTime };
	void SetPrintingOptions(PrintingOptions opt);

	bool show_margins_;
	bool show_prinatable_area_;
	bool print_page_number_;
	bool show_image_space_;
	String footer_;

protected:
	int columns_;
	CSize page_size_;
	CRect margins_rect_;
	CRect printable_area_rect_;
	int cur_page_;
	int image_copies_;
	ICMProfilePtr printer_profile_;
	ICMProfilePtr photo_profile_;
	PrintingOptions printing_options_;
	double zoom_;

	void PrepareDC(CDC& dc, const CRect& device_rect);

	virtual bool PrintFn(CDC& dc, CRect page_rect, VectPhotoInfo& photos, int page) = 0;
};

#endif // !defined(AFX_PRINTENGINE_H__2CDEBC49_C9E2_45BC_8DB3_7946A6D32262__INCLUDED_)

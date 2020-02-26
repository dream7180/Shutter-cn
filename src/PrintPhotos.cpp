/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// PrintPhotos.cpp: implementation of the PrintPhotos class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PrintPhotos.h"
#include "ImageDecoder.h"
#include <math.h>
#include "Config.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

PrintPhotos::PrintPhotos() : dib_cache_(20)
{
	image_gap_ = 5; // 0.5 mm
	SetPrinterProfile(g_Settings.default_printer_);
	SetPhotoProfile(g_Settings.default_photo_);
}

PrintPhotos::~PrintPhotos()
{}


int PrintPhotos::GetPageCount(int items_count) const
{
//TODO
	if (columns_ > 0)
		return (items_count * image_copies_ + columns_ - 1) / columns_;

	return 0;
}


#define THREE_COLUMNS_SIZE_RATIO(x)		(x * 3 / 5)


CSize PrintPhotos::GetImageSize() const
{
	CRect page_rect(CPoint(0, 0), page_size_);
	page_rect.DeflateRect(margins_rect_);

	if (columns_ > 0)
	{
		int space= image_gap_ * 2;

		if (columns_ == 1)
		{
			return page_rect.Size();
		}
		else if (columns_ == 2)
		{
			if (page_rect.Width() > page_rect.Height())
				return CSize(page_rect.Width() / 2 - space, page_rect.Height());
			else
				return CSize(page_rect.Width(), page_rect.Height() / 2 - space);
		}
		else if (columns_ == 3)
		{
			if (page_rect.Width() > page_rect.Height())
			{
				int width= THREE_COLUMNS_SIZE_RATIO(page_rect.Width()) - image_gap_;
				int height= std::min(page_rect.Width() - width - image_gap_ * 2, page_rect.Height() / 2 - image_gap_);
				return CSize(width, height);
			}
			else
			{
				int height= THREE_COLUMNS_SIZE_RATIO(page_rect.Height()) - image_gap_;
				int width= std::min(page_rect.Height() - height - image_gap_ * 2, page_rect.Width() / 2 - image_gap_);
				return CSize(width, height);
			}
		}
		else
		{
			int sqroot= static_cast<int>(sqrt(static_cast<double>(columns_)));
			int width= (page_rect.Width() - space * (sqroot - 1)) / sqroot;
			int height= (page_rect.Height() - space * (sqroot - 1)) / sqroot;

			if (width > 0 && height > 0)
				return CSize(width, height);
		}
	}

	return CSize(0, 0);
}


bool PrintPhotos::PrintFn(CDC& dc, CRect page_rect, VectPhotoInfo& photos, int page)
{
	try
	{
		return PrintFunction(dc, page_rect, photos, page);
	}
	catch (ColorException& ex)
	{
		oStringstream ost;
		ost << _T("Color management printing error: ") << ex.GetErrorCode() << std::endl << std::endl;
		ost << _T("\"") << ex.GetMessage() << _T("\"");
		AfxMessageBox(ost.str().c_str(), MB_OK | MB_ICONERROR);
	}
	catch (ImageStat status)
	{
		oStringstream ost;
		ost << _T("Error reading image file:") << std::endl << ImageStatMsg(status);
		AfxMessageBox(ost.str().c_str(), MB_OK | MB_ICONERROR);
	}
	catch (...)
	{
		AfxMessageBox(_T("Fatal error encountered while printing images."), MB_OK | MB_ICONERROR);
	}
	return false;
}


bool PrintPhotos::PrintFunction(CDC& dc, CRect page_rect, VectPhotoInfo& photos, int page)
{
	if (photos.empty())
		return false;

	ASSERT(columns_ > 0);
	ASSERT(image_copies_ > 0);
	if (columns_ < 1 || image_copies_ < 1)
		return false;

	if (!dc.IsPrinting())
		dc.IntersectClipRect(page_rect);

	page_rect.DeflateRect(margins_rect_);

	int index_from= page * columns_ / image_copies_;
	int remaining= image_copies_ - page * columns_ % image_copies_;

	if (index_from >= photos.size() || index_from < 0)
		return false;

	PhotoInfoPtr print[8 * 8];
	ASSERT(columns_ <= 64);

	for (int i= 0; i < columns_ && i < array_count(print); ++i)
	{
		if (remaining > 0)
			remaining--;
		else
		{
			index_from++;
			remaining = image_copies_ - 1;
		}

		if (index_from < photos.size())
			print[i] = photos[index_from];
		else
			print[i] = 0;
	}

	// if printing use halftoning
	bool halftone= !!dc.IsPrinting();

	// gap between photos (will be 2 times gap)
	int gap= image_gap_;
	CPoint center= page_rect.CenterPoint();

	if (columns_ == 1)
	{
		PrintPhoto(*print[0], dc, page_rect, halftone);
	}
	else if (columns_ == 2)
	{
		CRect rect1= page_rect;
		CRect rect2= page_rect;

		if (page_rect.Width() > page_rect.Height())
		{
			rect1.right = center.x - gap;
			rect2.left = center.x + gap;
		}
		else
		{
			rect1.bottom = center.y - gap;
			rect2.top = center.y + gap;
		}

		PrintPhoto(*print[0], dc, rect1, halftone);

		if (print[1])
			PrintPhoto(*print[1], dc, rect2, halftone);
	}
	else if (columns_ == 3)
	{
		CRect rect1= page_rect;
		CRect rect2= page_rect;
		CRect rect3= page_rect;

		if (page_rect.Width() > page_rect.Height())
		{
			int width= THREE_COLUMNS_SIZE_RATIO(page_rect.Width()) - gap;
			int height= std::min(page_rect.Width() - width - gap * 2, page_rect.Height() / 2 - gap);

			rect1.right = rect1.left + width;
			rect1.bottom = rect1.top + height;

			rect2.right = rect2.left + width;
			rect2.top = rect2.bottom - height;

			rect3.left = rect3.right - height;
			rect3.bottom = rect3.top + width;
		}
		else
		{
			int height= THREE_COLUMNS_SIZE_RATIO(page_rect.Height()) - gap;
			int width= std::min(page_rect.Height() - height - gap * 2, page_rect.Width() / 2 - gap);

			rect1.bottom = rect1.top + height;
			rect1.right = rect1.left + width;

			rect2.bottom = rect2.top + height;
			rect2.left = rect2.right - width;

			rect3.top = rect3.bottom - width;
			rect3.right = rect3.left + height;
		}

		if (print[0])
			PrintPhoto(*print[0], dc, rect1, halftone);
		if (print[1])
			PrintPhoto(*print[1], dc, rect2, halftone);
		if (print[2])
			PrintPhoto(*print[2], dc, rect3, halftone);
	}
	else if (columns_ == 4)
	{
		CRect rect[]= { page_rect, page_rect, page_rect, page_rect };

		rect[0].right = rect[2].right = center.x - gap;
		rect[1].left = rect[3].left = center.x + gap;

		rect[0].bottom = rect[1].bottom = center.y - gap;
		rect[2].top = rect[3].top = center.y + gap;

		for (int i= 0; i < columns_; ++i)
		{
			if (print[i] == 0)
				break;

			PrintPhoto(*print[i], dc, rect[i], halftone);
		}
	}
	else
	{
		int space= gap * 2;
		int sqroot= static_cast<int>(sqrt(static_cast<double>(columns_)));
		int width= (page_rect.Width() - space * (sqroot - 1)) / sqroot;
		int height= (page_rect.Height() - space * (sqroot - 1)) / sqroot;

		if (width > 0 && height > 0)
		{
			CRect rect(page_rect.TopLeft(), CSize(width, height));
			int index= 0;

			for (int y= 0; y < sqroot; ++y)
			{
				rect.left = page_rect.left;
				rect.right = rect.left + width;

				for (int x= 0; x < sqroot; ++x)
				{
					if (print[index] == 0)
						break;

					PrintPhoto(*print[index++], dc, rect, halftone);

					rect.OffsetRect(width + space, 0);
				}

				if (print[index] == 0)
					break;

				rect.OffsetRect(0, height + space);
			}
		}
	}


	return true;
}


void PrintPhotos::PrintPhoto(PhotoInfo& photo, CDC& dc, const CRect& dest_rect, bool halftone)
{
	double zoom= GetZoom();

	if (dc.IsPrinting())
		PrintPhotograph(photo, dc, dest_rect, halftone, zoom);
	else
		PreviewPhotograph(photo, dc, dest_rect, halftone, zoom);
}


void PrintPhotos::PrintPhotograph(PhotoInfo& photo, CDC& dc, const CRect& dest_rect, bool halftone, double zoom)
{
	Dib dib;
	CImageDecoderPtr decoder= photo.GetDecoder();

	if (printer_profile_ && printer_profile_->enabled_ && printer_profile_->profile_ &&
		photo_profile_ && photo_profile_->enabled_ && photo_profile_->profile_)
	{
		decoder->SetICCProfiles(photo_profile_->profile_, printer_profile_->profile_, printer_profile_->rendering_);
	}

	if (ImageStat stat= decoder->DecodeImg(dib, CSize(0, 0), false))
		throw stat;

	PrintDib(dib, dc, dest_rect, halftone, zoom);
}


void PrintPhotos::PreviewPhotograph(PhotoInfo& photo, CDC& dc, const CRect& dest_rect, bool halftone, double zoom)
{
	const CSize preview(500, 500);

	std::pair<DibImg*, bool> dib= GetCacheEntry(&photo, preview, dib_cache_);

	if (!dib.second || dib.first->dib.GetBuffer() == 0)		// no img yet?
	{
		// load photo now

//		if (photo.photo_stat_ == IS_NO_IMAGE || photo.photo_stat_ == IS_OUT_OF_MEM)
		{
			CSize img_size(0, 0);
			if (!dc.IsPrinting())
				img_size = preview;	// limit size of image for preview

			CImageDecoderPtr decoder= photo.GetDecoder();
//TODO: ICM
			photo.photo_stat_ = decoder->DecodeImg(dib.first->dib, img_size, false);
		}

//		thumbnail_stat_ = CreateThumbnail(*dib.first);
//		if (thumbnail_stat_ == IS_OK)
//			OrientThumbnail(*dib.first);
	}

	if (show_image_space_)
	{
		CBrush br(HS_FDIAGONAL, RGB(192,192,192));
		CBrush* old= dc.SelectObject(&br);
		dc.SetBkColor(RGB(255,255,255));
		dc.SelectStockObject(NULL_PEN);
		dc.Rectangle(dest_rect);
		dc.SelectObject(old);
	}

	if (dib.first->dib.IsValid())	// has valid image?
	{
		PrintDib(dib.first->dib, dc, dest_rect, halftone, zoom);
	}
	else
	{
//		Draw(0, dc, dest_rect, rgb_back, flags);

		if (!dc.IsPrinting())
		{
			// error image
			photo.photo_stat_;
			CPoint pt= dest_rect.CenterPoint() + CPoint(-11, -9);	// center status img
			//img_list_errors_.Draw(dc, 0, pt, ILD_TRANSPARENT);
		}
	}
}


void PrintPhotos::PrintDib(Dib& dib, CDC& dc, const CRect& dest_rect, bool halftone, double zoom)
{
	CSize bmp_size= dib.GetSize();
	if (bmp_size.cx < 1 || bmp_size.cy < 1)
	{
		ASSERT(false);
		return;
	}

	CSize dest_size= dest_rect.Size();

	if (dest_size.cx <= 0 || dest_size.cy <= 0)
	{
//		ASSERT(false);
		return;
	}

	if (dest_size.cx > dest_size.cy && bmp_size.cx < bmp_size.cy)
	{
		dib.RotateInPlace(true);
		bmp_size = dib.GetSize();
	}
	else if (dest_size.cx < dest_size.cy && bmp_size.cx > bmp_size.cy)
	{
		dib.RotateInPlace(false);
		bmp_size = dib.GetSize();
	}

	double bmp_ratio= double(bmp_size.cx) / double(bmp_size.cy);

	double dest_ratio= double(dest_size.cx) / double(dest_size.cy);

	// calc how to rescale bmp to fit into dest rect
	double scale_w= double(dest_size.cx) / double(bmp_size.cx);
	double scale_h= double(dest_size.cy) / double(bmp_size.cy);

	double scale= MIN(scale_w, scale_h);

	scale *= zoom;

	// rescale bmp
	bmp_size.cx = static_cast<LONG>(bmp_size.cx * scale);
	bmp_size.cy = static_cast<LONG>(bmp_size.cy * scale);

	if (zoom <= 1.0)
	{
		ASSERT(bmp_size.cx <= dest_size.cx);
		ASSERT(bmp_size.cy <= dest_size.cy);
	}
	CSize diff_size= dest_size - bmp_size;

	// center rescalled bitmap in destination rect
	CPoint pos(diff_size.cx / 2, diff_size.cy / 2);

	// finally draw the bitmap
	CRect rect(pos, bmp_size);
	rect.OffsetRect(dest_rect.TopLeft());

#ifdef _DEBUG
//dc.FillSolidRect(dest_rect, RGB(192,192,192));
#endif

	dib.Draw(&dc, rect, 0, halftone);

/* this code doesn't fix black print problem:
	{
		int x= 0, y= 0, w= dib.GetWidth(), h= dib.GetHeight();

		dc.SetStretchBltMode(halftone ? HALFTONE : COLORONCOLOR);

		HGLOBAL p= ::GlobalAlloc(GMEM_FIXED, dib.GetBufferSize());
		if (p == 0)
			throw String(_T("Printing: cannot allocate memory for a bitmap"));

		BYTE* buffer= (BYTE*)p;
		memcpy(p, dib.GetBuffer(), dib.GetBufferSize());

		::StretchDIBits(dc, rect.left, rect.top, rect.Width(), rect.Height(),
			x, y, w, h, buffer, &dib.AsBitmapInfo(), DIB_RGB_COLORS, SRCCOPY);

		::GlobalFree(p);
	}
*/
}

/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "PhotoInfo.h"
#include "BmpFunc.h"
#include "CatchAll.h"
#include "Path.h"


// Function to set given photo as a wallpaper; returns true if it succeeds
//
// It decodes image, trims it down (if it's too big) to the desk area,
// stores it in a temp directory, and selects it as a wallpaper.


void SetPhotoAsWallpaper(const PhotoInfo& photo)
{
	// for resizing wallpaper complete desktop size is needed, not a working area
	CRect desk(0,0,0,0);
	if (!::GetWindowRect(::GetDesktopWindow(), &desk))
		throw String(_T("Cannot obtain desktop's size"));

	//if (!::SystemParametersInfo(SPI_GETWORKAREA, 0, &desk, 0))
	//	return false;

	CImageDecoderPtr decoder= photo.GetDecoder();

	Dib img;
	ImageStat stat= decoder->DecodeImg(img);

	if (stat != IS_OK)
		throw String(_T("Error decoding image: ")) + ImageStatMsg(stat);

	int orientation= photo.rotation_flag_ & 3;
	if (orientation != 0)
	{
		AutoPtr<Dib> copy= ::RotateBitmap(img, orientation);

		if (copy.empty())
			throw String(_T("Bitmap format is not supported"));

		img.Swap(*copy);
	}

	CSize img_size= img.GetSize();

	// resize image to fill the desk area, if they (desk & image) are in the same orientation
	// (landscape & landscape, or portrait & portrait)
	bool size_to_fill= (desk.Width() > desk.Height()) == (img_size.cx > img_size.cy);

	// size to fill/cover entire desktop area (but do not at the cost of magnifying image beyond 100%)
	CRect fill_img_rect(0,0,0,0);
	
	if (size_to_fill)
		fill_img_rect = ::SizeToFill(img_size, desk.Size(), false);
	else
		fill_img_rect = CRect(CPoint(0, 0), Dib::SizeToFit(desk.Size(), img_size));

	if (fill_img_rect.IsRectEmpty())
		throw String(_T("Image cannot be resized to the requested desktop area"));

	CSize new_size= fill_img_rect.Size();
	if (new_size.cx < img_size.cx || new_size.cy < img_size.cy)
	{
		// request to reduce in size

		// magnify (or minify actually) to fill desktop area
		Dib tmp;
		::MagnifyBitmap(img, new_size, tmp, true);

		img.Swap(tmp);
	}

	// need to crop resulting image?
	if (fill_img_rect.left || fill_img_rect.top)
	{
		Dib crop(std::min<int>(new_size.cx, desk.Width()), std::min<int>(new_size.cy, desk.Height()), img.GetBitsPerPixel());

		CDC dc;
		dc.CreateCompatibleDC(0);
		dc.SelectObject(crop.GetBmp());

		img.Draw(&dc, CRect(CPoint(0, 0), new_size), &fill_img_rect, false, false);

		img.Swap(crop);
	}

	// save resulting bitmap

	TCHAR temp[MAX_PATH];
	::GetTempPath(array_count(temp), temp);
	Path path(temp);
	path.AppendDir(_T("ExifPro-Wallpaper.bmp"), false);

	img.Save(path.c_str());

	// set wallpaper

	if (!::SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, const_cast<TCHAR*>(path.c_str()), SPIF_UPDATEINIFILE | SPIF_SENDCHANGE))
		throw String(_T("Setting wallpaper image failed"));
}


extern bool SetWallpaper(const PhotoInfo& photo)
{
	try
	{
		CWaitCursor wait;

		SetPhotoAsWallpaper(photo);

		return true;
	}
	CATCH_ALL_W(AfxGetMainWnd());

	return false;
}

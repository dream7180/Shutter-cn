/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// ReloadJob.h: interface for the ReloadJob class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_RELOADJOB_H__59509F31_02C3_4EC2_B827_58081BC4BDBE__INCLUDED_)
#define AFX_RELOADJOB_H__59509F31_02C3_4EC2_B827_58081BC4BDBE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
class ItemIdList;
class PhotoInfoStorage;
//#include "ImageScanner.h"
//#include "Database/ImageDatabase.h"
class ImageDatabase;
class ImageScanner;
class ImgLogger;


class ReloadJob
{
public:
	ReloadJob(PhotoInfoStorage& photos, std::auto_ptr<ImageScanner> scanner, bool readOnlyPhotosWithEXIF,
			   bool recursiveScan, HWND parent_wnd, uint32 dbFileLengthLimitMB, ImageDatabase& dbImages);

	virtual ~ReloadJob();

	void Quit();

	enum { RELOADING_THREAD_MSG= WM_USER + 990 };

	enum Progress { RELOADING_START, RELOADING_PENDING, RELOADING_CANCELLED, RELOADING_FINISHED, RELOADING_QUIT };

	// is thread still alive?
	bool IsPending() const;

	// is scanning operation still pending?
	bool IsScanning() const;

	const ImgLogger& GetImageErrorLogger() const;

private:
	struct Impl;
	std::auto_ptr<Impl> pImpl_;
};

#endif // !defined(AFX_RELOADJOB_H__59509F31_02C3_4EC2_B827_58081BC4BDBE__INCLUDED_)

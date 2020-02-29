/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "PhotoInfoJPEG.h"
#include "PhotoInfoPtr.h"
#include "SamplePhoto.h"
#include "DateTimeUtils.h"

extern void PostProcessTime(PhotoInfoPtr info);


SmartPtr CreateSamplePhotoInfo()
{
	SmartPtr photo(new PhotoInfoJPEG());
	photo->SetFileSize(1024 * 500);
	photo->SetSize(1024, 768);
	SYSTEMTIME tm;
	::GetLocalTime(&tm);
	photo->SetDateTime(SytemTimeToDateTime(tm));
	photo->SetPhysicalPath(_T("C:\\DSC0001.JPG"));
	photo->SetPhotoName(_T("DSC0001"));
	photo->SetExposureTime(Rational(1, 250));
	photo->SetFStop(Rational(4, 1));
	photo->SetExposureProgram(0);
	//SRational	exposure_bias_;
	//uint16		orientation_;
	//uint16		metering_mode_;
	//uint16		light_source_;
	//uint16		flash_;
	photo->SetFocalLength(Rational(50, 1));
	//Rational	aperture_value_;
	//SRational	shutter_speed_value_;
	photo->SetMake(_T("数码相机"));
	photo->SetModel(_T("测试相机"));
	photo->SetExifDescription(L"图像描述");
	//uint16		thumbnail_orientation_;
	photo->SetSubjectDistance(Rational(100, 1));
	XmpData xmp;
	xmp.Author = _T("MK");
	photo->SetMetadata(xmp);
	photo->SetLensModel(_T("镜头"));

	PostProcessTime(photo.get());

	return photo;
}

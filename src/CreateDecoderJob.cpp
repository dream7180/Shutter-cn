/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "CreateDecoderJob.h"
#include "DecoderJob.h"
#include "ThumbnailDecoder.h"


DecoderJobInterface* CreateImageDecoderJob(PhotoInfo& photo, CSize size, CWnd* receiver)
{
	bool autoRotate= true;

	return new DecoderJob(photo, CSize(photo.GetWidth(), photo.GetHeight()), autoRotate, 0.0, size, receiver, photo.rotation_flag_);
}


DecoderJobInterface* CreateThumbnailDecoderJob(PhotoInfo& photo, CSize size, CWnd* receiver)
{
//LARGE_INTEGER t1, t2, f;
//::QueryPerformanceCounter(&t1);
	DecoderJobInterface* p= new ThumbnailDecoder(photo, size, receiver);
//::QueryPerformanceCounter(&t2);
//t2.QuadPart -= t1.QuadPart;
//::QueryPerformanceFrequency(&f);
//printf("", t2.QuadPart, f.QuadPart);
	return p;
}

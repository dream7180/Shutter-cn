/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "PhotoInfoARW.h"
#include "PhotoFactory.h"
#include "FileTypeIndex.h"
#include "scan.h"

extern bool StripBlackFrame(Dib& dib, bool YUV);

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

namespace {
	RegisterPhotoType<PhotoInfoARW> img(_T("arw"), FT_ARW);
}


PhotoInfoARW::PhotoInfoARW() : PhotoInfoPEF(_T("Sony raw"))
{
	file_type_index_ = FT_ARW;
}

PhotoInfoARW::~PhotoInfoARW()
{}


//int PhotoInfoARW::GetTypeMarkerIndex() const
//{
//	return 13;	// in indicators.png
//}


bool PhotoInfoARW::IsMainImage(int ifd_index)
{
	// first IFD in the ARW file is devoted to the main image
	return ifd_index == 0;
}

bool PhotoInfoARW::IsThumbnailImage(int ifd_index)
{
	// second IFD in the ARW file describes thumbnail
	return ifd_index == 1;
}

bool PhotoInfoARW::IsBigImage(int ifd_index)
{
	// there is no third IFD in the ARW, so big image preview is a first one IFD
	return ifd_index == 0;
}


void PhotoInfoARW::ParseMakerNote(FileStream& ifs)
{
	uint16 entries= ifs.GetUInt16();
/*
	if (entries > 0x200)
	{
		ASSERT(false);
		return;
	}

	uint32 base= 0;

	for (int i= 0; i < entries; ++i)
	{
		Offset tag_start= ifs.RPosition();

		uint16 tag= ifs.GetUInt16();
		Data data(ifs, base);
		Offset temp= ifs.RPosition();

		ifs.RPosition(tag_start, FileStream::beg);
		Offset offset= 0;
		ReadEntry(ifs, offset, make_, model_, this, output_);


		ifs.RPosition(temp, FileStream::beg);
	}
*/
}

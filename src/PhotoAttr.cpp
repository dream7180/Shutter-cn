/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// PhotoAttr.cpp: implementation of the PhotoAttr class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "PhotoAttr.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

PhotoAttr::PhotoAttr()
{
	Init();
}

PhotoAttr::~PhotoAttr()
{}


static const char g_HEADER[4]= { 'M', 'i', 'K', '©'' };
static const char g_MAGIC[4]= { '\xea', '\x0a', '\x01', '\x0a' };
static const char g_UNICODE[4]= { 'U', 'n', 'i', 0 };

void PhotoAttr::Init()
{
	memcpy(header_, g_HEADER, 4);
	memcpy(magic_, g_MAGIC, 4);
	version_major_ = 0x0001;
	version_minor_ = 0x0000;
	orientation_info_ = 0;
	reserved01_ = reserved02_ = reserved03_ = 0;
	flags2_ = 0;
	flags3_ = 0;
	flags4_ = 0;
	for (int i= 0; i < array_count(data_); ++i)
		data_[i] = 0;

	memcpy(desc_format_, g_UNICODE, 4);

	for (int j= 0; j < array_count(description_); ++j)
		description_[j] = 0;
}


bool PhotoAttr::IsValid() const
{
	if (memcmp(header_, g_HEADER, 4) != 0 ||
		memcmp(magic_, g_MAGIC, 4) != 0 ||
		memcmp(desc_format_, g_UNICODE, 4) != 0 ||
		version_major_ > 1)
		return false;

	return true;
}


void PhotoAttr::FillDescription(const std::wstring& description)
{
	const size_t str_length= description.size();

	for (size_t i= 0; i < MAX_DESC_LEN; ++i)
	{
		if (i < str_length)
			description_[i] = description[i];
		else
			description_[i] = 0;
	}
}


void PhotoAttr::ResetThumbnailOrientation(int rotated_clockwise, bool mirrored)
{
	orientation_info_ = 0;

	SetThumbnailOrientation2(rotated_clockwise, mirrored);
}


void PhotoAttr::SetThumbnailOrientation2(int rotated_clockwise, bool mirrored)
{
	int orientation= orientation_info_ & 0x03;

	if (mirrored)
	{
		if (orientation_info_ & MIRROR_FLAG)
			orientation = -rotated_clockwise;	// negate rotation when mirror flip is removed

		orientation_info_ ^= MIRROR_FLAG;	// toggle mirror flip flag
	}

	orientation += rotated_clockwise;

	// MODIFIED_FLAG is used, so orientation field is always != 0 meaning photo was altered
	// (that ensures auto-rotation won't be used any more)
	orientation_info_ |= MODIFIED_FLAG;

	orientation_info_ &= ~3;
	orientation_info_ |= uint8(orientation & 0x03);
}

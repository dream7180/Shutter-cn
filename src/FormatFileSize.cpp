/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
#include "stdafx.h"


extern String FormatFileSize(uint64 fileSize)
{
	oStringstream msg;

	if (fileSize > 1024*1024)
	{
		uint32 m= static_cast<uint32>(fileSize * 10 / (1024*1024));
		msg << m / 10;
		if (m < 100)
			msg << _T('.') << m % 10;
		msg << _T(" MB");
	}
	else if (fileSize > 1024)
	{
		uint32 k= static_cast<uint32>(fileSize * 10 / 1024);
		msg << k / 10;
		if (k < 100)
			msg << _T('.') << k % 10;
		msg << _T(" KB");
	}
	else
		msg << static_cast<uint32>(fileSize) << _T(" bytes");

	return msg.str();
}

/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once


template <class SRC>
uint16 ReadMarker(SRC& src)
{
	// required 0xff
	uint8 ff= src.GetUInt8();
	if (ff != 0xff)
		return 0;

	// skip subsequent 0xff's
	uint8 m= 0;
	do
	{
		m = src.GetUInt8();
	} while (m == 0xff);

	return 0xff00 | m;
}

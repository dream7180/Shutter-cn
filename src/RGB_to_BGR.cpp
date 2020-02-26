/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


void RGB_to_BGR(const BYTE* line, uint32 line_size, BYTE* out)
{
	const BYTE* end= line + line_size;

	for ( ; line < end; line += 3, out += 3)
	{
		out[2] = line[0];
		out[1] = line[1];
		out[0] = line[2];
	}
}


void RGB16_to_BGR(const BYTE* line, uint32 line_size, BYTE* out, bool little_endian)
{
	const BYTE* end= line + line_size;

//	if (little_endian)
		for ( ; line < end; line += 6, out += 3)
		{
			out[2] = line[0+1];
			out[1] = line[2+1];
			out[0] = line[4+1];
		}
	//else
	//	for ( ; line < end; line += 6, out += 3)
	//	{
	//		out[2] = line[0+1];
	//		out[1] = line[2+1];
	//		out[0] = line[4+1];
	//	}
}

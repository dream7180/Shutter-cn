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


union SerialName
{
	uint64 num;
	unsigned char name[8];
};


uint64 NameToNumber(const TCHAR* name)
{
	if (name == 0 || *name == 0)
		return 0;

	size_t len= _tcslen(name);

	SerialName serial_name;

	serial_name.num = 0;

	for (size_t i= 0; i < len; ++i)
	{
		size_t index= i & 7;
		serial_name.name[index] ^= static_cast<unsigned char>((name[i] & 0xff) << 1);
		serial_name.name[index] ^= i;
	}

	return serial_name.num;
}

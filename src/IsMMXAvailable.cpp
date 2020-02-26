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


static bool CheckIfMMXAvailable()
{
#ifdef  _WIN64
	return true;

#else

	__int32 present= 0;

	__try
	{
		__asm
		{
			mov		eax, 1
			cpuid
			test	edx, 0x00800000
			jz		mmx_not_present

			// see if sfence is functional
			sfence

			// try MMX instruction to make sure no floating point emulation is on
			emms

			mov		eax, 1
			mov		present, eax

mmx_not_present:
			nop

		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		return false;
	}

	return present != 0;
#endif
}


extern bool IsMMXAvailable()
{
	static bool MMX_present= CheckIfMMXAvailable();

	return MMX_present;
}

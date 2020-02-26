/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"


#ifdef _UNICODE // =========================================
extern std::string DumpTheStack(CONTEXT* context);


CString CurCallStackInfo()
{
//#ifndef _WIN64
//	try
//	{
//		CONTEXT ctx;
//		memset(&ctx, 0, sizeof ctx);
//		__asm
//		{
//			mov ctx.Ebp, ebp
//			lea eax, addr
//addr:
//			mov ctx.Eip, eax
//			mov ctx.Esp, esp
//		}
//
//		string stack= DumpTheStack(&ctx);
//
//		return CString(("\n" + stack).c_str());
//	}
//	catch (...)
//	{}
//#endif
	try
	{
		CONTEXT ctx;
		memset(&ctx, 0, sizeof ctx);
		::RtlCaptureContext(&ctx);
		std::string stack= DumpTheStack(&ctx);
		return CString(("\n" + stack).c_str());
	}
	catch (...)
	{}
	return CString();
}


extern std::string CurrentCallStackInfo(CONTEXT& ctx)
{
	try
	{
		//CONTEXT ctx;
		//memset(&ctx, 0, sizeof ctx);
		//::RtlCaptureContext(&ctx);
		return DumpTheStack(&ctx);
	}
	catch (...)
	{}
	return std::string();
}

#endif

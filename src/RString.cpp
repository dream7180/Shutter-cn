/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// RString.cpp: implementation of the RString class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "RString.h"
//#include "Config.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

RString::RString()
{}

RString::~RString()
{}

RString::RString(int rsrc_id)
{
	LoadString(rsrc_id);
}


bool RString::LoadString(int rsrc_id)
{
	// load string in currently selected language

	LPCTSTR name= MAKEINTRESOURCE((rsrc_id >> 4) + 1);
	HRSRC str= ::FindResourceEx(::AfxGetResourceHandle(), RT_STRING, name, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US)); // g_Settings.GetLanguageId());
	if (str == 0)
	{
		ASSERT(false);	// string not found; try default one
		str = ::FindResource(::AfxGetResourceHandle(), RT_STRING, name);
		if (str == 0)
			return false;
	}

	HGLOBAL str_mem= ::LoadResource(::AfxGetResourceHandle(), str);
	if (str_mem == 0)
		return false;

	int size= ::SizeofResource(::AfxGetResourceHandle(), str);
	void* str_data= ::LockResource(str_mem);
	if (str_data == 0)
		return false;

	const WORD* str_table= reinterpret_cast<const WORD*>(str_data);

	int index= rsrc_id & 0xf;

	for (;;)
	{
		WORD length= *str_table++;
		if (index == 0)
		{
#ifndef _UNICODE
			int len= length + 30;
			LPTSTR buffer= GetBuffer(len + 2);
			len = ::WideCharToMultiByte(CP_ACP, 0, reinterpret_cast<const wchar_t*>(str_table), length, buffer, len, 0, 0);
			ReleaseBuffer(len);
#else
			LPTSTR buffer= GetBuffer(length + 1);
			//StringTraits::ConvertToBaseType( buffer, dest_length, pch, length );
			memcpy(buffer, str_table, length * sizeof WCHAR);
			ReleaseBuffer(length);
#endif
			break;
		}
		--index;
		str_table += length;
	}

	::UnlockResource(str_mem);
	::FreeResource(str_mem);
	return true;
}


void RString::Format(UINT format_id, ...)
{
	RString format;
	VERIFY(format.LoadString(format_id) != 0);

	va_list argList;
	va_start(argList, format_id);
	FormatV(format, argList);
	va_end(argList);
}


void RString::Replace(TCHAR old, TCHAR copy)
{
	int len= GetLength();
	for (int i= 0; i < len; ++i)
		if (GetAt(i) == old)
			SetAt(i, copy);
}


RString RString::SubStr(int index)
{
	RString sub;
	AfxExtractSubString(sub, *this, index);
	return sub;
}

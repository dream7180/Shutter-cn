/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "StringConversions.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


void MultiByteToWideString(const char* in, int len, std::wstring& out, int code_page)
{
	std::vector<wchar_t> output;
	output.resize(len + 32);
	len = ::MultiByteToWideChar(code_page, 0, in, len, &output.front(), static_cast<int>(output.size()));
	out.assign(&output.front(), len);
}


void MultiByteToWideString(const std::string& in, std::wstring& out, int code_page/*= CP_UTF8*/)
{
	MultiByteToWideString(in.data(), static_cast<int>(in.length()), out, code_page);
}


std::wstring MultiByteToWideString(const std::string& in, int code_page/*= CP_UTF8*/)
{
	std::wstring out;
	MultiByteToWideString(in, out, code_page);
	return out;
}


std::wstring MultiByteToWideString(const char* in, int len, int code_page/*= CP_UTF8*/)
{
	std::wstring out;
	MultiByteToWideString(in, len, out, code_page);
	return out;
}


bool WideStringToMultiByte(const std::wstring& in, std::string& out)
{
	int len= ::WideCharToMultiByte(CP_UTF8, 0, in.data(), static_cast<int>(in.size()), 0, 0, 0, 0);
	if (len > 0)
	{
		out.resize(len, '\0');
		::WideCharToMultiByte(CP_UTF8, 0, in.data(), static_cast<int>(in.size()), &out[0], len, 0, 0);
		return true;
	}
	return false;
}


bool WideStringToMultiByte(const wchar_t* in, std::string& out, int code_page/*= CP_UTF8*/)
{
	out.clear();

	size_t str_len= wcslen(in);

	int len= in ? ::WideCharToMultiByte(code_page, 0, in, static_cast<int>(str_len), 0, 0, 0, 0) : 0;
	if (len > 0)
	{
		out.resize(len, '\0');
		::WideCharToMultiByte(code_page, 0, in, static_cast<int>(str_len), &out[0], len, 0, 0);
		return true;
	}
	return false;
}


/////////////////////////////////////////////////////////////////////////////////////////////////


std::string WStr2AStr(const wchar_t* in)
{
	std::string s;
	::WideStringToMultiByte(in, s, CP_ACP);
	return s;
}

std::string WStr2AStr(const std::wstring& in)
{
	std::string s;
	::WideStringToMultiByte(in.c_str(), s, CP_ACP);
	return s;
}

std::wstring AStr2WStr(const std::string& in)
{
	std::wstring s;
	::MultiByteToWideString(in.c_str(), s, CP_ACP);
	return s;
}


/////////////////////////////////////////////////////////////////////////////////////////////////


String WStr2String(const wchar_t* in)
{
#ifdef _UNICODE
	return String(in);
#else
	return WStr2AStr(in);
#endif
}


String WStr2String(std::wstring s)
{
#ifdef _UNICODE
	return s;
#else
	return WStr2AStr(s);
#endif
}


std::wstring String2WStr(String s)
{
#ifdef _UNICODE
	return s;
#else
	return AStr2WStr(s);
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////////////

std::string WStr2UTF8(const wchar_t* in)
{
	std::string out;
	WideStringToMultiByte(in, out, CP_UTF8);
	return out;
}

std::string WStr2UTF8(const std::wstring& in)
{
	return WStr2UTF8(in.c_str());
}

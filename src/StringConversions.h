/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once


extern void MultiByteToWideString(const std::string& in, std::wstring& out, int code_page= CP_UTF8);
extern std::wstring MultiByteToWideString(const std::string& in, int code_page= CP_UTF8);
extern std::wstring MultiByteToWideString(const char* in, int len, int code_page= CP_UTF8);
extern void MultiByteToWideString(const char* in, int len, std::wstring& out, int code_page);

extern bool WideStringToMultiByte(const std::wstring& in, std::string& out);
extern bool WideStringToMultiByte(const wchar_t* in, std::string& out, int code_page= CP_UTF8);

extern String WStr2String(const wchar_t* in);
extern String WStr2String(std::wstring s);
extern std::wstring String2WStr(String s);

extern std::string WStr2AStr(const wchar_t* in);
extern std::string WStr2AStr(const std::wstring& in);

extern std::string WStr2UTF8(const wchar_t* in);
extern std::string WStr2UTF8(const std::wstring& in);


#ifdef _UNICODE
	inline void TStringToAnsiString(const TCHAR* in, std::string& out)		{ ::WideStringToMultiByte(in, out); }
	inline void TStringToAnsiString(const String& in, std::string& out)		{ ::WideStringToMultiByte(in, out); }
	inline void AnsiStringToTString(const char* in, String& out)			{ ::MultiByteToWideString(in, out); }
	inline void AnsiStringToTString(const std::string& in, String& out)		{ ::MultiByteToWideString(in, out); }
	inline std::string TStr2AStr(const TCHAR* in)							{ return ::WStr2AStr(in); }
	inline std::string TStr2AStr(const String& in)							{ return ::WStr2AStr(in); }
//	inline std::wstring AStr2TStr(const char* in)							{ return ::(in); }
#else
	inline void TStringToAnsiString(const TCHAR* in, std::string& out)		{ out = in; }
	inline void TStringToAnsiString(const String& in, std::string& out)		{ out = in; }
	inline void AnsiStringToTString(const char* in, String& out)			{ out = in; }
	inline void AnsiStringToTString(const std::string& in, String& out)		{ out = in; }
	#define TStr2AStr(s)	(s)
//	#define AStr2TStr(s)	(s)
#endif

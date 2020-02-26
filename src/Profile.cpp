/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "Profile.h"
#include "DeleteArray.h"
#include "StringConversions.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


template<> void Profile<int>::StoreInRegistry(const TCHAR* section, const TCHAR* key, const int& value)
{
	AfxGetApp()->WriteProfileInt(section, key, value);
}

template<> void Profile<int>::RestoreFromRegistry(const TCHAR* section, const TCHAR* key, int& value, const int& default_val)
{
	value = AfxGetApp()->GetProfileInt(section, key, default_val);
}


template<> void Profile<bool>::StoreInRegistry(const TCHAR* section, const TCHAR* key, const bool& value)
{
	AfxGetApp()->WriteProfileInt(section, key, value ? 1 : 0);
}

template<> void Profile<bool>::RestoreFromRegistry(const TCHAR* section, const TCHAR* key, bool& value, const bool& default_val)
{
	value = !!AfxGetApp()->GetProfileInt(section, key, default_val ? 1 : 0);
}


template<> void Profile<std::string>::StoreInRegistry(const TCHAR* section, const TCHAR* key, const std::string& value)
{
	auto wstr= ::MultiByteToWideString(value);
	AfxGetApp()->WriteProfileString(section, key, wstr.c_str());
}

template<> void Profile<std::string>::RestoreFromRegistry(const TCHAR* section, const TCHAR* key, std::string& value, const std::string& default_val)
{
	auto wstr= ::MultiByteToWideString(default_val);
	auto wide= AfxGetApp()->GetProfileString(section, key, wstr.c_str());
	value = WStr2AStr(wide);
}


template<> void Profile<std::wstring>::StoreInRegistry(const TCHAR* section, const TCHAR* key, const std::wstring& value)
{
	AfxGetApp()->WriteProfileString(section, key, value.c_str());
}

template<> void Profile<std::wstring>::RestoreFromRegistry(const TCHAR* section, const TCHAR* key, std::wstring& value, const std::wstring& default_val)
{
	value = AfxGetApp()->GetProfileString(section, key, default_val.c_str());
}


template<> void Profile<unsigned long>::StoreInRegistry(const TCHAR* section, const TCHAR* key, const unsigned long& value)
{
	AfxGetApp()->WriteProfileInt(section, key, value);
}

template<> void Profile<unsigned long>::RestoreFromRegistry(const TCHAR* section, const TCHAR* key, unsigned long& value, const unsigned long& default_val)
{
	value = AfxGetApp()->GetProfileInt(section, key, default_val);
}


template<> void Profile<CString>::StoreInRegistry(const TCHAR* section, const TCHAR* key, const CString& value)
{
	AfxGetApp()->WriteProfileString(section, key, value);
}

template<> void Profile<CString>::RestoreFromRegistry(const TCHAR* section, const TCHAR* key, CString& value, const CString& default_val)
{
	value = AfxGetApp()->GetProfileString(section, key, default_val);
}


extern bool IsRegKeyPresent(const TCHAR* section, const TCHAR* key)
{
	HKEY sec_key = AfxGetApp()->GetSectionKey(section);
	if (sec_key == NULL)
		return false;

	DWORD type, count;
	LONG result= ::RegQueryValueEx(sec_key, const_cast<TCHAR*>(key), NULL, &type, NULL, &count);

	RegCloseKey(sec_key);

	return result == ERROR_SUCCESS;
}


template<> void Profile<float>::StoreInRegistry(const TCHAR* section, const TCHAR* key, const float& value)
{
	// lame; should really be written as a string
	AfxGetApp()->WriteProfileInt(section, key, static_cast<int>(value * 100.0f));
}

template<> void Profile<float>::RestoreFromRegistry(const TCHAR* section, const TCHAR* key, float& value, const float& default_val)
{
	value = AfxGetApp()->GetProfileInt(section, key, static_cast<int>(default_val * 100.0f)) / 100.0f;
}

namespace
{
	String RectToString(const CRect& rect)
	{
		oStringstream ost;
		ost << rect.left << _T(" ") << rect.right << _T(" ") << rect.top << _T(" ") << rect.bottom;
		return ost.str();
	}

	CRect StringToRect(const String& str)
	{
		CRect rect(0,0,0,0);
		iStringstream ist(str);
		ist >> rect.left >> rect.right >> rect.top >> rect.bottom;
		return rect;
	}

	String VectToString(const std::vector<uint16>& v)
	{
		oStringstream ost;
		const size_t count= v.size();
		for (size_t i= 0; i < count; ++i)
			ost << static_cast<uint32>(v[i]) << _T(" ");
		return ost.str();
	}

	void StringToVect(const String& str, std::vector<uint16>& v)
	{
		v.clear();
		iStringstream ist(str);
		for (;;)
		{
			uint32 n;
			ist >> n;
			if (!ist.good())
				break;
			v.push_back(static_cast<uint16>(n));
		}
	}
}

template<> void Profile<CRect>::StoreInRegistry(const TCHAR* section, const TCHAR* key, const CRect& value)
{
	AfxGetApp()->WriteProfileString(section, key, RectToString(value).c_str());
}

template<> void Profile<CRect>::RestoreFromRegistry(const TCHAR* section, const TCHAR* key, CRect& value, const CRect& default_val)
{
	String s= AfxGetApp()->GetProfileString(section, key, RectToString(default_val).c_str());
	value = StringToRect(s);
}


template<> void Profile<std::vector<uint16> >::StoreInRegistry(const TCHAR* section, const TCHAR* key, const std::vector<uint16>& value)
{
	AfxGetApp()->WriteProfileString(section, key, VectToString(value).c_str());
}

template<> void Profile<std::vector<uint16> >::RestoreFromRegistry(const TCHAR* section, const TCHAR* key, std::vector<uint16>& value, const std::vector<uint16>& defaultVect)
{
	String s= AfxGetApp()->GetProfileString(section, key, VectToString(defaultVect).c_str());
	StringToVect(s, value);
}


template<> void Profile<LOGFONT>::StoreInRegistry(const TCHAR* section, const TCHAR* key, const LOGFONT& lf)
{
	AfxGetApp()->WriteProfileBinary(section, key, const_cast<BYTE*>(reinterpret_cast<const BYTE*>(&lf)), sizeof(lf));
}

template<> void Profile<LOGFONT>::RestoreFromRegistry(const TCHAR* section, const TCHAR* key, LOGFONT& lf, const LOGFONT& defaultFnt)
{
	lf = defaultFnt;

	BYTE* data= 0;
	UINT bytes= 0;
	if (AfxGetApp()->GetProfileBinary(section, key, &data, &bytes))
	{
		DeleteArray<BYTE> p(data);

		if (data != 0 && bytes == sizeof(lf))
			lf = *reinterpret_cast<LOGFONT*>(data);
	}
}

/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "ColorProfile.h"
#include "StringConversions.h"


namespace {

int XcmsErrorHandlerFunction(int ErrorCode, const char* error_text)
{
	throw ColorException(ErrorCode, error_text);
}

int init_cms_err_handler= (cmsSetErrorHandler(&XcmsErrorHandlerFunction), 0);

}


///////////////////////////////////////////////////////////////////////////////


ColorProfile::ColorProfile()
{
	profile_ = 0;
}


ColorProfile::ColorProfile(CREATE_FLAG profile)
{
	if (profile == rgb)
		profile_ = ::cmsCreate_sRGBProfile();
	else if (profile == XYZ)
		profile_ = ::cmsCreateXYZProfile();
	else
	{
		profile_ = 0;
		ASSERT(false);
	}
}


ColorProfile::ColorProfile(void* profile_data, DWORD size)
{
	profile_ = ::cmsOpenProfileFromMem(profile_data, size);
}


ColorProfile::~ColorProfile()
{
	if (profile_)
		::cmsCloseProfile(profile_);
}


bool ColorProfile::Close()
{
	bool ok= false;

	if (profile_)
		ok = !!::cmsCloseProfile(profile_);

	profile_ = 0;

	return ok;
}


bool ColorProfile::Open(const TCHAR* file_path, const char* access/*= "r"*/)
{
	ASSERT(profile_ == 0);

#ifdef _UNICODE
	std::wstring str(file_path);
	std::string file;
	::WideStringToMultiByte(str, file);

	profile_ = ::cmsOpenProfileFromFile(file.c_str(), access);
#else
	profile_ = ::cmsOpenProfileFromFile(file_path, access);
#endif

	return profile_ != 0;
}


bool ColorProfile::Open(void* profile_data, DWORD size)
{
	ASSERT(profile_ == 0);

	profile_ = ::cmsOpenProfileFromMem(profile_data, size);

	return profile_ != 0;
}


bool ColorProfile::IsIntentSupported(int intent, int used_direction)
{
	return !!::cmsIsIntentSupported(profile_, intent, used_direction);
}


bool ColorProfile::Create_sRGB()
{
	ASSERT(profile_ == 0);
	profile_ = ::cmsCreate_sRGBProfile();
	return profile_ != 0;
}


bool ColorProfile::CreateXYZ()
{
	ASSERT(profile_ == 0);
	profile_ = ::cmsCreateXYZProfile();
	return profile_ != 0;
}


String ColorProfile::GetTagText(icTagSignature tag)
{
	char text[MAX_PATH + 2];//cmsReadICCText(profile_, tag, &length);
	try
	{
		ASSERT(profile_ != 0);

		if (profile_ == 0)
			return String();

		//int length= 0;
		*text = 0;
		int length= cmsReadICCTextEx(profile_, tag, text, MAX_PATH);

		if (*text == 0 || length <= 0)
			return String();


		String tag_text;

		if (length > 0)
		{
#ifdef _UNICODE
			tag_text = MultiByteToWideString(text, length);
#else
			tag_text.assign(text, text + length);
#endif
		}
		else if (length < 0)
		{
#ifdef _UNICODE
			tag_text = reinterpret_cast<wchar_t*>(text);
#else
			tag_text = text;
#endif
		}
		return tag_text;
	}
	catch (ColorException&)
	{
		return String(_T("-"));
	}
}


String ColorProfile::GetProfileInfo()
{
//	String name= GetProductName();
	String desc= GetProductDesc();
	String info= GetProductInfo();
	String cpyr= GetCopyrightStr();

	oStringstream ost;
	const TCHAR* space= _T("\r\n");
//	ost << name;
//	if (!name.empty())
//		ost << space;
	if (!desc.empty()) // && desc != name)
		ost << desc << space;
	if (!info.empty() && info != desc)// && info != name)
		ost << info << space;
	if (!cpyr.empty() && cpyr != desc && /*cpyr != name &&*/ cpyr != info)
		ost << cpyr << space;

	String str= ost.str();

	while (str[0] == _T(' '))
		str.erase(0);

//	while (!str.empty() && str[str.size() - 1] == _T(' '))
//		str.erase(str.size() - 1);

	return str;
}


icProfileClassSignature ColorProfile::GetDeviceClass() const
{
	ASSERT(profile_ != 0);
	return cmsGetDeviceClass(profile_);
}


String ColorProfile::GetDeviceClassName() const
{
	switch (GetDeviceClass())
	{
	case icSigInputClass:
		return _T("输入设备");
	case icSigDisplayClass:
		return _T("显示设备");
	case icSigOutputClass:
		return _T("输出设备");
	case icSigLinkClass:
		return _T("链接");
	case icSigAbstractClass:
		return _T("摘要");
	case icSigColorSpaceClass:
		return _T("色彩空间");
	case icSigNamedColorClass:
		return _T("命名的颜色");
	default:
		return _T("未知");
	}
}


icColorSpaceSignature ColorProfile::GetColorSpace() const
{
	ASSERT(profile_ != 0);
	return ::cmsGetColorSpace(profile_);
}


///////////////////////////////////////////////////////////////////////////////


ColorTransform::ColorTransform()
{
	transform_ = 0;
}

ColorTransform::~ColorTransform()
{
	if (transform_ != 0)
		::cmsDeleteTransform(transform_);
}


void ColorTransform::Delete()
{
	if (transform_ != 0)
		::cmsDeleteTransform(transform_);
	transform_ = 0;
}


void ColorTransform::Transform(const void* input_buffer, void* output_buffer, unsigned int Size)
{
	ASSERT(transform_ != 0);

//LARGE_INTEGER tm[9];
//::QueryPerformanceCounter(&tm[0]);

	::cmsDoTransform(transform_, const_cast<void*>(input_buffer), output_buffer, Size);

//::QueryPerformanceCounter(&tm[1]);
//LARGE_INTEGER tt;
//tt.QuadPart = tm[1].QuadPart - tm[0].QuadPart;
//int nn= tt.LowPart;
}


bool ColorTransform::Create(cmsHPROFILE input, DWORD input_format, cmsHPROFILE output, DWORD output_format,
							int intent, DWORD flags/*= 0*/)
{
	ASSERT(transform_ == 0);

	transform_ = ::cmsCreateTransform(input, input_format, output, output_format, intent, flags);

	return transform_ != 0;
}

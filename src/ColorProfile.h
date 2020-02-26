/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
//#include <icm.h>
#include "../libs/lcms/include/lcms.h"


class ColorProfile : public mik::counter_base
{
public:
	ColorProfile();
	~ColorProfile();

	ColorProfile(void* profile_data, DWORD size);

	enum CREATE_FLAG { rgb, XYZ };
	ColorProfile(CREATE_FLAG profile);

	bool Open(const TCHAR* file_path, const char* access= "r");
	bool Open(void* profile_data, DWORD size);

	bool Close();

	bool Create_sRGB();
	bool CreateXYZ();

	bool IsIntentSupported(int Intent, int UsedDirection);

	String GetProductName()		{ return GetTagText(icSigDeviceMfgDescTag); }
	String GetProductDesc()		{ return GetTagText(icSigDeviceModelDescTag); }
	String GetProductInfo()		{ return GetTagText(icSigProfileDescriptionTag); }
	String GetCopyrightStr()	{ return GetTagText(icSigCopyrightTag); }

	String GetProfileInfo();

	String GetTagText(icTagSignature tag);

	icProfileClassSignature GetDeviceClass() const;
	String GetDeviceClassName() const;

	icColorSpaceSignature GetColorSpace() const;

	operator cmsHPROFILE ()		{ return profile_; }

	cmsHPROFILE profile_;

//	static ColorProfile& Get_sRGB_Profile;

private:
	ColorProfile(const ColorProfile&);
	void operator = (const ColorProfile&);
};


class ColorTransform : public mik::counter_base
{
public:
	ColorTransform();
	~ColorTransform();

	bool Create(cmsHPROFILE input, DWORD input_format, cmsHPROFILE output, DWORD output_format,
		int intent, DWORD flags= 0);

	void Transform(const void* input_buffer, void* output_buffer, unsigned int Size);

	void Delete();

	operator cmsHTRANSFORM ()		{ return transform_; }

	cmsHTRANSFORM transform_;

private:
	ColorTransform(const ColorTransform&);
	void operator = (const ColorTransform&);
};


class ColorException
{
public:
	ColorException(int err, const char* msg) : err(err), msg(msg)
	{}

	const TCHAR* GetMessage() const		{ return msg; }

	int GetErrorCode() const			{ return err; }

private:
	int err;
	CString msg;
};


#include "ColorProfileForward.h"

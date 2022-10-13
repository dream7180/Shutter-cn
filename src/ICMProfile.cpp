/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "ICMProfile.h"
#include "ColorProfile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


ICMProfile::ICMProfile(const TCHAR* name) : name_(name)
{
	rendering_ = PICTURE;
	enabled_ = true;
	name_editing_ = false;
	default_s_rgb_ = false;
}


ICMProfile::ICMProfile(const ICMProfile* profile)
{
	rendering_ = PICTURE;
	enabled_ = true;
	name_editing_ = false;
	default_s_rgb_ = false;

	if (profile)
	{
		name_ = profile->name_;
		profile_path_ = profile->profile_path_;
		rendering_ = profile->rendering_;
		enabled_ = profile->enabled_;
		name_editing_ = profile->name_editing_;
		default_s_rgb_ = profile->default_s_rgb_;
		profile_ = profile->profile_;
	}
}


String ICMProfile::GetRenderingIntent(bool is_short/*= true*/) const
{
	switch (rendering_)
	{
	case PICTURE:
		return is_short ? _T("可感知") : _T("可感知 (常规图像)");

	case GRAPHIC:
		return is_short ? _T("饱和度") : _T("饱和度 (图形, 图表)");

	case PROOF:
		return is_short ? _T("校样") : _T("校样 (相对色度)");

	case MATCH:
		return is_short ? _T("匹配") : _T("匹配 (绝对色度)");

	default:
		ASSERT(false);
		return _T("");
	}
}


bool ICMProfile::LoadProfile()
{
	if (!profile_path_.empty())
	{
		profile_ = new ColorProfile();
		return profile_->Open(profile_path_.c_str());
	}
	return true;
}

namespace {
	const TCHAR* ENABLED= _T("En");
	const TCHAR* SRGB= _T("_sRGB");
	const TCHAR* REND= _T("Rend");	// rendering intent
}

void ICMProfile::Store(const TCHAR* reg_section, const TCHAR* reg_key)
{
	CWinApp* app= AfxGetApp();
	CString key= reg_key;
	app->WriteProfileString(reg_section, key, profile_path_.c_str());
	app->WriteProfileInt(reg_section, key + ENABLED, enabled_ ? 1 : 0);
	app->WriteProfileInt(reg_section, key + SRGB, default_s_rgb_ ? 1 : 0);
	app->WriteProfileInt(reg_section, key + REND, rendering_);
}


void ICMProfile::Restore(const TCHAR* reg_section, const TCHAR* reg_key)
{
	try
	{
		CWinApp* app= AfxGetApp();
		CString key= reg_key;
		profile_path_ = static_cast<const TCHAR*>(app->GetProfileString(reg_section, key));
		enabled_ = !!app->GetProfileInt(reg_section, key + ENABLED, enabled_ ? 1 : 0);
		default_s_rgb_ = !!app->GetProfileInt(reg_section, key + SRGB, default_s_rgb_ ? 1 : 0);
		switch (app->GetProfileInt(reg_section, key + REND, 0))
		{
		case 0:		rendering_ = PICTURE;		break;
		case 1:		rendering_ = GRAPHIC;		break;
		case 2:		rendering_ = PROOF;		break;
		case 3:		rendering_ = MATCH;		break;
		default:
			ASSERT(false);
			rendering_ = PICTURE;
			break;
		}

		//	if (enabled_)
		{
			if (default_s_rgb_)
				AssignDefault_sRGB();
			else if (!profile_path_.empty())
				LoadProfile();
			else
				AssignDefault_sRGB();
		}
	}
	catch (ColorException& ex)
	{
		AfxMessageBox(ex.GetMessage(), MB_OK | MB_ICONERROR);
	}
}


bool ICMProfile::AssignDefault_sRGB()
{
	ColorProfilePtr p_sRGB= new ColorProfile(ColorProfile::rgb);
	if (p_sRGB->profile_ == 0)
		return false;
	profile_path_.clear();
	profile_ = p_sRGB;
	default_s_rgb_ = true;
	return true;
}


void ICMProfile::AssignProfile(const Path& profile_file, ColorProfilePtr profile)
{
	profile_path_ = profile_file;
	profile_ = profile;
	default_s_rgb_ = false;
}


///////////////////////////////////////////////////////////////////////////////


bool ICMTransform::operator == (const ICMTransform& trans) const
{
	return in_ == trans.in_ && out_ == trans.out_ &&
		rendering_intent_ == trans.rendering_intent_;
}

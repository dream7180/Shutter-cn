/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once
#include "Path.h"
#include "intrusive_ptr.h"
#include "ColorProfile.h"


struct ICMProfile : public mik::counter_base
{
public:
	ICMProfile(const TCHAR* name);
	ICMProfile(const ICMProfile* profile);

	String GetRenderingIntent(bool is_short= true) const;

	bool LoadProfile();

	void Store(const TCHAR* reg_section, const TCHAR* reg_key);
	void Restore(const TCHAR* reg_section, const TCHAR* reg_key);

	bool AssignDefault_sRGB();

	void AssignProfile(const Path& profile_file, ColorProfilePtr profile);

	String name_;
	enum RenderingIntent { PICTURE, GRAPHIC, PROOF, MATCH } rendering_;
	bool enabled_;
	bool name_editing_;

	Path profile_path_;
	ColorProfilePtr profile_;
	bool default_s_rgb_;
};


typedef mik::intrusive_ptr<ICMProfile> ICMProfilePtr;



struct ICMTransform
{
	ICMTransform() : rendering_intent_(0)
	{}

	ICMTransform(ColorProfilePtr in, ColorProfilePtr out, int rendering_intent)
	{
		Reset(in, out, rendering_intent);
	}

	void Reset(ColorProfilePtr in, ColorProfilePtr out, int rendering_intent)
	{
		in_ = in;
		out_ = out;
		rendering_intent_ = rendering_intent;
	}

	bool IsValid() const
	{
		return in_ && out_;
	}

	bool operator == (const ICMTransform& trans) const;
	bool operator != (const ICMTransform& trans) const		{ return !(*this == trans); }

	ColorProfilePtr in_;
	ColorProfilePtr out_;
	int rendering_intent_;
};

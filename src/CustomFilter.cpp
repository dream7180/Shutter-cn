/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "CustomFilter.h"
#include "StringConversions.h"
#include "Lua.h"
#include "PhotoInfo.h"
#include "boost/bind.hpp"
extern void PassParams(Lua& lua, const PhotoInfo& photo);
extern void DefRoundFn(Lua& lua);
extern void DefGlobalImgTable(Lua& lua);
extern void InitializeLua(Lua& lua);


//CustomFilter::CustomFilter()
//{}

CustomFilter::CustomFilter(const String& rule) : rule_(rule)
{}


bool CustomFilter::CalcResult(const PhotoInfo& photo) const
{
	if (lua_.get() == 0)
		Init();

	PassParams(*lua_, photo);

	lua_->GetGlobal("filter_fn");
	lua_->GetGlobal("img");

	const int max_steps= 1000;
	lua_->Call(1, max_steps, 0);

	bool result= false;
	lua_->GetTop(result);

	lua_->PopTop();

	return result;
}


void CustomFilter::Init() const
{
	lua_.reset(new Lua());

	InitializeLua(*lua_);

	DefRoundFn(*lua_);
	DefGlobalImgTable(*lua_);

	std::string expr;
	TStringToAnsiString(rule_, expr);
	std::ostringstream ost;
	ost << "function filter_fn(img) " + expr + "\r\nend";

	const char* msg= 0;
	if (lua_->LoadBuffer(ost.str().c_str(), msg))
	{
		lua_->Call(0);
		lua_->PopTop();
	}
}

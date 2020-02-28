/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "CustomColumns.h"
#include "StringConversions.h"
#include "Lua.h"
#include "PhotoInfo.h"
#include "boost/bind.hpp"
extern void PassParams(Lua& lua, const PhotoInfo& photo);
extern void DefRoundFn(Lua& lua);
extern void DefGlobalImgTable(Lua& lua);
extern void InitializeLua(Lua& lua);


CustomColumns::CustomColumns()
{
	Defaults();
}

CustomColumns::CustomColumns(const CustomColumns& src)
{
	assign(src);
}

CustomColumns::~CustomColumns()
{
}


CustomColumnDef& CustomColumns::operator [] (size_t i)
{
	ASSERT(i < CUST_COLUMNS);
	return columns_[i];
}

const CustomColumnDef& CustomColumns::operator [] (size_t i) const
{
	ASSERT(i < CUST_COLUMNS);
	return columns_[i];
}


void CustomColumns::Defaults()
{
	TCHAR buf[200];
#define eol _T("\r\n")

	for (size_t i= 0; i < CUST_COLUMNS; ++i)
	{
		if (i == 0)
		{
			columns_[i].caption_ = _T("百万像素");
			columns_[i].expression_ = _T("-- image size in mega pixels") eol eol _T("return round(img.w * img.h / 1024^2, 2)") eol;
		}
		else if (i == 1)
		{
			columns_[i].caption_ = _T("相机增益");
			columns_[i].expression_ = _T("-- camera's amplification") eol eol
				_T("if img.fn > 0 then") eol
				_T("    local x= 1 / img.fn") eol
				_T("    return round(x^2 * img.iso * img.et, 3)") eol
				_T("end") eol
				eol
				_T("return 0") eol;
		}
		else
		{
			wsprintf(buf, _T("自定义列 %d"), static_cast<int>(i + 1));
			columns_[i].caption_ = buf;

			wsprintf(buf, _T("-- expression defining column's value\r\n\r\nreturn %d"), static_cast<int>(i + 1));
			columns_[i].expression_ = buf;
		}
		columns_[i].visible_ = false;
	}

#undef eol
}


CustomColumns::Result CustomColumns::CalcValue(size_t column, const PhotoInfo& photo, double& num_result, String& text_result) const
{
	if (lua_.get() == 0)
		Init();

	static const TCHAR* err= _T("<error>");

	if (column < CUST_COLUMNS)
	{
		PassParams(*lua_, photo);

		char fn_name[40];
		wsprintfA(fn_name, "column_fn_%d_", column);
		lua_->GetGlobal(fn_name);
		lua_->GetGlobal("img");

		const int max_steps= 1000;
		lua_->Call(1, max_steps, 0);

		Result res= Err;
		if (lua_->GetTop(num_result))
			res = Number;
		else
		{
			AnsiStringToTString(lua_->GetTop(), text_result);
			res = Text;
		}

		lua_->PopTop();

		return res;
	}
	else
	{
		ASSERT(false);
		return Err;
	}
}


void CustomColumns::Init() const
{
	lua_.reset(new Lua());

	InitializeLua(*lua_);

	DefRoundFn(*lua_);
	DefGlobalImgTable(*lua_);

	for (size_t i= 0; i < CUST_COLUMNS; ++i)
	{
		const CustomColumnDef& c= columns_[i];

		std::string expr;
		TStringToAnsiString(c.expression_, expr);
		std::ostringstream ost;
		ost << "function column_fn_" << i << "_(img) " + expr + "\r\nend";

		const char* msg= 0;
		if (lua_->LoadBuffer(ost.str().c_str(), msg))
		{
			lua_->Call(0);
			lua_->PopTop();
		}
	}
}


void CustomColumns::assign(const CustomColumns& src)
{
	for (size_t i= 0; i < CUST_COLUMNS; ++i)
		columns_[i] = src.columns_[i];
	lua_.reset();
}


bool CustomColumns::Less(size_t column, const PhotoInfo& photo1, const PhotoInfo& photo2) const
{
	double num1, num2;
	String out1, out2;

	Result res1= CalcValue(column, photo1, num1, out1);
	Result res2= CalcValue(column, photo2, num2, out2);

	if (res1 == res2)
	{
		switch (res1)
		{
		case Number:
			return num1 < num2;

		case Text:
			return _tcsicmp(out1.c_str(), out2.c_str()) < 0;

		case Err:
			return false;
		}
	}
	else
	{
		return res1 < res2;
	}

	return false;
}

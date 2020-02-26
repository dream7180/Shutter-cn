/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "ExprCalculator.h"
#include "StringConversions.h"
#include "Lua.h"
#include "PhotoInfo.h"
#include "PhotoInfoPtr.h"
#include "DateTimeUtils.h"


struct ExprCalculator::Impl
{
	Impl();

	Lua lua_;
};


ExprCalculator::ExprCalculator(const TCHAR* expression) : impl_(new Impl)
{
	TStringToAnsiString(expression, raw_expression_);

	expression_ = "function column_fn_(img) " + raw_expression_ + "\r\nend";
}

ExprCalculator::~ExprCalculator()
{
}

#define PHOTO_PTR "_photo_ptr_"


// set fields in a global table 'img'
extern void PassParams(Lua& lua, const PhotoInfo& photo)
{
	lua.PushUserValue(const_cast<void*>(static_cast<const void*>(&photo)));
	lua.SetGlobal(PHOTO_PTR);
}


void ExprCalculator::DefineGlobalVar(const char* name, const char* value)
{
	impl_->lua_.PushValue(value);
	impl_->lua_.SetGlobal(name);
}

void ExprCalculator::DefineGlobalVar(const char* name, int value)
{
	impl_->lua_.PushValue(value);
	impl_->lua_.SetGlobal(name);
}


// define global function 'round(x, n)'
extern void DefRoundFn(Lua& lua)
{
//	const char* trunc= "function round(x, n) return x - x % 10^(-n) end";
	const char* round= "function round(x, n) local m= 10^(-n) local rem= x % m return rem >= m / 2 and x - rem + m or x - rem end";
	const char* msg= 0;
	VERIFY(lua.LoadBuffer(round, msg));
	VERIFY(lua.Call(0) == 0);
	lua.PopTop();
}

// format d/t to make it script friendly: "2008:12:31 21:45:59.100"
char* DateTimeFormat(DateTime tm, char date_time[24])
{
	if (tm.is_not_a_date_time())
		strcpy(date_time, "0000:00:00 00:00:00.000");
	else
	{
		SYSTEMTIME t= DateTimeToSytemTime(tm);
		sprintf(date_time, "%04d:%02d:%02d %02d:%02d:%02d.%03d", int(t.wYear), int(t.wMonth), int(t.wDay), int (t.wHour), int(t.wMinute), int(t.wSecond), int(t.wMilliseconds % 1000));
	}
/*
	if (tm.GetAsSystemTime(t))
		sprintf(date_time, "%04d:%02d:%02d %02d:%02d:%02d.%03d", int(t.wYear), int(t.wMonth), int(t.wDay), int (t.wHour), int(t.wMinute), int(t.wSecond), int(ms % 1000));
	else
		strcpy(date_time, "0000:00:00 00:00:00.000");
*/
	return date_time;
}


// handle read access to the 'tags' table field
// img.tags is expected to be indexed with tag name; it returns true if tag is present (assigned to a photo), or nil otherwise
static int TagElementRead(Lua* lua)
{
	const char* field= lua->StackPeek(-1, false);

	if (field == 0 || *field == 0)
	{
		lua->PushNil();
		return 1;
	}

	lua->GetGlobal(PHOTO_PTR);
	PhotoInfo* photo= static_cast<PhotoInfo*>(lua->GetUserData());

	if (photo == 0)
	{
		lua->PushNil();
		return 1;
	}

	const PhotoTags& tags= photo->GetTags();

	if (!tags.empty())
	{
		String tag;
		AnsiStringToTString(field, tag);

		if (tags.FindTag(tag))
		{
			lua->PushValue(true);
			return 1;
		}
	}

	lua->PushNil();
	return 1;
}


// handle read access to the 'meta' table field
// img.meta is expected to be indexed with field name; it returns value of this field or nil if field is empty (or there's no XMP)
static int XmpElementRead(Lua* lua)
{
	const char* field= lua->StackPeek(-1, false);

	if (field == 0 || *field == 0)
	{
		lua->PushNil();
		return 1;
	}

	lua->GetGlobal(PHOTO_PTR);
	PhotoInfo* photo= static_cast<PhotoInfo*>(lua->GetUserData());

	if (photo == 0)
	{
		lua->PushNil();
		return 1;
	}

	const XmpData* xmp= photo->GetMetadata();

	if (xmp == 0)
	{
		lua->PushNil();
		return 1;
	}

	const size_t len= strlen(field);

	size_t id= 0;
	if (len == 1)
		id = field[0];
	else if (len == 2)
		id = size_t(field[0] << 8) + field[1];
	else if (len == 3)
		id = size_t(field[0] << 16) + (field[1] << 8) + field[2];
	else //if (len == 4)
		id = size_t(field[0] << 24) + (field[1] << 16) + (field[2] << 8) + field[3];

	bool error= false;

#define CASE(id, fld, v)	\
	case (id):	if (fld == 0 || strcmp(fld, field) == 0) lua->PushValue(TStr2AStr(v)); else error = true;	break;

	switch (id)
	{
	CASE('auth',	"author",				xmp->Author)
	CASE('copy',	"copyright_notice",		xmp->CopyrightNotice)
	CASE('desc',	"desc_writer",			xmp->DescriptionWriter)
	CASE('head',	"headline",				xmp->Headline)

	case 'crea':
		if (strcmp("creators_job", field) == 0)				lua->PushValue(TStr2AStr(xmp->CreatorsJob));
		else if (strcmp("creators_addr", field) == 0)		lua->PushValue(TStr2AStr(xmp->Address));
		else if (strcmp("creators_city", field) == 0)		lua->PushValue(TStr2AStr(xmp->City));
		else if (strcmp("creators_state", field) == 0)		lua->PushValue(TStr2AStr(xmp->State));
		else if (strcmp("creators_postal_code", field) == 0)lua->PushValue(TStr2AStr(xmp->PostalCode));
		else if (strcmp("creators_country", field) == 0)	lua->PushValue(TStr2AStr(xmp->Country));
		else if (strcmp("creators_phone", field) == 0)		lua->PushValue(TStr2AStr(xmp->Phones));
		else if (strcmp("creators_email", field) == 0)		lua->PushValue(TStr2AStr(xmp->EMails));
		else if (strcmp("creators_website", field) == 0)	lua->PushValue(TStr2AStr(xmp->WebSites));
		else
			error = true;
		break;

//	CASE('obje',	"object_name",		xmp->Title)

	CASE('titl',	"title",				xmp->DocumentTitle)
	CASE('job_',	"job_identifier",		xmp->JobIdentifier)
	CASE('inst',	"instructions",			xmp->Instructions)
	CASE('prov',	"provider",				xmp->Provider)
	CASE('sour',	"source",				xmp->Source)
	CASE('righ',	"rights_usage",			xmp->RightsUsageTerms)
	CASE('info',	"info_url",				xmp->CopyrightInfoURL)
	CASE('date',	"date_created",			xmp->CreationDate)
	CASE('inte',	"intellectual_genre",	xmp->IntellectualGenre)
	CASE('loca',	"location",				xmp->Location)
	CASE('city',	0,						xmp->City2)
	CASE('stat',	"state",				xmp->StateProvince)
	CASE('coun',	"country",				xmp->Country2)
	CASE('iso_',	"iso_country_code",		xmp->ISOCountryCode)
	CASE('scen',	"scene",				xmp->IPTCScene)
	CASE('subj',	"subject_code",			xmp->IPTCSubjectCode)

	default:
		error = true;
		break;
	}

#undef CASE

	if (error)	// field not recognized
	{
		std::ostringstream o;
		o << "Image metadata field '" << field << "' has not been recognized";
		lua->Abort(o.str().c_str());
	}

	return 1;
}


// handle read access to 'img' table field
static int ElementRead(Lua* lua)
{
	const char* field= lua->StackPeek(-1, false);

	if (field == 0 || *field == 0)
	{
		lua->PushNil();
		return 1;
	}

	lua->GetGlobal(PHOTO_PTR);
	PhotoInfo* photo= static_cast<PhotoInfo*>(lua->GetUserData());

	if (photo == 0)
	{
		lua->PushNil();
		return 1;
	}

	const size_t len= strlen(field);

	size_t id= 0;
	if (len == 1)
		id = field[0];
	else if (len == 2)
		id = size_t(field[0] << 8) + field[1];
	else if (len == 3)
		id = size_t(field[0] << 16) + (field[1] << 8) + field[2];
	else //if (len == 4)
		id = size_t(field[0] << 24) + (field[1] << 16) + (field[2] << 8) + field[3];

	bool error= false;
	char buffer[32];

#define CASE(id, fld, v)	\
	case (id):	if (fld == 0 || strcmp(fld, field) == 0) lua->PushValue((v)); else error = true;	break;

	switch (id)
	{
	CASE('w',		0,			photo->GetWidth())
	CASE('h',		0,			photo->GetHeight())
	CASE('fl',		0,			photo->GetFocalLength().Double())
	CASE('fl35',	0,			photo->GetFocalLength35mm())
	CASE('flas',	"flash",	TStr2AStr(photo->Flash()))
	CASE('fn',		0,			photo->GetFStop().Double())
	CASE('et',		0,			photo->GetExposureTime().Double())
	CASE('expp',	"expprog",	TStr2AStr(photo->ExposureProgram()))
	CASE('iso',		0,			static_cast<double>(photo->IsISOSpeedValid() ? photo->GetISOSpeed() : 0))
	CASE('expb',	"expbias",	photo->GetExposureBias().Double())
	CASE('time',	0,			DateTimeToFraction(photo->GetDateTime()))
	CASE('size',	0,			static_cast<double>(photo->GetFileSize()))
	CASE('subd',	"subdist",	photo->GetSubjectDistance().Double())
	CASE('rati',	"rating",	photo->GetRating())
	CASE('name',	0,			TStr2AStr(photo->GetName()))
	CASE('ext',		0,			TStr2AStr(photo->GetFileTypeName()))
	CASE('path',	0,			TStr2AStr(photo->GetPhysicalPath()))
	CASE('port',	"portrait",	!photo->HorizontalOrientation())
	CASE('srgb',	0,			photo->IsSRGB())
	CASE('metm',	"metmode",	TStr2AStr(photo->MeteringMode()))
	CASE('ligh',	"lightsrc",	TStr2AStr(photo->LightSource()))
	CASE('make',	0,			TStr2AStr(photo->GetMake()))
	CASE('mode',	"model",	TStr2AStr(photo->GetModel()))
	CASE('fovc',	0,			photo->GetFieldOfViewCrop())
	CASE('date',	0,			DateTimeFormat(photo->GetDateTime(), buffer))
	CASE('lens',	"lensmodel",		TStr2AStr(photo->GetLensModel()))

	default:
		error = true;
		break;
	}

#undef CASE

	if (error)	// field not recognized
	{
		std::ostringstream o;
		o << "Image attribute '" << field << "' has not been recognized";
		lua->Abort(o.str().c_str());
	}

	return 1;
}


// handle read access to 'file' table field
static int FileElementRead(Lua* lua)
{
	const char* field= lua->StackPeek(-1, false);

	if (field == 0 || *field == 0)
	{
		lua->PushNil();
		return 1;
	}

	lua->GetGlobal(PHOTO_PTR);
	PhotoInfo* photo= static_cast<PhotoInfo*>(lua->GetUserData());

	if (photo == nullptr)
	{
		lua->PushNil();
		return 1;
	}

	const size_t len= strlen(field);

	size_t id= 0;
	if (len == 1)
		id = field[0];
	else if (len == 2)
		id = size_t(field[0] << 8) + field[1];
	else if (len == 3)
		id = size_t(field[0] << 16) + (field[1] << 8) + field[2];
	else //if (len == 4)
		id = size_t(field[0] << 24) + (field[1] << 16) + (field[2] << 8) + field[3];

	bool error= false;

#define CASE(id, fld, v)	\
	case (id):	if (fld == 0 || strcmp(fld, field) == 0) lua->PushValue((v)); else error = true;	break;

	switch (id)
	{
	CASE('fnam',	"fname",	WStr2UTF8(photo->GetNameAndExt()))
	CASE('name',	0,			WStr2UTF8(photo->GetName()))
	CASE('ext',		0,			WStr2UTF8(photo->GetFileTypeName()))
	CASE('day',		0,			photo->GetDateTime().date().day())
	CASE('mont',	"month",	photo->GetDateTime().date().month())
	CASE('year',	0,			photo->GetDateTime().date().year())
	CASE('mill',	"millisecond",	static_cast<size_t>(GetDateTimeMilliseconds(photo->GetDateTime()))) //GetPhotoExactTime() % 1000))
	CASE('seco',	"second",	static_cast<int>(photo->GetDateTime().time_of_day().seconds()))
	CASE('minu',	"minute",	static_cast<int>(photo->GetDateTime().time_of_day().minutes()))
	CASE('hour',	0,			static_cast<int>(photo->GetDateTime().time_of_day().hours()))
	CASE('dir',		0,			WStr2UTF8(photo->GetPhysicalPath().GetDir()))
	CASE('path',	0,			WStr2UTF8(photo->GetPhysicalPath()))

	default:
		error = true;
		break;
	}

#undef CASE

	if (error)	// field not recognized
	{
		std::ostringstream o;
		o << "File attribute '" << field << "' has not been recognized";
		lua->Abort(o.str().c_str());
	}

	return 1;
}


static int ElementCreate(Lua* lua)
{
	lua->Abort("Modification of image record is not allowed.");
	return 0;
}

static int FileElementCreate(Lua* lua)
{
	lua->Abort("Modification of file record is not allowed.");
	return 0;
}


// define global table 'img'
extern void DefGlobalImgTable(Lua& lua)
{
	lua.NewTable();
	lua.SetGlobal("img");

	//---- tags ----
	lua.GetGlobal("img");
	// add tags table
	lua.NewTable();
	// create meta table for tag access
	lua.NewTable();
	lua.SetField("__index", &TagElementRead);
	lua.SetField("__newindex", &ElementCreate);
	lua.SetMetaTable();
	// store tag table in the 'tags' field in 'img'
	lua.SetField("tags");
	lua.PopTop();

	//---- metadata (XMP) ----
	lua.GetGlobal("img");
	// add tags table
	lua.NewTable();
	// create meta table for XMP access
	lua.NewTable();
	lua.SetField("__index", &XmpElementRead);
	lua.SetField("__newindex", &ElementCreate);
	lua.SetMetaTable();
	// store xmp table in the 'meta' field in 'img'
	lua.SetField("meta");
	lua.PopTop();

	//---- file name ----
	lua.NewTable();
	lua.SetGlobal("file");
	lua.GetGlobal("file");
	// create meta table for file name access
	lua.NewTable();
	lua.SetField("__index", &FileElementRead);
	lua.SetField("__newindex", &FileElementCreate);
	lua.SetMetaTable();
	lua.PopTop();


	lua.GetGlobal("img");
	// create meta table for attrib access
	lua.NewTable();
	lua.SetField("__index", &ElementRead);
	lua.SetField("__newindex", &ElementCreate);
	lua.SetMetaTable();
	lua.PopTop();
}


static void LuaLineHook(Lua::Event ev, int line, Lua* lua)
{
	lua->Abort("Program too complex; exceeded maximum number of steps allowed.");
}


extern void InitializeLua(Lua& lua)
{
	lua.InitializeBuiltInLibs();

	lua.SetCallback(boost::bind(&LuaLineHook, _1, _2, &lua));
}


ExprCalculator::Impl::Impl()
{
	InitializeLua(lua_);
}


bool ExprCalculator::IsValid(String& err_msg, const PhotoInfo& photo) const
{
	const int max_steps= 1000;
	return IsValid(err_msg, photo, max_steps);
}


bool ExprCalculator::IsValid(String& err_msg, const PhotoInfo& photo, int max_steps) const
{
//	Lua lua;
//	InitializeLua(lua);

	impl_->lua_.SetStrict();

	const char* name= "expression";
	const char* msg= 0;
	if (!impl_->lua_.LoadBuffer(raw_expression_.c_str(), msg, name))
	{
		AnsiStringToTString(msg, err_msg);
		return false;
	}
	impl_->lua_.PopTop();

	if (!impl_->lua_.LoadBuffer(expression_.c_str(), msg, name))
	{
		AnsiStringToTString(msg, err_msg);
		return false;
	}

	if (impl_->lua_.Call(0, &msg) != 0)
	{
		AnsiStringToTString(msg, err_msg);
		return false;
	}
	impl_->lua_.PopTop();

	DefRoundFn(impl_->lua_);
	DefGlobalImgTable(impl_->lua_);

	PassParams(impl_->lua_, photo);

	impl_->lua_.GetGlobal("column_fn_");
	impl_->lua_.GetGlobal("img");

	if (impl_->lua_.Call(1, max_steps, &msg) != 0)
	{
		AnsiStringToTString(msg, err_msg);
		return false;
	}

	AnsiStringToTString(impl_->lua_.GetTop(), err_msg);

	impl_->lua_.PopTop();

	return true;
}

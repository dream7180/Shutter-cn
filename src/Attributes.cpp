/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "resource.h"
#include "Attributes.h"


static const TCHAR* ShowAttribsPopup(CWnd* wnd, CPoint pos, int menu_id, const TCHAR* attribs[], size_t count)
{
	CMenu menu;
	if (!menu.LoadMenu(menu_id))
		return 0;
	CMenu* popup= menu.GetSubMenu(0);
	ASSERT(popup != NULL);

	if (popup == 0)
		return 0;

	int cmd= popup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, pos.x, pos.y, wnd);

	if (cmd == 0)
		return 0;

	cmd--;

	if (cmd >= 0 && cmd < count)
		return attribs[cmd];
	else
	{ ASSERT(false); }

	return 0;
}


static const TCHAR* attribs[]=
{
	_T("img.date"),
	_T("img.expprog"),
	_T("img.et"),
	_T("img.expbias"),
	_T("img.ext"),
	_T("img.fl"),
	_T("img.fl35"),
	_T("img.flash"),
	_T("img.fn"),
	_T("img.fovc"),
	_T("img.h"),
	_T("img.iso"),
	_T("img.lensmodel"),
	_T("img.lightsrc"),
	_T("img.make"),
	_T("img.metmode"),
	_T("img.model"),
	_T("img.name"),
	_T("img.path"),
	_T("img.portrait"),
	_T("img.rating"),
	_T("img.size"),
	_T("img.srgb"),
	_T("img.subdist"),
//		_T("img.tags"),
	_T("img.time"),
	_T("img.w"),
};


const TCHAR* GetImageAttribName(size_t index)
{
	if (index >= array_count(attribs))
		return 0;

	return attribs[index];
}


const TCHAR* ImgAttribsPopup(CWnd* wnd, CPoint pos)
{
	return ShowAttribsPopup(wnd, pos, IDR_ATTRIBUTES, attribs, array_count(attribs));
/*
	CMenu menu;
	if (!menu.LoadMenu(IDR_ATTRIBUTES))
		return 0;
	CMenu* popup= menu.GetSubMenu(0);
	ASSERT(popup != NULL);

	if (popup == 0)
		return 0;

	int cmd= popup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, pos.x, pos.y, wnd);

	if (cmd == 0)
		return 0;

	cmd--;

	if (cmd >= 0 && cmd < array_count(attribs))
		return attribs[cmd];
	else
	{ ASSERT(false); }

	return 0;*/
}


static const TCHAR* metadata[]=
{
	_T("img.meta.author"),
	_T("img.meta.copyright_notice"),
	_T("img.meta.desc_writer"),
	_T("img.meta.headline"),
	_T("img.meta.creators_job"),
	_T("img.meta.creators_addr"),
	_T("img.meta.creators_city"),
	_T("img.meta.creators_state"),
	_T("img.meta.creators_postal_code"),
	_T("img.meta.creators_country"),
	_T("img.meta.creators_phone"),
	_T("img.meta.creators_email"),
	_T("img.meta.creators_website"),
	_T("img.meta.title"),
	_T("img.meta.job_identifier"),
	_T("img.meta.instructions"),
	_T("img.meta.provider"),
	_T("img.meta.source"),
	_T("img.meta.rights_usage"),
	_T("img.meta.info_url"),
	_T("img.meta.date_created"),
	_T("img.meta.intellectual_genre"),
	_T("img.meta.location"),
	_T("img.meta.city"),
	_T("img.meta.state"),
	_T("img.meta.country"),
	_T("img.meta.iso_country_code"),
	_T("img.meta.scene"),
	_T("img.meta.subject_code"),
};


const TCHAR* GetMetaAttribName(size_t index)
{
	if (index >= array_count(metadata))
		return 0;

	return metadata[index];
}


const TCHAR* ImgMetaAttribsPopup(CWnd* wnd, CPoint pos)
{
	return ShowAttribsPopup(wnd, pos, IDR_METADATA, metadata, array_count(metadata));
/*
	CMenu menu;
	if (!menu.LoadMenu(IDR_METADATA))
		return 0;
	CMenu* popup= menu.GetSubMenu(0);
	ASSERT(popup != NULL);

	if (popup == 0)
		return 0;

	int cmd= popup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, pos.x, pos.y, wnd);

	if (cmd == 0)
		return 0;

	cmd--;

	if (cmd >= 0 && cmd < array_count(metadata))
		return metadata[cmd];
	else
	{ ASSERT(false); }

	return 0; */
}


static const TCHAR* file_attribs[]=
{
	_T("file.fname"),
	_T("file.name"),
	_T("file.ext"),
	_T("file.dir"),
	_T("file.path"),
	_T("file.day"),
	_T("file.month"),
	_T("file.year"),
	_T("file.millisecond"),
	_T("file.second"),
	_T("file.minute"),
	_T("file.hour"),
};

const TCHAR* GetFileAttribName(size_t index)
{
	if (index >= array_count(file_attribs))
		return 0;

	return file_attribs[index];
}


const TCHAR* ImgFileAttribsPopup(CWnd* wnd, CPoint pos)
{
	return ShowAttribsPopup(wnd, pos, IDR_FILE_ATTRIBUTES, file_attribs, array_count(file_attribs));
}


String GetImgAttribsHelpString(unsigned int include_help)
{
	oStringstream help;

	if (include_help & IAH_FILE_INFO)
	{
		help <<
			_T("File name attributes:\r\n")
			_T("file.fname\t- full file name with extension (\"DCS01234.JPG\")\r\n")
			_T("file.name\t- file name without extension (\"DCS01234\")\r\n")
			_T("file.ext\t- file name extension (\"JPG\")\r\n");
	}

	help <<
		_T("Image attributes:\r\n")
		_T("img.date\t- creation date & time (text)\r\n")
		_T("img.expprog\t- exposure program\r\n")
		_T("img.et\t- exposure time\r\n")
		_T("img.expbias\t- exposure bias\r\n")
		_T("img.ext\t- file extension name\r\n")
		_T("img.fl\t- focal length\r\n")
		_T("img.fl35\t- focal length (35 mm equivalent)\r\n")
		_T("img.flash\t- flash information\r\n")
		_T("img.fn\t- F/Stop (F Number, aperture)\r\n")
		_T("img.fovc\t- field of view crop\r\n")
		_T("img.h\t- image height\r\n")
		_T("img.iso\t- sensitivity\r\n")
		_T("img.lensmodel\t- lens model\r\n")
		_T("img.lightsrc\t- light source\r\n")
		_T("img.make\t- camera make\r\n")
		_T("img.metmode\t- metering mode\r\n")
		_T("img.model\t- camera model\r\n")
		_T("img.name\t- file name\r\n")
		_T("img.path\t- complete path to the image\r\n")
		_T("img.portrait\t- true if image is in portrait orientation, false if it's in landscape\r\n")
		_T("img.rating\t- number of stars\r\n")
		_T("img.size\t- file size in bytes\r\n")
		_T("img.srgb\t- true if sRGB color space, false otherwise\r\n")
		_T("img.subdist\t- subject distance\r\n");

	if (include_help & IAH_TAG_INFO)
		help << _T("img.tags\t- dictionary of image tags\r\n");

	help <<
		_T("img.time\t- creation time stamp (number)\r\n")
		_T("img.w\t- image width\r\n")
		_T("\r\n")
		_T("String functions:\t string.lower, string.upper, string.len, string.sub(str, from, to), string.find(str, what)\r\n")
		_T("\r\n")
		_T("Mathematical functions:\t math.abs math.acos math.asin math.atan math.atan2 math.ceil math.cos math.cosh math.deg math.exp math.floor math.fmod math.frexp math.huge math.ldexp math.log math.log10 math.max math.min math.modf math.pi math.pow math.rad math.random math.randomseed math.sin math.sinh math.sqrt math.tan math.tanh math.mod\r\n")
		_T("\r\n")
		_T("For a complete list of functions and documentation check www.lua.org");

/*
	if (including_tags)
		return String(
			_T("Image attributes:\r\n")
			_T("img.date\t- creation date & time (text)\r\n")
			_T("img.expprog\t- exposure program\r\n")
			_T("img.et\t- exposure time\r\n")
			_T("img.expbias\t- exposure bias\r\n")
			_T("img.ext\t- file extension name\r\n")
			_T("img.fl\t- focal length\r\n")
			_T("img.fl35\t- focal length (35 mm equivalent)\r\n")
			_T("img.flash\t- flash information\r\n")
			_T("img.fn\t- F/Stop (F Number, aperture)\r\n")
			_T("img.fovc\t- field of view crop\r\n")
			_T("img.h\t- image height\r\n")
			_T("img.iso\t- sensitivity\r\n")
			_T("img.lightsrc\t- light source\r\n")
			_T("img.make\t- camera make\r\n")
			_T("img.metmode\t- metering mode\r\n")
			_T("img.model\t- camera model\r\n")
			_T("img.name\t- file name\r\n")
			_T("img.path\t- complete path to the image\r\n")
			_T("img.portrait\t- true if image is in portrait orientation, false if it's in landscape\r\n")
			_T("img.rating\t- number of stars\r\n")
			_T("img.size\t- file size in bytes\r\n")
			_T("img.srgb\t- true if sRGB color space, false otherwise\r\n")
			_T("img.subdist\t- subject distance\r\n")
		_T("img.tags\t- dictionary of image tags\r\n")
			_T("img.time\t- creation time stamp (number)\r\n")
			_T("img.w\t- image width\r\n")
			_T("\r\n")
			_T("String functions:\t string.lower, string.upper, string.len, string.sub(str, from, to), string.find(str, what)\r\n")
			_T("\r\n")
			_T("Mathematical functions:\t math.abs math.acos math.asin math.atan math.atan2 math.ceil math.cos math.cosh math.deg math.exp math.floor math.fmod math.frexp math.huge math.ldexp math.log math.log10 math.max math.min math.modf math.pi math.pow math.rad math.random math.randomseed math.sin math.sinh math.sqrt math.tan math.tanh math.mod\r\n")
			_T("\r\n")
			_T("For a complete list of functions and documentation check www.lua.org")
		);
	else
		return String(
			_T("Image attributes:\r\n")
			_T("img.date\t- creation date & time (text)\r\n")
			_T("img.expprog\t- exposure program\r\n")
			_T("img.et\t- exposure time\r\n")
			_T("img.expbias\t- exposure bias\r\n")
			_T("img.ext\t- file extension name\r\n")
			_T("img.fl\t- focal length\r\n")
			_T("img.fl35\t- focal length (35 mm equivalent)\r\n")
			_T("img.flash\t- flash information\r\n")
			_T("img.fn\t- F/Stop (F Number, aperture)\r\n")
			_T("img.fovc\t- field of view crop\r\n")
			_T("img.h\t- image height\r\n")
			_T("img.iso\t- sensitivity\r\n")
			_T("img.lightsrc\t- light source\r\n")
			_T("img.make\t- camera make\r\n")
			_T("img.metmode\t- metering mode\r\n")
			_T("img.model\t- camera model\r\n")
			_T("img.name\t- file name\r\n")
			_T("img.path\t- complete path to the image\r\n")
			_T("img.portrait\t- true if image is in portrait orientation, false if it's in landscape\r\n")
			_T("img.rating\t- number of stars\r\n")
			_T("img.size\t- file size in bytes\r\n")
			_T("img.srgb\t- true if sRGB color space, false otherwise\r\n")
			_T("img.subdist\t- subject distance\r\n")
			_T("img.time\t- creation time stamp (number)\r\n")
			_T("img.w\t- image width\r\n")
			_T("\r\n")
			_T("String functions:\t string.lower, string.upper, string.len, string.sub(str, from, to), string.find(str, what)\r\n")
			_T("\r\n")
			_T("Mathematical functions:\t math.abs math.acos math.asin math.atan math.atan2 math.ceil math.cos math.cosh math.deg math.exp math.floor math.fmod math.frexp math.huge math.ldexp math.log math.log10 math.max math.min math.modf math.pi math.pow math.rad math.random math.randomseed math.sin math.sinh math.sqrt math.tan math.tanh math.mod\r\n")
			_T("\r\n")
			_T("For a complete list of functions and documentation check www.lua.org")
		);
*/

	return help.str();
}


void CreateScriptEditingFont(CFont& font)
{
	LOGFONT lf;
	HFONT hfont= static_cast<HFONT>(::GetStockObject(DEFAULT_GUI_FONT));
	::GetObject(hfont, sizeof(lf), &lf);
	_tcscpy(lf.lfFaceName, _T("Consolas"));
	lf.lfPitchAndFamily = FIXED_PITCH;
	//lf.lfHeight -= 2; // larger text
	font.DeleteObject();
	font.CreateFontIndirect(&lf);
}

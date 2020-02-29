/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// Columns.cpp: implementation of the Columns class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Columns.h"
#include "PhotoInfo.h"
#include "resource.h"
#include "UniqueLetter.h"
#include "DrawFields.h"
#include <boost/algorithm/string/replace.hpp>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Columns::Columns()
{}

Columns::~Columns()
{}

struct ColDef
{
	const TCHAR* long_name_;
	const TCHAR* short_name_;
	const TCHAR* unit_name_;		// currently not used
	uint16 alignment_;
	uint16 def_width_;
};


static const ColDef g_column_definitions[]=
{
	{ _T("照片文件名"),			_T("照片"),				_T(""),		LVCFMT_LEFT,	105 },
	{ _T("日期和时间"),				_T("日期和时间"),		_T(""),		LVCFMT_LEFT,	115 },
	{ _T("曝光时长"),				_T("曝光时长"),			_T("s"),	LVCFMT_RIGHT,	 65 },
	{ _T("光圈"),						_T("光圈"),				_T(""),		LVCFMT_RIGHT,	 60 },
	{ _T("曝光程序"),			_T("曝光程序"),			_T(""),		LVCFMT_LEFT,	 95 },
	{ _T("曝光补偿"),				_T("曝光补偿"),			_T("EV"),	LVCFMT_RIGHT,	 60 },
	{ _T("测光模式"),				_T("测光模式"),		_T(""),		LVCFMT_LEFT,	100 },
	{ _T("镜头型号"),					_T("镜头型号"),			_T(""),		LVCFMT_LEFT,	100 },
	{ _T("光源"),				_T("光源"),			_T(""),		LVCFMT_LEFT,	 90 },
	{ _T("闪光"),						_T("闪光"),				_T(""),		LVCFMT_LEFT,	 40 },
	{ _T("焦距"),				_T("焦距"),			_T("mm"),	LVCFMT_RIGHT,	 80 },
	{ _T("焦距 (35 mm 等效)"),	_T("焦距 (35 mm)"),		_T("mm"),		LVCFMT_RIGHT,	 80 },
	{ _T("感光度"),					_T("感光度"),			_T(""),		LVCFMT_RIGHT,	 70 },
	{ _T("照片方向"),			_T("方向"),			_T(""),		LVCFMT_LEFT,	 70 },
	{ _T("照片尺寸"),			_T("尺寸"),			_T(""),		LVCFMT_RIGHT,	 75 },
	{ _T("文件大小"),					_T("文件大小"),			_T(""),		LVCFMT_RIGHT,	 55 },
	{ _T("照片路径"),				_T("路径"),					_T(""),		LVCFMT_LEFT,	120 },
	{ _T("相机厂商"),				_T("厂商"),					_T(""),		LVCFMT_LEFT,	100 },
	{ _T("相机型号"),				_T("型号"),				_T(""),		LVCFMT_LEFT,	100 },
	{ _T("图像描述"),			_T("描述"),			_T(""),		LVCFMT_LEFT,	120 },
	{ _T("文件扩展名"),				_T("扩展名"),			_T(""),		LVCFMT_RIGHT,	 40 },
	{ _T("文件名"),					_T("文件名"),			_T(""),		LVCFMT_RIGHT,	115 },
	{ _T("目标距离"),			_T("目标距离"),		_T(""),		LVCFMT_RIGHT,	 60 },
	{ _T("评级 (星)"),				_T("评级"),				_T(""),		LVCFMT_RIGHT,	 60 },
#ifdef _TRUE_EXPOSURE_COL_
	{ _T("真实强度"),				_T("真实强度"),		_T(""),		LVCFMT_RIGHT,	 60 },
#endif

	// GPS
	{ _T("GPS 纬度"),				_T("纬度"),				_T(""),		LVCFMT_RIGHT,	 95 },
	{ _T("GPS 经度"),				_T("经度"),			_T(""),		LVCFMT_RIGHT,	 95 },
	{ _T("GPS 海拔"),				_T("海拔"),				_T(""),		LVCFMT_RIGHT,	 70 },
	{ _T("GPS 时间戳"),				_T("时间戳"),			_T(""),		LVCFMT_RIGHT,	 70 },

	// Canon
	{ _T("微距模式"),					_T("微距"),			_T(""),		LVCFMT_LEFT,	 40 },
	{ _T("定时器"),					_T("定时器"),			_T(""),		LVCFMT_RIGHT,	 60 },
	{ _T("闪光模式"),					_T("闪光模式"),			_T(""),		LVCFMT_LEFT,	 60 },
	{ _T("驱动模式"),					_T("驱动模式"),			_T(""),		LVCFMT_LEFT,	 80 },
	{ _T("对焦模式"),					_T("对焦模式"),			_T(""),		LVCFMT_LEFT,	 80 },
	{ _T("对焦类型"),					_T("对焦类型"),			_T(""),		LVCFMT_LEFT,	 80 },
	{ _T("图像大小"),					_T("图像大小"),			_T(""),		LVCFMT_LEFT,	 70 },
	{ _T("程序"),					_T("程序"),				_T(""),		LVCFMT_LEFT,	 80 },
	{ _T("对比度"),					_T("对比度"),				_T(""),		LVCFMT_LEFT,	 70 },
	{ _T("色彩饱和度"),			_T("饱和度"),			_T(""),		LVCFMT_LEFT,	 70 },
	{ _T("锐度"),					_T("锐度"),			_T(""),		LVCFMT_LEFT,	 70 },
	{ _T("感光度"),					_T("感光度"),					_T(""),		LVCFMT_RIGHT,	 65 },
	{ _T("测光模式"),				_T("测光模式"),		_T(""),		LVCFMT_LEFT,	 80 },
	{ _T("自动对焦点"),			_T("自动对焦点"),				_T(""),		LVCFMT_LEFT,	 70 },
	{ _T("曝光模式"),				_T("曝光模式"),		_T(""),		LVCFMT_LEFT,	 80 },
	{ _T("镜头"),				_T("镜头"),					_T(""),		LVCFMT_RIGHT,	 80 },
	{ _T("白平衡"),				_T("白平衡"),		_T(""),		LVCFMT_LEFT,	 70 },
	{ _T("序号"),			_T("序号"),		_T(""),		LVCFMT_RIGHT,	 50 },
	{ _T("闪光补偿"),					_T("闪光补偿"),			_T(""),		LVCFMT_RIGHT,	 60 },
	{ _T("目标距离"),			_T("目标距离"),		_T(""),		LVCFMT_RIGHT,	 60 },
	{ _T("图像类型"),					_T("图像类型"),			_T(""),		LVCFMT_LEFT,	 60 },
	{ _T("固件版本"),			_T("固件版本"),		_T(""),		LVCFMT_LEFT,	 70 },
	{ _T("主人名字"),					_T("主人名字"),			_T(""),		LVCFMT_LEFT,	 80 },
	{ _T("机身编号"),			_T("机身编号"),		_T(""),		LVCFMT_LEFT,	 70 },

	// Nikon
	{ _T("感光度设置"),				_T("感光度设置"),			_T(""),		LVCFMT_LEFT,	 70 },
	{ _T("颜色模式"),					_T("颜色模式"),			_T(""),		LVCFMT_LEFT,	 60 },
	{ _T("品质"),					_T("品质"),				_T(""),		LVCFMT_LEFT,	 60 },
	{ _T("白平衡"),				_T("白平衡"),		_T(""),		LVCFMT_LEFT,	 60 },
	{ _T("图像锐化"),			_T("图像锐化"),		_T(""),		LVCFMT_LEFT,	 60 },
	{ _T("对焦模式"),					_T("对焦模式"),			_T(""),		LVCFMT_LEFT,	 60 },
	{ _T("闪光设置"),				_T("闪光设置"),		_T(""),		LVCFMT_LEFT,	 60 },
	{ _T("感光度选择"),				_T("感光度选择"),		_T(""),		LVCFMT_LEFT,	 60 },
	{ _T("图像调整"),			_T("图像调整"),		_T(""),		LVCFMT_LEFT,	 60 },
	{ _T("光圈"),					_T("光圈"),				_T(""),		LVCFMT_LEFT,	 60 },
	{ _T("镜头类型"),					_T("镜头类型"),			_T(""),		LVCFMT_LEFT,	 70 },
	{ _T("手动对焦距离"),		_T("MF 距离"),			_T(""),		LVCFMT_RIGHT,	 60 },
	{ _T("数码变焦"),				_T("数码变焦"),			_T(""),		LVCFMT_RIGHT,	 60 },
	{ _T("自动对焦位置"),		_T("AF 位置"),			_T(""),		LVCFMT_LEFT,	 60 },

	// Fuji
	{ _T("MakerNote Version"),			_T("Version"),				_T(""),		LVCFMT_LEFT,	 70 },
	{ _T("品质"),					_T("品质"),				_T(""),		LVCFMT_LEFT,	 70 },
	{ _T("锐度"),					_T("锐度"),			_T(""),		LVCFMT_LEFT,	 70 },
	{ _T("白平衡"),				_T("白平衡"),		_T(""),		LVCFMT_LEFT,	 70 },
	{ _T("颜色"),						_T("颜色"),				_T(""),		LVCFMT_LEFT,	 70 },
	{ _T("色调"),						_T("色调"),					_T(""),		LVCFMT_LEFT,	 70 },
	{ _T("闪光模式"),					_T("闪光模式"),			_T(""),		LVCFMT_LEFT,	 70 },
	{ _T("闪光强度"),				_T("闪光强度"),		_T(""),		LVCFMT_LEFT,	 70 },
	{ _T("微距"),						_T("微距"),				_T(""),		LVCFMT_LEFT,	 70 },
	{ _T("对焦模式"),					_T("对焦模式"),			_T(""),		LVCFMT_LEFT,	 70 },
	{ _T("低速同步"),					_T("低速同步"),			_T(""),		LVCFMT_LEFT,	 70 },
	{ _T("图像模式"),				_T("图像模式"),			_T(""),		LVCFMT_LEFT,	 70 },

	//
	{ _T("连拍/包围曝光"),			_T("连拍/包围曝光"),		_T(""),		LVCFMT_LEFT,	 70 },

	//
	{ _T("模糊警示"),				_T("模糊警示"),			_T(""),		LVCFMT_LEFT,	 70 },
	{ _T("对焦警示"),				_T("对焦警示"),		_T(""),		LVCFMT_LEFT,	 70 },
	{ _T("自动曝光警示"),					_T("自动曝光警示"),			_T(""),		LVCFMT_LEFT,	 70 },

	// Olympus
	{ _T("特殊模式"),				_T("特殊模式"),			_T(""),		LVCFMT_LEFT,	 90 },
	{ _T("JPEG 品质"),				_T("JPEG 品质"),			_T(""),		LVCFMT_LEFT,	 70 },
	{ _T("微距模式"),					_T("微距"),				_T(""),		LVCFMT_LEFT,	 60 },
	{ _T("数码变焦"),				_T("数码变焦"),			_T(""),		LVCFMT_LEFT,	 65 },
	{ _T("软件版本"),			_T("软件版本"),		_T(""),		LVCFMT_LEFT,	 60 },
	{ _T("图像信息"),				_T("图像信息"),			_T(""),		LVCFMT_LEFT,	 70 },
	{ _T("相机编号"),					_T("相机编号"),			_T(""),		LVCFMT_LEFT,	 70 }

};


inline size_t PredefCount()
{
	return array_count(g_column_definitions);
}


const TCHAR* Columns::Name(int index, bool long_form/*= true*/) const
{
	ASSERT(index >= 0);

	if (index < 0)
		return  _T("");

	if (index < PredefCount())
	{
		return long_form ? g_column_definitions[index].long_name_ : g_column_definitions[index].short_name_;
//		int rsrc_index= IDS_COLUMN_01 + index * 2;
//		return RString(long_form ? rsrc_index : rsrc_index + 1);
	}
	else if (index >= COL_METADATA_SET_START && index <= COL_METADATA_SET_END)
	{
		// XMP metadata field name
		return ::GetXmpDataFieldName(index - COL_METADATA_SET_START);
	}

	index -= COL_CUSTOM_SET_START;

	if (index >= 0 && index < custom_.size())
		return custom_[index].caption_.c_str();

	return _T("");
}


const TCHAR* Columns::ShortName(int index) const
{
	return Name(index, false);
}


const TCHAR* Columns::Abbreviation(int index) const
{
	switch (index)
	{
	case COL_PHOTO_NAME:
	case COL_DATE_TIME:	// these two are obvious without a label
		return _T("");
	case COL_FNUMBER:
		return _T("A");	// aperture
	case COL_EXP_TIME:
		return _T("S");	// shatter speed
	case COL_FOCAL_LENGTH:
		return _T("FL");
	case COL_FOCAL_LENGTH_35MM:
		return _T("FL(35)");
	case COL_ISO:
		return _T("ISO");

	default:
		return ShortName(index);
	}
}


int Columns::Alignment(int index) const
{
	ASSERT(index >= 0);

	if (index < PredefCount())
		return g_column_definitions[index].alignment_;

	if (index <= COL_METADATA_SET_END)
		return LVCFMT_LEFT;	// file info columns

	return LVCFMT_RIGHT;	// custom columns
}


int Columns::DefaultWidth(int index) const
{
	ASSERT(index >= 0);

	if (index < PredefCount())
		return g_column_definitions[index].def_width_;

	return 60;	// file info or custom columns
}


int Columns::MaxCount()
{
	return static_cast<int>(PredefCount() + GetCount(METADATA) + GetCount(CUSTOM));
}


int Columns::GetLimit(Set set)
{
	switch (set)
	{
	case COMMON:
		return COL_GENERAL_SET_END;
	case CANON:
		return COL_CANON_SET_END;
	case NIKON:
		return COL_NIKON_SET_END;
	case FUJI:
		return COL_FUJI_SET_END;
	case OLYMPUS:
		return COL_OLY_SET_END;
	//case SONY:
	//	return 0;
	case METADATA:
		return COL_METADATA_SET_END;
	case CUSTOM:
		return COL_CUSTOM_SET_END;
	default:
		ASSERT(false);
		return 0;
	}
}


int Columns::GetStart(Columns::Set set)
{
	switch (set)
	{
	case Columns::COMMON:
		return COL_GENERAL_SET_START;
	case Columns::CANON:
		return COL_CANON_SET_START;
	case Columns::NIKON:
		return COL_NIKON_SET_START;
	case Columns::FUJI:
		return COL_FUJI_SET_START;
	case Columns::OLYMPUS:
		return COL_OLY_SET_START;
	//case SONY:
	//	return 0;
	case METADATA:
		return COL_METADATA_SET_START;
	case Columns::CUSTOM:
		return COL_CUSTOM_SET_START;
	default:
		ASSERT(false);
		return 0;
	}
}


int Columns::GetCount(Set set)
{
	switch (set)
	{
	case COMMON:
		return COL_GENERAL_SET_END + 1;
	case CANON:
		return COL_CANON_SET_END - COL_CANON_SET_START + 1;
	case NIKON:
		return COL_NIKON_SET_END - COL_NIKON_SET_START + 1;
	case FUJI:
		return COL_FUJI_SET_END - COL_FUJI_SET_START + 1;
	case OLYMPUS:
		return COL_OLY_SET_END - COL_OLY_SET_START + 1;
	//case SONY:
	//	return 0;
	case METADATA:
		return COL_METADATA_SET_END - COL_METADATA_SET_START + 1;
	case CUSTOM:
		return COL_CUSTOM_SET_END - COL_CUSTOM_SET_START + 1;
	default:
		ASSERT(false);
		return 0;
	}
}


// returns true if data for a column 'index' is present in 'inf' photo
//
bool Columns::IsDataPresent(int index, const PhotoInfo& inf) const
{
	ASSERT(index >= 0 && index < MaxCount() || index >= COL_CUSTOM_SET_START && index <= COL_CUSTOM_SET_END);

	if (index < 0)
		return false;
	else if (index <= GetLimit(COMMON))
	{
		if (index >= COL_GPS_FIRST_ITEM)
			return inf.GetGpsData() != 0;
		return true;
	}
	else if (index >= COL_METADATA_SET_START && index <= COL_METADATA_SET_END)
		return inf.HasMetadata();
	else if (index >= COL_CUSTOM_SET_START && index <= COL_CUSTOM_SET_END)
		return true;
	else if (inf.GetMakerNote() == 0)
		return false;
	else
		return inf.GetMakerNote()->IsDataPresent(index);

	return true;
}

//extern double log2(double x);


double GetTrueIntensity(const PhotoInfo& photo)
{
	//CG = FOV35*FL35*pi*(1/F#/2)^2*ISO*ET
	
	//FL35 = FL * FOVcrop
	//FOV35 = (a/(FL35-b))^n
	//(with a=3e3, b=10, and n=.72 as I said before)

//	double fl35= photo.GetFocalLength35mm();
	double fn= photo.GetFStop().Double();
	double iso= photo.GetISOSpeed();
	double et= photo.GetExposureTime().Double();

	//double b= 10.0;
	//double a= 3.0e3;
	//double n= 0.72;

	if (/*fl35 > 0.0 &&*/ fn > 0.0 && photo.IsISOSpeedValid() && et > 0.0 && iso > 0.0)
	{
		double x= (1.0 / fn / 2.0);
		double cg= M_PI * x * x * iso * et;

		//double fov35= pow(a / (fl35 - b), n);
		//double x= (1.0 / fn / 2.0);
		//double cg= fov35 * fl35 * M_PI * x * x * iso;

		return cg;	// camera gain
	}
	else
		return 0.0;
}


void Columns::GetInfo(String& out, int index, const PhotoInfo& inf) const
{
	ASSERT(index >= 0 && index < PredefCount() || index >= COL_CUSTOM_SET_START && index <= COL_CUSTOM_SET_END ||
		index >= COL_METADATA_SET_START && index <= COL_METADATA_SET_END);

	if (!IsDataPresent(index, inf))
	{
		out = _T("-");
		return;
	}

	switch (index)
	{
	case COL_PHOTO_NAME:
		out = inf.GetName();
		break;
	case COL_DATE_TIME:
		out = inf.DateTimeStr();
		break;
	case COL_EXP_TIME:
		out = inf.ExposureTime();
		break;
	case COL_FNUMBER:
		out = inf.FNumber();
		break;
	case COL_EXP_PROG:
		out = inf.ExposureProgram();
		break;
	case COL_EXP_BIAS:
		out = inf.ExposureBias();
//TEST		out = inf.index_.AsString();
		break;
	case COL_MET_MODE:
		out = inf.MeteringMode();
		break;
	case COL_LENS_MODEL:
		out = inf.GetLensModel();
		break;
	case COL_LIGHT_SRC:
		out = inf.LightSource();
		break;
	case COL_FLASH:
		out = inf.Flash();
		break;
	case COL_FOCAL_LENGTH:
		out = inf.FocalLength();
		break;
	case COL_FOCAL_LENGTH_35MM:
		out = inf.FocalLength35mm();
		break;
	case COL_ISO:
		out = inf.ISOSpeed();
		break;
	case COL_ORIENTATION:
		out = inf.Orientation();
		break;
	case COL_DIMENSIONS:
		out = inf.Size();
		break;
	case COL_FILE_SIZE:
		out = inf.FileSize();
		break;
	case COL_PATH:
		out = inf.GetOriginalPath();
		break;
	case COL_MAKE:
		out = inf.GetMake();
		break;
	case COL_MODEL:
		out = inf.GetModel();
		break;

	case COL_DESCRIPTION:
		inf.Description(out);
		break;
	case COL_FILE_TYPE_NAME:
		out = inf.GetFileTypeName();
		break;
	case COL_FILE_NAME:
		out = inf.GetNameAndExt();
		break;
//	case COL_COMPR_BPP:
		// TODO
//	{ "Compressed Bits Per Pixel",		"Compressed BPP",		LVCFMT_RIGHT,	 40 },
//		break;
	case COL_SUBJECT_DIST:
		{
			extern String SubjectDistance(Rational dist);
			out = SubjectDistance(inf.GetSubjectDistance());
		}
//	{ "Subject Distance",				"Subject Distance",		LVCFMT_RIGHT,	 60 },
		break;

	case COL_RATE:
		{
			int stars= inf.GetRating();
			if (stars != 0)
			{
				TCHAR buf[64];
				out = _itot(stars, buf, 10);
			}
			else
				out = _T("-");
		}
		break;

#ifdef _TRUE_EXPOSURE_COL_
	case COL_TRUE_EXPOSURE:
		{
			double ti= GetTrueIntensity(inf);
			if (ti > 0.0)
			{
				oStringstream ost;
				//ost.precision(3);
				ost << ti;
				out = ost.str();
			}
			else
				out = _T("-");
		}
		break;
#endif

	case COL_GPS_LATITUDE:
		out = inf.GetGpsData()->GetLatitude();
		break;
	case COL_GPS_LONGITUDE:
		out = inf.GetGpsData()->GetLongitude();
		break;
	case COL_GPS_ALTITUDE:
		out = inf.GetGpsData()->GetAltitude();
		break;
	case COL_GPS_TIME_STAMP:
		out = inf.GetGpsData()->GPSTimeStamp();
		break;

	default:
		if (index >= COL_METADATA_SET_START && index <= COL_METADATA_SET_END)
		{
			ASSERT(inf.HasMetadata());	// IsDataPresent should filter out img w/o metadata

			out = ::GetXmpDataField(*inf.GetMetadata(), index - COL_METADATA_SET_START);
			// no multiple lines
			boost::algorithm::replace_all(out, _T("\xd\xa"), _T("  "));

			break;
		}
		else if (index >= COL_CUSTOM_SET_START && index <= COL_CUSTOM_SET_END)
		{
			double num= 0.0;
			switch (custom_.CalcValue(index - COL_CUSTOM_SET_START, inf, num, out))
			{
			case CustomColumns::Text:
				// no op; out already contains the result
				break;

			case CustomColumns::Number:
				{
					TCHAR buf[128];
					_stprintf_s(buf, array_count(buf), _T("%.14g"), num);
					out = buf;
				}
				break;

			default:
				out = _T("<error>");
				break;
			}
		}
		else
			inf.GetMakerNote()->GetInfo(index, out);
		break;
	}
}


size_t NumLength(const TCHAR*& p, int& significant)
{
	const TCHAR* q= p;
	significant = 0;
/*
	if (*p >= '1' && *p <= '9')
		++significant;

	while (*p >= '0' && *p <= '9')
	{
		++significant;
		p++;
	}
*/
	// skip leading zeros; not significant as far as number is concerned
	while (*p == '0')
		p++;

	while (*p >= '0' && *p <= '9')
	{
		++significant;
		p++;
	}

	return p - q;
}


// compare numbers without constructing them
static int CompareNumbers(const TCHAR*& p, const TCHAR*& q)
{
	ASSERT(*p >= '0' && *p <= '9');
	ASSERT(*q >= '0' && *q <= '9');
	const TCHAR* start1= p;
	const TCHAR* start2= q;

	int significant1= 0;
	size_t len1= NumLength(p, significant1);

	int significant2= 0;
	size_t len2= NumLength(q, significant2);

/*	if (significant1 <= 0 && significant2 <= 0)
		return 0;
	else*/ if (significant1 < significant2)
		return -1;
	else if (significant1 > significant2)
		return 1;

	ASSERT(significant1 == significant2);
	int start= -significant1;

	for (int i= start; i < 0; ++i)
	{
		int cmp= static_cast<int>(p[i]) - static_cast<int>(q[i]);

		if (cmp)
			return cmp;
	}

	return len1 < len2;
}


// smart name comparison (case insensitive); it treats all consecutive digits as a single character
// with value equal to the number
//
extern bool NameIsLess(const TCHAR* p, const TCHAR* q)
{
	for ( ; *p && *q; ++p, ++q)
	{
		if (*p == *q)
			continue;

		unsigned int c1= _totlower(*p);
		unsigned int c2= _totlower(*q);

		if (c1 == c2)
			continue;

		if (c1 >= '0' && c1 <= '9' && c2 >= '0' && c2 <= '9')
		{
			int cmp= CompareNumbers(p, q);
			if (cmp < 0)
				return true;
			else if (cmp > 0)
				return false;
			else
				continue;
		}

		return c1 < c2;
	}

	if (*p == 0)
	{
		if (*q == 0)
			return false;
		else
			return true;
	}
	else
	{
		if (*q == 0)
			return false;
		else
		{ ASSERT(false); }
//			return true;
	}

	ASSERT(false);
	return false;
}


extern bool StringILess(const TCHAR* p, const TCHAR* q)
{
	for ( ; *p && *q; ++p, ++q)
	{
		if (*p == *q)
			continue;

		unsigned int c1= _totlower(*p);
		unsigned int c2= _totlower(*q);

		if (c1 == c2)
			continue;

		return c1 < c2;
	}

	if (*p == 0)
	{
		if (*q == 0)
			return false;
		else
			return true;
	}
	else
	{
		if (*q == 0)
			return false;
		else
		{ ASSERT(false); }
//			return true;
	}

	ASSERT(false);
	return false;
}


bool Columns::Less(int column, const PhotoInfo& info1, const PhotoInfo& info2) const
{
	// p1 < p2

	bool present1= IsDataPresent(column, info1);
	bool present2= IsDataPresent(column, info2);

	if (!present1 && !present2)
		return false;
	if (present1 && !present2)
		return true;	// present entries first (before empty ones)
	if (present2 && !present1)
		return false;

	switch (column)
	{
	case COL_PHOTO_NAME:	// file name
		//return info1.GetName() < info2.GetName();
		return NameIsLess/*StringILess*/(info1.GetName().c_str(), info2.GetName().c_str());

	case COL_DATE_TIME:		// date & time
		return info1.GetDateTime() < info2.GetDateTime();
//		return info1.GetPhotoExactTime() < info2.GetPhotoExactTime();
		//info1.tm_date_ < info2.tm_date_; //info1.date_time_ < info2.date_time_;

	case COL_EXP_TIME:		// exposure time
		return info1.ExposureTimeValue() < info2.ExposureTimeValue();

	case COL_FNUMBER:		// f-number
		return info1.FNumberValue() < info2.FNumberValue();

	case COL_EXP_PROG:		// exp. program
		return info1.GetExposureProgram() < info2.GetExposureProgram();

	case COL_EXP_BIAS:		// exp. bias
//TEST		return info1.index_.Feature() < info2.index_.Feature();
		return info1.GetExposureBias() > info2.GetExposureBias();

	case COL_MET_MODE:		// metering mode
		return info1.GetMeteringMode() < info2.GetMeteringMode();

	case COL_LENS_MODEL:	// lens model
		return info1.GetLensModel() < info2.GetLensModel();

	case COL_LIGHT_SRC:		// light source
		return info1.GetLightSource() < info2.GetLightSource();

	case COL_FLASH:			// flash
		return info1.GetFlash() < info2.GetFlash();

	case COL_FOCAL_LENGTH:		// focal length
		return info1.GetFocalLength() < info2.GetFocalLength();

	case COL_FOCAL_LENGTH_35MM:
		return info1.GetFocalLength35mm() < info2.GetFocalLength35mm();

	case COL_ISO:			// ISO speed
		return info1.GetISOSpeed() < info2.GetISOSpeed();

	case COL_ORIENTATION:	// orientation
		return info1.OrientationField() < info2.OrientationField();

	case COL_DIMENSIONS:	// dimensions
		return info1.GetWidth() * info1.GetHeight() < info2.GetWidth() * info2.GetHeight();

	case COL_FILE_SIZE:		// file size
		return info1.GetFileSize() < info2.GetFileSize();

	case COL_PATH:			// path
		return info1.GetOriginalPath() < info2.GetOriginalPath();

	case COL_MAKE:			// make
		{
			int cmp= info1.GetMake().compare(info2.GetMake());
			if (cmp != 0)
				return cmp < 0;
			return info1.GetModel() < info2.GetModel();
		}
	case COL_MODEL:			// model
		return info1.GetModel() < info2.GetModel();

	case COL_DESCRIPTION:
		return info1.photo_desc_ < info2.photo_desc_;
	case COL_FILE_TYPE_NAME:
		return info1.GetFileTypeName() < info2.GetFileTypeName();
	case COL_FILE_NAME:
		return info1.GetNameAndExt() < info2.GetNameAndExt();
//	case COL_COMPR_BPP:
//		return info1.compressed_bpp_ < info2.compressed_bpp_;
	case COL_SUBJECT_DIST:
		return info1.GetSubjectDistance() < info2.GetSubjectDistance();

	case COL_RATE:
		return info1.GetRating() < info2.GetRating();
#ifdef _TRUE_EXPOSURE_COL_
	case COL_TRUE_EXPOSURE:
		return GetTrueIntensity(info1) < GetTrueIntensity(info2);
#endif

	// GPS ===================

	case COL_GPS_LATITUDE:
		return info1.GetGpsData()->LessLatitude(*info2.GetGpsData());
		break;
	case COL_GPS_LONGITUDE:
		return info1.GetGpsData()->LessLongitude(*info2.GetGpsData());
		break;
	case COL_GPS_ALTITUDE:
		return info1.GetGpsData()->LessAltitude(*info2.GetGpsData());
		break;
	case COL_GPS_TIME_STAMP:
		return info1.GetGpsData()->LessTimeStamp(*info2.GetGpsData());
		break;

	default:
		if (column >= COL_METADATA_SET_START && column <= COL_METADATA_SET_END)
		{
			ASSERT(info1.HasMetadata());	// IsDataPresent should
			ASSERT(info2.HasMetadata());	// filter out img w/o metadata

			const String& s1= ::GetXmpDataField(*info1.GetMetadata(), column - COL_METADATA_SET_START);
			const String& s2= ::GetXmpDataField(*info2.GetMetadata(), column - COL_METADATA_SET_START);
			return s1 < s2;
		}
		else if (column >= COL_CUSTOM_SET_START && column <= COL_CUSTOM_SET_END)
			return custom_.Less(column - COL_CUSTOM_SET_START, info1, info2);
		else
			return info1.GetMakerNote()->Less(column, *info2.GetMakerNote());
	}
}


bool Columns::GetPopupMenu(CMenu& menu, std::vector<uint16>& selected_columns) const
{
	Set groups[]= { COMMON, CANON, NIKON, FUJI, OLYMPUS, METADATA, CUSTOM };
	static const TCHAR* group_names[]=
	{ 0, _T("佳能"), _T("尼康"), _T("富士"), _T("奥林巴斯"), _T("文件信息"), _T("自定义列") };

	UniqueLetter ShortCutsMain;
	UniqueLetter ShortCutsPopup;
	UniqueLetter* short_cuts= &ShortCutsMain;

	int index= 0;
	int item= 0;

	for (int n= 0; n < sizeof(groups) / sizeof groups[0]; ++n)
	{
		CMenu popup;
		CMenu* insert= &menu;

		if (n > 0)
		{
			if (!popup.CreatePopupMenu())
				return false;

			String name= group_names[n];
			ShortCutsMain.SelectUniqueLetter(name);
			menu.AppendMenu(MF_POPUP | MF_STRING, UINT_PTR(popup.m_hMenu), name.c_str());

			insert = &popup;
			index = 0;
	
			short_cuts = &ShortCutsPopup;
			ShortCutsPopup.Reset();
		}

		int first_item= GetStart(groups[n]);
		int item= first_item;
		for (int i= 0; i < GetCount(groups[n]); ++i, ++index, ++item)
		{
			String item_name= Name(item, true);
			//item_name.Replace(_T("&"), _T("&&"));
			short_cuts->SelectUniqueLetter(item_name);
			insert->InsertMenu(index, MF_BYPOSITION | MF_STRING, item + 1, item_name.c_str());

		}

		const size_t cols= selected_columns.size();
		for (size_t col= 0; col < cols; ++col)
		{
			int selected= selected_columns[col];
			if (selected >= first_item && selected < item)
			{
				insert->CheckMenuItem(selected + 1, MF_BYCOMMAND | MF_CHECKED);
			}
		}

		if (insert == &popup)
			popup.Detach();

		if (n == 0)
			insert->AppendMenu(MF_BYPOSITION | MF_SEPARATOR);
	}

	return true;
}


// prepare text in the format accepted by DrawFields class using given photo and selected columns
//
String Columns::GetStatusText(const std::vector<uint16>& selected_columns, const PhotoInfo& photo, bool includeNames) const
{
	oStringstream ost;
	const TCHAR* SEP= _T("\t");

	const size_t fields= selected_columns.size();

	for (size_t i= 0; i < fields; ++i)
	{
		int index= selected_columns[i];
		if (includeNames)
			ost << DrawFields::LABEL << ShortName(index) << _T(": ");
		String value;
		GetInfo(value, index, photo);
		ost << DrawFields::VALUE << value << SEP;
	}

	String str= ost.str();

	// lame trim right
	while (str.size() > 0 && str[str.size() - 1] == *SEP)
		str.resize(str.size() - 1);

	return str;
}


bool Columns::Less(int first_column, int second_column, const PhotoInfo& info1, const PhotoInfo& info2, const PhotoInfo& info1a, const PhotoInfo& info2a) const
{
	if (Less(first_column, info1, info2))
		return true;

	if (Less(first_column, info2, info1))
		return false;

	// photos are equivalent in ragard to the first sorting key; use secondary key:

	return Less(second_column, info1a, info2a);
}


const CustomColumns& Columns::GetCustomColumns() const
{
	return custom_;
}


void Columns::SetCustomColumns(const CustomColumns& columns)
{
	custom_.assign(columns);
}

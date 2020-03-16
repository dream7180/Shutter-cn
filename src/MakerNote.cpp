/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// MakerNote.cpp: implementation of the MakerNote class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MakerNote.h"
#include "OutputStr.h"
#include "PhotoInfo.h"
#include "Columns.h"
#include "ExifTags.h"
#include "TaggedValues.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////

MakerNote::~MakerNote()
{}


void MakerNote::CompletePhotoInfo(PhotoInfo& photo) const
{}


String& MakerNote::GetInfo(int index, String& out) const
{
	out = _T("-");
	return out;
}


int MakerNote::Compare(int index, const MakerNote& note) const
{
	bool present1= IsDataPresent(index);
	bool present2= note.IsDataPresent(index);

	if (!present1 && !present2)
		return 0;
	if (present1 && !present2)
		return -1;	// present entries first (before empty ones)
	if (present2 && !present1)
		return 1;

//	ASSERT(typeid(note) == typeid(*this));

	return CompareNotes(index, note);
}


bool MakerNote::Less(int index, const MakerNote& note) const
{
	bool present1= IsDataPresent(index);
	bool present2= note.IsDataPresent(index);

	if (!present1 && !present2)
		return false;
	if (present1 && !present2)
		return true;	// present entries first (before empty ones)
	if (present2 && !present1)
		return false;

	return LessNote(index, note);
}


bool MakerNote::IsDataPresent(int index) const
{
	return false;
}


void MakerNote::RecordInfo(uint16 tag, const Data& val, OutputStr& output)
{
	output.RecordInfo(tag | OutputStr::EXTRA_TAG, TagName(tag), val, TagValue(tag, val).c_str());
}

void MakerNote::RecordTag(uint32 tag, const TCHAR* name, const TCHAR* value, OutputStr& output)
{
	output.RecordInfo(tag | OutputStr::EXTRA_TAG, name, value, value);
}

void MakerNote::RecordTag(const TCHAR* name, const TCHAR* value, OutputStr& output)
{
	output.RecordInfo(0, name, value, value);
}

void MakerNote::RecordInfo(uint32 tag, const TCHAR* name, const Data& val, OutputStr& output)
{
	output.RecordInfo(tag | OutputStr::EXTRA_TAG, name, val);
}

//////////////////////////////////////////////////////////////////////

NikonNote::NikonNote(bool new_format/*= true*/) : new_format_(new_format)
{}


int NikonNote::CompareNotes(int index, const MakerNote& maker_note) const
{
	const NikonNote& note= static_cast<const NikonNote&>(maker_note);

	switch (index)
	{
	case COL_NIK_ISO_SET:		return _tcsicmp(data_.iso_setting_.c_str(), note.data_.iso_setting_.c_str());
	case COL_NIK_COLOR_MODE:	return _tcsicmp(data_.color_mode_.c_str(), note.data_.color_mode_.c_str());
	case COL_NIK_QUALITY:		return _tcsicmp(data_.quality_.c_str(), note.data_.quality_.c_str());
	case COL_NIK_WHITE_BAL:		return _tcsicmp(data_.white_balance_.c_str(), note.data_.white_balance_.c_str());
	case COL_NIK_IMG_SHARP:		return _tcsicmp(data_.image_sharpening_.c_str(), note.data_.image_sharpening_.c_str());
	case COL_NIK_FOCUS_MODE:	return _tcsicmp(data_.focus_mode_.c_str(), note.data_.focus_mode_.c_str());
	case COL_NIK_FLASH_SET:		return _tcsicmp(data_.flash_setting_.c_str(), note.data_.flash_setting_.c_str());
	case COL_NIK_ISO_SEL:		return _tcsicmp(data_.iso_selection_.c_str(), note.data_.iso_selection_.c_str());
	case COL_NIK_IMG_ADJ:		return _tcsicmp(data_.image_adjustment_.c_str(), note.data_.image_adjustment_.c_str());
	case COL_NIK_ADAPTER:		return _tcsicmp(data_.adapter_.c_str(), note.data_.adapter_.c_str());
	case COL_NIK_LENS_TYPE:		return _tcsicmp(data_.lens_type_.c_str(), note.data_.lens_type_.c_str());
	case COL_NIK_MF_DIST:		return _tcsicmp(data_.MF_distance_.c_str(), note.data_.MF_distance_.c_str());
	case COL_NIK_DIGI_ZOOM:		return _tcsicmp(data_.digital_zoom_.c_str(), note.data_.digital_zoom_.c_str());
	case COL_NIK_AF_POS:		return _tcsicmp(data_.AF_position_.c_str(), note.data_.AF_position_.c_str());

	default:
		ASSERT(false);
		return 0;
	}
}


bool NikonNote::LessNote(int index, const MakerNote& maker_note) const
{
	const NikonNote& note= static_cast<const NikonNote&>(maker_note);

	switch (index)
	{
	case COL_NIK_ISO_SET:		return data_.iso_setting_		< note.data_.iso_setting_;
	case COL_NIK_COLOR_MODE:	return data_.color_mode_		< note.data_.color_mode_;
	case COL_NIK_QUALITY:		return data_.quality_			< note.data_.quality_;
	case COL_NIK_WHITE_BAL:		return data_.white_balance_		< note.data_.white_balance_;
	case COL_NIK_IMG_SHARP:		return data_.image_sharpening_	< note.data_.image_sharpening_;
	case COL_NIK_FOCUS_MODE:	return data_.focus_mode_		< note.data_.focus_mode_;
	case COL_NIK_FLASH_SET:		return data_.flash_setting_		< note.data_.flash_setting_;
	case COL_NIK_ISO_SEL:		return data_.iso_selection_		< note.data_.iso_selection_;
	case COL_NIK_IMG_ADJ:		return data_.image_adjustment_	< note.data_.image_adjustment_;
	case COL_NIK_ADAPTER:		return data_.adapter_			< note.data_.adapter_;
	case COL_NIK_LENS_TYPE:		return data_.lens_type_			< note.data_.lens_type_;
	case COL_NIK_MF_DIST:		return data_.MF_distance_		< note.data_.MF_distance_;
	case COL_NIK_DIGI_ZOOM:		return data_.digital_zoom_		< note.data_.digital_zoom_;
	case COL_NIK_AF_POS:		return data_.AF_position_		< note.data_.AF_position_;

	default:
		ASSERT(false);
		return false;
	}
}


String& NikonNote::GetInfo(int index, String& out) const
{
	out = _T("-");

	switch (index)
	{
	case COL_NIK_ISO_SET:		out = data_.iso_setting_;		break;
	case COL_NIK_COLOR_MODE:	out = data_.color_mode_;		break;
	case COL_NIK_QUALITY:		out = data_.quality_;			break;
	case COL_NIK_WHITE_BAL:		out = data_.white_balance_;		break;
	case COL_NIK_IMG_SHARP:		out = data_.image_sharpening_;	break;
	case COL_NIK_FOCUS_MODE:	out = data_.focus_mode_;		break;
	case COL_NIK_FLASH_SET:		out = data_.flash_setting_;		break;
	case COL_NIK_ISO_SEL:		out = data_.iso_selection_;		break;
	case COL_NIK_IMG_ADJ:		out = data_.image_adjustment_;	break;
	case COL_NIK_ADAPTER:		out = data_.adapter_;			break;
	case COL_NIK_LENS_TYPE:		out = data_.lens_type_;			break;
	case COL_NIK_MF_DIST:		out = data_.MF_distance_;		break;
	case COL_NIK_DIGI_ZOOM:		out = data_.digital_zoom_;		break;
	case COL_NIK_AF_POS:		out = data_.AF_position_;		break;
	default:
		ASSERT(false);
		break;
	}

	return out;
}


bool NikonNote::IsDataPresent(int index) const
{
	return index >= COL_NIKON_SET_START && index <= COL_NIKON_SET_END;
}


String NikonNote::TagName(uint16 tag)
{
	String name;

	switch (tag)
	{
	case 0x01: name = _T("紧固件版本");				break;
	case 0x02: name = _T("感光度");					break;
	case 0x03: name = _T("色彩模式");						break;
	case 0x04: name = _T("JPEG 品质");					break;
	case 0x05: name = _T("白平衡");					break;
	case 0x06: name = _T("图像锐化");				break;
	case 0x07: name = _T("对焦模式");						break;
	case 0x08: name = _T("闪光灯设置");					break;
	case 0x09: name = _T("闪光模式");						break;
	case 0x0b: name = _T("白平衡偏差");				break;
	case 0x0c: name = _T("白平衡系数");		break;
	case 0x0d: name = _T("程序转换");					break;
	case 0x0e: name = _T("曝光差异"); 			break;
	case 0x0f: name = _T("ISO 选择");					break;
	case 0x11: name = _T("缩略图 IFD 偏移");			break;
	case 0x12: name = _T("闪光补偿");				break;
	case 0x13: name = _T("请求的感光度");			break;
	case 0x16: name = _T("照片图角坐标");		break;
	case 0x17: name = _T("闪光灯曝光补偿");	break;
	case 0x18: name = _T("使用闪光灯包围补偿");	break;
	case 0x19: name = _T("使用自动曝光包围补偿");	break;
	case 0x1a: name = _T("图像处理");				break;
	case 0x1b: name = _T("告诉裁切");				break;
	case 0x1c: name = _T("曝光微调");			break;
	case 0x1d: name = _T("序列号");					break;
	case 0x1e: name = _T("色彩空间");					break;
	case 0x1f: name = _T("防抖信息");		break;
	case 0x20: name = _T("图像认证");			break;
	case 0x22: name = _T("使用 D-Lighting");				break;
	case 0x23: name = _T("图像控制");				break;
	case 0x24: name = _T("时区");						break;
	case 0x25: name = _T("扩展 ISO");					break;
	case 0x2a: name = _T("虚光控制");				break;

	case 0x80: name = _T("图像调整");				break;
	case 0x81: name = _T("色调补偿");				break;
	case 0x82: name = _T("光圈");						break;
	case 0x83: name = _T("镜头型号");						break;
	case 0x84: name = _T("镜头");							break;
	case 0x85: name = _T("手动对焦距离");			break;
	case 0x86: name = _T("数码变焦");					break;
	case 0x87: name = _T("闪光灯型号");				break;
	case 0x88: name = _T("自动对焦位置");				break;
	case 0x89: name = _T("包围曝光拍摄模式");		break;
	case 0x8a: name = _T("自动包围曝光释放");			break;
	case 0x8b: name = _T("镜头光圈");					break;
	case 0x8c: name = _T("对比度曲线");					break;
	case 0x8d: name = _T("色彩模式");						break;
	case 0x8f: name = _T("场景模式");						break;
	case 0x90: name = _T("光照类型");					break;
	case 0x91: name = _T("拍摄信息");						break;
	case 0x92: name = _T("色度调整");					break;
	case 0x93: name = _T("NEF 压缩");				break;
	case 0x94: name = _T("饱和度调整");			break;
	case 0x95: name = _T("降噪");				break;
	case 0x96: name = _T("Raw 数据换算");				break;
	case 0x97: name = _T("色彩平衡");					break;
	case 0x98: name = _T("镜头数据");						break;
	case 0x99: name = _T("Raw 图像转换器");			break;
	case 0x9a: name = _T("感光器像素大小");				break;
//	case 0x9b:
	case 0x9c: name = _T("场景辅助");					break;
	case 0x9e: name = _T("润色历史");				break;
//	case 0x9f:
	case 0xa0: name = _T("序列号");					break;
	case 0xa2: name = _T("图像数据大小");				break;
	case 0xa5: name = _T("图像数量");					break;
	case 0xa6: name = _T("删除的图像数量");			break;
	case 0xa7: name = _T("快门数");			break;
	case 0xa8: name = _T("闪光灯信息");						break;
	case 0xa9: name = _T("图像优化");				break;
	case 0xaa: name = _T("饱和度");						break;
	case 0xab: name = _T("数字处理程序");			break;
	case 0xac: name = _T("防抖");			break;
	case 0xad: name = _T("AF 响应");					break;
	case 0xb0: name = _T("多重曝光");				break;
	case 0xb1: name = _T("高感光度降噪");		break;
	case 0xb3: name = _T("色调效果");					break;
	case 0xb7: name = _T("AF 信息");						break;
	case 0xb8: name = _T("文件信息");						break;
	case 0xe00: name = _T("PrintIM");						break;
	case 0xe01: name = _T("捕获数据");					break;
	case 0xe09: name = _T("捕获版本");				break;
	case 0xe0e: name = _T("捕获偏移");				break;
	case 0xe10: name = _T("扫描 IFD");						break;
	case 0xe1d: name = _T("ICC 配置");					break;
	case 0xe1e: name = _T("捕获输出");				break;

	default: name = _T("未知");							break;
	}

	return name;
}

String NikonNote::TagValue(uint16 tag, const Data& val)
{
	return val.AsString();
/*	string val;

	switch (tag)
	{
	case 0x03:
		switch (val.GetData())
		{
		case 1: val = _T("VGA Basic");	break;
		case 2: val = _T("VGA Normal");	break;
		case 3: val = _T("VGA Fine");	break;
		case 4: val = _T("SXGA Basic");	break;
		case 5: val = _T("SXGA Normal");	break;
		case 6: val = _T("SXGA Fine");	break;
		default: val = _T("(?)");		break;
		}
		return val;

	case 0x04:
		switch (val.GetData())
		{
		case 1: val = _T("Color");		break;
		case 2: val = _T("Monochrome");	break;
		default: val = _T("(?)");		break;
		}
		return val;

	case 0x05:
		switch (val.GetData())
		{
		case 0: val = _T("Normal");		break;
		case 1: val = _T("Bright+");		break;
		case 2: val = _T("Bright-");		break;
		case 3: val = _T("Contrast+");	break;
		case 4: val = _T("Contrast-");	break;
		default: val = _T("(?)");		break;
		}
		return val;

	case 0x06:
		switch (val.GetData())
		{
		case 0: val = _T("ISO 80");		break;
		case 2: val = _T("ISO 160");		break;
		case 4: val = _T("ISO 320");		break;
		case 5: val = _T("ISO 100");		break;
		default: val = _T("(?)");		break;
		}
		return val;

	case 0x07:
		switch (val.GetData())
		{
		case 0: val = _T("Auto");			break;
		case 1: val = _T("Preset");			break;
		case 2: val = _T("Daylight");		break;
		case 3: val = _T("Incandescense");	break;
		case 4: val = _T("Fluorescence");	break;
		case 5: val = _T("Cloudy");			break;
		case 6: val = _T("SpeedLight");		break;
		default: val = _T("(?)");			break;
		}
		return val;

	case 0x08:
		{
			Rational fxFocus= val.Rational();
			ostringstream ost;
			if (fxFocus.denominator_ == 0)
				ost << _T("Infinite");
			else
				ost << fxFocus.Double();
			return ost.str();
		}

	case 0x0a:
		{
			Rational fxDigitalZooom= val.Rational();
			ostringstream ost;
			if (fxDigitalZooom.numerator_ == 0)
				ost << _T("Not Used");
			else
				ost << fxDigitalZooom.Double();
			return ost.str();
		}

	case 0x0b:
		switch (val.GetData())
		{
		case 1: val = _T("Used");			break;
		default: val = _T("Not Used");		break;
		}
		return val;
	}

	return val;
*/
}


static String Capitalize(const String& str)
{
	String s= str;
	size_t len= s.length();

	for (size_t i= 1; i < len; ++i)
		if (s[i - 1] == _T(' ') || s[i - 1] == _T('-') || s[i - 1] == _T('('))
			s[i] = _totupper(s[i]);
		else
			s[i] = _totlower(s[i]);

	return s;
}

extern String TrimSpaces(const String& str);


static String BracketingAndShootingMode(uint32 v)
{
	String str;

	switch ((v & 0xf0) >> 4)
	{
	case 0:		str = _T("无包围曝光; ");	break;
	case 1:		str = _T("包围曝光; ");		break;	// ???
	case 2:		str = _T("Flash 包围曝光; ");	break;
	case 3:		str = _T("AE 包围曝光; ");	break;	// could be AE & Flash

	case 4: //?
	case 6:
				str = _T("WB 包围曝光; ");	break;

	default:	str = _T("(?); ");				break;
	}

	switch (v & 0x0f)
	{
	case 0:		str += _T("单帧");		break;
	case 1:		str += _T("连续");		break;
	case 2:		str += _T("定时器");				break;
	case 3:		str += _T("遥控延时");	break;
	case 4:		str += _T("遥控");			break;
	default:	str += _T("(?)");				break;
	}

	return str;
}


static String FormatDigitalZoom(const String& digital_zoom)
{
	if (digital_zoom.empty())
		return digital_zoom;

	if (!isdigit(digital_zoom[0]))
		return digital_zoom;

	const TCHAR* p= digital_zoom.c_str();
	TCHAR* end= 0;
	int num= _tcstol(p, &end, 10);

	if (num == 0 || end == 0 || *end != _T('/'))
		return digital_zoom;

	int denum= _tcstol(end + 1, &end, 10);

	if (num > 0 && denum > 0)
	{
		oStringstream ost;
		ost << _T("x ") << double(num) / denum;
		return ost.str();
	}
	else
		return digital_zoom;
}


static TaggedValue ActiveDLightingTags[]=
{
	TaggedValue(0, L"关闭"),
	TaggedValue(1, L"低"),
	TaggedValue(3, L"正常"),
	TaggedValue(5, L"高"),
	TaggedValue(7, L"很高"),
	TaggedValue(0xffff, L"自动"),
};

static TaggedValue ColorSpaceTags[]=
{
	TaggedValue(1, L"sRGB"),
	TaggedValue(2, L"Adobe RGB"),
};

static TaggedValue ExposureFineTuning[]=
{
	TaggedValue(0xfa, L"-1 EV"),
	TaggedValue(0xfb, L"-5/6 EV"),
	TaggedValue(0xfc, L"-2/3 EV"),
	TaggedValue(0xfd, L"-1/2 EV"),
	TaggedValue(0xfe, L"-1/3 EV"),
	TaggedValue(0xff, L"-1/6 EV"),
	TaggedValue(0x00, L"0 EV"),
	TaggedValue(0x01, L"+1/6 EV"),
	TaggedValue(0x02, L"+1/3 EV"),
	TaggedValue(0x03, L"+1/2 EV"),
	TaggedValue(0x04, L"+2/3 EV"),
	TaggedValue(0x05, L"+5/6 EV"),
	TaggedValue(0x06, L"+1 EV"),
};

static TaggedValue HighISONoiseReduction[]=
{
	TaggedValue(0, L"关闭"),
	TaggedValue(1, L"最小"),
	TaggedValue(2, L"低"),
	TaggedValue(4, L"正常"),
	TaggedValue(6, L"高"),
};

static TaggedValue NEFCompression[]=
{
	TaggedValue(1, L"有损 Type 1"),
	TaggedValue(2, L"非压缩"),
	TaggedValue(3, L"无损"),
	TaggedValue(4, L"有损 Type 2"),
};


String WorldTime(const Data& val)
{
	if (val.Components() != 4 || !val.IsUndefData())
		return val.AsString();

	MemPointer p(const_cast<uint8*>(val.DataBytes()), val.Length());
	p.SetByteOrder(val.GetByteOrder());

	int16 zone= static_cast<int16>(p.GetUInt16());

	oStringstream ost;
	bool negative= false;
	if (zone < 0)
	{
		negative = true;
		zone = -zone;
	}
	int minutes= zone % 60;

	ost << L"UTC" << (negative ? L'-' : L'+') << zone / 60 << L"h ";
	if (minutes)
		ost << std::setw(2) << std::setfill(L'0') << minutes << L" min ";

	uint8 dst= p.GetUInt8();

	ost << (dst != 0 ? L"DST" : L"no DST");

	return ost.str();
}


int ISOSpeed(int32 speed)
{
	double iso= 100.0 * exp((speed / 12.0 - 5.0) * log(2.0));
	return static_cast<int>(iso + 0.5);
}

int ISOSpeed(const Data& val)
{
	if (val.FormatWord() && val.Components() == 2)
	{
		uint16 iso[2];
		val.ReadWords(iso, 2);
		return iso[1];
	}
	return 0;
}

static TaggedValue ExpandedISO[]=
{
	TaggedValue(0x0000, L"关闭"),
	TaggedValue(0x0101, L"Hi 0.3"),
	TaggedValue(0x0102, L"Hi 0.5"),
	TaggedValue(0x0103, L"Hi 0.7"),
	TaggedValue(0x0104, L"Hi 1.0"),
	TaggedValue(0x0105, L"Hi 1.3"),
	TaggedValue(0x0106, L"Hi 1.5"),
	TaggedValue(0x0107, L"Hi 1.7"),
	TaggedValue(0x0108, L"Hi 2.0"),
	TaggedValue(0x0201, L"Lo 0.3"),
	TaggedValue(0x0202, L"Lo 0.5"),
	TaggedValue(0x0203, L"Lo 0.7"),
	TaggedValue(0x0204, L"Lo 1.0"),
};


String ISOInfo(const Data& val)
{
	const size_t SIZE= 14;
	if (val.Components() != SIZE)
		return val.AsString();

	uint8 buf[SIZE];
	val.ReadUChar(buf, SIZE);
	MemPointer p(buf, SIZE);
	p.SetByteOrder(val.GetByteOrder());

	oStringstream ost;

	ost << ISOSpeed(p.GetInt8()) << L", ";
	p += 3;
	ost << GetTag(p.GetUInt16(), ExpandedISO) << L", ";

	ost << ISOSpeed(p.GetInt8()) << L", ";
	p += 3;
	ost << GetTag(p.GetUInt16(), ExpandedISO);

	return ost.str();
}

String MultiExposureInfo(const Data& val)
{
	const size_t SIZE= 16;
	if (val.Components() != SIZE)
		return val.AsString();

	uint8 buf[SIZE];
	val.ReadUChar(buf, SIZE);
	MemPointer p(buf, SIZE);
	p.SetByteOrder(val.GetByteOrder());

	p.GetUInt32();	// version
	uint32 mode= p.GetUInt32();	// mode
	uint32 shots= p.GetUInt32();
	uint32 auto_gain= p.GetUInt32();

	oStringstream ost;
	switch (mode)
	{
	case 0:	ost << L"关闭"; break;
	case 1:	ost << L"打开"; break;
	case 2:	ost << L"图像叠加"; break;
	default:	ost << L"(?)"; break;
	}

	ost << L", " << shots << L" shots, ";

	switch (auto_gain)
	{
	case 0:	ost << L"关闭自动增益"; break;
	case 1:	ost << L"打开自动增益"; break;
	default:	ost << L"(?)"; break;
	}

	return ost.str();
}

String LensFStops(const Data& val)
{
	if (val.Components() == 4 && val.IsUndefData() && val.DataBytes()[2] != 0)
	{
		float a= val.DataBytes()[0];
		float b= val.DataBytes()[1];
		float c= val.DataBytes()[2];
		oStringstream ost;
		ost << a * b / c;
		return ost.str();
	}
	else
		return val.AsString();
}

String LensType(uint32 type)
{
	oStringstream ost;

	if (type & 0x0f)
	{
		if (type & 0x01)
			ost << L"MF ";
		if (type & 0x02)
			ost << L"D ";
		if (type & 0x04)
			ost << L"G ";
		if (type & 0x08)
			ost << L"VR ";
	}
	else
		ost << L'(' << type << L')';

	return ost.str();
}


static TaggedValue OnOffTags[]=
{
	TaggedValue(1, L"打开"),
	TaggedValue(2, L"关闭"),
};


String VibrationReductionInfo(const Data& val)
{
	const size_t SIZE= 8;
	if (val.Components() != SIZE)
		return val.AsString();

	uint8 buf[SIZE];
	val.ReadUChar(buf, SIZE);
//	MemPointer p(buf, SIZE);
//	p.SetByteOrder(val.GetByteOrder());

//	String s= PrintVersion(buf);
//	s += L' ' + 

	return GetTag(buf[4], OnOffTags);
}


static TaggedValue FlashSource[]=
{
	TaggedValue(0, L"无"),
	TaggedValue(1, L"外置"),
	TaggedValue(2, L"内置"),
};

static TaggedValue FlashFirmware[]=
{
	TaggedValue(0x0000, L"N/A"),
	TaggedValue(0x0101, L"1.01 (SB-800 or Metz 58 AF-1)"),
	TaggedValue(0x0103, L"1.03 (SB-800)"),
	TaggedValue(0x0201, L"2.01 (SB-800)"),
	TaggedValue(0x0204, L"2.04 (SB-600)"),
	TaggedValue(0x0205, L"2.05 (SB-600)"),
	TaggedValue(0x0301, L"3.01 (SB-800 遥控引闪)"),
	TaggedValue(0x0401, L"4.01 (SB-400)"),
	TaggedValue(0x0402, L"4.02 (SB-400)"),
	TaggedValue(0x0404, L"4.04 (SB-400)"),
	TaggedValue(0x0401, L"5.01 (SB-900)"),
	TaggedValue(0x0401, L"5.02 (SB-900)"),
};

static TaggedValue FlashFired[]=
{
	TaggedValue(0, L"否"),
	TaggedValue(1, L"是"),
	TaggedValue(2, L"跳闪"),
	TaggedValue(4, L"宽闪光灯适配器"),
};

static TaggedValue FlashMode[]=
{
	TaggedValue(0, L"关闭"),
	TaggedValue(1, L"TTL BL"),
	TaggedValue(2, L"TTL"),
	TaggedValue(3, L"自动光圈"),
	TaggedValue(4, L"自动"),
	TaggedValue(5, L"GN"),
	TaggedValue(6, L"手动"),
	TaggedValue(7, L"重复闪光"),
};

String FlashLevel(int val)
{
	oStringstream ost;
	if (val >= 0 && val <= 48)
	{
		if (val == 0)
			ost << L"Full";
		else
		{
			int level= val / 6;
			ost << L"1/" << (1 << level);
			int mod= val % 6;
			if (mod != 0)
			{
				static const TCHAR* fraction[]=
				{
					L"1/6",
					L"0.3",
					L"0.5",
					L"0.7",
					L"5/6",
					0,
				};
				ost << L" -" << fraction[mod - 1];
			}
		}
		ost << L" Power";
	}
	else
		ost << L"(" << val << L")";

	return ost.str();
}

String FlashCompensation(int val)
{
	oStringstream ost;
	if (val >= 0 && val <= 255)
	{
		bool negative= false;
		if (val >= 128)	// negative values?
		{
			negative = true;
			val = 256 - val;
		}

		int level= val / 6;

		if (val != 0)
			ost << (negative ? L'+' : L'-');
		ost << level;
		int mod= val % 6;
		if (mod != 0)
		{
			static const TCHAR* fraction[]=
			{
				L".17",	// those two values are
				L".3",
				L".5",
				L".7",
				L".83",	// not used in practice
				0,
			};
			ost << fraction[mod - 1];
		}
		ost << L" EV";
	}
	else
		ost << L"(" << val << L")";

	return ost.str();
}

String FlashLevelCompensation(int mode, int value)
{
	const int OFF= 0;
	const int MANUAL= 6;
	const int REPEATING= 7;

	switch (mode)
	{
	case OFF:
		return L"-";

	case MANUAL:
	case REPEATING:
		return FlashLevel(value);

	default:	//todo: not sure all modes use it:
		return FlashCompensation(value);
	}
}


void DecodeFlashInfo(OutputStr& output, const Data& val)
{
	if (!val.IsUndefData() || val.Components() < 16 || val.Components() > 32)
		return;

	std::vector<uint8> buf(val.Components());
	val.ReadUChar(&buf[0], static_cast<int>(buf.size()));
	MemPointer p(&buf[0], buf.size());
	p.SetByteOrder(val.GetByteOrder());

	if (buf[0] != '0' || buf[1] != '1' || buf[2] != '0' || (buf[3] > '4' || buf[3] < '1'))
		return;

	int version= buf[3];

	p += 4;	// skip version

	output.Indent();
	//output.RecordInfo(1 | OutputStr::EXTRA_TAG, TagName(tag), val, TagValue(tag, val).c_str());

	int fs= p.GetUInt8();
	output.RecordInfo(1 | OutputStr::SUB_TAG, _T("闪光光源"), fs, GetTag(fs, FlashSource));
	p += 1;	// unknown

	int fmw= p.GetUInt16();
	output.RecordInfo(2 | OutputStr::SUB_TAG, _T("外部闪光固件"), fmw, GetTag(fmw, FlashFirmware));

	int fired= p.GetUInt8();	// some flags
	output.RecordInfo(3 | OutputStr::SUB_TAG, _T("外部闪光灯闪光"), fired, GetTag(fired, FlashFired));

	int mode= p.GetUInt8();		// metering mode
	int built_in_mode= mode & ~0x80;
	String flmode;
	if (mode & 0x80)	// commander mode?
		flmode = L"引闪模式, 内置闪光灯 ";
	flmode += GetTag(built_in_mode, FlashMode);
	output.RecordInfo(4 | OutputStr::SUB_TAG, _T("闪光模式"), mode, flmode.c_str());

	int power= p.GetUInt8();	// manual mode flash level
	output.RecordInfo(5 | OutputStr::SUB_TAG, _T("闪光补偿/级别"), power, FlashLevelCompensation(built_in_mode, power).c_str());

	if (version > '1')
		p += 1;	// unknown

	int fl= p.GetUInt8();

	oStringstream focal_length;

	if (fl != 0xff && fl != 0)
		focal_length << fl << L" mm";
	else if (fl == 0)
		focal_length << L"N/A";
	else
		focal_length << L"?";
	output.RecordInfo(6 | OutputStr::SUB_TAG, _T("闪光焦距"), fl, focal_length.str().c_str());

	int rate= p.GetUInt8();
	oStringstream ost;
	ost << rate << " Hz";
	output.RecordInfo(7 | OutputStr::SUB_TAG, _T("重复闪光速率"), rate, ost.str().c_str());

	{
		TCHAR buf[64];
		int repeats= p.GetUInt8();
		output.RecordInfo(8 | OutputStr::SUB_TAG, _T("重复闪光次数"), repeats, _itot(repeats, buf, 10));
	}

	int dist= p.GetUInt8();	// is it? todo: verify
	if (dist <= 50 || dist == 0xff)
	{
		oStringstream ost;
		ost << std::fixed << std::setprecision(1);
		if (dist == 0)
			ost << L"无";
		else if (dist == 0xff)
			ost << L"N/A";
		else if (dist < 10)
			ost << dist / 10.0 << L" m";
		else
			ost << pow(1.1224, dist - 10) << L" m";

		output.RecordInfo(9 | OutputStr::SUB_TAG, _T("闪光级数距离"), dist, ost.str().c_str());
	}
	else
		output.RecordInfo(9 | OutputStr::SUB_TAG, _T("闪光级数距离"), dist, 0);

	if (version == '1')
	{
		// commander mode group A and B details follow

		int group_a= p.GetUInt8();
		int temp_b= p.GetUInt8();
		int group_b= temp_b >> 4;

		output.RecordInfo(10 | OutputStr::SUB_TAG, _T("闪光 A 组模式"), group_a, GetTag(group_a, FlashMode));
		output.RecordInfo(11 | OutputStr::SUB_TAG, _T("闪光 B 组模式"), temp_b, GetTag(group_b, FlashMode));

		int power_a= p.GetUInt8();
		int power_b= p.GetUInt8();

		output.RecordInfo(12 | OutputStr::SUB_TAG, _T("闪光 A 组 补偿/级别"), power_a, FlashLevelCompensation(group_a, power_a).c_str());
		output.RecordInfo(13 | OutputStr::SUB_TAG, _T("闪光 B 组 补偿/级别"), power_b, FlashLevelCompensation(group_b, power_b).c_str());
	}

	output.Outdent();
}


static TaggedValue FlashType[]=
{
	TaggedValue(0, L"无闪光"),
	TaggedValue(1, L"闪光, 手动"),
	TaggedValue(7, L"闪光, 外部"),
	TaggedValue(8, L"闪光, 引闪模式"),
	TaggedValue(9, L"闪光, TTL 模式"),
};


void NikonNote::RecordInfo(uint16 tag, const Data& val, OutputStr& output)
{
	if (!new_format_)
	{
		// old format...

		return;
	}

	switch (tag)
	{
	case 1:		// version
		MakerNote::RecordInfo(tag, val, output);
		output.SetInterpretedInfo(VersionFieldFmt(val));
		break;
	case 0x02:
		data_.iso_setting_		= TrimSpaces(Capitalize(val.AsString()));
		MakerNote::RecordInfo(tag, val, output);
		if (int iso= ISOSpeed(val))
			output.SetInterpretedInfo(iso);
		break;
	case 0x03:
		data_.color_mode_		= TrimSpaces(Capitalize(val.AsString()));
		MakerNote::RecordInfo(tag, val, output);
		output.SetInterpretedInfo(data_.color_mode_);
		break;
	case 0x04:
		data_.quality_			= TrimSpaces(Capitalize(val.AsString()));
		MakerNote::RecordInfo(tag, val, output);
		output.SetInterpretedInfo(data_.quality_);
		break;
	case 0x05:
		data_.white_balance_	= TrimSpaces(Capitalize(val.AsString()));
		MakerNote::RecordInfo(tag, val, output);
		output.SetInterpretedInfo(data_.white_balance_);
		break;
	case 0x06:
		data_.image_sharpening_	= TrimSpaces(Capitalize(val.AsString()));
		MakerNote::RecordInfo(tag, val, output);
		output.SetInterpretedInfo(data_.image_sharpening_);
		break;
	case 0x07:
		data_.focus_mode_		= TrimSpaces(/*Capitalize*/(val.AsString()));
		if (data_.focus_mode_ == _T("MANUAL"))
			data_.focus_mode_ = _T("Manual");
		MakerNote::RecordInfo(tag, val, output);
		output.SetInterpretedInfo(data_.focus_mode_);
		break;
	case 0x08:
		data_.flash_setting_	= TrimSpaces(Capitalize(val.AsString()));
		MakerNote::RecordInfo(tag, val, output);
		output.SetInterpretedInfo(data_.flash_setting_);
		break;
	case 0x09:		// flash mode
		MakerNote::RecordInfo(tag, TagName(tag).c_str(), val, output);
		break;
	case 0x0f:
		data_.iso_selection_	= TrimSpaces(Capitalize(val.AsString()));
		MakerNote::RecordInfo(tag, val, output);
		output.SetInterpretedInfo(data_.iso_selection_);
		break;
	case 0x13:
		MakerNote::RecordInfo(tag, val, output);
		if (int iso= ISOSpeed(val))
			output.SetInterpretedInfo(iso);
		break;
	case 0x0d:		// program shift?
	case 0x0e:		// exposure difference?
	case 0x12:		// flash compensation?
	case 0x17:		// flash exposure compensation?
	case 0x18:		// Flash Bracket Compensation Applied
		MakerNote::RecordInfo(tag, val, output);
		{
			char data[4];
			val.ReadChar(data, array_count(data));
			output.SetInterpretedInfo(FlashCompensation(data[0]));
		}
		break;
	case 0x19:		// AE Bracket Compensation Applied
		MakerNote::RecordInfo(tag, val, output);
//		output.SetInterpretedInfo(val.SRational().String(true, true) + _T(" EV"));
		extern String FormatEvValue(double val);
		output.SetInterpretedInfo(FormatEvValue(val.SRational().Double()) + _T(" EV"));
		break;
	case 0x1c:
		MakerNote::RecordInfo(tag, val, output);
		if (val.IsUndefData())
			output.SetInterpretedInfo(GetTag(*val.DataBytes(), ExposureFineTuning));
		break;
	case 0x1e:		// color space
		MakerNote::RecordInfo(tag, val, output);
		output.SetInterpretedInfo(GetTag(val.AsULong(), ColorSpaceTags));
		break;
	case 0x1f:		// VR
		MakerNote::RecordInfo(tag, val, output);
		output.SetInterpretedInfo(VibrationReductionInfo(val));
		break;

	case 0x22:		// active d-lighting
		MakerNote::RecordInfo(tag, val, output);
		output.SetInterpretedInfo(GetTag(val.AsULong(), ActiveDLightingTags));
		break;
	case 0x24:		// world time
		MakerNote::RecordInfo(tag, val, output);
		output.SetInterpretedInfo(WorldTime(val));
		break;
	case 0x25:		// expanded ISO
		MakerNote::RecordInfo(tag, val, output);
		output.SetInterpretedInfo(ISOInfo(val));
		break;
	case 0x80:
		data_.image_adjustment_	= TrimSpaces(Capitalize(val.AsString()));
		MakerNote::RecordInfo(tag, val, output);
		output.SetInterpretedInfo(data_.image_adjustment_);
		break;
	case 0x82:
		data_.adapter_			= TrimSpaces(Capitalize(val.AsString()));
		MakerNote::RecordInfo(tag, val, output);
		output.SetInterpretedInfo(data_.adapter_);
		break;
	case 0x83:
		MakerNote::RecordInfo(tag, val, output);
		output.SetInterpretedInfo(LensType(val.AsULong()));
		break;
	case 0x84:
		data_.lens_type_		= TrimSpaces(val.AsString());
		MakerNote::RecordInfo(tag, val, output);
		if (val.Components() == 4)
		{
			std::vector<double> v;
			if (val.AsDoubleVector(v))
			{
				ASSERT(v.size() == 4);
				bool zoom_lens= v[0] != v[1] || v[2] != v[3];	// zoom lens or fixed lens?
				oStringstream ost;
				ost << _T("FL ") << std::fixed << std::setprecision(0) << v[0];
				if (zoom_lens)
					ost << _T("-") << std::fixed << std::setprecision(0) << v[1];
				ost << _T("  F/Stop ") << std::fixed << std::setprecision(1) << v[2];
				if (zoom_lens)
					ost << _T("/") << std::fixed << std::setprecision(1) << v[3];
				data_.lens_type_ = ost.str();
			}
		}
		output.SetInterpretedInfo(data_.lens_type_);
		break;
	case 0x85:
		data_.MF_distance_		= TrimSpaces(Capitalize(val.AsString()));
		MakerNote::RecordInfo(tag, val, output);
		output.SetInterpretedInfo(data_.MF_distance_);
		break;
	case 0x86:
//		data_.digital_zoom_		= TrimSpaces(Capitalize(val.AsString()));
		data_.digital_zoom_		= FormatDigitalZoom(data_.digital_zoom_);
		MakerNote::RecordInfo(tag, val, output);
		output.SetInterpretedInfo(data_.digital_zoom_);
		break;
	case 0x87:	// flash type
		MakerNote::RecordInfo(tag, val, output);
		output.SetInterpretedInfo(GetTag(val.AsULong(), FlashType));
		break;
	case 0x88:
		data_.AF_position_		= TrimSpaces(Capitalize(val.AsString()));
		MakerNote::RecordInfo(tag, val, output);
		output.SetInterpretedInfo(data_.AF_position_);
		break;
	case 0x89:
		MakerNote::RecordInfo(tag, val, output);
		output.SetInterpretedInfo(BracketingAndShootingMode(val.GetData()));
		break;
	case 0x8b:		// lens f-stops
		MakerNote::RecordInfo(tag, val, output);
		output.SetInterpretedInfo(LensFStops(val));
		break;

	case 0xa8:	// flash info
		MakerNote::RecordInfo(tag, val, output);
		DecodeFlashInfo(output, val);
		break;

	case 0xac:	// VR
		MakerNote::RecordInfo(tag, val, output);
		{
			String s= TrimSpaces(val.AsString());

			if (s == _T("VR-ON"))	s = _T("VR-On");
			else if (s == _T("VR-OFF"))	s = _T("VR-Off");

			output.SetInterpretedInfo(s);
		}
		break;

	case 0xb0:	// multiple exposure
		MakerNote::RecordInfo(tag, val, output);
		output.SetInterpretedInfo(MultiExposureInfo(val));
		break;

	case 0xb1:	// HighISONoiseReduction
		MakerNote::RecordInfo(tag, val, output);
		output.SetInterpretedInfo(GetTag(val.AsULong(), HighISONoiseReduction));
		break;

	case 0x93:	// NEF compression
		MakerNote::RecordInfo(tag, val, output);
		output.SetInterpretedInfo(GetTag(val.AsULong(), NEFCompression));
		break;

	case 0x95:	// noise reduction
		MakerNote::RecordInfo(tag, val, output);
		{
			String s= TrimSpaces(val.AsString());
			if (s == _T("OFF"))		s = _T("Off");
			else if (s == _T("ON"))	s = _T("On");
			output.SetInterpretedInfo(s);
		}
		break;

	case 0x81:	// tone comp.
	case 0x8d:	// color mode
	case 0x90:	// lightning type
	case 0xa9:	// img. opt.
	case 0xaa:	// saturation
	case 0xab:	// digital vari-program
	case 0xad:	// unknown; present in 8800
		MakerNote::RecordInfo(tag, val, output);
		output.SetInterpretedInfo(TrimSpaces(Capitalize(val.AsString())));
		break;

	default:
		MakerNote::RecordInfo(tag, TagName(tag).c_str(), val, output);
		break;
	}
}


const TCHAR* NikonNote::FlashCompensation(int8 fc)
{
	switch (fc)
	{
	case 18:	return _T("+3.0 EV");
	case 16:	return _T("+2.7 EV");
	case 15:	return _T("+2.5 EV");
	case 14:	return _T("+2.3 EV");
	case 12:	return _T("+2.0 EV");
	case 10:	return _T("+1.7 EV");
	case 9:		return _T("+1.5 EV");
	case 8:		return _T("+1.3 EV");
	case 6:		return _T("+1.0 EV");
	case 4:		return _T("+0.7 EV");
	case 3:		return _T("+0.5 EV");
	case 2:		return _T("+0.3 EV");
	case 0:		return _T("0 EV");
	case -2:	return _T("-0.3 EV");
	case -3:	return _T("-0.5 EV");
	case -4:	return _T("-0.7 EV");
	case -6:	return _T("-1.0 EV");
	case -8:	return _T("-1.3 EV");
	case -9:	return _T("-1.5 EV");
	case -10:	return _T("-1.7 EV");
	case -12:	return _T("-2.0 EV");
	case -14:	return _T("-2.3 EV");
	case -15:	return _T("-2.5 EV");
	case -16:	return _T("-2.7 EV");
	case -18:	return _T("-3.0 EV");
	}

	return _T("(?)");
}


void NikonNote::CompletePhotoInfo(PhotoInfo& photo) const
{
	if (!photo.IsISOSpeedValid())
	{
		size_t pos= data_.iso_setting_.find(_T(','));
		if (pos != String::npos)
			photo.SetISOSpeed(_ttoi(data_.iso_setting_.c_str() + pos + 1));
	}
}


//////////////////////////////////////////////////////////////////////

CanonNote::CanonNote(const String& model)
{
	model_ = GENERIC;

	if (model == _T("Canon EOS D60") || model == _T("Canon EOS D30"))
		model_ = EOS_D30_60;
	else if (model == _T("Canon EOS 10D"))
		model_ = EOS_10D;
	else if (model == _T("Canon EOS-1D"))
		model_ = EOS_1D;
	else if (model == _T("Canon EOS-1DS"))
		model_ = EOS_1DS;
	else if (model == _T("Canon EOS 300D DIGITAL"))
		model_ = EOS_300D;
}


bool CanonNote::IsDataPresent(int index) const
{
	return index >= COL_CANON_SET_START && index <= COL_CANON_SET_END;
}


String& CanonNote::GetInfo(int index, String& out) const
{
	out = _T("-");

	switch (index)
	{
	case COL_CAN_MACRO:			out = data_.MacroMode();		break;
	case COL_CAN_TIMER:			out = data_.SelfTimer();		break;
	case COL_CAN_FLASH:			out = data_.FlashMode();		break;
	case COL_CAN_DRIVE:			out = data_.DriveMode();		break;
	case COL_CAN_FOCUS:			out = data_.FocusMode();		break;
	case COL_CAN_FOCUS_TYPE:	out = data_.FocusType();		break;
	case COL_CAN_IMG_SIZE:		out = data_.ImageSize();		break;
	case COL_CAN_PROGRAM:		out = data_.Program();			break;
	case COL_CAN_CONTRAST:		out = data_.Contrast();		break;
	case COL_CAN_SATURATION:	out = data_.Saturation();		break;
	case COL_CAN_SHARPNESS:		out = data_.Sharpness();		break;
	case COL_CAN_ISO:			out = data_.ISO();				break;
	case COL_CAN_MET_MODE:		out = data_.MeteringMode();	break;
	case COL_CAN_AF_POINT:		out = data_.AFPointSelected();	break;
	case COL_CAN_EXP_MODE:		out = data_.ExposureMode();	break;
	case COL_CAN_LENS:			out = data_.LensAttached();	break;
	case COL_CAN_WHITE_BAL:		out = data_.WhiteBalance();	break;
	case COL_CAN_SEQ_NO:		out = data_.SequenceNumber();	break;
	case COL_CAN_FLASH_BIAS:	out = data_.FlashBias();		break;
	case COL_CAN_SUBJECT_DIST:	out = data_.SubjectDistance();	break;
	case COL_CAN_IMG_TYPE:		out = data_.ImageType();		break;
	case COL_CAN_FIRMWARE:		out = data_.Firmware();		break;
	case COL_CAN_OWNER:			out = data_.OwnerName();		break;
	case COL_CAN_SERIAL:		out = data_.CameraBodyNumber();break;
	default:
		ASSERT(false);
		break;
	}

	return out;
}


int CanonNote::CompareNotes(int index, const MakerNote& maker_note) const
{
	const CanonNote& note= static_cast<const CanonNote&>(maker_note);

	switch (index)
	{
	case COL_CAN_MACRO:			return data_.macro_mode_ > note.data_.macro_mode_ ? 1 : -1;
	case COL_CAN_TIMER:			return data_.self_timer_ > note.data_.self_timer_ ? 1 : -1;
	case COL_CAN_FLASH:			return data_.flash_mode_ > note.data_.flash_mode_ ? 1 : -1;
	case COL_CAN_DRIVE:			return data_.drive_mode_ > note.data_.drive_mode_ ? 1 : -1;
	case COL_CAN_FOCUS:			return data_.focus_mode_ > note.data_.focus_mode_ ? 1 : -1;
	case COL_CAN_FOCUS_TYPE:	return data_.focus_type_ > note.data_.focus_type_ ? 1 : -1;
	case COL_CAN_IMG_SIZE:		return data_.image_size_ > note.data_.image_size_ ? 1 : -1;
	case COL_CAN_PROGRAM:		return data_.program_ > note.data_.program_ ? 1 : -1;
	case COL_CAN_CONTRAST:		return data_.contrast_ > note.data_.contrast_ ? 1 : -1;
	case COL_CAN_SATURATION:	return data_.saturation_ > note.data_.saturation_ ? 1 : -1;
	case COL_CAN_SHARPNESS:		return data_.sharpness_ > note.data_.sharpness_ ? 1 : -1;
	case COL_CAN_ISO:			return data_.iso_ > note.data_.iso_ ? 1 : -1;
	case COL_CAN_MET_MODE:		return data_.metering_mode_ > note.data_.metering_mode_ ? 1 : -1;
	case COL_CAN_AF_POINT:		return data_.AF_point_ > note.data_.AF_point_ ? 1 : -1;
	case COL_CAN_EXP_MODE:		return data_.exposure_mode_ > note.data_.exposure_mode_ ? 1 : -1;
	case COL_CAN_LENS:			return data_.lens_short_ != note.data_.lens_short_ ?
									(data_.lens_short_ > note.data_.lens_short_ ? 1 : -1) :
									(data_.lens_long_ > note.data_.lens_long_ ? 1 : -1);
	case COL_CAN_WHITE_BAL:		return data_.white_balance_ > note.data_.white_balance_ ? 1 : -1;
	case COL_CAN_SEQ_NO:		return data_.sequence_number_ > note.data_.sequence_number_ ? 1 : -1;
	case COL_CAN_FLASH_BIAS:	return data_.flash_bias_ > note.data_.flash_bias_ ? 1 : -1;
	case COL_CAN_SUBJECT_DIST:	return data_.subject_distance_ > note.data_.subject_distance_ ? 1 : -1;
	case COL_CAN_IMG_TYPE:		return _tcsicmp(data_.image_type_.c_str(), note.data_.image_type_.c_str());
	case COL_CAN_FIRMWARE:		return _tcsicmp(data_.firmware_.c_str(), note.data_.firmware_.c_str());
	case COL_CAN_OWNER:			return _tcsicmp(data_.owner_name_.c_str(), note.data_.owner_name_.c_str());
	case COL_CAN_SERIAL:		return data_.camera_body_number_ > note.data_.camera_body_number_ ? 1 : -1;

	default:
		ASSERT(false);
		return 0;
	}
}


bool CanonNote::LessNote(int index, const MakerNote& maker_note) const
{
	const CanonNote& note= static_cast<const CanonNote&>(maker_note);

	switch (index)
	{
	case COL_CAN_MACRO:			return data_.macro_mode_ < note.data_.macro_mode_;
	case COL_CAN_TIMER:			return data_.self_timer_ < note.data_.self_timer_;
	case COL_CAN_FLASH:			return data_.flash_mode_ < note.data_.flash_mode_;
	case COL_CAN_DRIVE:			return data_.drive_mode_ < note.data_.drive_mode_;
	case COL_CAN_FOCUS:			return data_.focus_mode_ < note.data_.focus_mode_;
	case COL_CAN_FOCUS_TYPE:	return data_.focus_type_ < note.data_.focus_type_;
	case COL_CAN_IMG_SIZE:		return data_.image_size_ < note.data_.image_size_;
	case COL_CAN_PROGRAM:		return data_.program_ < note.data_.program_;
	case COL_CAN_CONTRAST:		return data_.contrast_ < note.data_.contrast_;
	case COL_CAN_SATURATION:	return data_.saturation_ < note.data_.saturation_;
	case COL_CAN_SHARPNESS:		return data_.sharpness_ < note.data_.sharpness_;
	case COL_CAN_ISO:			return data_.iso_ < note.data_.iso_;
	case COL_CAN_MET_MODE:		return data_.metering_mode_ < note.data_.metering_mode_;
	case COL_CAN_AF_POINT:		return data_.AF_point_ < note.data_.AF_point_;
	case COL_CAN_EXP_MODE:		return data_.exposure_mode_ < note.data_.exposure_mode_;
	case COL_CAN_LENS:			return data_.lens_short_ != note.data_.lens_short_ ? data_.lens_short_ < note.data_.lens_short_ : data_.lens_long_ < note.data_.lens_long_;
	case COL_CAN_WHITE_BAL:		return data_.white_balance_ < note.data_.white_balance_;
	case COL_CAN_SEQ_NO:		return data_.sequence_number_ < note.data_.sequence_number_;
	case COL_CAN_FLASH_BIAS:	return data_.flash_bias_ < note.data_.flash_bias_;
	case COL_CAN_SUBJECT_DIST:	return data_.subject_distance_ < note.data_.subject_distance_;
	case COL_CAN_IMG_TYPE:		return data_.image_type_ < note.data_.image_type_;
	case COL_CAN_FIRMWARE:		return data_.firmware_ < note.data_.firmware_;
	case COL_CAN_OWNER:			return data_.owner_name_ < note.data_.owner_name_;
	case COL_CAN_SERIAL:		return data_.camera_body_number_ < note.data_.camera_body_number_;

	default:
		ASSERT(false);
		return false;
	}
}


String CanonNote::TagName(uint16 tag)
{
	switch (tag)
	{
	//case 0x06:	return _T("图像类型");
	case 0x06:	return _T("相机型号");
	case 0x07:	return _T("固件版本");
	case 0x09:	return _T("主人名字");
	case 0x0c:	return _T("机身编号");
	case 0xae:	return _T("色温");
	default:	return _T("未知");
	}
}


String CanonNote::TagValue(uint16 tag, const Data& val)
{
	switch (tag)
	{
	case 0x0c:
		{
			uint32 serial= val.GetData();
			oStringstream ost;
			ost << std::hex;
			ost.width(4);
			ost.fill(_T('0'));
			ost << (serial >> 16);
			ost << std::dec;
			ost.width(5);
			ost.fill(_T('0'));
			ost << (serial & 0xffff);
			return ost.str();
		}
	}

	return val.AsString();
}

///////////////////////////////////////////////////////////////////////////////
// tags classification according to:
// http://www.burren.cx/david/canon.html
//

void CanonNote::ReportTag1Info(const Data& val, OutputStr& output, CanonData& inf)
{
	const int COUNT= 32;
	if (!val.FormatWord() || val.Components() < COUNT)
		return;

	uint16 data[COUNT];
	val.ReadWords(data, COUNT);

	inf.macro_mode_ = data[1];
	RecordTag(1 | OutputStr::SUB_TAG, _T("微距模式"), inf.MacroMode(), output);

	inf.self_timer_ = data[2];
	RecordTag(2 | OutputStr::SUB_TAG, _T("自拍定时"), inf.SelfTimer().c_str(), output);

	inf.quality_ = data[3];
	RecordTag(3 | OutputStr::SUB_TAG, _T("品质"), inf.Quality(), output);

	inf.flash_mode_ = data[4];
	RecordTag(4 | OutputStr::SUB_TAG, _T("闪光灯模式"), inf.FlashMode(), output);

	inf.drive_mode_ = data[5];
	RecordTag(5 | OutputStr::SUB_TAG, _T("驱动模式"), inf.DriveMode(), output);

	inf.focus_mode_ = data[7];
//	if (data[32] == 1)
//		val = _T("Continuous");
	RecordTag(7 | OutputStr::SUB_TAG, _T("对焦模式"), inf.FocusMode(), output);

	inf.image_size_ = data[10];
	RecordTag(10 | OutputStr::SUB_TAG, _T("图像大小"), inf.ImageSize(), output);

	inf.program_ = data[11];
	RecordTag(11 | OutputStr::SUB_TAG, _T("程序"), inf.Program(), output);

	// digital zoom

	inf.contrast_ = data[13];
	RecordTag(13 | OutputStr::SUB_TAG, _T("对比度"), inf.Contrast(), output);

	inf.saturation_ = data[14];
	RecordTag(14 | OutputStr::SUB_TAG, _T("色彩饱和度"), inf.Saturation(), output);

	inf.sharpness_ = data[15];
	RecordTag(15 | OutputStr::SUB_TAG, _T("锐度"), inf.Sharpness(), output);

	inf.iso_ = data[16];
	RecordTag(16 | OutputStr::SUB_TAG, _T("ISO"), inf.ISO().c_str(), output);

	inf.metering_mode_ = data[17];
	RecordTag(17 | OutputStr::SUB_TAG, _T("测光模式"), inf.MeteringMode(), output);

	inf.focus_type_ = data[18];
	RecordTag(18 | OutputStr::SUB_TAG, _T("对焦类型"), inf.FocusType(), output);

	inf.AF_point_ = data[19];
	RecordTag(19 | OutputStr::SUB_TAG, _T("自动对焦点"), inf.AFPointSelected(), output);

	inf.exposure_mode_ = data[20];
	RecordTag(20 | OutputStr::SUB_TAG, _T("曝光模式"), inf.ExposureMode(), output);

	if (data[25])
	{
		uint32 lens_long= data[23] / data[25];
		uint32 lens_short= data[24] / data[25];
		inf.lens_short_ = lens_short;
		inf.lens_long_ = lens_long;
		RecordTag(23 | OutputStr::SUB_TAG, _T("镜头"), inf.LensAttached().c_str(), output);
	}

/*	val = 0;
	switch (data[29])
	{
	case 0:	val = _T("Easy shooting");	break;
	case 1:	val = _T("Program");		break;
	case 2:	val = _T("Tv-Priority");	break;
	case 3:	val = _T("Av-Priority");	break;
	case 4:	val = _T("Manual");			break;
	case 5:	val = _T("A-DEP");			break;
	}
	if (val)
		RecordTag(_T("Flash Details"), val, output); */

}


void CanonNote::ReportTag4Info(const Data& val, OutputStr& output, CanonData& inf)
{
	const int COUNT= 20;
	if (!val.FormatWord() || val.Components() < COUNT)
		return;

	const int MAX= 40;
	uint16 data[MAX];

	int count= std::min<int>(MAX, val.Components());

	val.ReadWords(data, count);

	inf.white_balance_ = data[7];
	RecordTag(7 | OutputStr::SUB_TAG, _T("白平衡"), inf.WhiteBalance(), output);

	inf.sequence_number_ = data[9];
	RecordTag(9 | OutputStr::SUB_TAG, _T("序列号"), inf.SequenceNumber().c_str(), output);

	inf.flash_bias_ = data[15];
	RecordTag(15 | OutputStr::SUB_TAG, _T("闪光补偿"), inf.FlashBias().c_str(), output);

	RecordTag(16 | OutputStr::SUB_TAG, _T("自动包围曝光"), data[16] ? _T("开") : _T("关"), output);

	oStringstream ost;
	double value= static_cast<int16>(data[17]);
	value /= 32.0;
	ost << value;
	RecordTag(17 | OutputStr::SUB_TAG, _T("AEB 包围值"), ost.str().c_str(), output);

	inf.subject_distance_ = data[19];
	RecordTag(19 | OutputStr::SUB_TAG, _T("主体距离"), inf.SubjectDistance().c_str(), output);

	if (count > 22)
	{
		ost.str(_T(""));
		double val= static_cast<int16>(data[21]);
		val /= 32.0;
		ost << val;
		RecordTag(21 | OutputStr::SUB_TAG, _T("光圈值"), ost.str().c_str(), output);

		ost.str(_T(""));
		val = static_cast<int16>(data[22]);
		val /= 32.0;
		ost << val;
		RecordTag(22 | OutputStr::SUB_TAG, _T("时间值"), ost.str().c_str(), output);
	}

	if (count > 24)
	{
		ost.str(_T(""));
		ost << static_cast<double>(data[24]) / 10.0;
		RecordTag(24 | OutputStr::SUB_TAG, _T("Bulb Duration"), ost.str().c_str(), output);
	}

	if (count > 29)
	{
		ost.str(_T(""));
		ost << static_cast<double>(data[29]) / 10.0;
		RecordTag(29 | OutputStr::SUB_TAG, _T("自拍定时"), ost.str().c_str(), output);
	}
}


void CanonNote::ReportTagFInfo(const Data& value, OutputStr& output)
{
	const int COUNT= 13 + 1;
	if (!value.FormatWord() || value.Components() < COUNT)
		return;

	uint16 data[COUNT + 2];

	int count= COUNT;

	if (value.Components() >= COUNT + 2)
		count = COUNT + 2;

	value.ReadWords(data, count);

	for (int i= 1; i < count; ++i)
	{
		const TCHAR* val= 0;
		const TCHAR* fun= 0;
		uint16 v= data[i] & 0xff;
		switch (data[i] >> 8)
		{
		case 1:
			fun = _T("Long exposure noise reduction");
			val = v ? _T("On") : _T("Off");
			break;

		case 2:
			fun = _T("Shutter/AE-lock buttons");
			switch (v)
			{
			case 0: val = _T("AF/AE lock"); break;
			case 1: val = _T("AE lock/AF"); break;
			case 2: val = _T("AF/AF lock"); break;
			case 3: val = _T("AE+release/AE+AF"); break;
			}
			break;

		case 3:
			fun = _T("Mirror lockup");
			val = v ? _T("Enabled") : _T("Disabled");
			break;

		case 4:
			fun = _T("Tv/Av and exposure level");
			switch (v)
			{
			case 0: val = _T("1/2 stop"); break;
			case 1: val = _T("1/3 stop"); break;
			}
			break;

		case 5:
			fun = _T("AF-assist light");
			val = v ? _T("Off") : _T("On (auto)");
			break;

		case 6:
			fun = _T("Shutter speed in Av mode");
			switch (v)
			{
			case 0: val = _T("Automatic"); break;
			case 1: val = _T("1/200 (fixed)"); break;
			}
			break;

		case 7:
			fun = _T("AEB sequence/auto cancellation");
			switch (v)
			{
			case 0: val = _T("0, -, + / Enabled"); break;
			case 1: val = _T("0, -, + / Disabled"); break;
			case 2: val = _T("-, 0, + / Enabled"); break;
			case 3: val = _T("-, 0, + / Disabled"); break;
			}
			break;

		case 8:
			fun = _T("Shutter curtain sync");
			switch (v)
			{
			case 0: val = _T("1st-curtain sync"); break;
			case 1: val = _T("2nd-curtain sync"); break;
			}
			break;

		case 9:
			fun = _T("Lens AF stop button Fn. Switch");
			switch (v)
			{
			case 0: val = _T("AF stop"); break;
			case 1: val = _T("Operate AF"); break;
			case 2: val = _T("Lock AE and start timer"); break;
			}
			break;

		case 10:
			fun = _T("Auto reduction of fill flash");
			switch (v)
			{
			case 0: val = _T("Enabled"); break;
			case 1: val = _T("Disabled"); break;
			}
			break;

		case 11:
			fun = _T("Menu button return position");
			switch (v)
			{
			case 0: val = _T("Top"); break;
			case 1: val = _T("Previous (volatile)"); break;
			case 2: val = _T("Previous"); break;
			}
			break;

		case 12:
			fun = _T("SET button func. when shooting");
			switch (v)
			{
			case 0: val = _T("Not assigned"); break;
			case 1: val = _T("Change quality"); break;
			case 2: val = _T("Change ISO speed"); break;
			case 3: val = _T("Select parameters"); break;
			}
			break;

		case 13:
			fun = _T("Sensor cleaning");
			switch (v)
			{
			case 0: val = _T("Disabled"); break;
			case 1: val = _T("Enabled"); break;
			}
			break;

		case 14:
			fun = _T("Superimposed display");
			val = v == 0 ? _T("On") : _T("Off");
			break;

		case 15:
			fun = _T("Shutter release no CF card");
			val = v == 0 ? _T("Yes") : _T("No");
			break;
		}

		if (val) // && *val)
			RecordTag(fun, val, output);
	}
}


void CanonNote::ReportTagFInfoEOS10D(const Data& value, OutputStr& output)
{
	const int COUNT= 17 + 1;
	if (!value.FormatWord() || value.Components() < COUNT)
		return;

	uint16 data[COUNT];

	int count= COUNT;

	value.ReadWords(data, COUNT);

	for (int i= 1; i < COUNT; ++i)
	{
		const TCHAR* val= 0;
		const TCHAR* fun= 0;
		uint16 v= data[i] & 0xff;
		switch (data[i] >> 8)
		{
		case 1:
			fun = _T("Set button function");
			switch (v)
			{
			case 0: val = _T("Not assigned"); break;
			case 1: val = _T("Change quality"); break;
			case 2: val = _T("Change parameters"); break;
			case 3: val = _T("Menu display"); break;
			case 4: val = _T("Image replay"); break;
			}
			break;

		case 2:
			fun = _T("Shutter release no CF card");
			val = v == 0 ? _T("Yes") : _T("No");
			break;

		case 3:
			fun = _T("Flash sync speed AV");
			val = v == 0 ? _T("Auto") : _T("Fixed (1/200)");
			break;

		case 4:
			fun = _T("Shutter/AE lock");
			switch (v)
			{
			case 0: val = _T("AF/AE lock"); break;
			case 1: val = _T("AE lock/AF"); break;
			case 2: val = _T("AF/AF lock, no AE lock"); break;
			case 3: val = _T("AE/AF, no AE lock"); break;
			}
			break;

		case 5:
			fun = _T("AF-assist light");
			switch (v)
			{
			case 0: val = _T("Emits/Fires"); break;
			case 1: val = _T("Doesn't emit/Fires"); break;
			case 2: val = _T("Only external flash emits/Fires"); break;
			case 3: val = _T("Emits/Doesn't fire"); break;
			}
			break;

		case 6:
			fun = _T("Exposure level increments");
			switch (v)
			{
			case 0: val = _T("1/2 Stop"); break;
			case 1: val = _T("1/3 Stop"); break;
			}
			break;

		case 7:
			fun = _T("AF point registration");
			switch (v)
			{
			case 0: val = _T("Center"); break;
			case 1: val = _T("Bottom"); break;
			case 2: val = _T("Right"); break;
			case 3: val = _T("Extreme right"); break;
			case 4: val = _T("Automatic"); break;
			case 5: val = _T("Extreme left"); break;
			case 6: val = _T("Left"); break;
			case 7: val = _T("Top"); break;
			}
			break;

		case 8:
			fun = _T("Raw and JPEG recording");
			switch (v)
			{
			case 0: val = _T("RAW+Small/Normal"); break;
			case 1: val = _T("RAW+Small/Fine"); break;
			case 2: val = _T("RAW+Medium/Normal"); break;
			case 3: val = _T("RAW+Medium/Fine"); break;
			case 4: val = _T("RAW+Large/Normal"); break;
			case 5: val = _T("RAW+Large/Fine"); break;
			}
			break;

		case 9:
			fun = _T("AEB sequence");
			switch (v)
			{
			case 0: val = _T("0, -, +/Enabled"); break;
			case 1: val = _T("0, -, +/Disabled"); break;
			case 2: val = _T("-, 0, +/Enabled"); break;
			case 3: val = _T("-, 0, +/Disabled"); break;
			}
			break;

		case 10:
			fun = _T("Superimposed display");
			val = v == 0 ? _T("On") : _T("Off");
			break;

		case 11:
			fun = _T("Menu button return position");
			switch (v)
			{
			case 0: val = _T("Top"); break;
			case 1: val = _T("Previous (volatile)"); break;
			case 2: val = _T("Previous"); break;
			}
			break;

		case 12:
			fun = _T("Mirror lockup");
			val = val == 0 ? _T("Disabled") : _T("Enabled");
			break;

		case 13:
			fun = _T("Assist button function");
			switch (v)
			{
			case 0: val = _T("Normal"); break;
			case 1: val = _T("Select home position"); break;
			case 2: val = _T("Select HP (while pressing)"); break;
			case 3: val = _T("Av+/- (AF point by QCD)"); break;
			case 4: val = _T("FE lock"); break;
			}
			break;

		case 14:
			fun = _T("Fill flash auto reduction");
			val = v == 0 ? _T("Enabled") : _T("Disabled");
			break;

		case 15:
			fun = _T("Shutter curtain sync");
			switch (v)
			{
			case 0: val = _T("1st-curtain sync"); break;
			case 1: val = _T("2nd-curtain sync"); break;
			}
			break;

		case 16:
			fun = _T("Safety shift in AV or TV");
			val = v ? _T("Enabled") : _T("Disabled");
			break;

		case 17:
			fun = _T("Lens AF stop button");
			switch (v)
			{
			case 0: val = _T("AF stop"); break;
			case 1: val = _T("Operate AF"); break;
			case 2: val = _T("Lock AE and start timer"); break;
			}
			break;
		}

		if (val) // && *val)
			RecordTag(fun, val, output);
	}
}


void CanonNote::ReportTagFInfoEOS_1D(const Data& value, OutputStr& output)
{
	const int COUNT= 21;
	if (!value.FormatWord() || value.Components() < COUNT)
		return;

	uint16 data[COUNT];

	int count= COUNT;

	value.ReadWords(data, COUNT);

	for (int i= 1; i < COUNT; ++i)
	{
		const TCHAR* val= 0;
		const TCHAR* fun= 0;
		uint16 v= data[i] & 0xff;
		switch (data[i] >> 8)
		{
		case 0:
			fun = _T("Focusing screen");
			switch (v)
			{
			case 0: val = _T("Ec-N, R"); break;
			case 1: val = _T("Ec-A, B, C, CII, CIII, D, H, I, L"); break;
			}
			break;

		case 1:
			fun = _T("Finder display during exposure");
			val = v == 0 ? _T("Off") : _T("On");
			break;

		case 2:
			fun = _T("Shutter release no CF card");
			val = v == 0 ? _T("Yes") : _T("No");
			break;

		case 3:
			fun = _T("ISO speed expansion");
			val = v == 0 ? _T("No") : _T("Yes");
			break;

		case 4:
			fun = _T("Shutter/AE lock button");
			switch (v)
			{
			case 0: val = _T("AF/AE lock stop"); break;
			case 1: val = _T("AE lock/AF"); break;
			case 2: val = _T("AF/AF lock, no AE lock"); break;
			case 3: val = _T("AE/AF, no AE lock"); break;
			}
			break;

		case 5:
			fun = _T("Manual TV");
			switch (v)
			{
			case 0: val = _T("Tv=Main/Av=Control"); break;
			case 1: val = _T("Tv=Control/Av=Main"); break;
			case 2: val = _T("Tv=Main/Av=Main w/o Lens"); break;
			case 3: val = _T("Control/Av=Main w/o Lens"); break;
			}
			break;

		case 6:
			fun = _T("Exposure level increments");
			switch (v)
			{
			case 0: val = _T("1/3-Stop set, 1/3-Stop comp"); break;
			case 1: val = _T("1-Stop set, 1/3-Stop comp"); break;
			case 2: val = _T("1/2-Stop set, 1/2-Stop comp"); break;
			}
			break;

		case 7:
			fun = _T("USM lens electronic MF");
			switch (v)
			{
			case 0: val = _T("Turns on after one shot AF"); break;
			case 1: val = _T("Turns off after one shot AF"); break;
			case 2: val = _T("Always turned off"); break;
			}
			break;

		case 8:
			fun = _T("LCD panels");
			switch (v)
			{
			case 0: val = _T("Remaining shots/File no."); break;
			case 1: val = _T("ISO/Remaining shots"); break;
			case 2: val = _T("ISO/File no."); break;
			case 3: val = _T("Shots in folder/Remaining shots"); break;
			}
			break;

		case 9:
			fun = _T("AEB sequence");
			switch (v)
			{
			case 0: val = _T("0, -, +/Enabled"); break;
			case 1: val = _T("0, -, +/Disabled"); break;
			case 2: val = _T("-, 0, +/Enabled"); break;
			case 3: val = _T("-, 0, +/Disabled"); break;
			}
			break;

		case 10:
			fun = _T("AF point illumination");
			switch (v)
			{
			case 0: val = _T("On"); break;
			case 1: val = _T("Off"); break;
			case 2: val = _T("On without dimming"); break;
			case 3: val = _T("Brighter"); break;
			}
			break;

		case 11:
			fun = _T("AF point selection");
			switch (v)
			{
			case 0: val = _T("H=AF+Main/V=AF+Command"); break;
			case 1: val = _T("H=Comp+Main/V=Comp+Command"); break;
			case 2: val = _T("H=Command Only/V=Assist+Main"); break;
			case 3: val = _T("H=FEL+Main/V=FEL+Command"); break;
			}
			break;

		case 12:
			fun = _T("Mirror lockup");
			val = v == 0 ? _T("Disabled") : _T("Enabled");
			break;

		case 13:
			fun = _T("AF point spot metering");
			switch (v)
			{
			case 0: val = _T("45/Center AF Point"); break;
			case 1: val = _T("11/Active AF Point"); break;
			case 2: val = _T("11/Center AF Point"); break;
			case 3: val = _T("9/Active AF Point"); break;
			}
			break;

		case 14:
			fun = _T("Fill flash auto reduction");
			val = v == 0 ? _T("Enabled") : _T("Disabled");
			break;

		case 15:
			fun = _T("Shutter curtain sync");
			switch (v)
			{
			case 0: val = _T("1st-curtain sync"); break;
			case 1: val = _T("2nd-curtain sync"); break;
			}
			break;

		case 16:
			fun = _T("Safety shift in AV or TV");
			val = v ? _T("Enabled") : _T("Disabled");
			break;

		case 17:
			fun = _T("AF point activation area");
			switch (v)
			{
			case 0: val = _T("Single AF point"); break;
			case 1: val = _T("Expanded (TTL. of 7 AF points)"); break;
			case 2: val = _T("Automatic expanded (max. 13)"); break;
			}
			break;

		case 18:
			fun = _T("Switch to registered AF point");
			switch (v)
			{
			case 0: val = _T("Assist + AF"); break;
			case 1: val = _T("Assist"); break;
			case 2: val = _T("Only while pressing assist"); break;
			}
			break;

		case 19:
			fun = _T("Lens AF stop button");
			switch (v)
			{
			case 0: val = _T("AF stop"); break;
			case 1: val = _T("AF start"); break;
			case 2: val = _T("AE lock while metering"); break;
			case 3: val = _T("AF point: M->Auto/Auto->Ctr"); break;
			case 4: val = _T("AF mode: ONESHOT<->SERVO"); break;
			case 5: val = _T("IS start"); break;
			}
			break;

		case 20:
			fun = _T("AI servo tracking sensitivity");
			switch (v)
			{
			case 0: val = _T("Standard"); break;
			case 1: val = _T("Slow"); break;
			case 2: val = _T("Moderately slow"); break;
			case 3: val = _T("Moderately fast"); break;
			case 4: val = _T("Fast"); break;
			}
			break;
		}

		if (val) // && val)
			RecordTag(fun, val, output);
	}
}


void CanonNote::RecordInfo(uint16 tag, const Data& val, OutputStr& output)
{
	switch (tag)
	{
	case 0x01:
		RecordTag(tag, _T("照相机设置"), _T(""), output);
		output.Indent();
			ReportTag1Info(val, output, data_);
		output.Outdent();
		break;

	case 0x04:
		RecordTag(tag, _T("拍摄信息"), _T(""), output);
		output.Indent();
			ReportTag4Info(val, output, data_);
		output.Outdent();
		break;

	case 0x06:	// img type
		data_.image_type_ = val.AsString();
		//RecordTag(tag, _T("图像类型"), data_.ImageType().c_str(), output);
		RecordTag(tag, _T("相机型号"), data_.ImageType().c_str(), output);
		break;
	case 0x07:	// firmware version
		data_.firmware_ = val.AsString();
		RecordTag(tag, _T("紧固件"), data_.Firmware().c_str(), output);
		break;
	case 0x08:	// file number
		RecordTag(tag, _T("文件编号"), val.AsString().c_str(), output);
		break;
	case 0x09:	// owner name
		data_.owner_name_ = val.AsString();
		RecordTag(tag, _T("主人名字"), data_.OwnerName().c_str(), output);
		break;
	case 0x0c:	// camera serial no
		data_.camera_body_number_ = val.GetData();
		RecordTag(tag, _T("机身编号"), data_.CameraBodyNumber().c_str(), output);
		break;
	case 0x0e:	// file length
		RecordTag(tag, _T("文件长度"), val.AsString().c_str(), output);
		break;

	case 0x0f:	// custom functions
		RecordTag(tag, _T("自定义功能"), _T(""), output);
		output.Indent();
		switch (model_)
		{
		case EOS_10D:
			ReportTagFInfoEOS10D(val, output);
			break;
		default:
			ReportTagFInfo(val, output);
			break;
		}
		output.Outdent();
		break;

	case 0x90:	// custom functions of EOS-1D
		RecordTag(tag, _T("自定义功能"), _T(""), output);
		output.Indent();
			ReportTagFInfoEOS_1D(val, output);
		output.Outdent();
		break;

	default:
		RecordTag(tag, TagName(tag).c_str(), val.AsString().c_str(), output);
		break;
	}
}


void CanonNote::CompletePhotoInfo(PhotoInfo& photo) const
{
	if (!photo.IsISOSpeedValid())
	{
		String iso= data_.ISO();

		if (iso == _T("Auto"))
			photo.SetAutoISOSpeed();
		else
		{
			// copy ISO settings; not the same as actual ISO, but better than nothing
			if (uint16 speed= _ttoi(iso.c_str()))
				photo.SetISOSpeed(speed);
		}
	}
}

//////////////////////////////////////////////////////////////////////

FujiNote::FujiNote()
{}


bool FujiNote::IsDataPresent(int index) const
{
	return index >= COL_FUJI_SET_START && index <= COL_FUJI_SET_END;
}


String& FujiNote::GetInfo(int index, String& out) const
{
	out = _T("-");

	switch (index)
	{
	case COL_FUJI_VERSION:			out = data_.Version();			break;
	case COL_FUJI_QUALITY:			out = data_.Quality();			break;
	case COL_FUJI_SHARPNESS:		out = data_.Sharpness();		break;
	case COL_FUJI_WHITE_BALANCE:	out = data_.WhiteBalance();		break;
	case COL_FUJI_COLOR:			out = data_.Color();			break;
	case COL_FUJI_TONE:				out = data_.Tone();				break;
	case COL_FUJI_FLASH_MODE:		out = data_.FlashMode();		break;
	case COL_FUJI_FLASH_STRENGTH:	out = data_.FlashStrength();	break;
	case COL_FUJI_MACRO:			out = data_.Macro();			break;
	case COL_FUJI_FOCUS_MODE:		out = data_.FocusMode();		break;
	case COL_FUJI_SLOWSYNC:			out = data_.SlowSync();			break;
	case COL_FUJI_PICTURE_MODE:		out = data_.PictureMode();		break;
	case COL_FUJI_CONTTAKEBRACKET:	out = data_.ContTakeBracket();	break;
	case COL_FUJI_BLUR_WARNING:		out = data_.BlurWarning();		break;
	case COL_FUJI_FOCUS_WARNING:	out = data_.FocusWarning();		break;
	case COL_FUJI_AE_WARNING:		out = data_.AEWarning();		break;
	default:
		ASSERT(false);
		break;
	}

	return out;
}


int FujiNote::CompareNotes(int index, const MakerNote& maker_note) const
{
	const FujiNote& note= static_cast<const FujiNote&>(maker_note);

	switch (index)
	{
	case COL_FUJI_VERSION:
		return data_.version_ > note.data_.version_ ? 1 : -1;
/*,
	COL_FUJI_QUALITY,
	COL_FUJI_SHARPNESS,
	COL_FUJI_WHITE_BALANCE,
	COL_FUJI_COLOR,
	COL_FUJI_TONE,
	COL_FUJI_FLASH_MODE,
	COL_FUJI_FLASH_STRENGTH,
	COL_FUJI_MACRO,
	COL_FUJI_FOCUS_MODE,
	COL_FUJI_SLOWSYNC,
	COL_FUJI_PICTURE_MODE,
//	COL_FUJI_UNKNOWN1,
	COL_FUJI_CONTTAKEBRACKET,
//	COL_FUJI_UNKNOWN2,
	COL_FUJI_BLUR_WARNING,
	COL_FUJI_FOCUS_WARNING,
	COL_FUJI_AE_WARNING,
	COL_FUJI_SET_END= COL_FUJI_AE_WARNING
*/

	default:
		ASSERT(false);
		return 0;
	}
}


bool FujiNote::LessNote(int index, const MakerNote& maker_note) const
{
	const FujiNote& note= static_cast<const FujiNote&>(maker_note);

	switch (index)
	{
	case COL_FUJI_VERSION:
		return data_.version_ < note.data_.version_;

	default:
		ASSERT(false);
		return false;
	}
}


void FujiNote::RecordInfo(uint16 tag, const Data& val, OutputStr& output)
{
	switch (tag)
	{
	case 0x0000: data_.version_				= val.GetData();	break;
	case 0x1000: data_.quality_				= val.AsString();	break;
	case 0x1001: data_.sharpness_			= val.GetData();	break;
	case 0x1002: data_.white_balance_		= val.GetData();	break;
	case 0x1003: data_.color_				= val.GetData();	break;
	case 0x1004: data_.tone_				= val.GetData();	break;
	case 0x1010: data_.flash_mode_			= val.GetData();	break;
	case 0x1011: data_.flash_strength_		= val.SRational();	break;
	case 0x1020: data_.macro_				= val.GetData();	break;
	case 0x1021: data_.focus_mode_			= val.GetData();	break;
	case 0x1030: data_.slow_sync_			= val.GetData();	break;
	case 0x1031: data_.picture_mode_		= val.GetData();	break;
	case 0x1032: data_.unknown1_			= val.GetData();	break;
	case 0x1100: data_.cont_take_bracket_	= val.GetData();	break;
	case 0x1200: data_.unknown2_			= val.GetData();	break;
	case 0x1300: data_.blur_warning_		= val.GetData();	break;
	case 0x1301: data_.focus_warning_		= val.GetData();	break;
	case 0x1302: data_.AE_warning_			= val.GetData();	break;

	default:
		return;
	}

	MakerNote::RecordInfo(tag, val, output);
}


String FujiNote::TagName(uint16 tag)
{
	String name;

	switch (tag)
	{
	case 0x0000: name = _T("版本");				break;
	case 0x1000: name = _T("品质");				break;
	case 0x1001: name = _T("锐度");			break;
	case 0x1002: name = _T("白平衡");		break;
	case 0x1003: name = _T("色彩");				break;
	case 0x1004: name = _T("色调");					break;
	case 0x1010: name = _T("闪光模式");			break;
	case 0x1011: name = _T("闪光强度");		break;
	case 0x1020: name = _T("微距");				break;
	case 0x1021: name = _T("对焦模式");			break;
	case 0x1030: name = _T("低速同步");			break;
	case 0x1031: name = _T("图像模式");			break;
	case 0x1032: name = _T("未知");				break;
	case 0x1100: name = _T("连拍/包围曝光");	break;
	case 0x1200: name = _T("未知");				break;
	case 0x1300: name = _T("模糊警告");			break;
	case 0x1301: name = _T("对焦警示");		break;
	case 0x1302: name = _T("AE 警告");			break;
	}

	return name;
}


String FujiNote::TagValue(uint16 tag, const Data& val)
{

	return val.AsString();
}


//////////////////////////////////////////////////////////////////////

OlympusNote::OlympusNote()
{}


void OlympusNote::RecordInfo(uint16 tag, const Data& val, OutputStr& output)
{
	switch (tag)
	{
	case 0x0200:
		if (val.Components() >= 3)
			val.ReadLongs(data_.special_mode_, 3);
		break;
	case 0x0201: data_.jpeg_quality_	= val.GetData();	break;
	case 0x0202: data_.macro_			= val.GetData();	break;
	case 0x0203: break;
	case 0x0204: data_.digital_zoom_	= val.Rational();	break;
	case 0x0205: break;
	case 0x0206: break;
	case 0x0207: data_.software_		= val.AsString();	break;
	case 0x0208: data_.pict_info_		= val.AsString();	break;
	case 0x0209: data_.camera_id_		= val.AsString(true, true);	break;
	}

	MakerNote::RecordInfo(tag, val, output);
}


bool OlympusNote::IsDataPresent(int index) const
{
	return index >= COL_OLY_SET_START && index <= COL_OLY_SET_END;
}


String& OlympusNote::GetInfo(int index, String& out) const
{
	out = _T("-");

	switch (index)
	{
	case COL_OLY_SPECIAL_MODE:		out = data_.SpecialMode();		break;
	case COL_OLY_JPEG_QUALITY:		out = data_.JPEGQuality();		break;
	case COL_OLY_MACRO:				out = data_.Macro();			break;
	case COL_OLY_DIGITAL_ZOOM:		out = data_.DigitalZoom();		break;
	case COL_OLY_SOFTWARE_RELEASE:	out = data_.software_;			break;
	case COL_OLY_PICTURE_INFO:		out = data_.pict_info_;			break;
	case COL_OLY_CAMERA_ID:			out = data_.camera_id_;			break;
	default:
		ASSERT(false);
		break;
	}

	return out;
}


int OlympusNote::CompareNotes(int index, const MakerNote& maker_note) const
{
	const OlympusNote& note= static_cast<const OlympusNote&>(maker_note);

	switch (index)
	{
	case COL_OLY_SPECIAL_MODE:		return 1; // TODO

	case COL_OLY_JPEG_QUALITY:		return data_.jpeg_quality_ > note.data_.jpeg_quality_ ? 1 : -1;
	case COL_OLY_MACRO:				return data_.macro_ > note.data_.macro_ ? 1 : -1;
	case COL_OLY_DIGITAL_ZOOM:		return data_.digital_zoom_ > note.data_.digital_zoom_ ? 1 : -1;
	case COL_OLY_SOFTWARE_RELEASE:	return _tcsicmp(data_.software_.c_str(), note.data_.software_.c_str());
	case COL_OLY_CAMERA_ID:			return _tcsicmp(data_.camera_id_.c_str(), note.data_.camera_id_.c_str());

	default:
		ASSERT(false);
		return 0;
	}
}


bool OlympusNote::LessNote(int index, const MakerNote& maker_note) const
{
	const OlympusNote& note= static_cast<const OlympusNote&>(maker_note);

	switch (index)
	{
	case COL_OLY_SPECIAL_MODE:		return false; // TODO

	case COL_OLY_JPEG_QUALITY:		return data_.jpeg_quality_ < note.data_.jpeg_quality_;
	case COL_OLY_MACRO:				return data_.macro_ < note.data_.macro_;
	case COL_OLY_DIGITAL_ZOOM:		return data_.digital_zoom_ < note.data_.digital_zoom_;
	case COL_OLY_SOFTWARE_RELEASE:	return data_.software_ < note.data_.software_;
	case COL_OLY_CAMERA_ID:			return data_.camera_id_ < note.data_.camera_id_;

	default:
		ASSERT(false);
		return false;
	}
}


String OlympusNote::TagName(uint16 tag)
{
	String name;

	switch (tag)
	{
	case 0x0200: name = _T("特殊模式");			break;
	case 0x0201: name = _T("JPEG 品质");			break;
	case 0x0202: name = _T("微距");				break;
	case 0x0203: name = _T("未知");				break;
	case 0x0204: name = _T("数码变焦");			break;
	case 0x0205: name = _T("未知");				break;
	case 0x0206: name = _T("未知");				break;
	case 0x0207: name = _T("软件版本");		break;
	case 0x0208: name = _T("图像信息");			break;
	case 0x0209: name = _T("相机 ID");			break;
	case 0x0f00: name = _T("数据转储");			break;
	}

	return name;
}


String OlympusNote::TagValue(uint16 tag, const Data& val)
{
	return val.AsString();
}


//////////////////////////////////////////////////////////////////////

CasioNote::CasioNote()
{}


void CasioNote::RecordInfo(uint16 tag, const Data& val, OutputStr& output)
{
	switch (tag)
	{
	case 0x0001: data_.recording_mode_	= val.GetData();	break;
	case 0x0002: data_.quality_			= val.GetData();	break;
	case 0x0003: data_.focus_mode_		= val.GetData();	break;
	case 0x0004: data_.flash_mode_		= val.GetData();	break;
	case 0x0005: data_.flash_intensity_	= val.GetData();	break;
	case 0x0006: data_.object_distance_	= val.GetData();	break;
	case 0x0007: data_.white_balance_	= val.GetData();	break;
	case 0x0008: break;
	case 0x0009: break;
	case 0x000a: data_.digital_zoom_	= val.GetData();	break;
	case 0x000b: data_.sharpness_		= val.GetData();	break;
	case 0x000c: data_.contrast_		= val.GetData();	break;
	case 0x000d: data_.saturation_		= val.GetData();	break;
	case 0x000e: break;
	case 0x000f: break;
	case 0x0010: break;
	case 0x0011: break;
	case 0x0012: break;
	case 0x0013: break;
	case 0x0014: data_.CCD_sensitivity_	= val.GetData();	break;
	}

	MakerNote::RecordInfo(tag, val, output);
}


String CasioNote::TagName(uint16 tag)
{
	String name;

	switch (tag)
	{
	case 0x0001: name = _T("录像模式");	break;
	case 0x0002: name = _T("品质");			break;
	case 0x0003: name = _T("对焦模式");		break;
	case 0x0004: name = _T("闪光灯模式");		break;
	case 0x0005: name = _T("闪光灯强度");	break;
	case 0x0006: name = _T("主体距离");	break;
	case 0x0007: name = _T("白平衡");		break;
//	case 0x0008: name = _T("unknown");			break;
//	case 0x0009: name = _T("unknown");			break;
	case 0x000a: name = _T("数码变焦");		break;
	case 0x000b: name = _T("锐度");		break;
	case 0x000c: name = _T("对比度");			break;
	case 0x000d: name = _T("饱和度");		break;
//	case 0x000e: name = _T("unknown");			break;
//	case 0x000f: name = _T("unknown");			break;
//	case 0x0010: name = _T("unknown");			break;
//	case 0x0011: name = _T("unknown");			break;
//	case 0x0012: name = _T("unknown");			break;
//	case 0x0013: name = _T("unknown");			break;
	case 0x0014: name = _T("CCD 感光度");	break;

	default:	 name = _T("未知");			break;
	}

	return name;
}


String CasioNote::TagValue(uint16 tag, const Data& value)
{
	String val;

	switch (tag)
	{
	case 0x0001:
		switch (value.GetData())
		{
		case 1: val = _T("单次快门");	break;
		case 2: val = _T("全景");		break;
		case 3: val = _T("夜景");	break;
		case 4: val = _T("肖像");		break;
		case 5: val = _T("风景");		break;
		default: val = _T('(') + value.AsString() + _T(')');	break;
		}
		return val;

	case 0x0002:
		switch (value.GetData())
		{
		case 1: val = _T("粗糙");	break;
		case 2: val = _T("正常");		break;
		case 3: val = _T("精细");		break;
		default: val = _T('(') + value.AsString() + _T(')');	break;
		}
		return val;

	case 0x0003:
		switch (value.GetData())
		{
		case 2: val = _T("微距");		break;
		case 3: val = _T("自动");		break;
		case 4: val = _T("手动");		break;
		case 5: val = _T("无穷远");	break;
		default: val = _T('(') + value.AsString() + _T(')');	break;
		}
		return val;

	case 0x0004:
		switch (value.GetData())
		{
		case 1: val = _T("自动");	break;
		case 2: val = _T("打开");		break;
		case 3: val = _T("关闭");	break;
		case 4: val = _T("消除红眼");	break;
		default: val = _T('(') + value.AsString() + _T(')');	break;
		}
		return val;

	case 0x0005:
		switch (value.GetData())
		{
		case 11: val = _T("弱");		break;
		case 13: val = _T("正常");	break;
		case 15: val = _T("强");	break;
		default: val = _T('(') + value.AsString() + _T(')');	break;
		}
		return val;

	case 0x0007:
		switch (value.GetData())
		{
		case 1: val = _T("自动");			break;
		case 2: val = _T("钨灯");		break;
		case 3: val = _T("日光灯");		break;
		case 4: val = _T("荧光灯");	break;
		case 5: val = _T("阴影");			break;
		case 129: val = _T("手动");		break;
		default: val = _T('(') + value.AsString() + _T(')');	break;
		}
		return val;

	case 0x000a:
		switch (value.GetData())
		{
		case 65536: val = _T("关闭");	break;
		case 65537: val = _T("x2");		break;
		default: val = _T('(') + value.AsString() + _T(')');	break;
		}
		return val;

	case 0x000b:
		switch (value.GetData())
		{
		case 0: val = _T("正常");		break;
		case 1: val = _T("柔和");		break;
		case 2: val = _T("锐利");		break;
		default: val = _T('(') + value.AsString() + _T(')');	break;
		}
		return val;

	case 0x000c:
		switch (value.GetData())
		{
		case 0: val = _T("正常");		break;
		case 1: val = _T("低");		break;
		case 2: val = _T("高");		break;
		default: val = _T('(') + value.AsString() + _T(')');	break;
		}
		return val;

	case 0x000d:
		switch (value.GetData())
		{
		case 0: val = _T("正常");		break;
		case 1: val = _T("低");		break;
		case 2: val = _T("高");		break;
		default: val = _T('(') + value.AsString() + _T(')');	break;
		}
		return val;

	case 0x0014:
		switch (value.GetData())
		{
		case 64:  val = _T("正常");	break;
		case 125: val = _T("+1.0");		break;
		case 250: val = _T("+2.0");		break;
		case 244: val = _T("+3.0");		break;
		case 80:  val = _T("正常");	break;
		case 100: val = _T("高");		break;
		default:  val = _T('(') + value.AsString() + _T(')');	break;
		}
		return val;

	default:
		return value.AsString();
	}
}


String& CasioNote::GetInfo(int index, String& out) const
{
	return out;
}


bool CasioNote::IsDataPresent(int index) const
{
	return false;
}


int CasioNote::CompareNotes(int index, const MakerNote& note) const
{
	return 0;
}


bool CasioNote::LessNote(int index, const MakerNote& note) const
{
	return false;
}

//-----------------------------------

Casio2Note::Casio2Note()
{}


void Casio2Note::RecordInfo(uint16 tag, const Data& val, OutputStr& output)
{
	switch (tag)
	{
	//case 0x0001: data_.recording_mode_	= val.GetData();	break;
	//case 0x0002: data_.quality_			= val.GetData();	break;
	//case 0x0003: data_.focus_mode_		= val.GetData();	break;
	//case 0x0004: data_.flash_mode_		= val.GetData();	break;
	//case 0x0005: data_.flash_intensity_	= val.GetData();	break;
	//case 0x0006: data_.object_distance_	= val.GetData();	break;
	//case 0x0007: data_.white_balance_		= val.GetData();	break;
	//case 0x0008: break;
	//case 0x0009: break;
	//case 0x000a: data_.digital_zoom_		= val.GetData();	break;
	//case 0x000b: data_.sharpness_		= val.GetData();	break;
	//case 0x000c: data_.contrast_			= val.GetData();	break;
	//case 0x000d: data_.saturation_		= val.GetData();	break;
	//case 0x000e: break;
	//case 0x000f: break;
	//case 0x0010: break;
	//case 0x0011: break;
	//case 0x0012: break;
	//case 0x0013: break;
	//case 0x0014: data_.CCD_sensitivity_	= val.GetData();	break;
	case 0x3014: // CCD ISO Sensitivity
		data_.ccd_iso_ = val.AsULong();
		break;
	}

	MakerNote::RecordInfo(tag, val, output);
}


String Casio2Note::TagName(uint16 tag)
{
	String name;

	switch (tag)
	{
	case 0x0002: name = _T("预览缩略图尺寸");	break;
	case 0x0003: name = _T("预览缩略图大小");		break;
	case 0x0004: name = _T("预览缩略图偏移");		break;
	case 0x0008: name = _T("品质");					break;
	case 0x0009: name = _T("图像大小");					break;
	case 0x000d: name = _T("对焦模式");					break;
	case 0x0014: name = _T("感光度");				break;
	case 0x0019: name = _T("白平衡");				break;
	case 0x001d: name = _T("焦距");					break;
	case 0x001f: name = _T("饱和度");					break;
	case 0x0020: name = _T("对比度");						break;
	case 0x0021: name = _T("锐度");					break;
	case 0x0e00: name = _T("打印图像匹配信息");	break;
	case 0x2000: name = _T("卡西欧预览缩略图");		break;
	case 0x2011: name = _T("白平衡偏差");			break;
	case 0x2012: name = _T("白平衡");				break;
	case 0x2022: name = _T("主体距离");				break;
	case 0x2034: name = _T("闪光距离");				break;
	case 0x3000: name = _T("记录模式");					break;
	case 0x3001: name = _T("自拍定时");					break;
	case 0x3002: name = _T("品质");						break;
	case 0x3003: name = _T("对焦模式");					break;
	case 0x3006: name = _T("时区");					break;
	case 0x3007: name = _T("最佳拍摄模式");				break;
	case 0x3014: name = _T("CCD 感光度");			break;
	case 0x3015: name = _T("色彩模式");					break;
	case 0x3016: name = _T("增强");					break;
	case 0x3017: name = _T("滤镜");						break;

	default:	 name = _T("未知");						break;
	}

	return name;
}


String Casio2Note::TagValue(uint16 tag, const Data& value)
{
	String val;

	switch (tag)
	{
//	case 0x0002: // Preview Thumbnail Dimensions
//		break;

//	case 0x3014: // _T("CCD ISO Sensitivity");

	case 0:
	default:
		return value.AsString();
	}
}


String& Casio2Note::GetInfo(int index, String& out) const
{
	return out;
}


bool Casio2Note::IsDataPresent(int index) const
{
	return false;
}


int Casio2Note::CompareNotes(int index, const MakerNote& note) const
{
	return 0;
}


bool Casio2Note::LessNote(int index, const MakerNote& note) const
{
	return false;
}


void Casio2Note::CompletePhotoInfo(PhotoInfo& photo) const
{
	if (!photo.IsISOSpeedValid() && data_.ccd_iso_ != 0)
		photo.SetISOSpeed(data_.ccd_iso_);
}


//////////////////////////////////////////////////////////////////////

PentaxNote::PentaxNote()
{
	iso_ = 0;
}

String PentaxNote::TagName(uint16 tag)
{
	switch (tag)
	{
	case 0x02:	return _T("缩略图尺寸");
	case 0x06:	return _T("日期");
	case 0x07:	return _T("时间");
	case 0x08:	return _T("品质");
	case 0x0d:	return _T("对焦模式");
	case 0x0f:	return _T("自动对焦点");
	case 0x10:	return _T("对焦位置");
	case 0x12:	return _T("曝光时长");
	case 0x13:	return _T("光圈");
	case 0x16:	return _T("曝光补偿");
	case 0x17:	return _T("测光模式");
	case 0x19:	return _T("白平衡");
	case 0x1a:	return _T("白平衡");
	case 0x1b:	return _T("蓝平衡");
	case 0x1c:	return _T("红平衡");
	case 0x1d:	return _T("焦距");
	case 0x1e:	return _T("数码变焦");
	case 0x1f:	return _T("饱和度");
	case 0x20:	return _T("对比度");
	case 0x21:	return _T("锐度");
	case 0x22:	return _T("时区");
	case 0x23:	return _T("家乡城市");
	case 0x24:	return _T("目的地城市");
	case 0x25:	return _T("家乡 DST");
	case 0x26:	return _T("目的地 DST");
	case 0x29:	return _T("帧数");
	case 0x37:	return _T("色彩空间");
	case 0x39:	return _T("Raw 图像大小");
	case 0x3f:	return _T("镜头型号");
	case 0x49:	return _T("降噪");
	case 0x403:	return _T("色调曲线");
	}

	return String(_T("未知"));
}

String PentaxNote::TagValue(uint16 tag, const Data& val)
{
	return String();
}


void PentaxNote::RecordInfo(uint16 tag, const Data& val, OutputStr& output)
{
//	String val;

//	ISO table
	static unsigned short iso[]=
	{
		0, 0, 0,
		50, 64, 80,
		100, 125, 160,
		200, 250, 320,
		400, 500, 640,
		800, 1000, 1250,
		1600, 2000, 2500,
		3200, 4000, 5000,
		6400, 8000, 10000,
		12800
	};

	switch (tag)
	{
	case 0x0006:	// date
		{
			int a= 0;
		}
		break;

	case 0x0007:	// time
		{
			int a= 0;
		}
		break;

	case 0x0014:	// ISO
		{
			uint32 v= val.AsULong();
			String str;
			if (v >= array_count(iso) || iso[v] == 0)
				str = _T("? (") + val.AsString() + _T(')');
			else
			{
				iso_ = iso[v];
				TCHAR buf[64];
				_itot(iso_, buf, 10);
				str = buf;
			}
			RecordTag(tag, _T("感光度"), val.AsString().c_str(), output);
			output.SetInterpretedInfo(str);
		}
		break;
/*
	case 0x0001:
		switch (val.GetData())
		{
		case 0: val = _T("Auto");		break;
		case 1: val = _T("Night scene");	break;
		case 2: val = _T("Manual");		break;
		default: val = _T('(') + val.AsString() + _T(')');	break;
		}
		RecordTag(tag, _T("Mode"), val.c_str(), output);
		break;

	case 0x0002:
		switch (val.GetData())
		{
		case 0: val = _T("Good");	break;
		case 1: val = _T("Better");	break;
		case 2: val = _T("Best");	break;
		default: val = _T('(') + val.AsString() + _T(')');	break;
		}
		RecordTag(tag, _T("Quality"), val.c_str(), output);
		break;

	case 0x0003:
		switch (val.GetData())
		{
		case 2: val = _T("Custom");	break;
		case 3: val = _T("Auto");	break;
		default: val = _T('(') + val.AsString() + _T(')');	break;
		}
		RecordTag(tag, _T("Focus"), val.c_str(), output);
		break;

	case 0x0004:
		switch (val.GetData())
		{
		case 1: val = _T("Auto");	break;
		case 2: val = _T("On");		break;
		case 4: val = _T("Off");		break;
		case 6: val = _T("Red-eye reduction");	break;
		default: val = _T('(') + val.AsString() + _T(')');	break;
		}
		RecordTag(tag, _T("Flash"), val.c_str(), output);
		break;

	case 0x0007:
		switch (val.GetData())
		{
		case 0: val = _T("Auto");		break;
		case 1: val = _T("Daylight");	break;
		case 2: val = _T("Shade");		break;
		case 3: val = _T("Tungsten");	break;
		case 4: val = _T("Fluorescent");	break;
		case 5: val = _T("Manual");		break;
		default: val = _T('(') + val.AsString() + _T(')');	break;
		}
		RecordTag(tag, _T("White Balance"), val.c_str(), output);
		break;

	case 0x000a:
		RecordTag(tag, _T("Zoom"), val.AsString().c_str(), output);
		break;

	case 0x000b:
		switch (val.GetData())
		{
		case 0: val = _T("Normal");	break;
		case 1: val = _T("Soft");	break;
		case 2: val = _T("Hard");	break;
		default: val = _T('(') + val.AsString() + _T(')');	break;
		}
		RecordTag(tag, _T("Sharpness"), val.c_str(), output);
		break;

	case 0x000c:
		switch (val.GetData())
		{
		case 0: val = _T("Normal");	break;
		case 1: val = _T("Low");		break;
		case 2: val = _T("High");	break;
		default: val = _T('(') + val.AsString() + _T(')');	break;
		}
		RecordTag(tag, _T("Contrast"), val.c_str(), output);
		break;

	case 0x000d:
		switch (val.GetData())
		{
		case 0: val = _T("Normal");	break;
		case 1: val = _T("Low");		break;
		case 2: val = _T("High");	break;
		default: val = _T('(') + val.AsString() + _T(')');	break;
		}
		RecordTag(tag, _T("Saturation"), val.c_str(), output);
		break;

	case 0x0014:
		switch (val.GetData())
		{
		case 10: val = _T("100");	break;
		case 16: val = _T("200");	break;
		default: val = _T('(') + val.AsString() + _T(')');	break;
		}
		RecordTag(tag, _T("ISO"), val.c_str(), output);
		break;

	case 0x0017:
		switch (val.GetData())
		{
		case 1: val = _T("Full");	break;
		case 2: val = _T("Black & white");	break;
		case 3: val = _T("Sepia");	break;
		default: val = _T('(') + val.AsString() + _T(')');	break;
		}
		RecordTag(tag, _T("Color"), val.c_str(), output);
		break;

	case 0x1000:
		RecordTag(tag, _T("Time Zone City"), val.AsString().c_str(), output);
		break;

	case 0x1001:
		RecordTag(tag, _T("Daylight Savings"), val.AsString().c_str(), output);
		break;
*/
	default:
		RecordTag(tag, TagName(tag).c_str(), val.AsString().c_str(), output);
		break;
	}
}


String& PentaxNote::GetInfo(int index, String& out) const
{
	out = _T("-");
	return out;
}

bool PentaxNote::IsDataPresent(int index) const
{
	return false;
}

int PentaxNote::CompareNotes(int index, const MakerNote& note) const
{
	return 0;
}

bool PentaxNote::LessNote(int index, const MakerNote& note) const
{
	return false;
}


void PentaxNote::CompletePhotoInfo(PhotoInfo& photo) const
{
	if (!photo.IsISOSpeedValid() && iso_ != 0)
		photo.SetISOSpeed(iso_);
}


//////////////////////////////////////////////////////////////////////


SonyNote::SonyNote()
{}

String SonyNote::TagName(uint16 tag)
{
	return String();
}

String SonyNote::TagValue(uint16 tag, const Data& val)
{
	return String();
}


void SonyNote::RecordInfo(uint16 tag, const Data& value, OutputStr& output)
{
	String val;

	switch (tag)
	{
/*
	case 0x0001:
		switch (val.GetData())
		{
		case 0: val = _T("Auto");		break;
		case 1: val = _T("Night scene");	break;
		case 2: val = _T("Manual");		break;
		default: val = _T('(') + val.AsString() + _T(')');	break;
		}
		RecordTag(tag, _T("Mode"), val.c_str(), output);
		break;

	case 0x0002:
		switch (val.GetData())
		{
		case 0: val = _T("Good");	break;
		case 1: val = _T("Better");	break;
		case 2: val = _T("Best");	break;
		default: val = _T('(') + val.AsString() + _T(')');	break;
		}
		RecordTag(tag, _T("Quality"), val.c_str(), output);
		break;

	case 0x0003:
		switch (val.GetData())
		{
		case 2: val = _T("Custom");	break;
		case 3: val = _T("Auto");	break;
		default: val = _T('(') + val.AsString() + _T(')');	break;
		}
		RecordTag(tag, _T("Focus"), val.c_str(), output);
		break;

	case 0x0004:
		switch (val.GetData())
		{
		case 1: val = _T("Auto");	break;
		case 2: val = _T("On");		break;
		case 4: val = _T("Off");		break;
		case 6: val = _T("Red-eye reduction");	break;
		default: val = _T('(') + val.AsString() + _T(')');	break;
		}
		RecordTag(tag, _T("Flash"), val.c_str(), output);
		break;

	case 0x0007:
		switch (val.GetData())
		{
		case 0: val = _T("Auto");		break;
		case 1: val = _T("Daylight");	break;
		case 2: val = _T("Shade");		break;
		case 3: val = _T("Tungsten");	break;
		case 4: val = _T("Fluorescent");	break;
		case 5: val = _T("Manual");		break;
		default: val = _T('(') + val.AsString() + _T(')');	break;
		}
		RecordTag(tag, _T("White Balance"), val.c_str(), output);
		break;

	case 0x000a:
		RecordTag(tag, _T("Zoom"), val.AsString().c_str(), output);
		break;

	case 0x000b:
		switch (val.GetData())
		{
		case 0: val = _T("Normal");	break;
		case 1: val = _T("Soft");	break;
		case 2: val = _T("Hard");	break;
		default: val = _T('(') + val.AsString() + _T(')');	break;
		}
		RecordTag(tag, _T("Sharpness"), val.c_str(), output);
		break;

	case 0x000c:
		switch (val.GetData())
		{
		case 0: val = _T("Normal");	break;
		case 1: val = _T("Low");		break;
		case 2: val = _T("High");	break;
		default: val = _T('(') + val.AsString() + _T(')');	break;
		}
		RecordTag(tag, _T("Contrast"), val.c_str(), output);
		break;

	case 0x000d:
		switch (val.GetData())
		{
		case 0: val = _T("Normal");	break;
		case 1: val = _T("Low");		break;
		case 2: val = _T("High");	break;
		default: val = _T('(') + val.AsString() + _T(')');	break;
		}
		RecordTag(tag, _T("Saturation"), val.c_str(), output);
		break;

	case 0x0014:
		switch (val.GetData())
		{
		case 10: val = _T("100");	break;
		case 16: val = _T("200");	break;
		default: val = _T('(') + val.AsString() + _T(')');	break;
		}
		RecordTag(tag, _T("ISO"), val.c_str(), output);
		break;

	case 0x0017:
		switch (val.GetData())
		{
		case 1: val = _T("Full");	break;
		case 2: val = _T("Black & white");	break;
		case 3: val = _T("Sepia");	break;
		default: val = _T('(') + val.AsString() + _T(')');	break;
		}
		RecordTag(tag, _T("Color"), val.c_str(), output);
		break;

	case 0x1000:
		RecordTag(tag, _T("Time Zone City"), val.AsString().c_str(), output);
		break;

	case 0x1001:
		RecordTag(tag, _T("Daylight Savings"), val.AsString().c_str(), output);
		break;
*/
	case 0:
	default:
		RecordTag(tag, _T("未知"), value.AsString().c_str(), output);
		break;
	}
}


String& SonyNote::GetInfo(int index, String& out) const
{
	out = _T("-");
	return out;
}

bool SonyNote::IsDataPresent(int index) const
{
	return false;
}

int SonyNote::CompareNotes(int index, const MakerNote& note) const
{
	return 0;
}

bool SonyNote::LessNote(int index, const MakerNote& note) const
{
	return false;
}


//////////////////////////////////////////////////////////////////////


SanyoNote::SanyoNote()
{}

String SanyoNote::TagName(uint16 tag)
{
	switch (tag)
	{
	case 0x0200:	return _T("特殊模式");
	case 0x0201:	return _T("JPEG 品质");
	case 0x0202:	return _T("微距");
	//case 0x0203:	return _T("");
	case 0x0204:	return _T("数码变焦");
	case 0x020e:	return _T("连拍方法");
	case 0x020f:	return _T("超广角");
	case 0x0210:	return _T("色彩调整模式");
	case 0x0213:	return _T("抓拍");
	case 0x0214:	return _T("自拍定时");
	case 0x0216:	return _T("语音备忘");
	case 0x0217:	return _T("快门释放记录");
	case 0x0218:	return _T("防抖");
	case 0x0219:	return _T("启用光学变焦");
	case 0x021b:	return _T("启用数码变焦");
	case 0x021d:	return _T("特殊光源");
	case 0x021e:	return _T("重新保存");
	case 0x021f:	return _T("场景选择");
	case 0x0223:	return _T("手动对焦距离");
	case 0x0224:	return _T("连拍间隔");
	case 0x0225:	return _T("闪光模式");
	case 0x0f00:	return _T("数据转储");
	default:		return _T("未知");
	}
}


String SanyoNote::TagValue(uint16 tag, const Data& val)
{
	return String();
}


void SanyoNote::RecordInfo(uint16 tag, const Data& value, OutputStr& output)
{
	String val;

	switch (tag)
	{
/*
	case 0x0001:
		switch (val.GetData())
		{
		case 0: val = _T("Auto");		break;
		case 1: val = _T("Night scene");	break;
		case 2: val = _T("Manual");		break;
		default: val = _T('(') + val.AsString() + _T(')');	break;
		}
		RecordTag(tag, _T("Mode"), val.c_str(), output);
		break;

	case 0x0002:
		switch (val.GetData())
		{
		case 0: val = _T("Good");	break;
		case 1: val = _T("Better");	break;
		case 2: val = _T("Best");	break;
		default: val = _T('(') + val.AsString() + _T(')');	break;
		}
		RecordTag(tag, _T("Quality"), val.c_str(), output);
		break;

	case 0x0003:
		switch (val.GetData())
		{
		case 2: val = _T("Custom");	break;
		case 3: val = _T("Auto");	break;
		default: val = _T('(') + val.AsString() + _T(')');	break;
		}
		RecordTag(tag, _T("Focus"), val.c_str(), output);
		break;

	case 0x0004:
		switch (val.GetData())
		{
		case 1: val = _T("Auto");	break;
		case 2: val = _T("On");		break;
		case 4: val = _T("Off");		break;
		case 6: val = _T("Red-eye reduction");	break;
		default: val = _T('(') + val.AsString() + _T(')');	break;
		}
		RecordTag(tag, _T("Flash"), val.c_str(), output);
		break;

	case 0x0007:
		switch (val.GetData())
		{
		case 0: val = _T("Auto");		break;
		case 1: val = _T("Daylight");	break;
		case 2: val = _T("Shade");		break;
		case 3: val = _T("Tungsten");	break;
		case 4: val = _T("Fluorescent");	break;
		case 5: val = _T("Manual");		break;
		default: val = _T('(') + val.AsString() + _T(')');	break;
		}
		RecordTag(tag, _T("White Balance"), val.c_str(), output);
		break;

	case 0x000a:
		RecordTag(tag, _T("Zoom"), val.AsString().c_str(), output);
		break;

	case 0x000b:
		switch (val.GetData())
		{
		case 0: val = _T("Normal");	break;
		case 1: val = _T("Soft");	break;
		case 2: val = _T("Hard");	break;
		default: val = _T('(') + val.AsString() + _T(')');	break;
		}
		RecordTag(tag, _T("Sharpness"), val.c_str(), output);
		break;

	case 0x000c:
		switch (val.GetData())
		{
		case 0: val = _T("Normal");	break;
		case 1: val = _T("Low");		break;
		case 2: val = _T("High");	break;
		default: val = _T('(') + val.AsString() + _T(')');	break;
		}
		RecordTag(tag, _T("Contrast"), val.c_str(), output);
		break;

	case 0x000d:
		switch (val.GetData())
		{
		case 0: val = _T("Normal");	break;
		case 1: val = _T("Low");		break;
		case 2: val = _T("High");	break;
		default: val = _T('(') + val.AsString() + _T(')');	break;
		}
		RecordTag(tag, _T("Saturation"), val.c_str(), output);
		break;

	case 0x0014:
		switch (val.GetData())
		{
		case 10: val = _T("100");	break;
		case 16: val = _T("200");	break;
		default: val = _T('(') + val.AsString() + _T(')');	break;
		}
		RecordTag(tag, _T("ISO"), val.c_str(), output);
		break;

	case 0x0017:
		switch (val.GetData())
		{
		case 1: val = _T("Full");	break;
		case 2: val = _T("Black & white");	break;
		case 3: val = _T("Sepia");	break;
		default: val = _T('(') + val.AsString() + _T(')');	break;
		}
		RecordTag(tag, _T("Color"), val.c_str(), output);
		break;

	case 0x1000:
		RecordTag(tag, _T("Time Zone City"), val.AsString().c_str(), output);
		break;

	case 0x1001:
		RecordTag(tag, _T("Daylight Savings"), val.AsString().c_str(), output);
		break;
*/
	case 0:
	default:
		RecordTag(tag, TagName(tag).c_str(), value.AsString().c_str(), output);
		break;
	}
}


String& SanyoNote::GetInfo(int index, String& out) const
{
	out = _T("-");
	return out;
}

bool SanyoNote::IsDataPresent(int index) const
{
	return false;
}

int SanyoNote::CompareNotes(int index, const MakerNote& note) const
{
	return 0;
}

bool SanyoNote::LessNote(int index, const MakerNote& note) const
{
	return false;
}


//////////////////////////////////////////////////////////////////////
/*
0x0001  Image Quality  	Unsigned Short  	1  	2 = High, 3 = Standard
0x0002 	? 	Undefined 	4 	Always 0, 1, 0, 7 in my camera - firmware version?
0x0003 	White Balance 	Unsigned Short 	1 	1 = Auto, 2 = Daylight, 3 = Cloudy, 4 = Halogen, 5 = Manual, 8 = Flash
0x0007 	Focus Mode 	Unsigned Short 	1 	1 = Auto, 2 = Manual
0x000f 	Spot Mode 	Unsigned Byte 	2 	Byte 1: Meaning unknown Byte 2: 1 = Spot mode on, 16 = Spot mode off
0x001a 	Image Stabilizer 	Unsigned Short 	1 	2 = Mode 1, 3 = Off, 4 = Mode 2
0x001c 	Macro Mode 	Unsigned Short 	1 	1 = Macro, 2 = Normal
0x001f 	Shooting Mode 	Unsigned Short 	1 	2 = Portrait, 3 = Scenery, 4 = Sports, 5 = Night Portrait,
6 = Program, 7 = Aperture Priority, 8 = Shutter Priority,
9 = Macro, 11 = Manual, 13 = Panning, 18 = Fireworks,
19 = Party, 20 = Snow, 21 = Night Scenery
0x0020 	Audio 	Unsigned Short 	1 	1 = Yes, 2 = No
0x0021 	? 	Undefined 	Varies 	
0x0022 	? 	Unsigned Short 	1 	
0x0023 	White Balance Adjust 	Short 	1 	White balance adjustment in units of 1/3EV
//Note that the EXIF declares this as unsigned but the data is signed
0x0024 	Flash Bias 	Short 	1 	Flash strength adjustment
//Note that the EXIF declares this as unsigned but the data is signed
0x0025 	? 	Undefined 	16 	
0x0026 	? 	Undefined 	4 	Always "0100" in my camera
0x0027 	? 	Unsigned Short 	1 	
0x0028 	Colour Effect 	Unsigned Short 	1 	1 = Off, 2 = Warm, 3 = Cool, 4 = Black and White, 5 = Sepia
0x0029 	? 	Unsigned Long 	1 	Some sort of subject distance measure?
0x002A 	? 	Unsigned Short 	1 	
0x002B 	? 	Unsigned Long 	1 	
0x002C 	Contrast 	Unsigned Short 	1 	0 = Standard, 1 = Low, 2 = High
0x002D 	Noise Reduction 	Unsigned Short 	1 	0 = Standard, 1 = Low, 2 = High
0x002E 	? 	Unsigned Short 	1 	
0x002F 	? 	Unsigned Short 	1 	
0x4449 	? 	Undefined 	512 
*/

PanasonicNote::PanasonicNote()
{}

String PanasonicNote::TagName(uint16 tag)
{
	switch (tag)
	{
	case 0x0001:	return _T("图像品质");
	//case 0x0002:	return _T("Firmware ver?");
	case 0x0003:	return _T("白平衡");
	case 0x0007:	return _T("对焦模式");
	case 0x000f:	return _T("Spot Mode");
	case 0x001a:	return _T("图像防抖");
	case 0x001c:	return _T("微距");
	case 0x001f:	return _T("拍摄模式");
	case 0x0020:	return _T("声音备忘");
	case 0x0023:	return _T("白平衡调整");
	case 0x0024:	return _T("闪光补偿");
	case 0x0028:	return _T("颜色效果");
	case 0x002b:	return _T("包围曝光顺序");
	case 0x002c:	return _T("对比度");
	case 0x002d:	return _T("降噪");
	case 0x0e00:	return _T("PrintIM");
	case 0x8001:	return _T("程序");
	default:		return _T("未知");
	}
}


String PanasonicNote::TagValue(uint16 tag, const Data& val)
{
	return String();
}


void PanasonicNote::RecordInfo(uint16 tag, const Data& val, OutputStr& output)
{
	// default val
	String str= _T('(') + val.AsString() + _T(')');

	switch (tag)
	{
	case 0x0001:
		switch (val.AsULong())
		{
		case 2: str = _T("高");		break;
		case 3: str = _T("标准");	break;
		}
		break;

	case 0x0003:	// WB
		switch (val.AsULong())
		{
		case 1: str = _T("自动");		break;
		case 2: str = _T("日光");	break;
		case 3: str = _T("多云");		break;
		case 4: str = _T("卤素灯");	break;
		case 5: str = _T("手动");		break;
		case 8: str = _T("闪光");		break;
		}
		break;

	case 0x0007:	// focus mode
		switch (val.AsULong())
		{
		case 1: str = _T("自动");	break;
		case 2: str = _T("手动");	break;
		}
		break;

	case 0x000f:	// spot?
		switch (val.AsULong())
		{
//0x000f 	Spot Mode 	Unsigned Byte 	2 	Byte 1: Meaning unknown Byte 2: 1 = Spot mode on, 16 = Spot mode off
		case 1:		str = _T("Spot On");	break;
		case 16:	str = _T("1 Area");		break;
		case 256:	str = _T("9 Area/On");	break;
		case 4096:	str = _T("3 Area/Off");	break;
		}
		break;

	case 0x001a:	// img stabilizer
		switch (val.AsULong())
		{
//0x001a 	Image Stabilizer 	Unsigned Short 	1 	2 = Mode 1, 3 = Off, 4 = Mode 2
		case 2: str = _T("Mode 1");	break;
		case 3: str = _T("Off");	break;
		case 4: str = _T("Mode 2");	break;
		}
		break;

	case 0x001c:
//0x001c 	Macro Mode 	Unsigned Short 	1 	1 = Macro, 2 = Normal
		switch (val.AsULong())
		{
		case 0x001: str = _T("On");		break;
		case 0x002: str = _T("Off");	break;
		case 0x101: str = _T("Tele-Macro");		break;
		}
		break;

	case 0x001f:
		switch (val.AsULong())
		{
//0x001f 	Shooting Mode 	Unsigned Short 	1
//2 = Portrait, 3 = Scenery, 4 = Sports, 5 = Night Portrait,
//6 = Program, 7 = Aperture Priority, 8 = Shutter Priority,
//9 = Macro, 11 = Manual, 13 = Panning, 18 = Fireworks,
//19 = Party, 20 = Snow, 21 = Night Scenery
		case 2: str = _T("肖像");		break;
		case 3: str = _T("风景");		break;
		case 4: str = _T("Sports");			break;
		case 5: str = _T("夜间肖像");	break;
		case 6: str = _T("程序");		break;
		case 7: str = _T("光圈优先");	break;
		case 8: str = _T("快门优先");	break;
		case 9: str = _T("微距");			break;
		case 11: str = _T("手动");		break;
		case 13: str = _T("追拍");		break;
		case 14: str = _T("简单");		break;
		case 18: str = _T("烟火");		break;
		case 19: str = _T("聚会");			break;
		case 20: str = _T("下雪");			break;
		case 21: str = _T("夜景");	break;
		case 22: str = _T("食物");	break;	// ?? verify
		}
		break;

	case 0x0020:
		switch (val.AsULong())
		{
//0x0020 	Audio 	Unsigned Short 	1 	1 = Yes, 2 = No
		case 1: str = _T("是");	break;
		case 2: str = _T("否");		break;
		}
		break;

	case 0x0023:	// White Balance Adjustment
		switch (val.AsULong() & 0xffff)
		{
		case 0xfffb: str = _T("-5"); break;
		case 0xfffc: str = _T("-4"); break;
		case 0xfffd: str = _T("-3"); break;
		case 0xfffe: str = _T("-2"); break;
		case 0xffff: str = _T("-1"); break;
		case 0x0000: str =  _T("0"); break;
		case 0x0001: str = _T("+1"); break;
		case 0x0002: str = _T("+2"); break;
		case 0x0003: str = _T("+3"); break;
		case 0x0004: str = _T("+4"); break;
		case 0x0005: str = _T("+5"); break;
		}
		break;

	case 0x0024:	// Flash Bias/Strength?
		switch (val.AsULong() & 0xffff)
		{
		case 0xfffa: str = _T("-2 EV");		break;
		case 0xfffb: str = _T("-1.67 EV");	break;
		case 0xfffc: str = _T("-1.33 EV");	break;
		case 0xfffd: str = _T("-1 EV");		break;
		case 0xfffe: str = _T("-0.67 EV");	break;
		case 0xffff: str = _T("-0.33 EV");	break;
		case 0x0000: str = _T("0 EV");		break;
		case 0x0001: str = _T("+0.33 EV");	break;
		case 0x0002: str = _T("+0.67 EV");	break;
		case 0x0003: str = _T("+1 EV");		break;
		case 0x0004: str = _T("+1.33 EV");	break;
		case 0x0005: str = _T("+1.67 EV");	break;
		case 0x0006: str = _T("+2 EV");		break;
		}
		break;

	case 0x0028:
		switch (val.AsULong())
		{
//0x0028 	Colour Effect 	Unsigned Short 	1 	1 = Off, 2 = Warm, 3 = Cool, 4 = Black and White, 5 = Sepia
		case 1: str = _T("关闭");	break;
		case 2: str = _T("暖");	break;
		case 3: str = _T("冷");	break;
		case 4: str = _T("黑白");	break;
		case 5: str = _T("褐色");	break;
		}
		break;

//	case 0x002b:	// bracketing order? (1=0EV, 2=-1/3EV and 3=+1/3EV)?
//		break;

	case 0x002c:
		switch (val.AsULong())
		{
//0x002C 	Contrast 	Unsigned Short 	1 	0 = Standard, 1 = Low, 2 = High
		case 0: str = _T("标准");	break;
		case 1: str = _T("低");	break;
		case 2: str = _T("高");	break;
		}
		break;

	case 0x002d:
		switch (val.AsULong())
		{
//0x002D 	Noise Reduction 	Unsigned Short 	1 	0 = Standard, 1 = Low, 2 = High
		case 0: str = _T("标准");	break;
		case 1: str = _T("低");	break;
		case 2: str = _T("高");	break;
		}
		break;

	case 0x8001:
		switch (val.AsULong())	// program
		{
		case 0: str = _T("正常");			break;
		//case 1: str = _T("?");			break;
		case 2: str = _T("肖像");		break;
		case 3: str = _T("风景");		break;
		case 4: str = _T("运动");			break;
		case 5: str = _T("夜景肖像");	break;
		//6-15  (not used on the FX01)
		case 16: str = _T("自拍");	break;
		//case 17: str = _T("");	break;
		case 18: str = _T("烟火");		break;
		case 19: str = _T("聚会");			break;
		case 20: str = _T("下雪");			break;
		case 21: str = _T("夜景");	break;
		case 22: str = _T("食物");			break;
		case 23: str = _T("婴儿");			break;
		case 24: str = _T("肤色柔和");		break;
		case 25: str = _T("烛光");	break;
		case 26: str = _T("星空");	break;
		case 27: str = _T("高感光度");	break;
		//case 28: str = _T("");	break;
		case 29: str = _T("水下");	break;
		default: str = _T("(?)");			break;
		}
		break;

	default:
		RecordTag(tag, TagName(tag).c_str(), val.AsString().c_str(), output);
		return;
	}

	RecordTag(tag, TagName(tag).c_str(), str.c_str(), output);
}


String& PanasonicNote::GetInfo(int index, String& out) const
{
	out = _T("-");
	return out;
}

bool PanasonicNote::IsDataPresent(int index) const
{
	return false;
}

int PanasonicNote::CompareNotes(int index, const MakerNote& note) const
{
	return 0;
}

bool PanasonicNote::LessNote(int index, const MakerNote& note) const
{
	return false;
}

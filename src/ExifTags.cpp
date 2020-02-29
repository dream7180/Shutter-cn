/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "File.h"
#include "Data.h"
#include "Rational.h"
#include "RString.h"
#include <math.h>
#include "resource.h"
#include "ExifTags.h"
#include <iomanip>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern const SRational g_UninitializedRationalVal(0x7ffe1234, 0x7ffdabcd);


const TCHAR* TagName(uint16 tag)
{
	const TCHAR* name= _T("未知");

	switch (tag)
	{
	case 0x001: name =	_T("互用索引");		break;
	case 0x002: name =	_T("互用版本");		break;

	case 0x0fe:	name =	_T("新建子文件类型");				break;
	case 0x0ff:	name =	_T("子文件类型");					break;

	case 0x100: name =	_T("图像宽度");					break;
	case 0x101: name =	_T("图像长度");					break;
	case 0x102: name =	_T("数据位数");				break;
	case 0x103: name =	_T("压缩");					break;
	case 0x106: name =	_T("像素合成");	break;
	case 0x10A: name =	_T("填充顺序");					break;
	case 0x10D: name =	_T("文档名称");				break;
	case 0x10E: name =	_T("图像描述");			break;
	case 0x10F: name =	_T("厂商");							break;
	case 0x110: name =	_T("型号");						break;
	case 0x111: name =	_T("图像资料位置");				break;
	case 0x112: name =	_T("拍摄方向");					break;
	case 0x115: name =	_T("每像素数据数");			break;
	case 0x116: name =	_T("每带行数");				break;
	case 0x117: name =	_T("每压缩带比特数");			break;
	case 0x11A: name =	_T("X 解析度");					break;
	case 0x11B: name =	_T("Y 解析度");					break;
	case 0x11C: name =	_T("平面配置");			break;
	case 0x128: name =	_T("分辨率单位");				break;
	case 0x12D: name =	_T("转移功能");			break;
	case 0x131: name =	_T("软件");						break;
	case 0x132: name =	_T("日期时间");					break;
	case 0x13B: name =	_T("作者");						break;
	case 0x13E: name =	_T("白点色度");					break;
	case 0x13F: name =	_T("主要色度");		break;
	case 0x14A: name =	_T("Sub IFDs");						break;
	case 0x156: name =	_T("转移范围");				break;
	case 0x200: name =	_T("JPEG Proc");					break;
	case 0x201: name =	_T("JPEG SOI 偏移量");	break;
	case 0x202: name =	_T("JPEG 比特数");	break;
	case 0x211: name =	_T("颜色空间转换矩阵系数");			break;
	case 0x212: name =	_T("色相抽样比率");			break;
	case 0x213: name =	_T("色相定位");			break;
	case 0x214: name =	_T("黑白参照值");		break;

	case 0x1000: name =	_T("关联的图像文件格式");	break;
	case 0x1001: name =	_T("关联的图像宽度");			break;
	case 0x1002: name =	_T("关联的图像高度");			break;

	case 0x828D: name =	_T("CFA Repeat Pattern Dim");		break;
	case 0x828E: name =	_T("CFA 模式");					break;
	case 0x828F: name =	_T("电池电量");				break;
	case 0x8298: name =	_T("版权");					break;
	case 0x829A: name =	_T("曝光时长");				break;
	case 0x829D: name =	_T("光圈");						break;
	case 0x83BB: name =	_T("IPTC/NAA");						break;
	case 0x8769: name =	_T("Exif 偏移");					break;
	case 0x8773: name =	_T("内部颜色配置");			break;
	case 0x8822: name =	_T("曝光程序");				break;
	case 0x8824: name =	_T("光谱灵敏度");			break;
	case 0x8825: name =	_T("GPS 信息");						break;
	case 0x8827: name =	_T("感光度");			break;
	case 0x8828: name =	_T("光电系数");		break;
	case 0x9000: name =	_T("Exif 版本");					break;
	case 0x9003: name =	_T("原始日期时间");			break;
	case 0x9004: name =	_T("数字化日期时间");			break;
	case 0x9101: name =	_T("组件配置");		break;
	case 0x9102: name =	_T("压缩时每像素色彩位");	break;
	case 0x9201: name =	_T("快门速度");			break;
	case 0x9202: name =	_T("光圈");				break;
	case 0x9203: name =	_T("亮度");				break;
	case 0x9204: name =	_T("曝光补偿");			break;
	case 0x9205: name =	_T("最大光圈");			break;
	case 0x9206: name =	_T("主体距离");				break;
	case 0x9207: name =	_T("测光模式");				break;
	case 0x9208: name =	_T("光源");					break;
	case 0x9209: name =	_T("闪光");						break;
	case 0x920A: name =	_T("焦距");					break;
	case 0x9214: name =	_T("主体区域");					break;
	case 0x9217: name =	_T("感光器类型");				break;	// TIFF/EP
	case 0x927C: name =	_T("厂商信息");					break;
	case 0x9286: name =	_T("用户评价");					break;
	case 0x9290: name =	_T("时间(秒)");				break;
	case 0x9291: name =	_T("原始时间(秒)");		break;
	case 0x9292: name =	_T("数字化时间(秒)");		break;

	case 0xA000: name =	_T("FlashPix 版本");			break;
	case 0xA001: name =	_T("色彩空间");					break;
	case 0xA002: name =	_T("EXIF 图像宽度");				break;
	case 0xA003: name =	_T("EXIF 图像长度");			break;
	case 0xA004: name =	_T("关联的声音文件");			break;
	case 0xA005: name =	_T("互用偏移量");		break;
	case 0xA20B: name =	_T("闪光能量");					break;	//case 0x920B in TIFF/EP
	case 0xA20C: name =	_T("空间频率响应");	break;	//case 0x920C    -  -
	case 0xA20E: name =	_T("焦平面 X 轴解析度");		break;	//case 0x920E    -  -
	case 0xA20F: name =	_T("焦平面 Y 轴解析度");		break;	//case 0x920F    -  -
	case 0xA210: name =	_T("焦平面分辨率单位");	break;	//case 0x9210    -  -
	case 0xA214: name =	_T("主体位置");				break;	//case 0x9214    -  -
	case 0xA215: name =	_T("曝光指数");				break;	//case 0x9215    -  -
	case 0xA217: name =	_T("感光器类型");				break;	//case 0x9217    -  -

	case 0xA300: name =	_T("源文件");					break;
	case 0xA301: name =	_T("场景类型");					break;
	case 0xA302: name =	_T("CFA 模式");					break;

	case 0xA401: name =	_T("自定义图像处理");				break;
	case 0xA402: name =	_T("曝光模式");				break;
	case 0xA403: name =	_T("白平衡");				break;
	case 0xA404: name =	_T("数码变焦");			break;
	case 0xA405: name =	_T("35毫米胶片焦距");	break;
	case 0xA406: name =	_T("场景拍摄类型");			break;
	case 0xA407: name =	_T("增益控制");					break;
	case 0xA408: name =	_T("对比度");						break;
	case 0xA409: name =	_T("饱和度");					break;
	case 0xA40A: name =	_T("锐度");					break;
	case 0xA40B: name =	_T("设备设定描述");	break;
	case 0xA40C: name =	_T("主体距离范围");		break;
	case 0xA420: name =	_T("图像唯一 ID");				break;

	// Lens
	// DC-010-2017
	// Exif 2.31 metadata for XMP
	// http://www.cipa.jp/std/documents/e/DC-010-2017_E.pdf
	case 0xA432: name = _T("镜头规格");			break;
	case 0xA433: name = _T("镜头制造商");			break;
	case 0xA434: name = _T("镜头型号");					break;
	case 0xA435: name = _T("镜头序列号");			break;

	// PrintIM?
	case 0xc4a5: name =	_T("打印图像匹配");			break;

	// Windows
	case 0x9c9b: name =	_T("窗口标题");				break;
	case 0x9c9c: name =	_T("窗口注释");				break;
	case 0x9c9d: name =	_T("窗口作者");				break;
	case 0x9c9e: name =	_T("窗口关键字");				break;
	case 0x9c9f: name =	_T("窗口主题");				break;

	}

	return name;
}


const TCHAR* FileSource(int val)
{
	switch (val)
	{
	case 3: return _T("DSC");
	default: return _T("?");
	}
}


const TCHAR* SceneType(int scene_type)
{
	switch (scene_type)
	{
	case 1: return _T("直接拍摄");
	default: return _T("?");
	}
}


const TCHAR* ColorSpace(int16 space)
{
	switch (space)
	{
	case 1: return _T("sRGB");
	case -1: return _T("未校正的");
	default: return _T("?");
	}
}


const TCHAR* CustomRendered(int16 val)
{
	switch (val)
	{
	case 0: return _T("常规处理");
	case 1: return _T("自定义处理");
	default: return _T("?");
	}
}


const TCHAR* ExposureMode(int16 val)
{
	switch (val)
	{
	case 0: return _T("自动曝光");
	case 1: return _T("手动曝光");
	case 2: return _T("自动包围曝光");
	default: return _T("?");
	}
}


const TCHAR* WhiteBalance(int16 val)
{
	switch (val)
	{
	case 0: return _T("自动白平衡");
	case 1: return _T("手动白平衡");
	default: return _T("?");
	}
}


const TCHAR* SensingMethod(int method)
{
	switch (method)
	{
	case 1: return _T("未定义");
	case 2: return _T("One-chip color area sensor");
	case 3: return _T("Two-chip color area sensor");
	case 4: return _T("Three-chip color area sensor");
	case 5: return _T("Color sequential area sensor");
	case 7: return _T("Trilinear sensor");
	case 8: return _T("Color sequential linear sensor");
	default: return _T("?");
	}
}


const TCHAR* SceneCaptureType(int scene_type)
{
	switch (scene_type)
	{
	case 0: return _T("标准");
	case 1: return _T("风景");
	case 2: return _T("肖像");
	case 3: return _T("夜景");
	default: return _T("?");
	}
}


const TCHAR* GainControl(int val)
{
	switch (val)
	{
	case 0: return _T("无");
	case 1: return _T("低增益加");
	case 2: return _T("高增益加");
	case 3: return _T("低增益减");
	case 4: return _T("高增益减");
	default: return _T("?");
	}
}


const TCHAR* Contrast(int val)
{
	switch (val)
	{
	case 0: return _T("常规");
	case 1: return _T("柔和");
	case 2: return _T("高对比度");
	default: return _T("?");
	}
}


const TCHAR* Saturation(int val)
{
	switch (val)
	{
	case 0: return _T("常规");
	case 1: return _T("低饱和度");
	case 2: return _T("高饱和度");
	default: return _T("?");
	}
}


const TCHAR* Sharpness(int val)
{
	switch (val)
	{
	case 0: return _T("常规");
	case 1: return _T("柔和");
	case 2: return _T("锐利");
	default: return _T("?");
	}
}


const TCHAR* SubjectDistanceRange(int val)
{
	switch (val)
	{
	case 0: return _T("未知");
	case 1: return _T("微距");
	case 2: return _T("近距");
	case 3: return _T("远距");
	default: return _T("?");
	}
}


String ApertureValAndFNum(Rational av)
{
	extern String FNumberFromAV(Rational aperture_value);

	oStringstream ost;
	ost << av.Double() << _T(" (F") << FNumberFromAV(av) << _T(")");

	return ost.str();
}


String ExposureValAndShutterSpeed(SRational ev)
{
	extern String ExposureTimeFromSV(SRational shutter_speed_value);

	oStringstream ost;
	ost << ev.Double() << _T(" (") << ExposureTimeFromSV(ev) << _T(" s)");

	return ost.str();
}


extern String VersionFieldFmt(const Data& ver)
{
	if (ver.IsUndefData() && ver.Components() == 4)
	{
		String str(4, 0);//= ver.AsString(false, true);

		const uint8* p= ver.DataBytes();

		for (int i= 0; i < 4; ++i)
			str[i] = p[i] < 9 ? p[i] | _T('0') : p[i];

//		if (str.length() == 4)
			str.insert(2, _T("."));
		if (str[0] == _T('0'))
			str.erase(0, 1);

		return str;
	}
	else
		return ver.AsString();
}


extern String ExifVersion(const Data& ver)
{
	return VersionFieldFmt(ver);
}


extern String FlashPixVersion(const Data& ver)
{
	return VersionFieldFmt(ver);
}


extern String InteroperabilityVersion(const Data& ver)
{
	return VersionFieldFmt(ver);
}


extern String ComponentConfiguration(const Data& v)
{
	if (v.IsUndefData() && v.Components() == 4)
	{
		const uint8* p= v.DataBytes();
		oStringstream ost;
		for (int i= 0; i < 4; ++i)
		{
			if (p[i] == 0 || p[i] == '0')
				; // skip
			else if (p[i] == 1 || p[i] == '1')
				ost << _T('Y');
			else if (p[i] == 2 || p[i] == '2')
				ost << _T("Cb");
			else if (p[i] == 3 || p[i] == '3')
				ost << _T("Cr");
			else if (p[i] == 4 || p[i] == '4')
				ost << _T('R');
			else if (p[i] == 5 || p[i] == '5')
				ost << _T('G');
			else if (p[i] == 6 || p[i] == '6')
				ost << _T('B');
			else
				ost << _T('?');
		}
		return ost.str();
	}
	else
		return v.AsString();
}


extern const TCHAR* ResolutionUnit(int n)
{
	switch (n)
	{
	case 2: return _T("英寸");
	case 3: return _T("厘米");
	default: return _T("?");
	}
}


extern const TCHAR* ImgCompressionType(int n)
{
	switch (n)
	{
	case 1:		return _T("未压缩");
	case 2:		return _T("CCIT 1D Modified Huffman RLE");
	case 3:		return _T("Group 3 Fax");
	case 4:		return _T("Group 4 Fax");
	case 5:		return _T("LZW");
	case 6:		return _T("JPEG");
	case 7:		return _T("JPEG");
	case 8:		return _T("Adobe Deflate");
	case 9:		return _T("JBIG B&W");
	case 10:	return _T("JBIG Color");
	case 32766:	return _T("Next");
	case 32771:	return _T("CCIRLEW");
	case 32773:	return _T("PackBits");
	case 32809:	return _T("Thunderscan");
	case 32895:	return _T("IT8CTPAD");
	case 32896:	return _T("IT8LW");
	case 32897:	return _T("IT8MP");
	case 32898:	return _T("IT8BL");
	case 32908:	return _T("PixarFilm");
	case 32909:	return _T("PixarLog");
	case 32946:	return _T("Deflate");
	case 32947:	return _T("DCS");
	case 34661:	return _T("JBIG");
	case 34676:	return _T("SGILog");
	case 34677:	return _T("SGILog24");
	case 34712:	return _T("JPEG 2000");
	case 34713:	return _T("Nikon NEF Compressed");
	default: return _T("?");
	}
}


extern String BrightnessValue(SRational bv)
{
	if (!bv.Valid() || bv.numerator_ == 0)
		return _T("未知");
	else if (bv.numerator_ == ~0)
		return _T("未知");
	else
	{
		oStringstream ost;
		ost << bv.Double();
		return ost.str();
	}
}


extern String FNumberFromAV(Rational aperture_value)
{
	if (aperture_value.Valid())
	{
		double FNum= pow(1.4142135623730950488016887242097, aperture_value.Double());
		oStringstream ost;
		ost.precision(2);
		ost << FNum;
		return ost.str();
	}
	else
		return _T("-");
}


extern String FNumber(Rational fnumber)
{
	if (fnumber.Valid())
		return fnumber.UnitNumerator();
	else
		return _T("-");
}


extern String SubjectDistance(Rational dist)
{
	if (!dist.Valid() || dist.numerator_ == 0)
		return _T("未知");
	else if (dist.numerator_ == ~0)
//		|| dist.numerator_ == 0xffff)	// some lame Canon's cameras...
		return _T("无穷远");
	else
	{
		oStringstream ost;
		ost << dist.Double() << _T(" m");
		return ost.str();
	}
}


extern String ExposureTime(Rational exp_time)
{
	if (exp_time.Valid())
		return exp_time.UnitNumerator(true);
	else
		return _T("-");
}


extern String ExposureTimeFromSV(SRational shutter_speed_value)
{
	if (shutter_speed_value.Valid())
	{
		double speed= 0.0;
		// minolta 2300 writes negative values here on word instead of dword
		if (shutter_speed_value.numerator_ > 0x7fff && shutter_speed_value.numerator_ < 0x10000)
		{
			int16 num= static_cast<int16>(shutter_speed_value.numerator_);
			speed = double(num) / shutter_speed_value.denominator_;
		}
		else
			speed = shutter_speed_value.Double();

		oStringstream ost;
		if (speed == 0.0)
		{
			ost << _T("1");
		}
		else if (speed <= 0.0)
		{
			double exp_time= pow(2.0, -speed);
			ost.precision(2);
			ost << std::fixed << exp_time;
		}
		else if (speed < 32.0)
		{
			double exp_time= pow(2.0, speed);
			if (exp_time < 10.0)
				ost.precision(1);
			else
				ost.precision(0);
			ost << _T("1/") << std::fixed << exp_time;
		}
		else
			return _T("-");
		return ost.str();
	}
	else
		return _T("-");
}


extern String DateTimeISO(const COleDateTime& tm, bool dateOnly)
{
	oStringstream ost;

	SYSTEMTIME time;
	if (tm.GetAsSystemTime(time))
	{
		// convert WORDs to ints to avoid confusion with wchar_t
		int y= time.wYear;
		int m= time.wMonth;
		int d= time.wDay;

		ost << std::setfill(_T('0')) << std::setw(4) << y << _T('-') << std::setw(2) << m << _T('-') << std::setw(2) << d;

		if (!dateOnly)
		{
			//TODO: time zone...

		}
	}

	return ost.str();
}


extern bool CFAPattern(FileStream& ifs, uint32 components, String& pattern)
{
	if (components <= 4)
		return false;

	uint32 w= ifs.GetUInt16();
	uint32 h= ifs.GetUInt16();
	uint32 size= w * h;

	if (size != components - 2 * sizeof(uint16))
		return false;

	if (size > 16)
		return false;	// valid, but too big

	uint8 pat[16];
	ifs.Read(pat, size);

	oStringstream ost;
	ost << w << _T("x") << h << _T(": [");

	size_t index= 0;
	for (uint32 y= 0; y < h; ++y)
	{
		for (uint32 x= 0; x < w; ++x)
		{
			switch (pat[index++])
			{
			case 0:		ost << _T("R"); break;
			case 1:		ost << _T("G"); break;
			case 2:		ost << _T("B"); break;
			case 3:		ost << _T("C"); break;
			case 4:		ost << _T("M"); break;
			case 5:		ost << _T("Y"); break;
			case 6:		ost << _T("W"); break;
			default:	ost << _T("?"); break;
			}
			if (x < w - 1)
				ost << _T(" ");
		}
		if (y < h - 1)
			ost << _T(", ");
	}
	ost << _T("]");

	pattern = ost.str();

	return true;
}


extern String Orientation(uint16 orientation)
{
	switch (orientation)
	{
	case 1:	// "Normal";
		return RString(IDS_PHOTO_ORIENTATION_0).CStr();
	case 3:	// "Upside down";
		return RString(IDS_PHOTO_ORIENTATION_1).CStr();
	case 6:	// "90� CCW";
		return RString(IDS_PHOTO_ORIENTATION_2).CStr();
	case 8:	// "90� CW";
		return RString(IDS_PHOTO_ORIENTATION_3).CStr();
	case 9:	// "Unknown";
		return RString(IDS_PHOTO_ORIENTATION_4).CStr();
	default:
		return _T("-");
	}
}


extern String ExposureProgram(uint32 exposure_program)
{
	switch (exposure_program)
	{
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
		return RString(IDS_PHOTO_PROG_0 + exposure_program).CStr();
	default:
		return _T("-");
	}
}


extern String MeteringMode(uint32 metering_mode)
{
	switch (metering_mode)
	{
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:		return RString(IDS_PHOTO_MET_MODE_0 + metering_mode).CStr();

	case 255:	return _T("Other");
	default:	return _T("-");
	}
}


extern String ISOSpeed(uint16 iso_speed)
{
	if (iso_speed == uint16(ISO_SPEED_UNKNOWN))
		return _T("-");
	else if (iso_speed == uint16(ISO_SPEED_AUTO))
		return _T("Auto");
	else
	{
		oStringstream ost;
		ost << static_cast<uint32>(iso_speed);
		return ost.str();
	}
}


extern String LightSource(uint32 light_source)
{
	switch (light_source)
	{
	case 0:
	case 1:
	case 2:
	case 3:		return RString(IDS_PHOTO_LIGHT_0 + light_source).CStr();
	case 4:		return RString(IDS_PHOTO_LIGHT_10).CStr();

	case 9:		return _T("晴朗");
	case 10:	return _T("多云");
	case 11:	return _T("阴影");
	case 12:	return _T("日光荧光灯 (D 5700 � 7100K)");
	case 13:	return _T("白昼荧光灯 (N 4600 � 5400K)");
	case 14:	return _T("冷白光荧光灯 (W 3900 � 4500K)");
	case 15:	return _T("白光荧光灯 (WW 3200 � 3700K)");
	case 17:	return _T("标准光 A");
	case 18:	return _T("标准光 B");
	case 19:	return _T("标准光 C");
	case 20:	return _T("D55");
	case 21:	return _T("D65");
	case 22:	return _T("D75");
	case 23:	return _T("D50");
	case 24:	return _T("ISO 工作室钨灯");
	case 255:	return _T("其他光源");

	default:
		return _T("-");
	}
}


extern String Flash(uint16 flash)
{
	if (flash == uint16(~0))
		return _T("-");		// flash info not present

	switch (flash)
	{
	case 0x0000: return _T("无闪光");
	case 0x0001: return _T("闪光灯闪光");
	case 0x0005: return _T("未检测到频闪反射光");
	case 0x0007: return _T("检测到频闪反射光");
	case 0x0008: return _T("闪光灯未闪光, 强制闪光模式");	// ??? Canon G5 reports this too
	case 0x0009: return _T("闪光灯闪光, 强制闪光模式");
	case 0x000D: return _T("闪光灯闪光, 强制闪光模式, 未检测到反射光");
	case 0x000F: return _T("闪光灯闪光, 强制闪光模式, 检测到反射光");
	case 0x0010: return _T("未闪光, 强制闪光模式");
	case 0x0014: return _T("强制闪光被抑制, 未检测到频闪反射光");	// HP
	case 0x0018: return _T("未闪光, 自动模式");
	case 0x0019: return _T("闪光灯闪光, 自动模式");
	case 0x001D: return _T("闪光灯闪光, 自动模式, 未检测到反射光");
	case 0x001F: return _T("闪光灯闪光, 自动模式, 检测到反射光");
	case 0x0020: return _T("无闪光灯功能");
	case 0x0041: return _T("闪光灯闪光, 消除红眼模式");
	case 0x0045: return _T("闪光灯闪光, 消除红眼模式, 未检测到反射光");
	case 0x0047: return _T("闪光灯闪光, 消除红眼模式, 检测到反射光");
	case 0x0049: return _T("闪光灯闪光, 强制闪光模式, 消除红眼模式");
	case 0x004D: return _T("闪光灯闪光, 强制闪光模式, 消除红眼模式, 未检测到反射光");
	case 0x004F: return _T("闪光灯闪光, 强制闪光模式, 消除红眼模式, 检测到反射光");
	case 0x0050: return _T("未闪光, 强制闪光被抑制");
	case 0x0059: return _T("闪光灯闪光, 自动模式, 消除红眼模式");
	case 0x005D: return _T("闪光灯闪光, 自动模式, 未检测到反射光, 消除红眼模式");
	case 0x005F: return _T("闪光灯闪光, 自动模式, 检测到反射光, 消除红眼模式");
	}

	ASSERT(false);
	return _T("?");		// unknown value
}


extern String FocalLength(Rational focal_length)
{
	double fl= focal_length.Double();
	if (fl == 0.0)
		return _T("-");
	oStringstream ost;
	ost << std::fixed << std::setprecision(1) << fl;
	return ost.str();
}


extern String FormatEvValue(double val)
{
	if (val == 0.0)
		return _T("0");

	oStringstream ost;
	if (val > 0.0)
		ost << _T("+");

	ost << std::fixed << std::setprecision(1) << val;

	return ost.str();
}


extern String ExposureBias(SRational exp_bias)
{
	if (exp_bias == g_UninitializedRationalVal)
		return _T("-");

	return FormatEvValue(exp_bias.Double());
}


extern String PhotoWidth(uint32 width)
{
	oStringstream ost; ost << width; return ost.str();
}


extern String PhotoHeight(uint32 height)
{
	oStringstream ost; ost << height; return ost.str();
}


extern const TCHAR* PhotometricInterpretation(int val)
{
	switch (val)
	{
	case 0:		return _T("WhiteIsZero");
	case 1:		return _T("BlackIsZero");
	case 2:		return _T("RGB");
	case 3:		return _T("RGB Palette");
	case 4:		return _T("Transparency Mask");
	case 5:		return _T("CMYK");
	case 6:		return _T("YCbCr");
	case 8:		return _T("CIELab");
	default:	return _T("?");
	}
}

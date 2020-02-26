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
	const TCHAR* name= _T("Unknown");

	switch (tag)
	{
	case 0x001: name =	_T("Interoperability Index");		break;
	case 0x002: name =	_T("Interoperability Version");		break;

	case 0x0fe:	name =	_T("New Subfile Type");				break;
	case 0x0ff:	name =	_T("Subfile Type");					break;

	case 0x100: name =	_T("Image Width");					break;
	case 0x101: name =	_T("Image Length");					break;
	case 0x102: name =	_T("Bits Per Sample");				break;
	case 0x103: name =	_T("Compression");					break;
	case 0x106: name =	_T("Photometric Interpretation");	break;
	case 0x10A: name =	_T("Fill Order");					break;
	case 0x10D: name =	_T("Document Name");				break;
	case 0x10E: name =	_T("Image Description");			break;
	case 0x10F: name =	_T("Make");							break;
	case 0x110: name =	_T("Model");						break;
	case 0x111: name =	_T("Strip Offsets");				break;
	case 0x112: name =	_T("Orientation");					break;
	case 0x115: name =	_T("Samples Per Pixel");			break;
	case 0x116: name =	_T("Rows Per Strip");				break;
	case 0x117: name =	_T("Strip Byte Counts");			break;
	case 0x11A: name =	_T("X Resolution");					break;
	case 0x11B: name =	_T("Y Resolution");					break;
	case 0x11C: name =	_T("Planar Configuration");			break;
	case 0x128: name =	_T("Resolution Unit");				break;
	case 0x12D: name =	_T("Transfer Function");			break;
	case 0x131: name =	_T("Software");						break;
	case 0x132: name =	_T("Date Time");					break;
	case 0x13B: name =	_T("Artist");						break;
	case 0x13E: name =	_T("White Point");					break;
	case 0x13F: name =	_T("Primary Chromaticities");		break;
	case 0x14A: name =	_T("Sub IFDs");						break;
	case 0x156: name =	_T("Transfer Range");				break;
	case 0x200: name =	_T("JPEG Proc");					break;
	case 0x201: name =	_T("JPEG Interchange Format Offset");	break;
	case 0x202: name =	_T("JPEG Interchange Format Length");	break;
	case 0x211: name =	_T("YCbCr Coefficients");			break;
	case 0x212: name =	_T("YCbCr Subsampling");			break;
	case 0x213: name =	_T("YCbCr Positioning");			break;
	case 0x214: name =	_T("Reference Black White");		break;

	case 0x1000: name =	_T("Related Image File Format");	break;
	case 0x1001: name =	_T("Related Image Width");			break;
	case 0x1002: name =	_T("Related Image Height");			break;

	case 0x828D: name =	_T("CFA Repeat Pattern Dim");		break;
	case 0x828E: name =	_T("CFA Pattern");					break;
	case 0x828F: name =	_T("Battery Level");				break;
	case 0x8298: name =	_T("Copyright");					break;
	case 0x829A: name =	_T("Exposure Time");				break;
	case 0x829D: name =	_T("F-Number");						break;
	case 0x83BB: name =	_T("IPTC/NAA");						break;
	case 0x8769: name =	_T("Exif Offset");					break;
	case 0x8773: name =	_T("Inter Color Profile");			break;
	case 0x8822: name =	_T("Exposure Program");				break;
	case 0x8824: name =	_T("Spectral Sensitivity");			break;
	case 0x8825: name =	_T("GPS Info");						break;
	case 0x8827: name =	_T("ISO Speed Ratings");			break;
	case 0x8828: name =	_T("Optoelectric Coefficient");		break;
	case 0x9000: name =	_T("Exif Version");					break;
	case 0x9003: name =	_T("Date Time Original");			break;
	case 0x9004: name =	_T("Date Time Digitized");			break;
	case 0x9101: name =	_T("Components Configuration");		break;
	case 0x9102: name =	_T("Compressed Bits Per Pixel");	break;
	case 0x9201: name =	_T("Shutter Speed Value");			break;
	case 0x9202: name =	_T("Aperture Value");				break;
	case 0x9203: name =	_T("Brightness Value");				break;
	case 0x9204: name =	_T("Exposure Bias Value");			break;
	case 0x9205: name =	_T("Max Aperture Value");			break;
	case 0x9206: name =	_T("Subject Distance");				break;
	case 0x9207: name =	_T("Metering Mode");				break;
	case 0x9208: name =	_T("Light Source");					break;
	case 0x9209: name =	_T("Flash");						break;
	case 0x920A: name =	_T("Focal Length");					break;
	case 0x9214: name =	_T("Subject Area");					break;
	case 0x9217: name =	_T("Sensing Method");				break;	// TIFF/EP
	case 0x927C: name =	_T("Maker Note");					break;
	case 0x9286: name =	_T("User Comment");					break;
	case 0x9290: name =	_T("Subsecond Time");				break;
	case 0x9291: name =	_T("Subsecond Time Original");		break;
	case 0x9292: name =	_T("Subsecond Time Digitized");		break;

	case 0xA000: name =	_T("Flash Pix Version");			break;
	case 0xA001: name =	_T("Color Space");					break;
	case 0xA002: name =	_T("EXIF Image Width");				break;
	case 0xA003: name =	_T("EXIF Image Length");			break;
	case 0xA004: name =	_T("Related Sound File");			break;
	case 0xA005: name =	_T("Interoperability Offset");		break;
	case 0xA20B: name =	_T("Flash Energy");					break;	//case 0x920B in TIFF/EP
	case 0xA20C: name =	_T("Spatial Frequency Response");	break;	//case 0x920C    -  -
	case 0xA20E: name =	_T("Focal Plane X Resolution");		break;	//case 0x920E    -  -
	case 0xA20F: name =	_T("Focal Plane Y Resolution");		break;	//case 0x920F    -  -
	case 0xA210: name =	_T("Focal Plane Resolution Unit");	break;	//case 0x9210    -  -
	case 0xA214: name =	_T("Subject Location");				break;	//case 0x9214    -  -
	case 0xA215: name =	_T("Exposure Index");				break;	//case 0x9215    -  -
	case 0xA217: name =	_T("Sensing Method");				break;	//case 0x9217    -  -

	case 0xA300: name =	_T("File Source");					break;
	case 0xA301: name =	_T("Scene Type");					break;
	case 0xA302: name =	_T("CFA Pattern");					break;

	case 0xA401: name =	_T("Custom Rendered");				break;
	case 0xA402: name =	_T("Exposure Mode");				break;
	case 0xA403: name =	_T("White Balance");				break;
	case 0xA404: name =	_T("Digital Zoom Ratio");			break;
	case 0xA405: name =	_T("Focal Length in 35 mm Film");	break;
	case 0xA406: name =	_T("Scene Capture Type");			break;
	case 0xA407: name =	_T("Gain Control");					break;
	case 0xA408: name =	_T("Contrast");						break;
	case 0xA409: name =	_T("Saturation");					break;
	case 0xA40A: name =	_T("Sharpness");					break;
	case 0xA40B: name =	_T("Device Setting Description");	break;
	case 0xA40C: name =	_T("Subject Distance Range");		break;
	case 0xA420: name =	_T("Image Unique ID");				break;

	// Lens
	// DC-010-2017
	// Exif 2.31 metadata for XMP
	// http://www.cipa.jp/std/documents/e/DC-010-2017_E.pdf
	case 0xA432: name = _T("Lens Specification");			break;
	case 0xA433: name = _T("Lens Manufacturer");			break;
	case 0xA434: name = _T("Lens Model");					break;
	case 0xA435: name = _T("Lens Serial Number");			break;

	// PrintIM?
	case 0xc4a5: name =	_T("Print Image Matching");			break;

	// Windows
	case 0x9c9b: name =	_T("Windows Title");				break;
	case 0x9c9c: name =	_T("Windows Comment");				break;
	case 0x9c9d: name =	_T("Windows Author");				break;
	case 0x9c9e: name =	_T("Windows Keywords");				break;
	case 0x9c9f: name =	_T("Windows Subject");				break;

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
	case 1: return _T("Directly photographed image");
	default: return _T("?");
	}
}


const TCHAR* ColorSpace(int16 space)
{
	switch (space)
	{
	case 1: return _T("sRGB");
	case -1: return _T("Uncalibrated");
	default: return _T("?");
	}
}


const TCHAR* CustomRendered(int16 val)
{
	switch (val)
	{
	case 0: return _T("Normal process");
	case 1: return _T("Custom process");
	default: return _T("?");
	}
}


const TCHAR* ExposureMode(int16 val)
{
	switch (val)
	{
	case 0: return _T("Auto exposure");
	case 1: return _T("Manual exposure");
	case 2: return _T("Auto bracket");
	default: return _T("?");
	}
}


const TCHAR* WhiteBalance(int16 val)
{
	switch (val)
	{
	case 0: return _T("Auto white balance");
	case 1: return _T("Manual white balance");
	default: return _T("?");
	}
}


const TCHAR* SensingMethod(int method)
{
	switch (method)
	{
	case 1: return _T("Not defined");
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
	case 0: return _T("Standard");
	case 1: return _T("Landscape");
	case 2: return _T("Portrait");
	case 3: return _T("Night Scene");
	default: return _T("?");
	}
}


const TCHAR* GainControl(int val)
{
	switch (val)
	{
	case 0: return _T("None");
	case 1: return _T("Low gain up");
	case 2: return _T("High gain up");
	case 3: return _T("Low gain down");
	case 4: return _T("High gain down");
	default: return _T("?");
	}
}


const TCHAR* Contrast(int val)
{
	switch (val)
	{
	case 0: return _T("Normal");
	case 1: return _T("Soft");
	case 2: return _T("Hard");
	default: return _T("?");
	}
}


const TCHAR* Saturation(int val)
{
	switch (val)
	{
	case 0: return _T("Normal");
	case 1: return _T("Low saturation");
	case 2: return _T("High saturation");
	default: return _T("?");
	}
}


const TCHAR* Sharpness(int val)
{
	switch (val)
	{
	case 0: return _T("Normal");
	case 1: return _T("Soft");
	case 2: return _T("Hard");
	default: return _T("?");
	}
}


const TCHAR* SubjectDistanceRange(int val)
{
	switch (val)
	{
	case 0: return _T("Unknown");
	case 1: return _T("Macro");
	case 2: return _T("Close view");
	case 3: return _T("Distant view");
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
	case 2: return _T("inch");
	case 3: return _T("cm");
	default: return _T("?");
	}
}


extern const TCHAR* ImgCompressionType(int n)
{
	switch (n)
	{
	case 1:		return _T("Uncompressed");
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
		return _T("Unknown");
	else if (bv.numerator_ == ~0)
		return _T("Unknown");
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
		return _T("Unknown");
	else if (dist.numerator_ == ~0)
//		|| dist.numerator_ == 0xffff)	// some lame Canon's cameras...
		return _T("Infinity");
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
	case 6:	// "90° CCW";
		return RString(IDS_PHOTO_ORIENTATION_2).CStr();
	case 8:	// "90° CW";
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

	case 9:		return _T("Fine weather");
	case 10:	return _T("Cloudy weather");
	case 11:	return _T("Shade");
	case 12:	return _T("Daylight fluorescent (D 5700 – 7100K)");
	case 13:	return _T("Day white fluorescent (N 4600 – 5400K)");
	case 14:	return _T("Cool white fluorescent (W 3900 – 4500K)");
	case 15:	return _T("White fluorescent (WW 3200 – 3700K)");
	case 17:	return _T("Standard light A");
	case 18:	return _T("Standard light B");
	case 19:	return _T("Standard light C");
	case 20:	return _T("D55");
	case 21:	return _T("D65");
	case 22:	return _T("D75");
	case 23:	return _T("D50");
	case 24:	return _T("ISO studio tungsten");
	case 255:	return _T("Other light source");

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
	case 0x0000: return _T("Not fired");
	case 0x0001: return _T("Flash fired");
	case 0x0005: return _T("Strobe return light not detected");
	case 0x0007: return _T("Strobe return light detected");
	case 0x0008: return _T("Flash did not fire, compulsory flash mode");	// ??? Canon G5 reports this too
	case 0x0009: return _T("Flash fired, compulsory flash mode");
	case 0x000D: return _T("Flash fired, compulsory flash mode, return light not detected");
	case 0x000F: return _T("Flash fired, compulsory flash mode, return light detected");
	case 0x0010: return _T("Not fired, compulsory flash mode");
	case 0x0014: return _T("Compulsory flash suppression, strobe return light not detected");	// HP
	case 0x0018: return _T("Not fired, auto mode");
	case 0x0019: return _T("Flash fired, auto mode");
	case 0x001D: return _T("Flash fired, auto mode, return light not detected");
	case 0x001F: return _T("Flash fired, auto mode, return light detected");
	case 0x0020: return _T("No flash function");
	case 0x0041: return _T("Flash fired, red-eye reduction mode");
	case 0x0045: return _T("Flash fired, red-eye reduction mode, return light not detected");
	case 0x0047: return _T("Flash fired, red-eye reduction mode, return light detected");
	case 0x0049: return _T("Flash fired, compulsory flash mode, red-eye reduction mode");
	case 0x004D: return _T("Flash fired, compulsory flash mode, red-eye reduction mode, return light not detected");
	case 0x004F: return _T("Flash fired, compulsory flash mode, red-eye reduction mode, return light detected");
	case 0x0050: return _T("Not fired, compulsory flash suppression");
	case 0x0059: return _T("Flash fired, auto mode, red-eye reduction mode");
	case 0x005D: return _T("Flash fired, auto mode, return light not detected, red-eye reduction mode");
	case 0x005F: return _T("Flash fired, auto mode, return light detected, red-eye reduction mode");
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

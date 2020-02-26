/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once

enum ExifTags
{
	EXIF_ORIENTATION=		0x0112,
	EXIF_SUB_IFD=			0x8769,
	EXIF_USER_DESC=			0x9286,
	EXIF_MAKER_NOTE=		0x927c,
	EXIF_IMG_WIDTH=			0xa002,
	EXIF_IMG_HEIGHT=		0xa003,
	EXIF_SUB_INTEROP=		0xa005,

	EXIF_WIN_F1=			0x9c9b,
	EXIF_WIN_F2=			0x9c9c,
	EXIF_WIN_F3=			0x9c9d,
	EXIF_WIN_F4=			0x9c9e,
	EXIF_WIN_F5=			0x9c9f,

	TIFF_NEW_SUBFILE_TYPE=	0x00fe,
	TIFF_IMAGE_WIDTH=		0x0100,
	TIFF_IMAGE_HEIGHT=		0x0101,
	TIFF_BITS_PER_SAMPLE=	0x0102,
	TIFF_COMPRESSION=		0x0103,
	TIFF_MAKE=				0x010f,
	TIFF_MODEL=				0x0110,
	TIFF_STRIP_OFFSETS=		0x0111,
	TIFF_ORIENTATION=		0x0112,
	TIFF_ROWS_PER_STRIP=	0x0116,	// rows per strip of data
	TIFF_STRIP_BYTE_COUNTS=	0x0117,	// bytes counts for strips
	TIFF_MIN_SAMPLE_VALUE=	0x0118,	// +minimum sample value
	TIFF_MAX_SAMPLE_VALUE=	0x0119,	// +maximum sample value
	TIFF_X_RESOLUTION=		0x011a,	// pixels/resolution in x
	TIFF_Y_RESOLUTION=		0x011b,	// pixels/resolution in y
	TIFF_PLANAR_CONFIG=		0x011c,	// storage organization
	TIFF_SUB_IFD=			0x014a,	// SubIFD
	// old-style fields, but still in use in raw files
	TIFF_JPEG_INTERCHANGE_FORMAT=			0x0201,
	TIFF_JPEG_INTERCHANGE_FORMAT_LENGTH=	0x0202,
	//
	TIFF_XMP=				0x02bc,	// metadata in XML string
	TIFF_EXIF_IFD=			0x8769,	// offset to EXIF data
	TIFF_GPS_IFD=			0x8825,

	// extended, not in a TIFF spec
	// used inside NRW
	TIFF_EX_CFARepeatPatternDim=		0x828d, // 33421
	TIFF_EX_CFAPattern=					0x828e, // 33422
	TIFF_EX_BatteryLevel=				0x828f, // 33423
	TIFF_EX_IPTC_NAA=					0x83bb, // 33723
	TIFF_EX_InterColorProfile3=			0x8773, // 34675
	TIFF_EX_Interlace=					0x8829, // 34857
	TIFF_EX_TimeZoneOffset=				0x882a, // 34858
	TIFF_EX_SelfTimerMode=				0x882b, // 34859
	TIFF_EX_FlashEnergy=				0x920b, // 37387
	TIFF_EX_SpatialFrequencyResponse=	0x920c, // 37388
	TIFF_EX_Noise=						0x920d, // 37389
	TIFF_EX_FocalPlaneXResolution=		0x920e, // 37390
	TIFF_EX_FocalPlaneYResolution=		0x920f, // 37391
	TIFF_EX_FocalPlaneResolutionUnit=	0x9210, // 37392
	TIFF_EX_ImageNumber=				0x9211, // 37393
	TIFF_EX_SecurityClassification=		0x9212, // 37394
	TIFF_EX_ImageHistory=				0x9213, // 37395
	TIFF_EX_ExposureIndex=				0x9215, // 37397
	TIFF_EX_TIFF_EPStandardID=			0x9216, // 37398
	TIFF_EX_SensingMethod=				0x9217, // 37399
};

enum IsoSpeedConstants
{
	ISO_SPEED_UNKNOWN= -1,
	ISO_SPEED_AUTO= -2,
	ISO_SPEED_MAX_VAL= 0xfff0
};

enum ExifOrientation
{
	EXIF_ORIENTATION_NORMAL= 1,
	EXIF_ORIENTATION_UPSDN= 3,
	EXIF_ORIENTATION_90CCW= 6,
	EXIF_ORIENTATION_90CW= 8,
	EXIF_ORIENTATION_UNKNOWN= 9
};


extern const TCHAR* TagName(uint16 tag);

class Data;
struct Rational;
struct SRational;
class FileStream;

extern const TCHAR* FileSource(int val);
extern const TCHAR* SceneType(int scene_type);
extern const TCHAR* ColorSpace(int16 space);
extern const TCHAR* CustomRendered(int16 val);
extern const TCHAR* ExposureMode(int16 val);
extern const TCHAR* WhiteBalance(int16 val);
extern const TCHAR* SensingMethod(int method);
extern const TCHAR* SceneCaptureType(int scene_type);
extern const TCHAR* GainControl(int val);
extern const TCHAR* Contrast(int val);
extern const TCHAR* Saturation(int val);
extern const TCHAR* Sharpness(int val);
extern const TCHAR* SubjectDistanceRange(int val);
extern String ApertureValAndFNum(Rational av);
extern String ExposureValAndShutterSpeed(SRational ev);
extern String SubjectDistance(Rational dist);
extern String ExifVersion(const Data& ver);
extern String FlashPixVersion(const Data& ver);
extern String InteroperabilityVersion(const Data& ver);
extern String ComponentConfiguration(const Data& v);
extern const TCHAR* ResolutionUnit(int n);
extern const TCHAR* ImgCompressionType(int n);

extern String BrightnessValue(SRational bv);

extern String FNumberFromAV(Rational aperture_value);
extern String FNumber(Rational fnumber);
extern String SubjectDistance(Rational dist);
extern String ExposureTimeFromSV(SRational shutter_speed_value);
extern String ExposureTime(Rational exp_time);

extern String VersionFieldFmt(const Data& ver);

extern bool CFAPattern(FileStream& ifs, uint32 components, String& pattern);

extern String Orientation(uint16 orientation);
extern String ExposureProgram(uint32 exposure_program);
extern String MeteringMode(uint32 metering_mode);
extern String ISOSpeed(uint16 iso_speed);
extern String LightSource(uint32 light_source);
extern String Flash(uint16 flash);
extern String FocalLength(Rational focal_length);
extern String ExposureBias(SRational exp_bias);
extern String PhotoWidth(uint32 width);
extern String PhotoHeight(uint32 height);
extern const TCHAR* PhotometricInterpretation(int val);

extern const SRational g_UninitializedRationalVal;

extern String DateTimeISO(const COleDateTime& tm, bool dateOnly);

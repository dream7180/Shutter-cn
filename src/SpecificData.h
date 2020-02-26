/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// SpecificData.h: interface for the SpecificData class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SPECIFICDATA_H__1256E347_93BC_4186_A4C9_8A94BF956FBE__INCLUDED_)
#define AFX_SPECIFICDATA_H__1256E347_93BC_4186_A4C9_8A94BF956FBE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "Rational.h"


struct CanonData
{
	CanonData();
//	CanonData(const CanonData& src)
//	{ *this = src; };

//	CanonData& operator = (const CanonData& src);

	const TCHAR* MacroMode() const;
	String SelfTimer() const;
	const TCHAR* FlashMode() const;
	const TCHAR* FocusType() const;
	const TCHAR* DriveMode() const;
	const TCHAR* FocusMode() const;
	const TCHAR* ImageSize() const;
	const TCHAR* Program() const;
	const TCHAR* Contrast() const;
	const TCHAR* Saturation() const;
	const TCHAR* Sharpness() const;
	String ISO() const;
	const TCHAR* MeteringMode() const;
	const TCHAR* AFPointSelected() const;
	const TCHAR* ExposureMode() const;
	const TCHAR* Quality() const;
	String LensAttached() const;
	const TCHAR* WhiteBalance() const;
	String SequenceNumber() const;
	String FlashBias() const;
	String SubjectDistance() const;
	String ImageType() const;
	String Firmware() const;
	String OwnerName() const;
	String CameraBodyNumber() const;

	uint16 macro_mode_;
	uint16 self_timer_;
	uint16 quality_;
	uint16 flash_mode_;
	uint16 drive_mode_;
	uint16 focus_mode_;
	uint16 image_size_;
	uint16 program_;
	uint16 contrast_;
	uint16 saturation_;
	uint16 sharpness_;
	uint16 iso_;
	uint16 metering_mode_;
	uint16 AF_point_;
	uint16 exposure_mode_;
	uint16 lens_short_;
	uint16 lens_long_;
	uint16 white_balance_;
	uint16 sequence_number_;
	int16  flash_bias_;
	uint16 subject_distance_;
	String image_type_;
	String firmware_;
	String owner_name_;
	uint32 camera_body_number_;
	uint16 focus_type_;
};


struct NikonData
{
	NikonData();
	NikonData(const NikonData& src)
	{ *this = src; };

	NikonData& operator = (const NikonData& src);

	String iso_setting_;
	String color_mode_;
	String quality_;
	String white_balance_;
	String image_sharpening_;
	String focus_mode_;
	String flash_setting_;
	String iso_selection_;
	String image_adjustment_;
	String adapter_;
	String MF_distance_;
	String digital_zoom_;
	String AF_position_;
	String lens_type_;
};


struct FujiData
{
	FujiData();

	String Version() const;
	const TCHAR* Quality() const;
	String Sharpness() const;
	String WhiteBalance() const;
	String Color() const;
	String Tone() const;
	String FlashMode() const;
	String FlashStrength() const;
	String Macro() const;
	String FocusMode() const;
	String SlowSync() const;
	String PictureMode() const;
	String ContTakeBracket() const;
	String BlurWarning() const;
	String FocusWarning() const;
	String AEWarning() const;

	uint32 version_;			// Undefined 4 Version of MakerNote information. At present, value is "0130".
	String quality_;		// Ascii string 8 Quality setting. Ascii string "BASIC","NORMAL","FINE"
	uint16 sharpness_;		// Sharpness setting. 1or2:soft, 3:normal, 4or5:hard.
	uint16 white_balance_;		// White balance setting. 0:Auto, 256:Daylight, 512:Cloudy, 768:DaylightColor-fluorescence, 769:DaywhiteColor-fluorescence, 770:White-fluorescence, 1024:Incandenscense, 3840:Custom white balance.
	uint16 color_;			// Chroma saturation setting. 0:normal(STD), 256:High, 512:Low(ORG).
	uint16 tone_;				// Contrast setting. 0:normal(STD), 256:High(HARD), 512:Low(ORG).
	uint16 flash_mode_;		// Flash firing mode setting. 0:Auto, 1:On, 2:Off, 3:Red-eye reduction.
	SRational flash_strength_;// Flash firing strength compensation setting. Unit is APEX(EV) and value is -6/10, -3/10, 0/10, 3/10, 6/10 etc.
	uint16 macro_;			// Macro mode setting. 0:Off, 1:On.
	uint16 focus_mode_;		// Focusing mode setting. 0:Auto focus, 1:Manual focus.
	uint16 slow_sync_;			// Slow synchro mode setting. 0:Off, 1:On.
	uint16 picture_mode_;		// Picture mode setting. 0:Auto, 1:Portrait scene, 2:Landscape scene, 4:Sports scene, 5:Night scene, 6:Program AE, 256:Aperture prior AE, 512:Shutter prior AE, 768:Manual exposure.
	uint16 unknown1_;			// Unknown
	uint16 cont_take_bracket_;	// Continuous taking or auto bracketting mode setting. 0:off, 1:on.
	uint16 unknown2_;			// Unknown
	uint16 blur_warning_;		// Blur warning status. 0:No blur warning, 1:Blur warning.
	uint16 focus_warning_;		// Auto Focus warning status. 0:Auto Focus good, 1:Out of focus.
	uint16 AE_warning_;		// Auto Exposure warning status. 0:AE good, 1:Over exposure (>1/1000s,F11).
};


struct OlympusData
{
	OlympusData();

	String SpecialMode() const;
	const TCHAR* JPEGQuality() const;
	const TCHAR* Macro() const;
	String DigitalZoom() const;

	uint32 special_mode_[3];	// [0x0200] SpecialMode Unsigned Long 3 Shows picture taking mode.
								// First value means 0=normal, 1=unknown, 2=fast, 3=panorama.
								// Second value means sequence number,
								// third value means panorama direction, 1=left to right, 2=right to left, 3=bottom to top, 4=top to bottom.
	uint16 jpeg_quality_;		// [0x0201] JpegQual Unsigned Short 1 Shows JPEG quality. 1=SQ,2=HQ,3=SHQ.
	uint16 macro_;			// [0x0202] Macro Unsigned Short 1 Shows Macro mode or not. 0=normal, 1=macro.
	//0x0203 Unknown Unsigned Short 1 Unknown 
	Rational digital_zoom_;	// [0x0204] DigiZoom Unsigned Rational 1 Shows Digital Zoom ratio. 0=normal, 2=digital 2x zoom. 
	//0x0205 Unknown Unsigned Rational 1 Unknown 
	//0x0206 Unknown Signed Short 6 Unknown 
	String software_;		// [0x0207] SoftwareRelease Ascii string 5 Shows Firmware version. 
	String pict_info_;		// [0x0208] PictInfo Ascii string 52 Contains ASCII format data such as [PictureInfo].
								// This is the same data format of older Olympus digicam that not used Exif data format
								// (C1400/C820/D620/D340 etc). 
	String camera_id_;		// [0x0209] CameraID Undefined 32 Contains CameraID data, which is user changeable by some utilities 
	// 0x0f00 DataDump Unsigned Long 30 Unknown 
};


struct CasioData
{

	uint16 recording_mode_;	// 0x0001 RecordingMode Unsigned Short 1 1:Single Shutter, 2:Panorama, 3:Night Scene, 4:Portrait, 5:Landscape 
	uint16 quality_;			// 0x0002 Quality Unsigned Short 1 1:Economy, 2:Normal, 3:Fine 
	uint16 focus_mode_;		// 0x0003 Focusing Mode Unsigned Short 1 2:Macro, 3:Auto Focus, 4:Manual Focus, 5:Infinity 
	uint16 flash_mode_;		// 0x0004 Flash Mode Unsigned Short 1 1:Auto, 2:On, 3:Off, 4:Red Eye Reduction 
	uint16 flash_intensity_;	// 0x0005 Flash Intensity Unsigned Short 1 11:Weak, 13:Normal, 15:Strong 
	uint32 object_distance_;	// 0x0006 Object distance Unsigned Long 1 Object distance in [mm]
	uint16 white_balance_;		// 0x0007 White Balance Unsigned Short 1 1:Auto, 2:Tungsten, 3:Daylight, 4:Fluorescent, 5:Shade, 129:Manual 
	// 0x0008 Unknown Unsigned short 1 Unknown 
	// 0x0009 Unknown Unsigned short 1 Unknown 
	uint32 digital_zoom_;		// 0x000a Digital Zoom Unsigned Long 1 0x10000(65536):'Off', 0x10001(65537):'2X Digital Zoom' 
	uint16 sharpness_;		// 0x000b Sharpness Unsigned Short 1 0:Normal, 1:Soft, 2:Hard 
	uint16 contrast_;			// 0x000c Contrast Unsigned Short 1 0:Normal, 1:Low, 2:High 
	uint16 saturation_;		// 0x000d Saturation Unsigned Short 1 0:Normal, 1:Low, 2:High 
	//0x000e Unknown Unsigned short 1 Unknown 
	//0x000f Unknown Unsigned short 1 Unknown 
	//0x0010 Unknown Unsigned short 1 Unknown 
	//0x0011 Unknown Unsigned long 1 Unknown 
	//0x0012 Unknown Unsigned short 1 Unknown 
	//0x0013 Unknown Unsigned short 1 Unknown 
	uint16 CCD_sensitivity_;	// 0x0014 CCD Sensitivity Unsigned short 1
								// QV3000:		64:Normal, 125:+1.0, 250:+2.0, 244:+3.0
								// QV8000/2000:	80:Normal, 100:High 
};


struct Casio2Data
{
	Casio2Data();

	uint32 ccd_iso_;
};

#endif // !defined(AFX_SPECIFICDATA_H__1256E347_93BC_4186_A4C9_8A94BF956FBE__INCLUDED_)

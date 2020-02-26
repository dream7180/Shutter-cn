/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// SpecificData.cpp: implementation of the SpecificData class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SpecificData.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////

CanonData::CanonData()
{
	macro_mode_ = ~0;
	self_timer_ = 0;
	quality_ = ~0;
	drive_mode_ = ~0;
	focus_mode_ = ~0;
	image_size_ = ~0;
	program_ = ~0;
	contrast_ = 0x8000;
	saturation_ = 0x8000;
	sharpness_ = 0x8000;
	iso_ = 0;
	metering_mode_ = ~0;
	AF_point_ = ~0;
	exposure_mode_ = ~0;
	lens_short_ = 0;
	lens_long_ = 0;
	white_balance_ = ~0;
	sequence_number_ = 0;
	flash_bias_ = 0;
	subject_distance_ = 0;
	camera_body_number_ = 0;
	focus_type_ = ~0;
}
/*
CanonData& CanonData::operator = (const CanonData& src)
{
	macro_mode_		= src.macro_mode_;
	self_timer_		= src.self_timer_;
	flash_mode_		= src.flash_mode_;
	drive_mode_		= src.drive_mode_;
	focus_mode_		= src.focus_mode_;
	image_size_		= src.image_size_;
	program_			= src.program_;
	contrast_			= src.contrast_;
	saturation_		= src.saturation_;
	sharpness_		= src.sharpness_;
	iso_				= src.iso_;
	metering_mode_		= src.metering_mode_;
	AF_point_			= src.AF_point_;
	exposure_mode_		= src.exposure_mode_;
	lens_short_		= src.lens_short_;
	lens_long_			= src.lens_long_;
	white_balance_		= src.white_balance_;
	sequence_number_	= src.sequence_number_;
	flash_bias_		= src.flash_bias_;
	subject_distance_	= src.subject_distance_;
	image_type_		= src.image_type_;
	firmware_		= src.firmware_;
	owner_name_		= src.owner_name_;
	camera_body_number_	= src.camera_body_number_;

	return *this;
}
*/

const TCHAR* CanonData::MacroMode() const
{
	switch (macro_mode_)
	{
	case 0:		return _T("Unknown");
	case 1:		return _T("Macro");
	case 2:		return _T("Normal");
	default:	return _T("(?)");
	}
}

String CanonData::SelfTimer() const
{
	if (self_timer_ == 0)
		return _T("Not Used");

	oStringstream ost;
	ost << uint32(self_timer_) / 10 << _T(".") << uint32(self_timer_) % 10 << _T(" s");
	return ost.str();
}

const TCHAR* CanonData::FlashMode() const
{
	switch (flash_mode_)
	{
	case 0:		return _T("Off");
	case 1:		return _T("Auto");
	case 2:		return _T("On");
	case 3:		return _T("Red-eye reduction");
	case 4:		return _T("Slow-sync");
	case 5:		return _T("Red-eye reduction (Auto)");
	case 6:		return _T("Red-eye reduction (On)");
	case 16:	return _T("External flash");
	default:	return _T("(?)");
	}
}

const TCHAR* CanonData::DriveMode() const
{
	switch (drive_mode_)
	{
	case 0:		return _T("Single");
	case 1:		return _T("Continuous");
	default:	return _T("(?)");
	}
}

const TCHAR* CanonData::FocusMode() const
{
	switch (focus_mode_)
	{
	case 0:		return _T("One-Shot");
	case 1:		return _T("AI Servo");
	case 2:		return _T("AI Focus");
	case 3:		return _T("Manual Focus");
	case 4:		// G1 differs here
				return _T("Single");
	case 5:		return _T("Continuous");
	case 6:		return _T("Manual Focus");
	default:	return _T("(?)");
	}
}

const TCHAR* CanonData::FocusType() const
{
	switch (focus_type_)
	{
	case 0:		return _T("Manual");
	case 1:		return _T("Auto (1)");
	case 2:		return _T("Auto (2)");
	case 3:		return _T("Close-up (Macro)");
	case 7:		return _T("Infinity Mode");
	case 8:		return _T("Locked (Pan Mode)");
	default:	return _T("(?)");
	}
}


const TCHAR* CanonData::ImageSize() const
{
	switch (image_size_)
	{
	case 0:		return _T("Large");
	case 1:		return _T("Medium");
	case 2:		return _T("Small");
	case uint16(~0):	return _T("-");
	default:	return _T("(?)");
	}
}

const TCHAR* CanonData::Program() const
{
	switch (program_)
	{
	case 0:		return _T("Full Auto");
	case 1:		return _T("Manual");
	case 2:		return _T("Landscape");
	case 3:		return _T("Fast Shutter");
	case 4:		return _T("Slow Shutter");
	case 5:		return _T("Night");
	case 6:		return _T("B&W");
	case 7:		return _T("Sepia");
	case 8:		return _T("Portrait");
	case 9:		return _T("Sports");
	case 10:	return _T("Close-Up");
	case 11:	return _T("Pan Focus");
	default:	return _T("(?)");
	}
}

const TCHAR* CanonData::Contrast() const
{
	switch (contrast_)
	{
	case 0xffff:	return _T("Low");
	case 0x0000:	return _T("Normal");
	case 0x0001:	return _T("High");
	default:		return _T("(?)");
	}
}

const TCHAR* CanonData::Saturation() const
{
	switch (saturation_)
	{
	case 0xffff:	return _T("Low");
	case 0x0000:	return _T("Normal");
	case 0x0001:	return _T("High");
	default:		return _T("(?)");
	}
}

const TCHAR* CanonData::Sharpness() const
{
	switch (sharpness_)
	{
	case 0xffff:	return _T("Low");
	case 0x0000:	return _T("Normal");
	case 0x0001:	return _T("High");
	default:		return _T("(?)");
	}
}

String CanonData::ISO() const
{
	switch (iso_)
	{
	case 0:		return _T("-");
	case 15:	return _T("Auto");
	case 16:	return _T("50");
	case 17:	return _T("100");
	case 18:	return _T("200");
	case 19:	return _T("400");
	case 20:	return _T("800");
	case 21:	return _T("1600");
	case 22:	return _T("3200");
	default:
		if (iso_ & 0x4000)	// new standard: verbatim value ored with 0x4000
		{
			oStringstream ost;
			ost << uint32(iso_ & 0x3fff);
			return ost.str();
		}

		return _T("(?)");
	}
}

const TCHAR* CanonData::MeteringMode() const
{
	switch (metering_mode_)
	{
	case 3:		return _T("Evaluative");
	case 4:		return _T("Partial");
	case 5:		return _T("Center-weighted");
	default:	return _T("(?)");
	}
}

const TCHAR* CanonData::AFPointSelected() const
{
	switch (AF_point_)
	{
	case 0x0:		return _T("None (MF)");
	case 0x3000:	return _T("None (MF)");
	case 0x3001:	return _T("Auto-selected");
	case 0x3002:	return _T("Right");
	case 0x3003:	return _T("Center");
	case 0x3004:	return _T("Left");
	case 0x4001:	return _T("Auto AF point selection");
	case 0x2005:	return _T("Manual AF point selection");
	default:		return _T("(?)");
	}
}

const TCHAR* CanonData::ExposureMode() const
{
	switch (exposure_mode_)
	{
	case 0:		return _T("Easy shooting");
	case 1:		return _T("Program");
	case 2:		return _T("Tv-Priority");
	case 3:		return _T("Av-Priority");
	case 4:		return _T("Manual");
	case 5:		return _T("A-DEP");
	default:	return _T("(?)");
	}
}

String CanonData::LensAttached() const
{
	if (lens_short_ > 0 && lens_long_ > 0)
	{
		oStringstream ost;
		ost << uint32(lens_short_) << _T("-") << uint32(lens_long_) << _T(" mm");
		return ost.str();
	}
	else
		return _T("(?)");
}

const TCHAR* CanonData::WhiteBalance() const
{
	switch (white_balance_)
	{
	case 0:		return _T("Auto");
	case 1:		return _T("Sunny");
	case 2:		return _T("Cloudy");
	case 3:		return _T("Tungsten");
	case 4:		return _T("Flourescent");
	case 5:		return _T("Flash");
	case 6:		return _T("Custom");
	case 7:		return _T("Black & white");
	case 8:		return _T("Shade");
	case 9:		return _T("Manual temperature");
	default:	return _T("(?)");
	}
}

String CanonData::SequenceNumber() const
{
	oStringstream ost;
	ost << uint32(sequence_number_);
	return ost.str();
}

String CanonData::FlashBias() const
{
	switch (uint16(flash_bias_))
	{
		// strange non-linear values
	case 0xffc0:	return _T("-2 EV");
	case 0xffcc:	return _T("-1 2/3 EV");
	case 0xffd0:	return _T("-1 1/2 EV");
	case 0xffd4:	return _T("-1 1/3 EV");
	case 0xffe0:	return _T("-1 EV");
	case 0xffec:	return _T("-2/3 EV");
	case 0xfff0:	return _T("-1/2 EV");
	case 0xfff4:	return _T("-1/3 EV");
	case 0x0000:	return _T("0 EV");
	case 0x000c:	return _T("1/3 EV");
	case 0x0010:	return _T("1/2 EV");
	case 0x0014:	return _T("2/3 EV");
	case 0x0020:	return _T("1 EV");
	case 0x002c:	return _T("1 1/3 EV");
	case 0x0030:	return _T("1 1/2 EV");
	case 0x0034:	return _T("1 2/3 EV");
	case 0x0040:	return _T("2 EV"); 
	default:
		{
			oStringstream ost;
			ost.precision(3);
			ost << double(flash_bias_) / 32.0 << _T(" EV");
			return ost.str();
		}
	}
}

String CanonData::SubjectDistance() const
{
	if (subject_distance_ == uint16(~0))
		return _T("Infinite");

	oStringstream ost;
	ost << uint32(subject_distance_);
	return ost.str();
}

String CanonData::ImageType() const
{
	return image_type_;
}

String CanonData::Firmware() const
{
	return firmware_;
}

String CanonData::OwnerName() const
{
	return owner_name_;
}

String CanonData::CameraBodyNumber() const
{
	if (camera_body_number_ == 0)
		return String();

	uint32 serial= camera_body_number_;
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


const TCHAR* CanonData::Quality() const
{
	switch (quality_)
	{
	case 2:		return _T("Normal");
	case 3:		return _T("Fine");
	case 4:		return _T("RAW");
	case 5:		return _T("Superfine");
	default:	return _T("(?)");
	}
}


///////////////////////////////////////////////////////////////////////////////


NikonData::NikonData()
{
}

NikonData& NikonData::operator = (const NikonData& src)
{
	iso_setting_		= src.iso_setting_;
	color_mode_		= src.color_mode_;
	quality_		= src.quality_;
	white_balance_	= src.white_balance_;
	image_sharpening_= src.image_sharpening_;
	focus_mode_		= src.focus_mode_;
	flash_setting_	= src.flash_setting_;
	iso_selection_	= src.iso_selection_;
	image_adjustment_= src.image_adjustment_;
	adapter_		= src.adapter_;
	MF_distance_		= src.MF_distance_;
	digital_zoom_	= src.digital_zoom_;
	AF_position_		= src.AF_position_;

	return *this;
}


///////////////////////////////////////////////////////////////////////////////


FujiData::FujiData()
{
}

String FujiData::Version() const
{
	oStringstream ost;
//	ost.precision(3);
	ost << int(version_);
	return ost.str();
}

const TCHAR* FujiData::Quality() const
{
	return quality_.c_str();
}

String FujiData::Sharpness() const
{
	// Sharpness setting. 1or2:soft, 3:normal, 4or5:hard.
	switch (sharpness_)
	{
	case 1:
	case 2:
		return _T("Soft");
	case 3:
		return _T("Normal");
	case 4:
	case 5:
		return _T("Hard");
	default:
		return _T("?");
	}
}

String FujiData::WhiteBalance() const
{
	// White balance setting. 0:Auto, 256:Daylight, 512:Cloudy,
	//  768:DaylightColor-fluorescence, 769:DaywhiteColor-fluorescence, 770:White-fluorescence,
	//  1024:Incandenscense, 3840:Custom white balance.
	switch (white_balance_)
	{
	case 0:
		return _T("Auto");
	case 0x100:
		return _T("Daylight");
	case 0x200:
		return _T("Cloudy");
	case 0x300:
		return _T("DaylightColor-fluorescence");
	case 0x301:
		return _T("DaywhiteColor-fluorescence");
	case 0x302:
		return _T("White-fluorescence");
	case 0x400:
		return _T("Incandenscense");
	case 0xf00:
		return _T("Custom");
	default:
		return _T("?");
	}
}

String FujiData::Color() const
{
	// Chroma saturation setting. 0:normal(STD), 256:High, 512:Low(ORG).
	switch (color_)
	{
	case 0:
		return _T("Normal (STD)");
	case 0x100:
		return _T("High");
	case 0x200:
		return _T("Low (ORG)");
	default:
		return _T("?");
	}
}

String FujiData::Tone() const
{
	// Contrast setting. 0:normal(STD), 256:High(HARD), 512:Low(ORG).
	switch (tone_)
	{
	case 0:
		return _T("Normal (STD)");
	case 0x100:
		return _T("High (HARD)");
	case 0x200:
		return _T("Low (ORG)");
	default:
		return _T("?");
	}
}

String FujiData::FlashMode() const
{
	// Flash firing mode setting. 0:Auto, 1:On, 2:Off, 3:Red-eye reduction.
	switch (flash_mode_)
	{
	case 0:
		return _T("Auto");
	case 1:
		return _T("On");
	case 2:
		return _T("Off");
	case 3:
		return _T("Red-eye reduction");
	default:
		return _T("?");
	}
}

String FujiData::FlashStrength() const
{
	// Flash firing strength compensation setting. Unit is APEX(EV) and value is -6/10, -3/10, 0/10, 3/10, 6/10 etc.
	double val= flash_strength_.Double();

	return String();
}

String FujiData::Macro() const
{
	// Macro mode setting. 0:Off, 1:On.
	switch (macro_)
	{
	case 0:
		return _T("Off");
	case 1:
		return _T("On");
	default:
		return _T("?");
	}
}

String FujiData::FocusMode() const
{
	// Focusing mode setting. 0:Auto focus, 1:Manual focus.
	switch (focus_mode_)
	{
	case 0:
		return _T("Auto");
	case 1:
		return _T("Manual");
	default:
		return _T("?");
	}
}

String FujiData::SlowSync() const
{
	// Slow synchro mode setting. 0:Off, 1:On.
	switch (slow_sync_)
	{
	case 0:
		return _T("Off");
	case 1:
		return _T("On");
	default:
		return _T("?");
	}
}

String FujiData::PictureMode() const
{
	// Picture mode setting. 0:Auto, 1:Portrait scene, 2:Landscape scene, 4:Sports scene,
	//  5:Night scene, 6:Program AE, 256:Aperture prior AE, 512:Shutter prior AE, 768:Manual exposure.
	switch (picture_mode_)
	{
	case 0:
		return _T("Auto");
	case 1:
		return _T("Portrait Scene");
	case 2:
		return _T("Landscape Scene");
	case 4:
		return _T("Sports Scene");
	case 5:
		return _T("Night Scene");
	case 6:
		return _T("Program AE");
	case 0x100:
		return _T("Aperture Prior AE");
	case 0x200:
		return _T("Shutter Prior AE");
	case 0x300:
		return _T("Manual Exposure");
	default:
		return _T("?");
	}
}

String FujiData::ContTakeBracket() const
{
	// Continuous taking or auto bracketting mode setting. 0:off, 1:on.
	switch (cont_take_bracket_)
	{
	case 0:
		return _T("Off");
	case 1:
		return _T("On");
	default:
		return _T("?");
	}
}

String FujiData::BlurWarning() const
{
	// Blur warning status. 0:No blur warning, 1:Blur warning.
	switch (blur_warning_)
	{
	case 0:
		return _T("No");
	case 1:
		return _T("Yes");
	default:
		return _T("?");
	}
}

String FujiData::FocusWarning() const
{
	// Auto Focus warning status. 0:Auto Focus good, 1:Out of focus.
	switch (focus_warning_)
	{
	case 0:
		return _T("Auto Focus OK");
	case 1:
		return _T("Out Of Focus");
	default:
		return _T("?");
	}
}

String FujiData::AEWarning() const
{
	// Auto Exposure warning status. 0:AE good, 1:Over exposure (>1/1000s,F11).
	switch (AE_warning_)
	{
	case 0:
		return _T("AE OK");
	case 1:
		return _T("Over Exposure");
	default:
		return _T("?");
	}
}


///////////////////////////////////////////////////////////////////////////////


OlympusData::OlympusData()
{
	special_mode_[0] = special_mode_[1] = special_mode_[2] = 0 - 1;
	jpeg_quality_ = 0 - 1;
	macro_ = 0 - 1;
}


String OlympusData::SpecialMode() const
{
	String mode;

	switch (special_mode_[0])	// First value means 0=normal, 1=unknown, 2=fast, 3=panorama.
	{
	case 0:
		mode = _T("Normal");
		break;
	case 1:
		mode = _T("Unknown");
		break;
	case 2:
		mode = _T("Fast");
		break;
	case 3:
		mode = _T("Panorama");
		break;
	}

	switch (special_mode_[1])	// Second value means sequence number
	{
	case 1:
		break;
	}

	switch (special_mode_[2])	// third value means panorama direction, 1=left to right, 2=right to left, 3=bottom to top, 4=top to bottom.
	{
	case 0:
		break;
	}

	return mode;
}


const TCHAR* OlympusData::JPEGQuality() const
{
	const TCHAR* text= _T("(?)");

	switch (jpeg_quality_)
	{
	case 1:
		text = _T("Standard Quality");
		break;
	case 2:
		text = _T("High Quality");
		break;
	case 3:
		text = _T("Super High Quality");
		break;
	}

	return text;
}


const TCHAR* OlympusData::Macro() const
{
	const TCHAR* text= _T("(?)");

	switch (macro_)
	{
	case 0:
		text = _T("Normal");
		break;
	case 1:
		text = _T("Macro");
		break;
	}

	return text;
}


String OlympusData::DigitalZoom() const
{
	if (digital_zoom_.Double() == 0.0)
		return _T("-");
	else if (digital_zoom_.Double() == 1.0)
		return _T("Not Used");
	else
		return digital_zoom_.AsString();
}


Casio2Data::Casio2Data()
{
	ccd_iso_ = 0;
}

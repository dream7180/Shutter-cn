/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// GPSData.cpp: implementation of the GPSData class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "File.h"
#include "Data.h"
#include "GPSData.h"
#include "OutputStr.h"
#include <iomanip>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

GPSData::GPSData(FileStream& ifs, Offset ifd_start, OutputStr* output)
{
	gps_version_id_[0] = gps_version_id_[1] = gps_version_id_[2] = gps_version_id_[3] = 0;
	gps_latitude_ref_ = 0;
	gps_longitude_ref_ = 0;
	gps_altitude_ref_ = 0x8000;
	gps_status_ = 0;
	gps_measure_mode_ = 0;
	gps_speed_ref_ = 0;
	gps_track_ref_ = 0;
	gps_img_direction_ref_ = 0;
	gps_dest_latitude_ref_ = 0;
	gps_dest_longitude_ref_ = 0;
	gps_dest_bearing_ref_ = 0;
	gps_dest_distance_ref_ = 0;

	uint16 entries= ifs.GetUInt16();	// no of entries in IFD

	for (uint32 i= 0; i < entries; ++i)
	{
		uint16 tag= ifs.GetUInt16();
		Data val(ifs, ifd_start);
		uint16 ref= val.GetSwapedWord();

		if (output) output->RecordInfo(tag | OutputStr::EXTRA_TAG, TagName(tag), val);

		switch (tag)
		{
		case 0:		// GPSVersionID
			if (val.Components() == 4 && !val.IsLongData())
			{
				val.ReadUChar(gps_version_id_, 4);
				if (output) output->SetInterpretedInfo(GetVersion());
			}
			break;
		case 1:		// GPSLatitudeRef
			gps_latitude_ref_ = ref;
			break;
		case 2:		// GPSLatitude
			val.GetDegMinSec(gps_latitude_);
			break;
		case 3:		// GPSLongitudeRef
			gps_longitude_ref_ = ref;
			break;
		case 4:		// GPSLongitude
			val.GetDegMinSec(gps_longitude_);
			break;
		case 5:		// GPSAltitudeRef
			gps_altitude_ref_ = static_cast<uint8>(val.GetData());	// byte data
			break;
		case 6:		// GPSAltitude
			gps_altitude_ = val.Rational();
			break;
		case 7:		// GPSTimeStamp
			val.GetDegMinSec(gps_time_stamp_);
			if (output) output->SetInterpretedInfo(GPSTimeStamp());
			break;
		case 8:		// GPSSatellites
			gps_satellites_ = val.AsString();
			break;
		case 9:		// GPSStatus
			gps_status_ = ref;
			if (output) output->SetInterpretedInfo(GPSStatus());
			break;
		case 10:	// GPSMeasureMode
			gps_measure_mode_ = ref;
			if (output) output->SetInterpretedInfo(GPSMeasureMode());
			break;
		case 11:	// GPSDOP
			gps_dop_ = val.Rational();
			break;
		case 12:	// GPSSpeedRef
			gps_speed_ref_ = ref;
			if (output) output->SetInterpretedInfo(GPSSpeedRef());
			break;
		case 13:	// GPSSpeed
			gps_speed_ = val.Rational();
			break;
		case 14:	// GPSTrackRef
			gps_track_ref_ = ref;
			if (output) output->SetInterpretedInfo(GPSTrackRef());
			break;
		case 15:	// GPSTrack
			gps_track_ = val.Rational();
			break;
		case 16:	// GPSImgDirectionRef
			gps_img_direction_ref_ = ref;
			if (output) output->SetInterpretedInfo(GPSImgDirectionRef());
			break;
		case 17:	// GPSImgDirection
			gps_img_direction_ = val.Rational();
			break;
		case 18:	// GPSMapDatum
			gps_map_datum_ = val.AsString();
			break;
		case 19:	// GPSDestLatitudeRef
			gps_dest_latitude_ref_ = ref;
			break;
		case 20:	// GPSDestLatitude
			val.GetDegMinSec(gps_dest_latitude_);
			break;
		case 21:	// GPSDestLongitudeRef
			gps_dest_longitude_ref_ = ref;
			break;
		case 22:	// GPSDestLongitude
			val.GetDegMinSec(gps_dest_longitude_);
			break;
		case 23:	// GPSDestBearingRef
			gps_dest_bearing_ref_ = ref;
			if (output) output->SetInterpretedInfo(GPSDestBearingRef());
			break;
		case 24:	// GPSDestBearing
			gps_dest_bearing_ = val.Rational();
			break;
		case 25:	// GPSDestDistanceRef
			gps_dest_distance_ref_ = ref;
			break;
		case 26:	// GPSDestDistance
			gps_dest_distance_ = val.Rational();
			break;
		}
	}

	if (output)
	{
		if (gps_latitude_ref_)
			output->RecordInfo(0, TagName(2), 0, GetLatitude().c_str());
		if (gps_longitude_ref_)
			output->RecordInfo(0, TagName(4), 0, GetLongitude().c_str());
		if (gps_altitude_ref_ != 0x8000)
			output->RecordInfo(0, TagName(6), 0, GetAltitude().c_str());
		//	if (gps_dest_latitude_ref_)
		//		;
		//	if (gps_dest_longitude_ref_)
		//		;
	}
}


GPSData::~GPSData()
{}


const TCHAR* GPSData::TagName(uint16 tag) const
{
	const TCHAR* name= _T("?");

	switch (tag)
	{
	case 0x00: name =	_T("GPSVersionID");			break;
	case 0x01: name =	_T("GPSLatitudeRef");		break;
	case 0x02: name =	_T("GPSLatitude");			break;
	case 0x03: name =	_T("GPSLongitudeRef");		break;
	case 0x04: name =	_T("GPSLongitude");			break;
	case 0x05: name =	_T("GPSAltitudeRef");		break;
	case 0x06: name =	_T("GPSAltitude");			break;
	case 0x07: name =	_T("GPSTimeStamp");			break;
	case 0x08: name =	_T("GPSSatellites");		break;
	case 0x09: name =	_T("GPSStatus");			break;
	case 0x0a: name =	_T("GPSMeasureMode");		break;
	case 0x0b: name =	_T("GPSDOP");				break;
	case 0x0c: name =	_T("GPSSpeedRef");			break;
	case 0x0d: name =	_T("GPSSpeed");				break;
	case 0x0e: name =	_T("GPSTrackRef");			break;
	case 0x0f: name =	_T("GPSTrack");				break;
	case 0x10: name =	_T("GPSImgDirectionRef");	break;
	case 0x11: name =	_T("GPSImgDirection");		break;
	case 0x12: name =	_T("GPSMapDatum");			break;
	case 0x13: name =	_T("GPSDestLatitudeRef");	break;
	case 0x14: name =	_T("GPSDestLatitude");		break;
	case 0x15: name =	_T("GPSDestLongitudeRef");	break;
	case 0x16: name =	_T("GPSDestLongitude");		break;
	case 0x17: name =	_T("GPSDestBearingRef");	break;
	case 0x18: name =	_T("GPSDestBearing");		break;
	case 0x19: name =	_T("GPSDestDistanceRef");	break;
	case 0x1a: name =	_T("GPSDestDistance");		break;
	case 0x1b: name =	_T("GPSProcessingMethod");	break;
	case 0x1c: name =	_T("GPSAreaInformation");	break;
	case 0x1d: name =	_T("GPSDateStamp");			break;
	case 0x1e: name =	_T("GPSDifferential");		break;
	}

	return name;
}


String GPSData::GetVersion() const
{
	oStringstream ost;
	ost << uint32(gps_version_id_[0]) << _T(".") << uint32(gps_version_id_[1]) << _T(".")
		<< uint32(gps_version_id_[2]) << _T(".") << uint32(gps_version_id_[3]);

	return ost.str();
}


const TCHAR* GPSData::GPSStatus() const
{
	const TCHAR* ret= _T("?");

	switch (gps_status_)
	{
	case 'A':
		ret = _T("Measurement in progress");
		break;
	case 'V':
		ret = _T("Measurement Interoperability");
		break;
	}

	return ret;
}


const TCHAR* GPSData::GPSMeasureMode() const
{
	const TCHAR* ret= _T("?");

	switch (gps_measure_mode_)
	{
	case '2':
		ret = _T("2-dimensional measurement");
		break;
	case '3':
		ret = _T("3-dimensional measurement");
		break;
	}

	return ret;
}


const TCHAR* GPSData::GPSSpeedRef() const
{
	const TCHAR* ret= _T("?");

	switch (gps_speed_ref_)
	{
	case 'K':
		ret = _T("Kilometers per hour");
		break;
	case 'M':
		ret = _T("Miles per hour");
		break;
	case 'N':
		ret = _T("Knots");
		break;
	}

	return ret;
}


const TCHAR* GPSData::GetReference(uint16 ref) const
{
	switch (ref)
	{
	case 'T':
		return _T("True direction");
	case 'M':
		return _T("Magnetic direction");
	default:
		return _T("?");
	}
}


const TCHAR* GPSData::GPSImgDirectionRef() const
{
	return GetReference(gps_img_direction_ref_);
}


const TCHAR* GPSData::GPSTrackRef() const
{
	return GetReference(gps_track_ref_);
}


const TCHAR* GPSData::GPSDestBearingRef() const
{
	return GetReference(gps_dest_bearing_ref_);
}


String GPSData::GetDegMinSec(const Rational val[3], uint16 ref) const
{
	String s= Rational::AsDegMinSec(val);

	switch (ref)
	{
	case 'N':
		s += _T(" N");
		break;
	case 'S':
		s += _T(" S");
		break;
	case 'E':
		s += _T(" E");
		break;
	case 'W':
		s += _T(" W");
		break;
	default:
		s += _T(" (?)");
		break;
	}

	return s;
}


String GPSData::GetLatitude() const
{
	return GetDegMinSec(gps_latitude_, gps_latitude_ref_);
}


String GPSData::GetLongitude() const
{
	return GetDegMinSec(gps_longitude_, gps_longitude_ref_);
}

String GPSData::GetAltitude() const
{
	oStringstream ost;
	ost.precision(1);	// one decimal place
	ost << std::fixed << gps_altitude_.Double() << _T(" m");
	if (gps_altitude_ref_ != 0)
		ost << _T(" (unknown reference level)");
	return ost.str();
}

String GPSData::GPSTimeStamp() const
{
	oStringstream ost;
	ost.setf(std::ios::fixed, std::ios::floatfield);
	ost.precision(0);
	ost << std::setfill(_T('0')) << std::setw(2) << gps_time_stamp_[0].Double() << _T(":");
	ost << std::setw(2) << gps_time_stamp_[1].Double() << _T(":");
	ost.precision(2);
	ost << std::setw(5) << gps_time_stamp_[2].Double();

	return ost.str();
}


int GPSData::CmpDegMinSec(const Rational val1[3], const Rational val2[3])
{
	if (val1[0].denominator_ == 0 && val2[0].denominator_ == 0)
		return 0;
	if (val1[0].denominator_ == 0)
		return -1;
	if (val2[0].denominator_ == 0)
		return 1;

	for (int i= 0; i < 3; ++i)
	{
		double d1= val1[i].Double();
		double d2= val2[i].Double();

		if (d1 > d2)
			return 1;
		if (d1 < d2)
			return -1;
	}

	return 0;
}


bool GPSData::LessDegMinSec(const Rational val1[3], const Rational val2[3])
{
	if (val1[0].denominator_ == 0 && val2[0].denominator_ == 0)
		return false;
	if (val1[0].denominator_ == 0)
		return false;
	if (val2[0].denominator_ == 0)
		return true;

	for (int i= 0; i < 3; ++i)
	{
		double d1= val1[i].Double();
		double d2= val2[i].Double();

		if (d1 > d2)
			return false;
		if (d1 < d2)
			return true;
	}

	return false;
}


int GPSData::CmpAltitude(const GPSData& dat) const
{
	if (gps_altitude_ > dat.gps_altitude_)
		return 1;
	if (dat.gps_altitude_ > gps_altitude_)
		return -1;
	return 0;
}


int GPSData::CmpLatitude(const GPSData& dat) const
{
	if (gps_latitude_ref_ != dat.gps_latitude_ref_)
		return gps_latitude_ref_ < dat.gps_latitude_ref_ ? 1 : -1;

	return CmpDegMinSec(gps_latitude_, dat.gps_latitude_);
}


int GPSData::CmpLongitude(const GPSData& dat) const
{
	if (gps_longitude_ref_ != dat.gps_longitude_ref_)
		return gps_longitude_ref_ < dat.gps_longitude_ref_ ? 1 : -1;

	return CmpDegMinSec(gps_longitude_, dat.gps_longitude_);
}



bool GPSData::LessLatitude(const GPSData& dat) const
{
	if (gps_latitude_ref_ != dat.gps_latitude_ref_)
		return gps_latitude_ref_ < dat.gps_latitude_ref_;

	return LessDegMinSec(gps_latitude_, dat.gps_latitude_);
}


bool GPSData::LessLongitude(const GPSData& dat) const
{
	if (gps_longitude_ref_ != dat.gps_longitude_ref_)
		return gps_longitude_ref_ < dat.gps_longitude_ref_;

	return LessDegMinSec(gps_longitude_, dat.gps_longitude_);
}


bool GPSData::LessAltitude(const GPSData& dat) const
{
	return gps_altitude_ < dat.gps_altitude_;
}

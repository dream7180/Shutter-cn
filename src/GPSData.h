/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// GPSData.h: interface for the GPSData class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GPSDATA_H__9D79FE3D_1FC9_463D_8B66_774E7FBAF4DC__INCLUDED_)
#define AFX_GPSDATA_H__9D79FE3D_1FC9_463D_8B66_774E7FBAF4DC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Rational.h"
#include "File.h"
class Data;
class OutputStr;


class GPSData
{
public:
	GPSData(FileStream& ifs, Offset ifd_start, OutputStr* output);
	~GPSData();

	String GetVersion() const;
	const TCHAR* GPSStatus() const;
	const TCHAR* GPSMeasureMode() const;
	const TCHAR* GPSSpeedRef() const;
	const TCHAR* GPSTrackRef() const;
	const TCHAR* GPSImgDirectionRef() const;
	const TCHAR* GPSDestBearingRef() const;

	String GetLatitude() const;
	String GetLongitude() const;
	String GetAltitude() const;
	String GPSTimeStamp() const;

	int CmpLatitude(const GPSData& dat) const;
	int CmpLongitude(const GPSData& dat) const;
	int CmpAltitude(const GPSData& dat) const;
	int CmpTimeStamp(const GPSData& dat) const	{ return CmpDegMinSec(gps_time_stamp_, dat.gps_time_stamp_); }

	bool LessLatitude(const GPSData& dat) const;
	bool LessLongitude(const GPSData& dat) const;
	bool LessAltitude(const GPSData& dat) const;
	bool LessTimeStamp(const GPSData& dat) const	{ return LessDegMinSec(gps_time_stamp_, dat.gps_time_stamp_); }

private:
	uint8		gps_version_id_[4];			// 00 GPS tag version
	uint16		gps_latitude_ref_;			// 01 North or South Latitude
	Rational	gps_latitude_[3];			// 02 Latitude
	uint16		gps_longitude_ref_;			// 03 East or West Longitude
	Rational	gps_longitude_[3];			// 04 Longitude
	uint16		gps_altitude_ref_;			// 05 Altitude reference
	Rational	gps_altitude_;				// 06 Altitude
	Rational	gps_time_stamp_[3];			// 07 GPS time (atomic clock)
	String		gps_satellites_;			// 08 GPS satellites used for measurement
	uint16		gps_status_;				// 09 GPS receiver status
	uint16		gps_measure_mode_;			// 10 GPS measurement mode
	Rational	gps_dop_;					// 11 Measurement precision
	uint16		gps_speed_ref_;				// 12 Speed unit
	Rational	gps_speed_;					// 13 Speed of GPS receiver
	uint16		gps_track_ref_;				// 14 Reference for direction of movement
	Rational	gps_track_;					// 15 Direction of movement
	uint16		gps_img_direction_ref_;		// 16 Reference for direction of image
	Rational	gps_img_direction_;			// 17 Direction of image
	String		gps_map_datum_;				// 18 Geodetic survey data used
	uint16		gps_dest_latitude_ref_;		// 19 Reference for latitude of destination
	Rational	gps_dest_latitude_[3];		// 20 Latitude of destination
	uint16		gps_dest_longitude_ref_;	// 21 Reference for longitude of destination
	Rational	gps_dest_longitude_[3];		// 22 Longitude of destination
	uint16		gps_dest_bearing_ref_;		// 23 Reference for bearing of destination
	Rational	gps_dest_bearing_;			// 24 Bearing of destination
	uint16		gps_dest_distance_ref_;		// 25 Reference for distance to destination
	Rational	gps_dest_distance_;			// 26 Distance to destination


	const TCHAR* TagName(uint16 tag) const;
	const TCHAR* GetReference(uint16 ref) const;
	String GetDegMinSec(const Rational val[3], uint16 ref) const;

	static int CmpDegMinSec(const Rational val1[3], const Rational val2[3]);
	static bool LessDegMinSec(const Rational val1[3], const Rational val2[3]);
};

#endif // !defined(AFX_GPSDATA_H__9D79FE3D_1FC9_463D_8B66_774E7FBAF4DC__INCLUDED_)

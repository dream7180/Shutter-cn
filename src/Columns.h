/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// Columns.h: interface for the Columns class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_COLUMNS_H__C4C5374C_29D0_4654_8D3A_F295AF4582DA__INCLUDED_)
#define AFX_COLUMNS_H__C4C5374C_29D0_4654_8D3A_F295AF4582DA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
class PhotoInfo;
#include "RString.h"
#include "CustomColumns.h"

//#define _TRUE_EXPOSURE_COL_


class Columns
{
public:
	Columns();
	~Columns();

	static int MaxCount();
	enum Set { COMMON, CANON, NIKON, FUJI, OLYMPUS, /*SONY*/ METADATA, CUSTOM };
	static int GetSetsCount()		{ return 6; }
	static int GetCount(Set set);
	static int GetLimit(Set set);
	static int GetStart(Set set);

	const TCHAR* Name(int index, bool long_form= true) const;
	const TCHAR* ShortName(int index) const;
	const TCHAR* Abbreviation(int index) const;

	int Alignment(int index) const;
	int DefaultWidth(int index) const;

	void GetInfo(String& rstrOut, int index, const PhotoInfo& inf) const;
	//int Compare(int column, int ascending, const PhotoInfo& info1, const PhotoInfo& info2) const;

	// returns true if data for a column 'index' is present in 'inf' photo
	bool IsDataPresent(int index, const PhotoInfo& inf) const;

	// compare info1 & info2 by the 'column' attribute (calculate "less than")
	bool Less(int column, const PhotoInfo& info1, const PhotoInfo& info2) const;

	// as above, but using primary and then secondary key, if primary attribs are equivalent
	bool Less(int first_column, int second_column, const PhotoInfo& info1, const PhotoInfo& info2, const PhotoInfo& info1a, const PhotoInfo& info2a) const;

	// prepare popup menu with all possible columns names
	bool GetPopupMenu(CMenu& menu, std::vector<uint16>& selected_columns) const;

	// prepare text in the format accepted by CDrawFields class using given photo and selected columns
	String GetStatusText(const std::vector<uint16>& selected_columns, const PhotoInfo& photo, bool includeNames) const;

	// access to the custom columns
	const CustomColumns& GetCustomColumns() const;
	void SetCustomColumns(const CustomColumns& columns);

private:
	// user-defined columns
	CustomColumns custom_;
};



enum ColumnEnums
{
	COL_GENERAL_SET_START= 0,
	/*
	COL_PHOTO_NAME= COL_GENERAL_SET_START,
	COL_DATE_TIME,
	COL_EXP_TIME,
	COL_FNUMBER,
	COL_EXP_PROG,
	COL_EXP_BIAS,
	COL_MET_MODE,
	COL_LENS_MODEL,
	COL_LIGHT_SRC,
	COL_FLASH,
	COL_FOCAL_LENGTH,
	COL_FOCAL_LENGTH_35MM,
	COL_ISO,
	COL_ORIENTATION,
	COL_DIMENSIONS,
	COL_FILE_SIZE,
	COL_PATH,
	COL_MAKE,
	COL_MODEL,
	COL_DESCRIPTION,
//	COL_COMPR_BPP,
	COL_FILE_TYPE_NAME,
	COL_FILE_NAME,		// file name and extension
	COL_SUBJECT_DIST,
	COL_RATE,	// stars
	*/
	COL_PHOTO_NAME= COL_GENERAL_SET_START,
	COL_FILE_TYPE_NAME,
	COL_DATE_TIME,
	COL_MAKE,
	COL_MODEL,
	COL_LENS_MODEL,
	COL_ISO,
	COL_EXP_TIME,
	COL_FNUMBER,
	COL_FOCAL_LENGTH,
	COL_DIMENSIONS,
	COL_FILE_SIZE,
	COL_FOCAL_LENGTH_35MM,
	COL_EXP_BIAS,
	COL_FLASH,
	COL_EXP_PROG,
	COL_MET_MODE,
	COL_LIGHT_SRC,
	COL_ORIENTATION,
	COL_PATH,
	COL_DESCRIPTION,
//	COL_COMPR_BPP,
	COL_FILE_NAME,		// file name and extension
	COL_SUBJECT_DIST,
	COL_RATE,	// stars
#ifdef _TRUE_EXPOSURE_COL_
	COL_TRUE_EXPOSURE,	// D = FL(35) / F#; A = Pi * (D / 2)^2; TrueExp = A * ISO / ShutterSpeed
#endif

	COL_GPS_FIRST_ITEM,
	COL_GPS_LATITUDE= COL_GPS_FIRST_ITEM,
	COL_GPS_LONGITUDE,
	COL_GPS_ALTITUDE,
	COL_GPS_TIME_STAMP,

	COL_GENERAL_SET_END= COL_GPS_TIME_STAMP,

	COL_CANON_SET_START,
	COL_CAN_MACRO= COL_CANON_SET_START,
	COL_CAN_TIMER,
	COL_CAN_FLASH,
	COL_CAN_DRIVE,
	COL_CAN_FOCUS,
	COL_CAN_FOCUS_TYPE,
	COL_CAN_IMG_SIZE,
	COL_CAN_PROGRAM,
	COL_CAN_CONTRAST,
	COL_CAN_SATURATION,
	COL_CAN_SHARPNESS,
	COL_CAN_ISO,
	COL_CAN_MET_MODE,
	COL_CAN_AF_POINT,
	COL_CAN_EXP_MODE,
	COL_CAN_LENS,
	COL_CAN_WHITE_BAL,
	COL_CAN_SEQ_NO,
	COL_CAN_FLASH_BIAS,
	COL_CAN_SUBJECT_DIST,
	COL_CAN_IMG_TYPE,
	COL_CAN_FIRMWARE,
	COL_CAN_OWNER,
	COL_CAN_SERIAL,
	COL_CANON_SET_END= COL_CAN_SERIAL,

	COL_NIKON_SET_START,
	COL_NIK_ISO_SET= COL_NIKON_SET_START,
	COL_NIK_COLOR_MODE,
	COL_NIK_QUALITY,
	COL_NIK_WHITE_BAL,
	COL_NIK_IMG_SHARP,
	COL_NIK_FOCUS_MODE,
	COL_NIK_FLASH_SET,
	COL_NIK_ISO_SEL,
	COL_NIK_IMG_ADJ,
	COL_NIK_ADAPTER,
	COL_NIK_LENS_TYPE,
	COL_NIK_MF_DIST,
	COL_NIK_DIGI_ZOOM,
	COL_NIK_AF_POS,
	COL_NIKON_SET_END= COL_NIK_AF_POS,

	COL_FUJI_SET_START,
	COL_FUJI_VERSION= COL_FUJI_SET_START,
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
	COL_FUJI_SET_END= COL_FUJI_AE_WARNING,

	COL_OLY_SET_START,
	COL_OLY_SPECIAL_MODE= COL_OLY_SET_START,
	COL_OLY_JPEG_QUALITY,
	COL_OLY_MACRO,
	COL_OLY_DIGITAL_ZOOM,
	COL_OLY_SOFTWARE_RELEASE,
	COL_OLY_PICTURE_INFO,
	COL_OLY_CAMERA_ID,
	COL_OLY_SET_END= COL_OLY_CAMERA_ID,

	// XMP metadata (and IPTC)
	COL_METADATA_SET_START,
	COL_METADATA_DOCUMENT_TITLE= COL_METADATA_SET_START,
	COL_METADATA_AUTHOR,
	COL_METADATA_DESCRIPTION,
	COL_METADATA_IMAGE_RATING,
	COL_METADATA_COPYRIGHT_NOTICE,
	COL_METADATA_KEYWORDS,
	COL_METADATA_DESCRIPTION_WRITER,
	COL_METADATA_HEADLINE,
//	COL_METADATA_TITLE,	// object name
//	COL_METADATA_CREATOR,	= author
	COL_METADATA_CREATORS_JOB,
	COL_METADATA_ADDRESS,
	COL_METADATA_CITY,
	COL_METADATA_STATE,
	COL_METADATA_POSTAL_CODE,
	COL_METADATA_COUNTRY,
	COL_METADATA_PHONES,
	COL_METADATA_EMAILS,
	COL_METADATA_WEB_SITES,
	COL_METADATA_JOB_IDENTIFIER,
	COL_METADATA_INSTRUCTIONS,
	COL_METADATA_PROVIDER,
	COL_METADATA_SOURCE,
	COL_METADATA_RIGHTS_USAGE_TERMS,
	COL_METADATA_COPYRIGHT_INFO_URL,
	COL_METADATA_CREATION_DATE,
	COL_METADATA_INTELLECTUAL_GENRE,
	COL_METADATA_LOCATION,
	COL_METADATA_CITY2,
	COL_METADATA_STATE_PROVINCE,
	COL_METADATA_COUNTRY2,
	COL_METADATA_ISO_COUNTRY_CODE,
	COL_METADATA_IPTC_SCENE,
	COL_METADATA_IPTC_SUBJECT_CODE,
	COL_METADATA_SET_END= COL_METADATA_IPTC_SUBJECT_CODE,

	COL_CUSTOM_SET_START= 10000,
	COL_CUSTOM_1= COL_CUSTOM_SET_START,
	COL_CUSTOM_2,
	COL_CUSTOM_3,
	COL_CUSTOM_4,
	COL_CUSTOM_5,
	COL_CUSTOM_6,
	COL_CUSTOM_7,
	COL_CUSTOM_8,
	COL_CUSTOM_9,
	COL_CUSTOM_10,
	COL_CUSTOM_11,
	COL_CUSTOM_12,
	COL_CUSTOM_13,
	COL_CUSTOM_14,
	COL_CUSTOM_15,
	COL_CUSTOM_16,
	COL_CUSTOM_17,
	COL_CUSTOM_18,
	COL_CUSTOM_19,
	COL_CUSTOM_20,
	COL_CUSTOM_SET_END= COL_CUSTOM_20
};


#endif // !defined(AFX_COLUMNS_H__C4C5374C_29D0_4654_8D3A_F295AF4582DA__INCLUDED_)

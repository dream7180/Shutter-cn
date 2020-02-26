/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once

struct XmpData
{
	String DocumentTitle;
	String Author;
	String Description;
	String ImageRating;
	String CopyrightNotice;
	String Keywords;

	String DescriptionWriter;
	String Headline;
//	String Title;	// object name
	//
//	String Creator;	= author
	String CreatorsJob;	//
	String Address;
	String City;
	String State;
	String PostalCode;
	String Country;
	//
	String Phones;
	String EMails;
	String WebSites;

	String JobIdentifier;
	String Instructions;
	String Provider;
	String Source;
	String RightsUsageTerms;
	String CopyrightInfoURL;
	//
	String CreationDate;
	String IntellectualGenre;
	String Location;
	String City2;
	String StateProvince;
	String Country2;
	String ISOCountryCode;
	//
	String IPTCScene;	//
	String IPTCSubjectCode;	//

	String CreatorTool;
};


// 'index' is a field number (0-based) in the above structure, returned is a display name for the field
const TCHAR* GetXmpDataFieldName(size_t index);

// return reference ot the field in 'xmp', 'index' is a 0-based field number in XmpData
const String& GetXmpDataField(const XmpData& xmp, size_t index);

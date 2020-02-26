/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "XmpData.h"

const TCHAR* GetXmpDataFieldName(size_t index)
{
	switch (index)
	{
	case 0: return _T("Document Title");
	case 1: return _T("Author");
	case 2: return _T("Description");
	case 3: return _T("Image Rating");
	case 4: return _T("Copyright Notice");
	case 5: return _T("Keywords");

	case 6: return _T("Description Writer");
	case 7: return _T("Headline");
//	case 0: return _T("Title");	// object name
//	case 0: return _T("Creator");	= author
	case 8: return _T("Creator's Job");	//
	case 9: return _T("Creator's Address");
	case 10: return _T("Creator's City");
	case 11: return _T("Creator's State");
	case 12: return _T("Creator's Postal Code");
	case 13: return _T("Creator's Country");

	case 14: return _T("Creator's Phones");
	case 15: return _T("Creator's E-Mails");
	case 16: return _T("Creator's Web Sites");

	case 17: return _T("Job Identifier");
	case 18: return _T("Instructions");
	case 19: return _T("Provider");
	case 20: return _T("Source");
	case 21: return _T("Rights Usage Terms");
	case 22: return _T("Copyright Info URL");

	case 23: return _T("Creation Date");
	case 24: return _T("Intellectual Genre");
	case 25: return _T("Location");
	case 26: return _T("City");
	case 27: return _T("State/Province");
	case 28: return _T("Country");
	case 29: return _T("ISO Country Code");

	case 30: return _T("IPTC Scene");
	case 31: return _T("IPTC Subject Code");

	case 32: return _T("Creator Tool");

	default:
		ASSERT(false);
		throw "Invalid index in " __FUNCTION__;
	}
}


const String& GetXmpDataField(const XmpData& xmp, size_t index)
{
	switch (index)
	{
	case 0: return xmp.DocumentTitle;
	case 1: return xmp.Author;
	case 2: return xmp.Description;
	case 3: return xmp.ImageRating;
	case 4: return xmp.CopyrightNotice;
	case 5: return xmp.Keywords;

	case 6: return xmp.DescriptionWriter;
	case 7: return xmp.Headline;
//	case 0: return xmp.Title;	// object name
//	case 0: return xmp.Creator;	= author
	case 8: return xmp.CreatorsJob;	//
	case 9: return xmp.Address;
	case 10: return xmp.City;
	case 11: return xmp.State;
	case 12: return xmp.PostalCode;
	case 13: return xmp.Country;

	case 14: return xmp.Phones;
	case 15: return xmp.EMails;
	case 16: return xmp.WebSites;

	case 17: return xmp.JobIdentifier;
	case 18: return xmp.Instructions;
	case 19: return xmp.Provider;
	case 20: return xmp.Source;
	case 21: return xmp.RightsUsageTerms;
	case 22: return xmp.CopyrightInfoURL;

	case 23: return xmp.CreationDate;
	case 24: return xmp.IntellectualGenre;
	case 25: return xmp.Location;
	case 26: return xmp.City2;
	case 27: return xmp.StateProvince;
	case 28: return xmp.Country2;
	case 29: return xmp.ISOCountryCode;

	case 30: return xmp.IPTCScene;
	case 31: return xmp.IPTCSubjectCode;

	case 32: return xmp.CreatorTool;

	default:
		ASSERT(false);
		throw "Invalid index in " __FUNCTION__;
	}
}

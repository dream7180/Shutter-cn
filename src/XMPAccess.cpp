/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "XmpInclude.h"
#include "StringConversions.h"
#include "Path.h"
#include "XMPAccess.h"
#include "FileGuard.h"
#include "File.h"
#include "Markers.h"
#include "ReadMarker.h"
#include "SEException.h"
#include "XMPData.h"
#include <boost/algorithm/string/replace.hpp>
#include "Exception.h"

// Clients must compile XMP.incl_cpp to ensure that all client-side glue code is generated
#include "XMP-Toolkit\public\include\XMP.incl_cpp"

using namespace boost::algorithm;

extern bool IsJpegMarkerToBeSkipped(uint16 marker);

namespace Xmp
{

const size_t MaxXmpFileSize= 100 * 1024 * 1024;	// 100 MB

const char* const CreatorContactInfo= "CreatorContactInfo";
extern const char* HEADER_NAMESPACE= XMP_NAMESPACE;
extern const size_t HEADER_NAMESPACE_LEN= strlen(HEADER_NAMESPACE) + 1;	// include \nul


void ToUnixEols(std::string& str)
{
	replace_all(str, "\xd\xa", "\xa");
}

void ToUnixEols(CStringA& str)
{
	str.Replace("\xd\xa", "\xa");
}

void ToWindowsEols(std::string& str)
{
	replace_all(str, "\xa", "\xd\xa");
}


#undef W2A

CStringA W2A(const String& str)
{
#ifdef _UNICODE
	std::string utf8;
	::WideStringToMultiByte(str, utf8);
	ToUnixEols(utf8);
	return utf8.c_str();
#else
	CStringA s= str.c_str();
	ToUnixEols(s);
	return s;
#endif
}

CStringA W2A(const CString& str)
{
#ifdef _UNICODE
	std::string utf8;
	::WideStringToMultiByte(str, utf8);
	ToUnixEols(utf8);
	return utf8.c_str();
#else
	CStringA s= str;
	ToUnixEols(s);
	return s;
#endif
}


String concat_strings(const TCHAR* msg, const char* err)
{
	String str= msg;
	if (err)
	{
#ifdef _UNICODE
		String s;
		::MultiByteToWideString(err, -1, s, CP_UTF8);
		str += s;
#else
		str += err;
#endif
	}
	return str;
}



extern void ParseItems(const String& items, std::vector<String>& keys)
{
	keys.clear();

	if (items.empty())
		return;

	CString keywords= items.c_str();
	keywords.Replace(_T("\xd\xa"), _T("\n"));
	keywords.Replace(_T(","), _T("\n"));
	keywords.Replace(_T(";"), _T("\n"));

	for (int i= 0; i < 99999; ++i)	// safety valve
	{
		CString item;
		if (!AfxExtractSubString(item, keywords, i))
			break;
		item.Trim(_T(" \t\n\r"));
		if (!item.IsEmpty())
			keys.push_back(String(item));
	}
}


static void AddItems(const String& data, SXMPMeta& meta, const char* ns, const char* field)
{
	meta.DeleteProperty(ns, field);

	if (!data.empty())
	{
		// add keywords

		CString keywords= data.c_str();
		keywords.Replace(_T("\xd\xa"), _T("\n"));
		keywords.Replace(_T(","), _T("\n"));
		keywords.Replace(_T(";"), _T("\n"));

		for (int i= 0; i < 99999; ++i)	// safety valve
		{
			CString item;
			if (!AfxExtractSubString(item, keywords, i))
				break;
			item.Trim(_T(" \t\n\r"));
			if (!item.IsEmpty())
				meta.AppendArrayItem(ns, field, kXMP_PropValueIsArray, W2A(item), 0);
		}
	}
}

/*
template<class META, class XMP, typename F>
void Serialize(META meta, XMP xmp, F f)
{
	f(meta, kXMP_NS_IPTCCore, CreatorContactInfo, "CiAdrExtadr", xmp.Address);
	f(meta, kXMP_NS_IPTCCore, CreatorContactInfo, "CiAdrCity", xmp.City);
	f(meta, kXMP_NS_IPTCCore, CreatorContactInfo, "CiAdrRegion", xmp.State);
}

namespace {

void read_field(SXMPMeta& meta, const char* namesp, const char* struct_name, const char* field_ns, const char* field_name, const String& str)
{
	meta.SetStructField(namesp, struct_name, field_ns, field_name, W2A(str), 0);
}

} */


static void SetStructField(SXMPMeta& meta, const char* namesp, const char* struct_name, const char* field_ns, const char* field_name, const String& str, int options)
{
	if (!str.empty())
		meta.SetStructField(namesp, struct_name, field_ns, field_name, W2A(str), options);
}


static void SetArrayItem(SXMPMeta& meta, const char* ns, const char* field, unsigned int options, const String& text, bool default_lang= true)
{
	meta.DeleteProperty(ns, field);
	if (default_lang)
		meta.SetLocalizedText(ns, field, 0, "x-default", W2A(text), 0);
	else
		meta.AppendArrayItem(ns, field, options, W2A(text), 0);
}


void XmpDataToMeta(const XmpData& xmp, SXMPMeta& meta)
{
	SetStructField(meta, kXMP_NS_IPTCCore, CreatorContactInfo, kXMP_NS_IPTCCore, "CiAdrExtadr",	xmp.Address, 0);
	SetStructField(meta, kXMP_NS_IPTCCore, CreatorContactInfo, kXMP_NS_IPTCCore, "CiAdrCity",	xmp.City, 0);
	SetStructField(meta, kXMP_NS_IPTCCore, CreatorContactInfo, kXMP_NS_IPTCCore, "CiAdrRegion",	xmp.State, 0);
	SetStructField(meta, kXMP_NS_IPTCCore, CreatorContactInfo, kXMP_NS_IPTCCore, "CiAdrPcode",	xmp.PostalCode, 0);
	SetStructField(meta, kXMP_NS_IPTCCore, CreatorContactInfo, kXMP_NS_IPTCCore, "CiAdrCtry",	xmp.Country, 0);
	SetStructField(meta, kXMP_NS_IPTCCore, CreatorContactInfo, kXMP_NS_IPTCCore, "CiTelWork",	xmp.Phones, 0);
	SetStructField(meta, kXMP_NS_IPTCCore, CreatorContactInfo, kXMP_NS_IPTCCore, "CiEmailWork",	xmp.EMails, 0);
	SetStructField(meta, kXMP_NS_IPTCCore, CreatorContactInfo, kXMP_NS_IPTCCore, "CiUrlWork",	xmp.WebSites, 0);

	meta.SetProperty(kXMP_NS_IPTCCore, "IntellectualGenre", W2A(xmp.IntellectualGenre), 0);
	meta.SetProperty(kXMP_NS_IPTCCore, "Location", W2A(xmp.Location), 0);
	meta.SetProperty(kXMP_NS_IPTCCore, "CountryCode", W2A(xmp.ISOCountryCode), 0);
	AddItems(xmp.IPTCSubjectCode, meta, kXMP_NS_IPTCCore, "SubjectCode");
	AddItems(xmp.IPTCScene, meta, kXMP_NS_IPTCCore, "Scene");

	meta.SetProperty(kXMP_NS_Photoshop, "TransmissionReference", W2A(xmp.JobIdentifier), 0);
	meta.SetProperty(kXMP_NS_Photoshop, "AuthorsPosition", W2A(xmp.CreatorsJob), 0);
	meta.SetProperty(kXMP_NS_Photoshop, "Credit", W2A(xmp.Provider), 0);
	meta.SetProperty(kXMP_NS_Photoshop, "Source", W2A(xmp.Source), 0);
	meta.SetProperty(kXMP_NS_Photoshop, "Instructions", W2A(xmp.Instructions), 0);
	meta.SetProperty(kXMP_NS_Photoshop, "Headline", W2A(xmp.Headline), 0);
	meta.SetProperty(kXMP_NS_Photoshop, "CaptionWriter", W2A(xmp.DescriptionWriter), 0);
	meta.SetProperty(kXMP_NS_Photoshop, "City", W2A(xmp.City2), 0);
	meta.SetProperty(kXMP_NS_Photoshop, "State", W2A(xmp.StateProvince), 0);
	meta.SetProperty(kXMP_NS_Photoshop, "Country", W2A(xmp.Country2), 0);
	meta.SetProperty(kXMP_NS_Photoshop, "DateCreated", W2A(xmp.CreationDate), 0);

	meta.SetProperty(kXMP_NS_XMP, "Rating", W2A(xmp.ImageRating), 0);
	meta.SetProperty(kXMP_NS_XMP, "CreatorTool", W2A(xmp.CreatorTool), 0);

	SetArrayItem(meta, kXMP_NS_XMP_Rights, "UsageTerms", kXMP_PropArrayIsAlternate, xmp.RightsUsageTerms);
	meta.SetProperty(kXMP_NS_XMP_Rights, "WebStatement", W2A(xmp.CopyrightInfoURL), 0);

	// clear array first
	SetArrayItem(meta, kXMP_NS_DC, "title", kXMP_PropArrayIsAlternate, xmp.DocumentTitle);
	SetArrayItem(meta, kXMP_NS_DC, "rights", kXMP_PropArrayIsAlternate, xmp.CopyrightNotice);
	SetArrayItem(meta, kXMP_NS_DC, "description", kXMP_PropArrayIsAlternate, xmp.Description);
	SetArrayItem(meta, kXMP_NS_DC, "creator", kXMP_PropArrayIsOrdered, xmp.Author/*.Creator*/, false);

	AddItems(xmp.Keywords, meta, kXMP_NS_DC, "subject");
}


template <class Meta>
std::string MetaToString(const Meta& meta, bool insert_namespace_header, bool insert_packet_wrapper, size_t ideal_size, size_t size_limit)
{
	std::string out;
	XMP_OptionBits options= kXMP_EncodeUTF8;// | (insert_namespace_header ? 0 : kXMP_OmitPacketWrapper);
	if (!insert_packet_wrapper)
		options |= kXMP_OmitPacketWrapper;

	XMP_StringLen padding= 0;	// default padding
	if (!insert_namespace_header)
		padding = 2;	// minimal allowed padding

	if (ideal_size > HEADER_NAMESPACE_LEN)
	{
		if (insert_namespace_header)
			ideal_size -= HEADER_NAMESPACE_LEN;

		options |= kXMP_ExactPacketLength;
	}

	try
	{
		const char* indent_char= " ";
		const char* new_line= "\n";

		meta.SerializeToBuffer(&out, options, ideal_size > 0 ? ideal_size : padding, new_line, indent_char);
	}
	catch (XMP_Error& err)
	{
		// skip expected "can't fit" exception
		if (err.GetID() != kXMPErr_BadSerialize || ideal_size == 0)
			throw;

		// retry w/o size limit
		options &= ~kXMP_ExactPacketLength;
		meta.SerializeToBuffer(&out, options, padding);
	}

	if (insert_namespace_header)
		out.insert(0, HEADER_NAMESPACE, HEADER_NAMESPACE_LEN);

	if (size_limit && out.size() > size_limit)
	{
		// packet too long, try again w/o padding (may throw)
		options |= kXMP_ExactPacketLength;

		out.clear();

		meta.SerializeToBuffer(&out, options, insert_namespace_header ? size_limit - HEADER_NAMESPACE_LEN : size_limit);

		if (insert_namespace_header)
			out.insert(0, HEADER_NAMESPACE, HEADER_NAMESPACE_LEN);
	}

	return out;
}


std::string XmpDataToMeta(const XmpData& xmp, bool insert_namespace_header, bool insert_packet_wrapper, size_t ideal_size, size_t size_limit)
{
	try
	{
		SXMPMeta meta;

		XmpDataToMeta(xmp, meta);

		return MetaToString(meta, insert_namespace_header, insert_packet_wrapper, ideal_size, size_limit);
	}
	catch (XMP_Error& err)
	{
		THROW_EXCEPTION(L"Translating file info to XMP data failed", CString(err.GetErrMsg()));
	}
}


//=============================================================================================================


void UniformLineEnds(String& text)
{
	replace_all(text, _T("\xd"), _T(""));
	replace_all(text, _T("\xa"), _T("\xd\xa"));
/*
	const size_t count= text.size();

	String out;
	out.reserve(count);

	for (size_t i= 0; i < count; ++i)
	{
	// this is buggy: enters extra eols
		if (text[i] == 0xd && text[i + 1] != 0xa
			||
			text[i] == 0xa)
		{
			out.push_back(0xd);
			out.push_back(0xa);
		}
		else
			out.push_back(text[i]);
	}

	return out; */
}


class A2Str
{
public:
	A2Str(String& s) : str(s)
	{
		str.clear();
	}

	void assign(const char* s, size_t count)
	{
		String raw;
#ifdef _UNICODE
		::MultiByteToWideString(s, static_cast<int>(count), raw, CP_UTF8);
#else
		raw.assign(s, count);
#endif
		UniformLineEnds(raw);
		str = raw;
	}

private:
	String& str;
};


typedef TXMPMeta<A2Str> AMeta;


static void GetItems(String& data, const AMeta& meta, const char* ns, const char* field, const TCHAR* separator= _T(", "))
{
	data.clear();

	const size_t count= meta.CountArrayItems(ns, field);

	if (count == 0)
		return;

	oStringstream ost;

	for (size_t i= 0; i < count; ++i)
	{
		String item;
		meta.GetArrayItem(ns, field, i + 1, &A2Str(item), 0);

		ost << item;
		if (i != count - 1)
			ost << separator;
	}

	data = ost.str();

	// uniform line ends!
	UniformLineEnds(data);
}


void MetaToXmpData(const std::vector<char>& xmp_buf, XmpData& xmp)
{
	if (xmp_buf.empty())
		THROW_EXCEPTION(L"Missing XMP data", L"Internal error: Expected XMP data is empty");

	try
	{
		AMeta meta(&xmp_buf.front(), xmp_buf.size());

		const TCHAR* LINE_END= _T("\r\n");
		const TCHAR* SPACE= _T(" ");

		meta.GetStructField(kXMP_NS_IPTCCore, CreatorContactInfo, kXMP_NS_IPTCCore, "CiAdrExtadr",	&A2Str(xmp.Address), 0);
		meta.GetStructField(kXMP_NS_IPTCCore, CreatorContactInfo, kXMP_NS_IPTCCore, "CiAdrCity",	&A2Str(xmp.City), 0);
		meta.GetStructField(kXMP_NS_IPTCCore, CreatorContactInfo, kXMP_NS_IPTCCore, "CiAdrRegion",	&A2Str(xmp.State), 0);
		meta.GetStructField(kXMP_NS_IPTCCore, CreatorContactInfo, kXMP_NS_IPTCCore, "CiAdrPcode",	&A2Str(xmp.PostalCode), 0);
		meta.GetStructField(kXMP_NS_IPTCCore, CreatorContactInfo, kXMP_NS_IPTCCore, "CiAdrCtry",	&A2Str(xmp.Country), 0);
		meta.GetStructField(kXMP_NS_IPTCCore, CreatorContactInfo, kXMP_NS_IPTCCore, "CiTelWork",	&A2Str(xmp.Phones), 0);
		meta.GetStructField(kXMP_NS_IPTCCore, CreatorContactInfo, kXMP_NS_IPTCCore, "CiEmailWork",	&A2Str(xmp.EMails), 0);
		meta.GetStructField(kXMP_NS_IPTCCore, CreatorContactInfo, kXMP_NS_IPTCCore, "CiUrlWork",	&A2Str(xmp.WebSites), 0);

		meta.GetProperty(kXMP_NS_IPTCCore, "IntellectualGenre", &A2Str(xmp.IntellectualGenre), 0);
		meta.GetProperty(kXMP_NS_IPTCCore, "Location", &A2Str(xmp.Location), 0);
		meta.GetProperty(kXMP_NS_IPTCCore, "CountryCode", &A2Str(xmp.ISOCountryCode), 0);
		GetItems(xmp.IPTCSubjectCode, meta, kXMP_NS_IPTCCore, "SubjectCode");
		GetItems(xmp.IPTCScene, meta, kXMP_NS_IPTCCore, "Scene");

		meta.GetProperty(kXMP_NS_Photoshop, "TransmissionReference", &A2Str(xmp.JobIdentifier), 0);
		meta.GetProperty(kXMP_NS_Photoshop, "AuthorsPosition", &A2Str(xmp.CreatorsJob), 0);
		meta.GetProperty(kXMP_NS_Photoshop, "Credit", &A2Str(xmp.Provider), 0);
		meta.GetProperty(kXMP_NS_Photoshop, "Source", &A2Str(xmp.Source), 0);
		meta.GetProperty(kXMP_NS_Photoshop, "Instructions", &A2Str(xmp.Instructions), 0);
		meta.GetProperty(kXMP_NS_Photoshop, "Headline", &A2Str(xmp.Headline), 0);
		meta.GetProperty(kXMP_NS_Photoshop, "CaptionWriter", &A2Str(xmp.DescriptionWriter), 0);
		meta.GetProperty(kXMP_NS_Photoshop, "City", &A2Str(xmp.City2), 0);
		meta.GetProperty(kXMP_NS_Photoshop, "State", &A2Str(xmp.StateProvince), 0);
		meta.GetProperty(kXMP_NS_Photoshop, "Country", &A2Str(xmp.Country2), 0);
		meta.GetProperty(kXMP_NS_Photoshop, "DateCreated", &A2Str(xmp.CreationDate), 0);

		meta.GetProperty(kXMP_NS_XMP, "Rating", &A2Str(xmp.ImageRating), 0);
		meta.GetProperty(kXMP_NS_XMP, "CreatorTool", &A2Str(xmp.CreatorTool), 0);

		GetItems(xmp.RightsUsageTerms, meta, kXMP_NS_XMP_Rights, "UsageTerms", LINE_END);
		//(kXMP_NS_XMP_Rights, "UsageTerms", kXMP_PropArrayIsAlternate, &A2Str(xmp.RightsUsageTerms), 0);
		meta.GetProperty(kXMP_NS_XMP_Rights, "WebStatement", &A2Str(xmp.CopyrightInfoURL), 0);

//		meta.GetProperty(kXMP_NS_DC, "title", &A2Str(xmp.DocumentTitle), 0);
		GetItems(xmp.DocumentTitle, meta, kXMP_NS_DC, "title", SPACE);
		GetItems(xmp.CopyrightNotice, meta, kXMP_NS_DC, "rights", SPACE);
		GetItems(xmp.Description, meta, kXMP_NS_DC, "description", LINE_END);
//		meta.GetProperty(kXMP_NS_DC, "creator", &A2Str(xmp.Author/*.Creator*/), 0);
		GetItems(xmp.Author, meta, kXMP_NS_DC, "creator", SPACE);
		GetItems(xmp.Keywords, meta, kXMP_NS_DC, "subject");
	}
	catch (XMP_Error& err)
	{
		THROW_EXCEPTION(L"Reading XMP metadata failed", CString(err.GetErrMsg()));
	}
}


bool ReadXmpFile(const TCHAR* xmpFile, XmpData& xmp)
{
	CFile file;
	if (!file.Open(xmpFile, CFile::modeRead))
		return false;

	uint64 length= file.GetLength();
	if (length > MaxXmpFileSize)	// come on, too big...
		return false;

	if (length == 0)
		return false;

	std::vector<char> buf(static_cast<size_t>(length));
	file.Read(&buf[0], buf.size());

	MetaToXmpData(buf, xmp);

	return true;
}

//char* strrstr(char* s, char* wanted)
//{
//	/*
//	* The odd placement of the two tests is so "" is findable.
//	* Also, we inline the first char for speed.
//	* The ++ on scan has been moved down for optimization.
//	*/
//	char firstc= *wanted;
//	size_t len= strlen(wanted);
//
//	for (char* scan= s + strlen(s) - len ; scan >= s ; scan--)
//		if (*scan == firstc)
//			if (strncmp(scan, wanted, len) == 0)
//				return scan;
//
//	return 0;
//}


bool RemoveXPacket(std::vector<char>& xmp_buf)
{
	const char* XPACKET= "<?xpacket";
	const char* END= "?>";
	const size_t LEN= strlen(XPACKET);
	const char* MAGIC= "W5M0MpCehiHzreSzNTczkc9d";

	if (xmp_buf.size() < 3 * LEN)
		return false;

	size_t i= 0;

	while (xmp_buf[i] != '<' && i < LEN)	// skip some white characters
		++i;

	if (i == LEN)
		return false;

	if (memcmp(&xmp_buf[i], XPACKET, LEN) != 0)
		return false;

	i += LEN;

	size_t limit= std::min<size_t>(i + 200, xmp_buf.size());

	for (size_t k= 0; i < limit; ++i)
		if (xmp_buf[i] == MAGIC[k])
		{
			k++;
			if (MAGIC[k] == 0)
				break;
		}

	for (size_t k= 0; i < limit; ++i)
		if (xmp_buf[i] == END[k])
		{
			k++;
			if (END[k] == 0)
				break;
		}

	for ( ; i < limit; ++i)
		if (xmp_buf[i] == '<')
			break;

	if (i == limit)
		return false;

	// find closing xpacket

	size_t end= 0; //xmp_buf.size() - 22;	// 22 - length of closing sequence plus 1
	char last_char= XPACKET[LEN - 1];

	for (size_t k= xmp_buf.size() - 1; k > LEN; --k)
		if (xmp_buf[k] == last_char)
			if (memcmp(&xmp_buf.front() + k - LEN + 1, XPACKET, LEN) == 0)
			{
				end = k - LEN + 1;
				break;
			}

	//for (size_t k= 0; end < xmp_buf.size(); ++end)
	//	if (xmp_buf[end] == XPACKET[k])
	//	{
	//		k++;
	//		if (XPACKET[k] == 0)
	//			break;
	//	}

	if (end == 0) //xmp_buf.size())
		return false;	// closing xpacket not found

//	end -= strlen(XPACKET);
	size_t remove= xmp_buf.size() - end; // - 1;

	xmp_buf.erase(xmp_buf.end() - remove, xmp_buf.end());

	xmp_buf.erase(xmp_buf.begin(), xmp_buf.begin() + i);

	return true;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////


std::string MergeXmpDataToMeta(const XmpData& xmp, std::vector<char>& existing_xmp, bool insert_namespace_header,
						  bool insert_packet_wrapper, size_t ideal_size, size_t size_limit)
{
	try
	{
		SXMPMeta meta;

		if (RemoveXPacket(existing_xmp))
			meta.ParseFromBuffer(&existing_xmp.front(), existing_xmp.size());

		XmpDataToMeta(xmp, meta);

		return MetaToString(meta, insert_namespace_header, insert_packet_wrapper, ideal_size, size_limit);
	}
	catch (XMP_Error& err)
	{
		THROW_EXCEPTION(L"Merging of XMP metadata failed", CString(err.GetErrMsg()));
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////


namespace {

struct InitXMP
{
	InitXMP()
	{
		SXMPMeta::Initialize();
	}
	~InitXMP()
	{
		SXMPMeta::Terminate();
	}
};

static InitXMP _initializeXmp;	// initialize XMP toolkit

}

#if 0
void test()
{
	if (0)
	{
//	char xmp[]= "<xmp?> blah blah";
	CFile xmp(_T("C:\\zdjecia\\zdjecia 20\\18-200\\kwiatek\\DSC_9574.xmp"), CFile::modeRead);
	size_t len= size_t(xmp.GetLength());
	vector<char> buf(len);
	xmp.Read(&buf[0], len);

	SXMPMeta meta(&buf[0], buf.size());

	string xpath;
	//SXMPUtils::ComposeStructFieldPath(kXMP_NS_RDF, "subject", kXMP_NS_DC, "", &xpath);
	SXMPUtils::ComposeArrayItemPath(kXMP_NS_DC, "subject", 1, &xpath);

	string str;
	XMP_OptionBits bits= 0;
//	if (meta.GetProperty(kXMP_NS_DC, xpath.c_str() /*"Description/subject/Bag"*/, &str, &bits))
//	if (meta.GetProperty(kXMP_NS_DC, "xmpmeta/rdf:RDF/rdf:Description/dc:subject/rdf:Bag", &str, &bits))
//	if (meta.GetProperty(kXMP_NS_DC, "RDF/rdf:Description/dc:subject/rdf:Bag", &str, &bits))
	if (meta.GetProperty(kXMP_NS_DC, "subject[1]", &str, &bits))
	{
		str;
	}
	}

	SXMPMeta meta;
//	meta.SetProperty(kXMP_NS_DC, "subject[1]", "dudu", kXMP_PropValueIsArray);
	meta.SetStructField(kXMP_NS_IPTCCore, "CreatorContactInfo", kXMP_NS_IPTCCore, "CiAdrCity", "Sant Cruz", 0);
	meta.SetStructField(kXMP_NS_IPTCCore, "CreatorContactInfo", kXMP_NS_IPTCCore, "CiAdrRegion", "CA", 0);
//	meta.AppendArrayItem(kXMP_NS_DC, "subject", kXMP_PropValueIsArray, "dudu", 0);//kXMP_PropValueIsArray);
//SetStructField ( XMP_StringPtr	schemaNS,
	//				 XMP_StringPtr	structName,
	//				 XMP_StringPtr	fieldNS,
	//				 XMP_StringPtr	fieldName,
	//				 XMP_StringPtr	fieldValue,
	//				 XMP_OptionBits options );
//#define kXMP_NS_DC       "http://purl.org/dc/elements/1.1/"
//#define kXMP_NS_IPTCCore "http://iptc.org/std/Iptc4xmpCore/1.0/xmlns/"
//#define kXMP_NS_RDF      "http://www.w3.org/1999/02/22-rdf-syntax-ns#"
//#define kXMP_NS_XML      "http://www.w3.org/XML/1998/namespace"
/*
 <rdf:Description rdf:about=''
  xmlns:Iptc4xmpCore='http://iptc.org/std/Iptc4xmpCore/1.0/xmlns/'>
  <Iptc4xmpCore:CreatorContactInfo rdf:parseType='Resource'>
   <Iptc4xmpCore:CiAdrCity>Santa Cruz</Iptc4xmpCore:CiAdrCity>
   <Iptc4xmpCore:CiAdrRegion>CA</Iptc4xmpCore:CiAdrRegion>
  </Iptc4xmpCore:CreatorContactInfo>
 </rdf:Description>
*/
	string out;
	meta.SerializeToBuffer(&out);

	out.size();
}


struct X
{
	X(int) { test(); }
};

static X xxx(10);
#endif

namespace {

	const char* TAGS_ARRAY= "subject";

}


void Utf8ToString(const std::string& utf8, String& out)
{
#ifdef _UNICODE
	::MultiByteToWideString(utf8, out);
#else
	out = utf8;
#endif
}


void StringToUtf8(const String& str, std::string& utf8)
{
#ifdef _UNICODE
	utf8.clear();
	::WideStringToMultiByte(str, utf8);
#else
	utf8 = str;
#endif
}


// Copy tags to the metadata replacing existing 'subject' array (if any)
//
void CopyTagsToMetadata(const std::vector<String>& tags, SXMPMeta& meta)
{
	/*
	XMP_Int32 count= meta.CountArrayItems(kXMP_NS_DC, TAGS_ARRAY);

	while (count > 0)
		meta.DeleteArrayItem(kXMP_NS_DC, TAGS_ARRAY, count--);
*/
	meta.DeleteProperty(kXMP_NS_DC, TAGS_ARRAY);

	for (size_t i= 0; i < tags.size(); ++i)
	{
		std::string utf8;
		StringToUtf8(tags[i], utf8);
		meta.AppendArrayItem(kXMP_NS_DC, TAGS_ARRAY, kXMP_PropValueIsArray, utf8, 0);
	}
}

// Copy keywords from XMP to the string vector
//
void CopyTagsToVector(const SXMPMeta& meta, std::vector<String>& tags)
{
	tags.clear();

	XMP_Int32 count= meta.CountArrayItems(kXMP_NS_DC, TAGS_ARRAY);

	if (count <= 0)
		return;

	tags.reserve(count);

	for (XMP_Int32 i= 0; i < count; ++i)
	{
		std::string utf8;
		XMP_OptionBits opt= 0;
		meta.GetArrayItem(kXMP_NS_DC, TAGS_ARRAY, i + 1, &utf8, &opt);

		String str;
		Utf8ToString(utf8, str);

		tags.push_back(str);
	}
}

// Read XMP file (if it exists) into 'meta'; can throw
//
bool OpenXmpFile(const TCHAR* xmpFile, SXMPMeta& meta)
{
	CFile xmp;
	if (!xmp.Open(xmpFile, CFile::modeRead))
		return false;

	uint64 length= xmp.GetLength();
	if (length > MaxXmpFileSize)	// come on, too big...
		return false;

	std::vector<char> buf(static_cast<size_t>(length));
	xmp.Read(&buf[0], buf.size());

	meta.ParseFromBuffer(&buf[0], buf.size());

	return true;
}


void LoadFromXmpFile(XmpData& xmp, const TCHAR* xmpFile)
{
	try
	{
		CFile file;
		if (!file.Open(xmpFile, CFile::modeRead))
			THROW_EXCEPTION(L"Cannot open XMP file for reading", xmpFile);

		uint64 length= file.GetLength();
		if (length > MaxXmpFileSize)	// come on, too big...
			THROW_EXCEPTION(L"XMP file is too big to read", xmpFile);

		std::vector<char> buf(static_cast<size_t>(length));
		file.Read(&buf[0], buf.size());

		Xmp::MetaToXmpData(buf, xmp);
	}
	catch (XMP_Error& err)
	{
		THROW_EXCEPTION(L"Loading information from XMP file failed", CString(err.GetErrMsg()));
	}
}


void SaveToXmpFile(const SXMPMeta& meta, const TCHAR* xmpFile)
{
	std::string out= MetaToString(meta, false, false, 0, 0);

	CFile xmp(xmpFile, CFile::modeWrite | CFile::modeCreate);

	if (!out.empty())
		xmp.Write(&out[0], out.size());

	xmp.Close();
}


void SaveToXmpFile(const XmpData& xmp, const TCHAR* xmpFile)
{
	try
	{
		std::string xmp_string= Xmp::XmpDataToMeta(xmp, false, false, 0, 0);

		if (xmp_string.empty())
			THROW_EXCEPTION(L"Generated XMP packet is empty", L"Internal error: XMP packet is empty");

		CFile file(xmpFile, CFile::modeWrite | CFile::modeCreate);

		file.Write(&xmp_string[0], xmp_string.size());

		file.Close();
	}
	catch (XMP_Error& err)
	{
		THROW_EXCEPTION(L"Saving file info to XMP file failed", CString(err.GetErrMsg()));
	}
}


void UpdateXmpFile(const XmpData& xmp, const TCHAR* xmpFile)
{
	try
	{
		bool back_up= false;

		SXMPMeta meta;
		try
		{
			// open returns false if file does not exist
			OpenXmpFile(xmpFile, meta);
		}
		catch (XMP_Error&)
		{
			// cleanup
			swap(meta, SXMPMeta());
			// back up bogus xmp file
			back_up = true;
		}

		// update
		XmpDataToMeta(xmp, meta);

		if (back_up)
		{
			Path p= xmpFile;
			p += _T(".bak");
			if (!::CopyFile(xmpFile, p.c_str(), false))
				THROW_EXCEPTION(L"Error creating backup copy of a file", xmpFile);
		}

		SaveToXmpFile(meta, xmpFile);
	}
	catch (XMP_Error& err)
	{
		THROW_EXCEPTION(L"Updating XMP file failed", CString(err.GetErrMsg()));
	}
}

/* this fn shouldn't be necessary any more
void SavePhotoTags(Path photoPath, const vector<String>* tags, int rating, const String& creatorTool)
{
	bool back_up= false;

	SXMPMeta meta;
	photoPath.ReplaceExtension(_T("xmp"));
	try
	{
		OpenXmpFile(photoPath.c_str(), meta);
	}
	catch (XMP_Error&)
	{
		// cleanup
		swap(meta, SXMPMeta());
		// back up bogus xmp file
		back_up = true;
	}

	if (tags)
		CopyTagsToMetadata(*tags, meta);

	char ratingStr[32];
	itoa(rating, ratingStr, 10);
	meta.SetProperty(kXMP_NS_XMP, "Rating", ratingStr, 0);

	meta.SetProperty(kXMP_NS_XMP, "CreatorTool", W2A(creatorTool), 0);

	if (back_up)
	{
		Path p= photoPath;
		p += _T(".bak");
		if (!::CopyFile(photoPath.c_str(), p.c_str(), false))
			throw String(_T("Error creating backup copy of ")) + photoPath;
	}

	SaveToXmpFile(meta, photoPath.c_str());
}*/

#if 0
void LoadPhotoTags(Path photoPath, vector<String>& tags)
{
	tags.clear();
	photoPath.ReplaceExtension(_T("xmp"));
	SXMPMeta meta;
	if (!OpenXmpFile(photoPath.c_str(), meta))
		return;

	CopyTagsToVector(meta, tags);
}
#endif


extern bool CheckIfXmpData(MemPointer& data, uint16 dataSize)
{
	const uint32 size= strlen(XMP_NAMESPACE) + 1;		// namespace size
	ASSERT(size == 29);
	if (dataSize < 2 + size)	// size field + ns size
		return false;

	BYTE ns[29/*size*/];
	data.Read(ns, size);		// namespace is first in XMP

	// cmp with ns (including '\0' at the end)
	return memcmp(ns, XMP_NAMESPACE, size) == 0;
}


void SaveXmpHelper(const XmpData& xmp, MemMappedFile& jpeg_file, const TCHAR* file_name)
{
/*
	const size_t new_xmp_size= 2 + 2 + xmp.size();

	const size_t max_packet= 0xffff;
	if (new_xmp_size > max_packet)	// according to specs XMP must fit in one app marker
	{
		oStringstream ost;
		ost << _T("Maximal XMP (file info) size exceeded by ") << new_xmp_size - max_packet << _T(" bytes.");
		throw ost.str();
	}
*/
	// note: it is expected that 'photo' is already open, but view may need to be established
	if (!jpeg_file.CreateWriteView(file_name, 0))	// open view
		THROW_EXCEPTION(L"Cannot write file info", SF(L"Cannot open file: " << file_name))

	MemPointer data(jpeg_file.GetBaseAddrChr(), jpeg_file.GetFileSize());

	if (ReadMarker(data) != MARK_SOI)		// not a JPEG image?
	{
		String msg= _T("Photograph '");
		msg += file_name;
		msg += _T("'\nis not a valid JPEG file.");
		throw String(msg);
	}

	const size_t MAX_APP_MARKER_SIZE= 0xfffe;	//TODO: what's the limit for JPEG marker?
	const size_t MAX_META_SIZE= MAX_APP_MARKER_SIZE - 2 - 2;

	std::string xmp_string;

	// check if APP1 is already present
	uint32 xmp_marker_size= 0;	// app1 block size including marker

	for (int i= 0; ; ++i)
	{
		bool found_marker_1= false;

		uint32 pos= data.RPosition();

		uint16 marker= ReadMarker(data);

		uint32 size_pos= data.RPosition();

		if (marker == MARK_APP1)
		{
			// marker found, read ptr is at the size field
			found_marker_1 = true;
		}
		// looking for app markers only (and make an exception for comment marker)
		else if (!IsJpegMarkerToBeSkipped(marker))
		{
			data.SetPos(pos);	// position before marker
			break;
		}

		// data length
		uint16 dataSize= data.GetUInt16();

		if (dataSize < 2)
		{
			// not a valid length; corrupted JPEG file?
			String msg= _T("Photograph '");
			msg += file_name;
			msg += _T("'\ncontains application marker blocks with invalid size.");
			throw String(msg);
		}

		if (found_marker_1)
		{
			if (CheckIfXmpData(data, dataSize))
			{
				// complete size (marker, size field includes self (2 bytes) and contents size)
				xmp_marker_size = (size_pos - pos) + dataSize;

				// read in XMP and merge
				std::vector<char> xmp_buf(dataSize - Xmp::HEADER_NAMESPACE_LEN, 0);
				data.Read(&xmp_buf.front(), xmp_buf.size());

				// ideal size is the same as the existing package; this may be possible thanks to padding;
				// minus 2 due to the fact that dataSize includes marker size field too (2 bytes)
				const size_t ideal_size= dataSize - 2;
				xmp_string = Xmp::MergeXmpDataToMeta(xmp, xmp_buf, true, true, ideal_size, MAX_META_SIZE);

				data.SetPos(pos);
				break;
			}
			else
				data.SetPos(size_pos + 2);
		}

		data += dataSize - 2;	// skip app marker data

		if (i == 1000)	// safety counter
		{
			String msg= _T("Photograph '");
			msg += file_name;
			msg += _T("'\nhas too many markers.");
			throw String(msg);
		}
	}

	if (xmp_string.empty())
		xmp_string = Xmp::XmpDataToMeta(xmp, true, true, 0, MAX_META_SIZE);

	if (xmp_string.empty())
		throw String(_T("Generated XMP packet is empty"));

	const size_t new_xmp_size= 2 + 2 + xmp_string.size();

	if (new_xmp_size > MAX_APP_MARKER_SIZE)	// according to specs XMP must fit in one app marker
	{
		oStringstream ost;
		ost << _T("Maximal XMP (file info) size exceeded by ") << new_xmp_size - MAX_APP_MARKER_SIZE << _T(" bytes.");
		throw ost.str();
	}

	// read position is set before marker; xmp will be inserted here

	uint32 writing_location= data.RPosition();

	size_t bytes_to_insert= 0;
	size_t bytes_to_delete= 0;

	if (xmp_marker_size > 0)	// overwrite marker that follows
	{
		if (xmp_marker_size >= new_xmp_size)	// enough size already?
			bytes_to_delete = xmp_marker_size - new_xmp_size;
		else
			bytes_to_insert = new_xmp_size - xmp_marker_size;
	}
	else
	{
		bytes_to_insert = new_xmp_size;
	}

	if (!jpeg_file.CreateWriteView(file_name, static_cast<int>(bytes_to_insert)))
	{
		String msg= _T("Error writing to the '");
		msg += file_name;
		msg += _T("'\nfile.");
		throw String(msg);
	}

	// prepare space and write xmp block

	data.Reset(jpeg_file.GetBaseAddrChr(), jpeg_file.GetFileSize(), writing_location);

	// move file contents to make/contract space

	size_t length= jpeg_file.GetFileSize() - writing_location;

	if (bytes_to_insert > 0)
		memmove(data.GetPtr() + bytes_to_insert, data.GetPtr(), length);
	else if (bytes_to_delete > 0)
		memmove(data.GetPtr() + new_xmp_size, data.GetPtr() + xmp_marker_size, length - xmp_marker_size);

	// finally write XMP packet

	data.PutUInt16(MARK_APP1);
	data.PutUInt16(new_xmp_size - 2);
	data.Write(&xmp_string[0], xmp_string.size());

	//if (jpeg_file.IsOpen())
	//{
	//	//TODO: check if this is necessary
	//	jpeg_file.Touch();	// cache may miss changes without updating date/time stamp
	//}

	// shorten photo?
	if (bytes_to_delete != 0)
		jpeg_file.SetFileSize(jpeg_file.GetFileSize() - bytes_to_delete);
}


void SaveXmpHelper(const XmpData& xmp, const TCHAR* file_name)
{
	MemMappedFile photo;
	if (!photo.CreateWriteView(file_name, 0))
	{
		String msg= _T("Photograph '");
		msg += file_name;
		msg += _T("'\ncannot be opened for writing.");
		throw String(msg);
	}

	SaveXmpHelper(xmp, photo, file_name);
}


void SaveXmpIntoJpeg(const XmpData& xmp, MemMappedFile& jpeg_file, const TCHAR* file_name)
{
	SaveXmpHelper(xmp, jpeg_file, file_name);
}


// save XMP in the JPEG's APP1 marker merging it with existing metadata
void SaveXmpIntoJpeg(const XmpData& xmp, const TCHAR* jpeg_file)
{
	try
	{
		// create backup of original file (with auto-restore in case of exception)
		FileGuard backup(jpeg_file, true);

		SaveXmpHelper(xmp, jpeg_file);

		// delete backup if saving succeeded
		backup.DeleteBackup();
	}
	catch (XMP_Error& err)
	{
		THROW_EXCEPTION(L"Saving file info to JPEG image failed", SF(L"File: " <<  jpeg_file << L"\nError: " << err.GetErrMsg()));
	}
}


// check to see if XMP file accompanying image can be edited
bool CanEditXmpFile(Path photoPath, int& err_code)
{
	err_code = 0;

	photoPath.ReplaceExtension(_T("xmp"));

	if (photoPath.FileExists())
	{
		MemMappedFile view;
		if (!view.CreateWriteView(photoPath.c_str(), 0, true))	// try to open in write mode
		{
			err_code = ::GetLastError();
			return false;
		}

		return true;
	}
	else
	{
		// no accompanying XMP file

		// check if one can be created

		HANDLE file= ::CreateFile(photoPath.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, 0);

		if (file == INVALID_HANDLE_VALUE)
		{
			err_code = ::GetLastError();
			return false;
		}

		::CloseHandle(file);

		::DeleteFile(photoPath.c_str());

		return true;
	}
}


} // namespace

/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// IPTCRecord.cpp: implementation of the IPTCRecord class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "IPTCRecord.h"
#include "StringConversions.h"
#include "MemPointer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////

void WideToMultibyte(const wchar_t* s, std::string& out)
{
	// writing as well as size calc routines have to use the same wide string conversion routine;
	// IPTC does not support UTF-8
	::WideStringToMultiByte(s, out, CP_ACP);
}

size_t StrLength(const std::string& str)
{
	return str.size();
}

size_t StrLength(const std::wstring& str)
{
	std::string tmp;
	WideToMultibyte(str.c_str(), tmp);
	return tmp.size();
}


size_t IPTCRecord::CalcRecordSize() const
{
	const size_t RECORD= 5;	// 5 bytes per short record entry
	size_t size= (17 + keywords_.size()) * RECORD;

	size += 2;		// mandatory version record
	size += StrLength(byline_);
	size += StrLength(byline_title_);
	size += StrLength(credits_);
	size += StrLength(source_);
	size += StrLength(caption_writer_);
	size += StrLength(caption_);
	size += StrLength(headline_);
	size += StrLength(special_instructions_);
	size += StrLength(object_name_);
	size += StrLength(date_created_);
	size += StrLength(city_);
	size += StrLength(state_);
	size += StrLength(country_);
	size += StrLength(original_transmission_reference_);
	size += StrLength(copyright_notice_);
	size += StrLength(contact_);

	const size_t keys= keywords_.size();
	for (size_t i= 0; i < keys; ++i)
		size += StrLength(keywords_[i]);

	return size;
}


void IPTCRecord::CopyTo(uint8* data) const
{
	LowWriteRecord(data, 0x200, "\x0\x02", 2);
	WriteRecord(data, 0x250, byline_);
	WriteRecord(data, 0x255, byline_title_);
	WriteRecord(data, 0x26e, credits_);
	WriteRecord(data, 0x273, source_);
	WriteRecord(data, 0x205, object_name_);
	WriteRecord(data, 0x237, date_created_);
	WriteRecord(data, 0x25a, city_);
	WriteRecord(data, 0x25f, state_);
	WriteRecord(data, 0x265, country_);
	WriteRecord(data, 0x267, original_transmission_reference_);
	WriteRecord(data, 0x274, copyright_notice_);
	WriteRecord(data, 0x276, contact_);
	WriteRecord(data, 0x278, caption_);
	WriteRecord(data, 0x27a, caption_writer_);
	WriteRecord(data, 0x269, headline_);
	WriteRecord(data, 0x228, special_instructions_);

	size_t keys= keywords_.size();
	for (size_t i= 0; i < keys; ++i)
		WriteRecord(data, 0x219, keywords_[i]);
}


void IPTCRecord::LowWriteRecord(uint8*& data, uint16 rec_data_set, const char* mem, int len) const
{
	*data++ = 0x1c;
	*data++ = rec_data_set >> 8;
	*data++ = rec_data_set & 0xff;
	ASSERT(len < 0x8000);
	if (len >= 0x8000)
		len = 0x7fff;
	*data++ = len >> 8;
	*data++ = len & 0xff;

	for (int i= 0; i < len; ++i)
		*data++ = mem[i];
}


void IPTCRecord::WriteRecord(uint8*& data, uint16 rec_data_set, const char* string) const
{
	LowWriteRecord(data, rec_data_set, string, strlen(string));
}


void IPTCRecord::WriteRecord(uint8*& data, uint16 rec_data_set, const wchar_t* string) const
{
	std::string str;
	WideToMultibyte(string, str);

	*data++ = 0x1c;
	*data++ = rec_data_set >> 8;
	*data++ = rec_data_set & 0xff;
	size_t len= str.size(); // wcslen(string);
	ASSERT(len < 0x8000);
	if (len >= 0x8000)
		len = 0x7fff;
	*data++ = static_cast<uint8>(len >> 8);
	*data++ = static_cast<uint8>(len & 0xff);

	for (int i= 0; i < len; ++i)
		*data++ = str[i];
}


void IPTCRecord::WriteRecord(uint8*& data, uint16 rec_data_set, const String& str) const
{
/*
#ifdef _UNICODE

	extern bool WideStringToMultiByte(const wstring& in, string& out);

	string ascii;
	WideStringToMultiByte(str, ascii);
	WriteRecord(data, rec_data_set, ascii.c_str());

#else

	WriteRecord(data, rec_data_set, str.c_str());

#endif
  */

	WriteRecord(data, rec_data_set, str.c_str());
}


void IPTCRecord::LimitKeywordsLength()
{
	int keys= keywords_.size();
	for (int i= 0; i < keys; ++i)
		if (keywords_[i].length() > KEYWORD_LENGTH_LIMIT)
			keywords_[i] = keywords_[i].substr(0, KEYWORD_LENGTH_LIMIT);
}


static void WriteWString(MemPointer& ptr, const String& str)
{
	ptr.WriteWString(str);
}

static void ReadWString(MemPointer& ptr, String& str)
{
	ptr.ReadWString(str);
}


void IPTCRecord::Read(MemPointer& ptr)
{
	ReadWString(ptr, byline_);
	ReadWString(ptr, byline_title_);
	ReadWString(ptr, credits_);
	ReadWString(ptr, source_);
	ReadWString(ptr, caption_writer_);
	ReadWString(ptr, headline_);
	ReadWString(ptr, special_instructions_);
	ReadWString(ptr, object_name_);
	ReadWString(ptr, date_created_);
	ReadWString(ptr, city_);
	ReadWString(ptr, state_);
	ReadWString(ptr, country_);
	ReadWString(ptr, original_transmission_reference_);

	const size_t size= ptr.GetUInt32();
	keywords_.resize(size);
	for (size_t i= 0; i < size; ++i)
		ReadWString(ptr, keywords_[i]);

	ReadWString(ptr, copyright_notice_);
	ReadWString(ptr, contact_);

	// reserved
	ptr.GetUInt32();
	ptr.GetUInt32();
	ptr.GetUInt32();
	ptr.GetUInt32();
}


void IPTCRecord::Write(MemPointer& ptr) const
{
	WriteWString(ptr, byline_);
	WriteWString(ptr, byline_title_);
	WriteWString(ptr, credits_);
	WriteWString(ptr, source_);
	WriteWString(ptr, caption_writer_);
	WriteWString(ptr, headline_);
	WriteWString(ptr, special_instructions_);
	WriteWString(ptr, object_name_);
	WriteWString(ptr, date_created_);
	WriteWString(ptr, city_);
	WriteWString(ptr, state_);
	WriteWString(ptr, country_);
	WriteWString(ptr, original_transmission_reference_);

	const size_t size= keywords_.size();
	ptr.PutUInt32(size);
	for (size_t i= 0; i < size; ++i)
		WriteWString(ptr, keywords_[i]);

	WriteWString(ptr, copyright_notice_);
	WriteWString(ptr, contact_);

	// reserved
	ptr.PutUInt32(0);
	ptr.PutUInt32(0);
	ptr.PutUInt32(0);
	ptr.PutUInt32(0);
}


void IPTCRecord::Clear()
{
	byline_.clear();
	byline_title_.clear();
	credits_.clear();
	source_.clear();
	caption_writer_.clear();
	caption_.clear();
	headline_.clear();
	special_instructions_.clear();
	object_name_.clear();
	date_created_.clear();
	city_.clear();
	state_.clear();
	country_.clear();
	original_transmission_reference_.clear();
	keywords_.clear();
	copyright_notice_.clear();
	contact_.clear();
}

/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// IPTCRecord.h: interface for the IPTCRecord class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IPTCRECORD_H__D39CB625_7BA5_4E00_AFE5_3470AB93F94B__INCLUDED_)
#define AFX_IPTCRECORD_H__D39CB625_7BA5_4E00_AFE5_3470AB93F94B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
class MemPointer;


struct IPTCRecord
{
	// Credits
	String byline_;
	String byline_title_;
	String credits_;
	String source_;

	// Caption
	String caption_writer_;
	String caption_;
	String headline_;
	String special_instructions_;

	// Origin
	String object_name_;
	String date_created_;
	String city_;
	String state_;
	String country_;
	String original_transmission_reference_;

	// Categories
//	String category_;
//	vector<String> supplemental_categories_;

	// Keywords
	std::vector<String> keywords_;

	enum { KEYWORD_LENGTH_LIMIT= 64 };	// 64 octets; this limit is imposed by IPTC v.4 standard

	// Copyright
	String copyright_notice_;
	String contact_;
//	String imageURL_;

	size_t CalcRecordSize() const;

	void CopyTo(uint8* data) const;

	void LimitKeywordsLength();

	void Read(MemPointer& ptr);
	void Write(MemPointer& ptr) const;

	void Clear();

private:
	void LowWriteRecord(uint8*& data, uint16 rec_data_set, const char* mem, int len) const;
	void WriteRecord(uint8*& data, uint16 rec_data_set, const char* string) const;
	void WriteRecord(uint8*& data, uint16 rec_data_set, const wchar_t* string) const;
	void WriteRecord(uint8*& data, uint16 rec_data_set, const String& str) const;
};

#endif // !defined(AFX_IPTCRECORD_H__D39CB625_7BA5_4E00_AFE5_3470AB93F94B__INCLUDED_)

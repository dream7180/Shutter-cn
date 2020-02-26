/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// OutputStr.h: interface for the OutputStr class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_OUTPUTSTR_H__FE2AD076_16DC_4523_8F68_65BF4F17ADAC__INCLUDED_)
#define AFX_OUTPUTSTR_H__FE2AD076_16DC_4523_8F68_65BF4F17ADAC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class Data;


class OutputStr
{
public:
	OutputStr();
	~OutputStr();

	enum { EXTRA_TAG= 0x80000000, INDENT= 0x40000000, SUB_TAG= 0x20000000 };

	// for low level (and high level) data
	void RecordInfo(uint32 tag, const TCHAR* tag_name, const Data& val, const TCHAR* interpreted_val= 0);

	void RecordInfo(uint32 tag, const String& tag_name, const Data& val, const TCHAR* interpreted_val= 0)
	{ RecordInfo(tag, tag_name.c_str(), val, interpreted_val); }

	void RecordInfo(uint32 tag, const TCHAR* tag_name, const TCHAR* val, const TCHAR* interpreted_val);

	void RecordInfo(uint32 tag, const TCHAR* tag_name, int val, const TCHAR* interpreted_val);

	void SetInterpretedInfo(const String& value);
	void SetInterpretedInfo(double value);

	// for high level data only
//	void RecordInfo(uint32 tag, const TCHAR* tag_name, const TCHAR* interpreted_val);

	void RecordInfo(uint32 tag, const TCHAR* tag_name, const String& interpreted_val)
	{ RecordInfo(tag, tag_name, interpreted_val.c_str()); }
/*
	void RecordInfo(const TCHAR* tag_name, const TCHAR* interpreted_val)
	{ RecordInfoHi(tag_name, interpreted_val); }

	void RecordInfo(const TCHAR* tag_name, const String& interpreted_val)
	{ RecordInfoHi(tag_name, interpreted_val.c_str()); }

	void RecordInfoHi(const TCHAR* tag_name, const TCHAR* interpreted_val);

	void RecordInfoHi(const TCHAR* tag_name, double val);

	void RecordInfoHi(const TCHAR* tag_name, const String& interpreted_val)
	{ RecordInfoHi(tag_name, interpreted_val.c_str()); }
*/
	void Indent();
	void Outdent();

	String GetInfo(bool raw) const;

	void StartRecording();

	// no of lines
	int Count() const				{ return static_cast<int>(tags_.size()); }

	void Clear();

	// format tag
	void GetTagText(int line, TCHAR* output, int max_size) const;
	bool GetNameText(int line, TCHAR* output, int max_size) const;
	bool GetValueText(int line, TCHAR* output, int max_size) const;
	bool GetInterpretedText(int line, TCHAR* output, int max_size) const;

private:
	std::vector<uint32> tags_;	// tags
	String tags_names_;		// tag names
	String values_;			// raw values
	String interpreted_;	// interpreted values
	int indent_;

	bool GetText(const TCHAR* full_string, int line, TCHAR* output, int max_size) const;
};

#endif // !defined(AFX_OUTPUTSTR_H__FE2AD076_16DC_4523_8F68_65BF4F17ADAC__INCLUDED_)

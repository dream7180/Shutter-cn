/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// OutputStr.cpp: implementation of the OutputStr class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Data.h"
#include "OutputStr.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

OutputStr::OutputStr()
{
	tags_.reserve(60);
	tags_names_.reserve(60 * 10);
	values_.reserve(60 * 15);
	interpreted_.reserve(200);
	indent_ = 0;
}


OutputStr::~OutputStr()
{}


void OutputStr::RecordInfo(uint32 tag, const TCHAR* tag_name, const Data& val, const TCHAR* interpreted_val/*= 0*/)
{
	if (interpreted_val)
		RecordInfo(tag, tag_name, val.AsString().c_str(), interpreted_val);
	else
		RecordInfo(tag, tag_name, val.AsString(false).c_str(), val.AsString().c_str());
}


CString ReplaceNewLines(const TCHAR* text, const TCHAR* replacement= _T("  "))
{
	if (text == 0 || *text == 0)
		return CString();

	CString str= text;
	str.Replace(_T("\n"), replacement);
	str.Replace(_T("\r"), _T(""));
	return str;
}


void OutputStr::RecordInfo(uint32 tag, const TCHAR* tag_name, int val, const TCHAR* interpreted_val)
{
	TCHAR buf[64];
	_itot(val, buf, 10);
	RecordInfo(tag, tag_name, buf, interpreted_val);
}


void OutputStr::RecordInfo(uint32 tag, const TCHAR* tag_name, const TCHAR* val, const TCHAR* interpreted_val)
{
	tags_.push_back(indent_ ? tag | INDENT : tag);

	tags_names_.append(tag_name);
	tags_names_.append(_T("\n"));

	if (val)
		values_.append(ReplaceNewLines(val));
	values_.append(_T("\n"));

	if (interpreted_val == 0)
		interpreted_val = val;

	if (interpreted_val)
		interpreted_.append(ReplaceNewLines(interpreted_val));
	interpreted_.append(_T("\n"));
}


void OutputStr::SetInterpretedInfo(double value)
{
	oStringstream ost;
	ost << value;
	SetInterpretedInfo(ost.str());
}


void OutputStr::SetInterpretedInfo(const String& value)
{
	if (interpreted_.empty())
	{
		ASSERT(false);
		return;
	}

	String::size_type pos= 0;
	if (interpreted_.length() > 1)
	{
		pos = interpreted_.rfind(_T('\n'), interpreted_.length() - 2);
		if (pos == String::npos)
			pos = 0;
		else
			pos++;
	}

	String::size_type n= interpreted_.length() - pos - 1;

	if (value.find(_T('\n')) != String::npos)
	{
		// get rid of new lines first
		interpreted_.replace(pos, n, ReplaceNewLines(value.c_str()));
	}
	else
		interpreted_.replace(pos, n, value);
}


void OutputStr::Indent()
{
	++indent_;
}


void OutputStr::Outdent()
{
	--indent_;
}


String OutputStr::GetInfo(bool raw) const
{
	String str;
	int count= tags_.size();
	for (int i= 0; i < count; ++i)
	{
		TCHAR buf[MAX_PATH];
//		memset(buf, 0xff, 2*(MAX_PATH + 5));
		GetTagText(i, buf, MAX_PATH);
//		if (buf[MAX_PATH] != 0xffff) { ASSERT(false); }
		str.append(buf);
		str += _T('\t');

		GetNameText(i, buf, MAX_PATH);
//		if (buf[MAX_PATH] != 0xffff) { ASSERT(false); }
		str.append(buf);
		str += _T('\t');

		if (raw)
			GetValueText(i, buf, MAX_PATH);
		else
			GetInterpretedText(i, buf, MAX_PATH);
		//if (buf[MAX_PATH] != 0xffff) { ASSERT(false); }
		//if (_tcslen(buf) >= MAX_PATH - 1)
		//{ ASSERT(false); }
		str.append(buf);

		str.append(_T("\r\n"));
	}
	return str;
}


void OutputStr::StartRecording()
{
	indent_ = 0;
/*	output_raw_ = _T("File: ");
	output_raw_ += file_name;
	output_raw_ += _T("\r\n\r\nTag\tName\tValue\r\n");

	output_hi_ = _T("File: ");
	output_hi_ += file_name;
	output_hi_ += _T("\r\n\r\nName\tValue\r\n"); */
}


void OutputStr::GetTagText(int line, TCHAR* output, int max_size) const
{
	if (output == 0)
		return;

	if (line < 0 || line >= tags_.size())
		*output = 0;
	else if ((tags_[line] & ~INDENT) == 0)
		_tcscpy(output, _T("-"));
	else
	{
		if (tags_[line] & EXTRA_TAG)
			*output++ = _T('.'), --max_size;
		if (tags_[line] & SUB_TAG)
			*output++ = _T('.'), --max_size;

		int tag= tags_[line] & ~(EXTRA_TAG | INDENT | SUB_TAG);

		if (tags_[line] & SUB_TAG)
			_stprintf_s(output, max_size, _T("%02x"), tag);
		else
			_stprintf_s(output, max_size, _T("%04x"), tag);
	}
}


bool OutputStr::GetNameText(int line, TCHAR* output, int max_size) const
{
	return GetText(tags_names_.c_str(), line, output, max_size);
}

bool OutputStr::GetValueText(int line, TCHAR* output, int max_size) const
{
	return GetText(values_.c_str(), line, output, max_size);
}

bool OutputStr::GetInterpretedText(int line, TCHAR* output, int max_size) const
{
	return GetText(interpreted_.c_str(), line, output, max_size);
}


bool OutputStr::GetText(const TCHAR* full_string, int line, TCHAR* output, int max_size) const
{
	if (full_string == 0 || output == 0 || max_size < 1)
	{
		ASSERT(false);
		return false;
	}

	*output = 0;
	TCHAR sep= _T('\n');

	while (line--)
	{
		full_string = _tcschr(full_string, sep);

		if (full_string == 0)
			return false;

		full_string++;		// point past the separator
	}

	const TCHAR* end= _tcschr(full_string, sep);
	int len = (end == 0) ? _tcslen(full_string) : (int)(end - full_string);
	ASSERT(len >= 0);

	int size= std::min(max_size - 1, len);
	memcpy(output, full_string, size * sizeof(TCHAR));
	output[size] = 0;

	return true;
}


void OutputStr::Clear()
{
	tags_.swap(std::vector<uint32>());
	tags_names_.swap(String());		// tag names
	values_.swap(String());			// raw values
	interpreted_.swap(String());	// interpreted values
	indent_ = 0;
}

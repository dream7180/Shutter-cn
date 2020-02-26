/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#ifndef _data_def_h_
#define _data_def_h_

#include "File.h"
#include "Rational.h"


class Data
{
public:
	enum { tUBYTE= 1, tASCII, tUSHORT, tULONG, tURATIONAL, tSBYTE, tUNDEF, tSSHORT, tSLONG, tSRATIONAL, tFLOAT, tDOUBLE };

	Data(FileStream& ifs, Offset ifd_start);

	uint32 GetData() const		{ return data_offset_; }
	uint16 GetSwapedWord() const;

	String AsString(bool dec_rational= true, bool force_string= false) const;
	String AsUnicodeString() const;
	std::string AsAnsiString() const;
	uint32 AsULong() const;
	double AsDouble() const;
	// return tUBYTE or tASCII data in a char vector
	bool AsRawData(std::vector<char>& data) const;

	Rational Rational() const;
	SRational SRational() const;

	// 3 Rational numbers forming degrees, minutes & seconds
	bool GetDegMinSec(::Rational val[3]);

	bool AsDoubleVector(std::vector<double>& vec) const;

	uint32 Length() const		{ return Length(data_format_, components_); }
	uint32 Components() const	{ return components_; }
	bool FormatWord() const		{ return data_format_ == tUSHORT || data_format_ == tSSHORT; }
	bool FormatSingleByte() const
	{ return data_format_ == tUBYTE || data_format_ == tASCII || data_format_ == tSBYTE || data_format_ == tUNDEF; }
	uint16 Format() const		{ return data_format_; }
	Offset IfdOffset() const	{ return ifd_start_; }

	bool IsUndefData() const	{ return data_format_ == tUNDEF; }
	static uint16 UndefDataType();

	void ReadChar(char* buf, int len) const;
	void ReadUChar(uint8* buf, int len) const;
	void ReadWords(uint16* buff, int size) const;
	void ReadLongs(uint32* buff, int size) const;

	bool IsLongData() const		{ return long_data_; }

	static bool HasOffsetData(uint16 data_format, uint32 components);

	const uint8* DataBytes() const	{ return data_; }

	bool IsValid() const;

	bool GetByteOrder() const;

private:
	FileStream& ifs_;
	Offset ifd_start_;
	bool long_data_;		// if true data stored in file (doesn't fit into 'data_offset_' component)
	uint16 data_format_;
	uint16 data_ext_;
	uint32 components_;
	uint32 data_offset_;	// this is either single value data or offset to long data
	uint8 data_[4];

	static uint32 Length(uint16 data_format);
	static uint32 Length(uint16 data_format, uint32 components);
};


#endif // _data_def_h_

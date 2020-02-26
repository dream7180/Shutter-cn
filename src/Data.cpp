/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "Data.h"
#include "StringConversions.h"
#include "StringFormat.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// format of TIFF data entry
// field:size in bytes
// [tag:2] - tag (outside of this struct, preceeds Data struct)
// [fmt:2] - data type (enum)
// [cnt:4] - count of items (if array, 1 for singular pieces)
// [offset/data:4] - offset to data > 4B, or data itself if fits in 4B
//

Data::Data(FileStream& ifs, Offset ifd_start) : ifs_(ifs), ifd_start_(ifd_start)
{
	ifs >> data_format_;
	ifs >> components_;
	data_offset_ = 0;
	data_ext_ = 0;
	long_data_ = false;
	data_[0] = data_[1] = data_[2] = data_[3] = 0;

	uint32 length= Length(data_format_, components_);

	if (length > 4)
	{
		data_offset_ = ifs.GetUInt32();
		long_data_ = true;
	}
	else if (data_format_ == tUSHORT || data_format_ == tSSHORT)
	{
		data_offset_ = ifs.GetUInt16();
		data_ext_ = ifs.GetUInt16();
	}
	else if (data_format_ == tUBYTE || data_format_ == tSBYTE)
	{
		data_offset_ = data_[0] = ifs.GetUInt8();
		data_[1] = ifs.GetUInt8();
		data_[2] = ifs.GetUInt8();
		data_[3] = ifs.GetUInt8();
	}
	else if (data_format_ == tUNDEF)
	{
		if (length == 1)
		{
			data_offset_ = data_[0] = ifs.GetUInt8();
			data_[1] = ifs.GetUInt8();
			data_[2] = ifs.GetUInt8();
			data_[3] = ifs.GetUInt8();
		}
		else if (length == 2)
		{
			data_offset_ = ifs.GetUInt16();
			data_ext_ = ifs.GetUInt16();
		}
		else
			data_offset_ = ifs.GetUInt32(data_);
	}
	else
		data_offset_ = ifs.GetUInt32(data_);
}


std::string Data::AsAnsiString() const
{
	if (data_format_ != tUBYTE && data_format_ != tASCII)
		return std::string();

	Offset temp= ifs_.RPosition();

	uint32 len= std::min(components_, 64536u);	// limit to 64k chars

	std::string str;
	str.resize(len, 0);

	ifs_.RPosition(ifd_start_, data_offset_, false);

	for (uint32 i= 0; i < len; ++i)
		str[i] = ifs_.GetUInt8();

	ifs_.RPosition(temp, FileStream::beg);

	if (str.length() > 0 && str[str.length() - 1] == 0)
		str.resize(str.length() - 1);

	return str;
}


bool Data::AsRawData(std::vector<char>& data) const
{
	data.clear();

	if (data_format_ != tUBYTE && data_format_ != tASCII)
		return false;

	Offset temp= ifs_.RPosition();

	uint32 len= components_;

	data.resize(len, 0);

	ifs_.RPosition(ifd_start_, data_offset_, false);

	for (uint32 i= 0; i < len; ++i)
		data[i] = ifs_.GetUInt8();

	ifs_.RPosition(temp, FileStream::beg);

	return true;
}


String Data::AsUnicodeString() const
{
	if (data_format_ != tUBYTE)
		return String();

	Offset temp= ifs_.RPosition();

	uint32 len= std::min(components_ / 2, 64536u);	// limit to 64k chars

	std::wstring str;
	str.resize(len, 0);

	ifs_.RPosition(ifd_start_, data_offset_, false);

	for (uint32 i= 0; i < len; ++i)
		str[i] = ifs_.GetUInt16();

	ifs_.RPosition(temp, FileStream::beg);

	if (str.length() > 0 && str[str.length() - 1] == 0)
		str.resize(str.length() - 1);

#ifdef _UNICODE
	return str;
#else
	string s;
	WideStringToMultiByte(str, s);
	return s;
#endif
}


String Data::AsString(bool dec_rational/*= true*/, bool force_string/*= false*/) const
{
//	oStringstream ost;
	string_format ost;
	Offset temp= ifs_.RPosition();
	const uint32 MAX_COUNT= 20u;
	const TCHAR* SEPARATOR= _T(", ");

	if (long_data_)
		ifs_.RPosition(ifd_start_, data_offset_, false);

	switch (data_format_)
	{
	case tUBYTE:		// unsigned byte
		if (long_data_)
		{
			//ifs_.RPosition(ifd_start_, data_offset_, false);
			uint32 count= std::min(components_, MAX_COUNT);
			for (uint32 i= 0; i < count; ++i)
			{
				if (i > 0)
					ost << SEPARATOR;
				// cast to uint32 to avoid confusion with wchar_t!
				ost << static_cast<uint32>(ifs_.GetUInt8());
			}
		}
		else
		{
			for (uint32 i= 0; i < components_; ++i)
			{
				if (i > 0)
					ost << SEPARATOR;
				ost << static_cast<uint32>(data_[i]);
			}
		}
		break;

	case tASCII:		// ascii string
		{
			std::string buf;
			if (components_ > 4)
			{
				uint32 len= components_;
				int max= 8 * 1024;
				if (len > max)
				{
					ASSERT(false);
					len = max;
				}
				//ifs_.RPosition(ifd_start_, data_offset_, false);
				ifs_.GetString(buf, len);
				ifs_.RPosition(temp, FileStream::beg);
			}
			else
			{
				buf.assign(reinterpret_cast<const char*>(data_), components_);
			}
#ifdef _UNICODE
			return MultiByteToWideString(buf);
#else
			return buf;
#endif
		}
		break;

	case tUSHORT:
		if (long_data_)
		{
			//ifs_.RPosition(ifd_start_, data_offset_, false);
			uint32 count= std::min(components_, MAX_COUNT);
			for (uint32 i= 0; i < count; ++i)
			{
				if (i > 0)
					ost << SEPARATOR;
				// cast to uint32 to avoid confusion with wchar_t!
				ost << static_cast<uint32>(ifs_.GetUInt16());
			}
		}
		else if (components_ == 2)
			ost << data_offset_ << SEPARATOR << static_cast<uint32>(data_ext_);
		else
			ost << static_cast<uint32>(data_offset_);
		break;

	case tULONG:
		if (long_data_)
		{
			//ifs_.RPosition(ifd_start_, data_offset_, false);
			uint32 count= std::min(components_, MAX_COUNT);
			for (uint32 i= 0; i < count; ++i)
			{
				if (i > 0)
					ost << SEPARATOR;
				ost << ifs_.GetUInt32();
			}
		}
		else
			ost << uint32(data_offset_);
		break;

	case tURATIONAL:
		{
			//ifs_.RPosition(ifd_start_, data_offset_, false);
			uint32 count= std::min(components_, MAX_COUNT);
			for (uint32 i= 0; i < count; ++i)
			{
				uint32 num= ifs_.GetUInt32();
				uint32 denom= ifs_.GetUInt32();
				if (i > 0)
					ost << SEPARATOR;
				if (dec_rational && denom == 1)
					ost << num << _T(".0");
				else if (dec_rational && denom == 10)
					ost << num / 10 << _T(".") << num % 10;
				else
					ost << num << _T("/") << denom;
			}
		}
		break;

	case tSBYTE:
		if (long_data_)
		{
			//ifs_.RPosition(ifd_start_, data_offset_, false);
			uint32 count= std::min(components_, MAX_COUNT);
			for (uint32 i= 0; i < count; ++i)
			{
				if (i > 0)
					ost << SEPARATOR;
				ost << static_cast<int16>(ifs_.GetUInt8());
			}
		}
		else
		{
			for (uint32 i= 0; i < components_; ++i)
			{
				if (i > 0)
					ost << SEPARATOR;
				ost << static_cast<int16>(data_[i]);
			}
		}
		break;

	case tUNDEF:
		if (force_string)
		{
			std::string buf;
			if (components_ > 4)
				ifs_.RPosition(ifd_start_, data_offset_, false);
			else
				ifs_.RPosition(temp - 4, FileStream::beg);
			ifs_.GetString(buf, components_);
			ifs_.RPosition(temp, FileStream::beg);
#ifdef _UNICODE
			return MultiByteToWideString(buf);
#else
			return buf;
#endif
		}
		else if (!long_data_ && components_ != 0)
		{
			//ost << std::hex;
			ost.hex();
			ost.fill(_T('0'));
			for (uint32 i= 0; i < components_; ++i)
			{
				ost << _T("0x");
//				ost.width(2);
				ost << uint32(data_[i]);
				if (i < components_ - 1)
					ost << _T(" ");
			}
		}
		else
		{
			ost << _T("[") << components_ << _T(" bytes]");

			if (components_ > 4)
			{
				//ifs_.RPosition(ifd_start_, data_offset_, false);
				ost << _T(" ");
				const uint32 MAX_COUNT= 70;
				uint32 count= std::min(components_, MAX_COUNT);
				ost.reserve(count * 5);
				for (uint32 i= 0; i < count; ++i)
				{
					if (i > 0)
						ost << SEPARATOR;
					// cast to uint32 to avoid confusion with wchar_t!
					ost << static_cast<uint32>(ifs_.GetUInt8());
				}
				if (components_ > MAX_COUNT)
					ost << SEPARATOR << _T("...");
			}
		}
		break;

	case tSSHORT:
		if (long_data_)
		{
			//ifs_.RPosition(ifd_start_, data_offset_, false);
			uint32 count= std::min(components_, MAX_COUNT);
			for (uint32 i= 0; i < count; ++i)
			{
				if (i > 0)
					ost << SEPARATOR;
				ost << static_cast<int16>(ifs_.GetUInt16());
			}
		}
		else if (components_ == 2)
			ost << data_offset_ << SEPARATOR << static_cast<uint32>(data_ext_);
		else
			ost << static_cast<int16>(data_offset_ & 0xffff);
		break;

	case tSLONG:
		if (long_data_)
		{
			//ifs_.RPosition(ifd_start_, data_offset_, false);
			uint32 count= std::min(components_, MAX_COUNT);
			for (uint32 i= 0; i < count; ++i)
			{
				if (i > 0)
					ost << SEPARATOR;
				ost << static_cast<int32>(ifs_.GetUInt32());
			}
		}
		else
			ost << static_cast<int32>(data_offset_);
		break;

	case tSRATIONAL:
		{
			//ifs_.RPosition(ifd_start_, data_offset_, false);
			uint32 count= std::min(components_, MAX_COUNT);
			for (uint32 i= 0; i < count; ++i)
			{
				int32 num= ifs_.GetUInt32();
				int32 denom= ifs_.GetUInt32();
				if (num < 0 && denom < 0)
					num = -num, denom = -denom;
				else if (denom < 0)
					num = -num, denom = -denom;
				if (i > 0)
					ost << SEPARATOR;
				if (dec_rational && denom == 1)
					ost << num << _T(".0");
				else if (dec_rational && denom == 10)
					ost << num / 10 << _T(".") << abs(num) % 10;
				else
					ost << num << _T("/") << denom;
			}
		}
		break;

	case tFLOAT:
		if (long_data_)
		{
			//ifs_.RPosition(ifd_start_, data_offset_, false);
			uint32 count= std::min(components_, MAX_COUNT);
			for (uint32 i= 0; i < count; ++i)
			{
				if (i > 0)
					ost << SEPARATOR;
				uint32 float_val= ifs_.GetUInt32();
				ost << *reinterpret_cast<const float*>(&float_val);
			}
		}
		else
			ost << *reinterpret_cast<const float*>(&data_offset_);
		break;

	case tDOUBLE:
		ASSERT(false);
		break;

	case 0:
		break;

	default:
		ASSERT(false);
		break;
	}

	ifs_.RPosition(temp, FileStream::beg);

	return ost.str();
}


uint32 Data::Length(uint16 data_format, uint32 components)
{
	return Length(data_format) * components;
}


uint32 Data::Length(uint16 data_format)
{
	switch (data_format)
	{
	case tUBYTE:
	case tASCII:
	case tUNDEF:
	case tSBYTE:
		return 1;

	case tUSHORT:
	case tSSHORT:
		return 2;

	case tULONG:
	case tSLONG:
	case tFLOAT:
		return 4;

	case tURATIONAL:
	case tSRATIONAL:
	case tDOUBLE:
		return 8;

	default:
		return 1;
	}
}


Rational Data::Rational() const
{
	::Rational ret;

	Offset temp= ifs_.RPosition();
	if (data_format_ == tURATIONAL)
	{
		ifs_.RPosition(ifd_start_, data_offset_, false);
		ret.numerator_ = ifs_.GetUInt32();
		ret.denominator_ = ifs_.GetUInt32();
	}
	else if (data_format_ == tSRATIONAL)
	{
		ifs_.RPosition(ifd_start_, data_offset_, false);
		int32 num= ifs_.GetUInt32();
		int32 denom= ifs_.GetUInt32();
		if (num < 0 && denom < 0)
			num = -num, denom = -denom;
		else if (denom < 0)
			num = -num, denom = -denom;
		ret.numerator_ = num;
		ret.denominator_ = denom;
	}
	else
	{
		ret.numerator_ = data_offset_;
		ret.denominator_ = 1;
	}
	ifs_.RPosition(temp, FileStream::beg);

	return ret;
}


SRational Data::SRational() const
{
	return Data::Rational();
}


void Data::ReadChar(char* buf, int len) const
{
	memset(buf, 0, len);
	Offset temp= ifs_.RPosition();
	if (long_data_)
	{
		ifs_.RPosition(ifd_start_, data_offset_, false);
		ifs_.Read(buf, std::min(components_, static_cast<uint32>(len)));
	}
	else
	{
		ifs_.RPosition(-4);
		ifs_.Read(buf, std::min(components_, static_cast<uint32>(len)));
	}
	ifs_.RPosition(temp, FileStream::beg);
}


void Data::ReadUChar(uint8* buf, int len) const
{
	ReadChar(reinterpret_cast<char*>(buf), len);
}


void Data::ReadWords(uint16* buff, int size) const
{
	memset(buff, 0, size * sizeof buff[0]);
	if (long_data_)
	{
		Offset temp= ifs_.RPosition();
		ifs_.RPosition(ifd_start_, data_offset_, false);
		//TODO: fix it: not reliable
		int count= std::min(components_, static_cast<uint32>(size));
		for (int i= 0; i < count; ++i)
			buff[i] = ifs_.GetUInt16();
		ifs_.RPosition(temp, FileStream::beg);
	}
	else
	{
		ASSERT(components_ == 1 || components_ == 2);
		if (size >= 1)
			buff[0] = static_cast<uint16>(data_offset_);
		if (size >= 2)
			buff[1] = static_cast<uint16>(data_ext_);
	}
}


void Data::ReadLongs(uint32* buff, int size) const
{
	memset(buff, 0, size * sizeof buff[0]);
	if (long_data_)
	{
		Offset temp= ifs_.RPosition();
		ifs_.RPosition(ifd_start_, data_offset_, false);
		int count= std::min(components_, static_cast<uint32>(size));
		for (int i= 0; i < count; ++i)
			buff[i] = ifs_.GetUInt32();
		ifs_.RPosition(temp, FileStream::beg);
	}
	else
	{
		ASSERT(components_ == 1);
		if (size > 0)
			buff[0] = data_offset_;
	}
}


uint32 Data::AsULong() const
{
	if (!long_data_)
		return data_offset_;

	uint32 val= 0;

	Offset temp= ifs_.RPosition();
	ifs_.RPosition(ifd_start_, data_offset_, false);
	switch (data_format_)
	{
	case tUBYTE:
	case tSBYTE:
//	case tUNDEF:
		val = ifs_.GetUInt8();
		break;

	case tUSHORT:
	case tSSHORT:
		val = ifs_.GetUInt16();
		break;

	case tULONG:
	case tSLONG:
		val = ifs_.GetUInt32();
		break;

	default:
		ASSERT(false);
		break;
	}

	return val;
}


bool Data::HasOffsetData(uint16 data_format, uint32 components)
{
	uint32 size= Data::Length(data_format, components);
	return size > 4;
}

uint16 Data::UndefDataType()
{
	return tUNDEF;
}


// 3 Rational numbers forming degrees, minutes & seconds
//
bool Data::GetDegMinSec(::Rational val[3])
{
	if (components_ != 3 || data_format_ != tURATIONAL)
		return false;

	Offset temp= ifs_.RPosition();

	ifs_.RPosition(ifd_start_, data_offset_, false);

	for (int i= 0; i < 3; ++i)
	{
		val[i].numerator_ = ifs_.GetUInt32();
		val[i].denominator_ = ifs_.GetUInt32();
	}

	ifs_.RPosition(temp, FileStream::beg);

	return true;
}


bool Data::AsDoubleVector(std::vector<double>& vec) const
{
	if (components_ < 2 && data_format_ != tURATIONAL && data_format_ != tSRATIONAL)
		return false;

	vec.clear();
	vec.resize(components_, 0.0);

	Offset temp= ifs_.RPosition();
	ifs_.RPosition(ifd_start_, data_offset_, false);

	for (int i= 0; i < components_; ++i)
	{
		uint32 numerator= ifs_.GetUInt32();
		uint32 denominator= ifs_.GetUInt32();

		if (data_format_ == tSRATIONAL)
		{
			vec[i] = static_cast<int32>(numerator);
			vec[i] /= static_cast<int32>(denominator);
		}
		else
		{
			vec[i] = numerator;
			vec[i] /= denominator;
		}
	}

	ifs_.RPosition(temp, FileStream::beg);

	return true;
}


uint16 Data::GetSwapedWord() const
{
	if (components_ != 2 || data_format_ != tASCII)
		return 0;

	return data_[0] | (data_[1] << 8);
}


double Data::AsDouble() const
{
	switch (data_format_)
	{
		case tURATIONAL:
			return Rational().Double();

		case tSRATIONAL:
			return SRational().Double();

		default:
			return AsULong();
	}
}


bool Data::IsValid() const
{
	if (data_format_ == 0 || data_format_ > tDOUBLE)
		return false;

	//TODO: more checking

	return true;
}


bool Data::GetByteOrder() const
{
	return ifs_.GetByteOrder();
}

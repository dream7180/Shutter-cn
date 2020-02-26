/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#pragma once


class MemWriter
{
public:
	MemWriter(BYTE* buffer) : buffer_(buffer) {}

	MemWriter& operator << (DWORD val)
	{
		memcpy(buffer_, &val, sizeof val);
		buffer_ += sizeof val;
		return *this;
	}

	MemWriter& operator << (UINT val)
	{
		memcpy(buffer_, &val, sizeof val);
		buffer_ += sizeof val;
		return *this;
	}

	MemWriter& operator << (const RECT& rect)
	{
		memcpy(buffer_, &rect, sizeof rect);
		buffer_ += sizeof rect;
		return *this;
	}

	MemWriter& operator << (const CString& str)
	{
		DWORD len= static_cast<DWORD>(str.GetLength());
		*this << len;
		int str_len= len * sizeof TCHAR;
		memcpy(buffer_, static_cast<const TCHAR*>(str), str_len);
		buffer_ += str_len;
		return *this;
	}

	MemWriter& operator << (const std::vector<BYTE>& vect)
	{
		DWORD len= static_cast<DWORD>(vect.size());
		*this << len;
		if (len > 0)
            memcpy(buffer_, &vect.front(), len);
		buffer_ += len;
		return *this;
	}

	BYTE* GetPtr()			{ return buffer_; }

private:
	BYTE* buffer_;
};


class MemReader
{
public:
	MemReader(const BYTE* buffer) : buffer_(buffer) {}

	MemReader& operator >> (DWORD& val)
	{
		memcpy(&val, buffer_, sizeof val);
		buffer_ += sizeof val;
		return *this;
	}

	MemReader& operator >> (UINT& val)
	{
		memcpy(&val, buffer_, sizeof val);
		buffer_ += sizeof val;
		return *this;
	}

	MemReader& operator >> (RECT& rect)
	{
		memcpy(&rect, buffer_, sizeof rect);
		buffer_ += sizeof rect;
		return *this;
	}

	MemReader& operator >> (CString& str)
	{
		DWORD len= 0;
		*this >> len;
		if (len > 0)
			str = CString(reinterpret_cast<const TCHAR*>(buffer_), len);
		else
			str.Empty();
		buffer_ += len * sizeof TCHAR;
		return *this;
	}

	MemReader& operator >> (std::vector<BYTE>& vect)
	{
		vect.clear();
		DWORD len= 0;
		*this >> len;
		if (len > 0)
			vect.assign(buffer_, buffer_ + len);
		buffer_ += len;
		return *this;
	}

private:
	const BYTE* buffer_;
};

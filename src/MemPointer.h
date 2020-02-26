/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// MemPointer.h: interface for the MemPointer class.
//
//////////////////////////////////////////////////////////////////////

#pragma once
#include <boost/function.hpp>


class MemPointer
{
public:
	class MemPtrException : public std::exception
	{
	public:
		MemPtrException() {}

		virtual const char* what() const;
	};


	MemPointer() : data_(0), base_(0), end_(0), big_endian_(true), ret_zero_at_err_(false)
	{}
	MemPointer(uint8* data, size_t size) : data_(data), base_(data), end_(data + size),
		big_endian_(true), ret_zero_at_err_(false)
	{}
	MemPointer(const MemPointer& p) : data_(p.data_), base_(p.base_), end_(p.end_),
		big_endian_(p.big_endian_), ret_zero_at_err_(false)
	{}
	~MemPointer()
	{}

	void SetByteOrder(bool big_endian= false)	{ big_endian_ = big_endian; }
	bool GetByteOrder() const					{ return big_endian_; }

	uint32 GetUInt32(uint8* external_buff);
	uint64 GetUInt64();
	uint32 GetUInt32();
	uint16 GetUInt16();
	uint32 GetBigEndianUInt32();
	int32 GetInt32();
	int16 GetInt16();
	int8 GetInt8();

	uint8 GetUInt8()
	{
		if (data_ < end_)
			return *data_++;
		else
			return Error();
	}

	// for char arrays only
	template<class TArray>
		void Read(TArray& array)
	{
		ASSERT(sizeof(array[0]) == 1);
		if (data_ + array_count(array) >= end_)
		{
			Error();
			memset(array, 0, array_count(array));
		}
		else
			memcpy(array, data_, array_count(array)); data_ += array_count(array);
	}

	void Read(void* data, size_t size);

	void Write(const void* data, size_t size);

	void WriteBufWithSize(const std::vector<uint8>& data);	// write buffer storing its size too

	void ReadBufWithSize(std::vector<uint8>& data);			// read back buffer from memory

	void WriteString(const String& str);
	void WriteWString(const std::wstring& str);
	void WriteWString(const std::string& str);	// writes char string as wstring

	void ReadString(String& str);
	void ReadWString(std::wstring& str);
	void ReadWString(std::string& str);

	void PutUInt64(uint64 val);
	void PutUInt32(uint32 val);
	void PutUInt16(uint16 val);

	void PutChar(char val)
	{
		MakeSpace(sizeof val);
		*data_++ = val;
	}

	void SetPos(ptrdiff_t pos)
	{
		data_ = base_ + pos;
		if (data_ > end_ || data_ < base_)
			Error();
	}

	ptrdiff_t GetPos() const
	{
		return data_ - base_;
	}

	MemPointer& operator += (ptrdiff_t offset)
	{
		data_ += offset;
		if (data_ > end_ || data_ < base_)
			Error();
		return *this;
	}

	MemPointer& operator -= (ptrdiff_t offset)
	{
		data_ -= offset;
		if (data_ > end_ || data_ < base_)
			Error();
		return *this;
	}

	uint32 RPosition() const
	{
		ASSERT(data_ - base_ < uint64(0x100000000));
		return static_cast<uint32>(data_ - base_);
	}

	uint8* GetPtr() const
	{
		return data_;
	}

	void Reset(uint8* data, size_t size, size_t offset)
	{
		base_ = data + offset;
		end_ = data + size;
		data_ = base_;
	}

	void GetString(std::string& buf, uint32 size);

	bool IsValid(ptrdiff_t pos) const
	{
		uint8* data= base_ + pos;
		return data <= end_ && data >= base_;
	}

	// skip zeros starting from current position
	uint32 SkipZeros()
	{
		uint8* start= data_;
		if (data_ < end_ && *data_ == 0)
			data_++;
		return static_cast<uint32>(data_ - start);
	}

	void UncheckedOffset(int offset)
	{
		data_ += offset;
	}

	uint32 RemainingBytes() const		{ return static_cast<uint32>(end_ - data_); }

	void SetExceptions(bool enable)		{ ret_zero_at_err_ = !enable; }

	void SetResizeCallback(const boost::function<void (size_t size)>& resize)	{ resize_ = resize; }

	// underlying buffer size has changed
	void Resize(uint8* data, size_t size);

private:
	uint8* base_;
	uint8* end_;
	uint8* data_;
	bool big_endian_;
	bool ret_zero_at_err_;
	boost::function<void (size_t size)> resize_;

	uint8 Error() const;
	void MakeSpace(size_t size);
};

/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#ifndef _file_stream_h_
#define _file_stream_h_

// Helper file that can read data from file or memory

typedef uint32 Offset;

#define USE_MEM_MAP_FILE

#ifdef USE_MEM_MAP_FILE // ==============================================================

#include "MemMappedFile.h"
#include "MemPointer.h"

// binary input stream
//
class FileStream
{
public:
	FileStream() {}

	bool Open(const TCHAR* file_name, bool read_only= true);

	bool Open(const std::vector<uint8>& buffer);

	void Close()								{ file_map_.CloseFile(); }

	void SetByteOrder(bool big_endian= false)	{ ptr_.SetByteOrder(big_endian); }
	bool GetByteOrder() const					{ return ptr_.GetByteOrder(); }

	// reading
	FileStream& operator >> (uint32& val)		{ val = GetUInt32(); return *this; }
	FileStream& operator >> (int32& val)		{ val = static_cast<int32>(GetUInt32()); return *this; }

	FileStream& operator >> (uint16& val)		{ val = GetUInt16(); return *this; }
	FileStream& operator >> (int16& val)		{ val = static_cast<int16>(GetUInt16()); return *this; }

	FileStream& operator >> (uint8& val)		{ val = GetUInt8(); return *this; }
	FileStream& operator >> (int8& val)			{ val = static_cast<int8>(GetUInt8()); return *this; }

	FileStream& operator >> (bool& flag)		{ flag = GetUInt8() != 0; return *this; }

	// for char arrays only
	template<class TArray>
		void Read(TArray& array)				{ ptr_.Read(array, array_count(array)); }

	void Read(char* buf, size_t size)			{ ptr_.Read(buf, size); }

	void Read(uint8* buf, size_t size)			{ ptr_.Read(buf, size); }

	void Read(uint32* vect, size_t size);

	void Write(const char* data, size_t size)	{ ptr_.Write(data, size); }

	void GetString(std::string& str, uint32 size)	{ ptr_.GetString(str, size); }

	enum seekdir { cur, beg, end };
	void RPosition(int32 offset, seekdir dir= cur);
	void RPosition(int32 offset_base, int32 offset, bool dummy)	{ ptr_.SetPos(offset_base + offset); }
	Offset RPosition()							{ return ptr_.RPosition(); }

	// set read position to 'offset' from the beggining
	void RPosFromBeg(uint32 offset)				{ ptr_.SetPos(offset); }

	uint32 GetUInt32()							{ return ptr_.GetUInt32(); }
	uint16 GetUInt16()							{ return ptr_.GetUInt16(); }
	uint8  GetUInt8()							{ return ptr_.GetUInt8(); }
	uint32 GetUInt32(uint8* external_buff);

	uint32 GetBigEndianUInt32()					{ return ptr_.GetBigEndianUInt32(); }

	uint32 RemainingBytes() const				{ return ptr_.RemainingBytes(); }

	uint32 GetLength32() const					{ return file_map_.GetFileSize(); }

	uint8* GetMapBaseAddr() const				{ return file_map_.GetBaseAddrChr(); }

	uint32 SkipZeros()							{ return ptr_.SkipZeros(); }

	void SetExceptions(bool enable)				{ ptr_.SetExceptions(enable); }

	~FileStream();

	bool IsOffsetValid(ptrdiff_t pos) const		{ return ptr_.IsValid(pos); }

private:
	MemMappedFile file_map_;
	MemPointer ptr_;

	// no copy allowed
	void operator = (const FileStream&);
	FileStream(const FileStream&);
};


#else	// USE_MEM_MAP_FILE =============================================================


// binary input stream
//
class FileStream
{
public:
//	FileStream(const TCHAR* file_name); // : fs_(file_name, ios_base::in /*| ios_base::out*/ | ios_base::binary),
//		big_endian_(true)
//	{ fs_.exceptions(ios_base::badbit | ios_base::failbit | ios_base::eofbit); }

	FileStream() : big_endian_(true)
	{}

	bool Open(const TCHAR* file_name)		{ return !!file_.Open(file_name, CFile::modeRead, 0); }

	void Close()								{ file_.Close(); }

//	void Flush()								{ file_.Flush(); }

	void SetByteOrder(bool big_endian= false)	{ big_endian_ = big_endian; }
	bool GetByteOrder() const					{ return big_endian_; }

	// reading
	FileStream& operator >> (uint32& val)		{ val = GetUInt32(); return *this; }
	FileStream& operator >> (int32& val)		{ val = static_cast<int32>(GetUInt32()); return *this; }

	FileStream& operator >> (uint16& val)		{ val = GetUInt16(); return *this; }
	FileStream& operator >> (int16& val)		{ val = static_cast<int16>(GetUInt16()); return *this; }

	FileStream& operator >> (uint8& val)		{ val = GetUInt8(); return *this; }
	FileStream& operator >> (int8& val)		{ val = static_cast<int8>(GetUInt8()); return *this; }

//	FileStream& operator >> (string& rstrText)	{ getline(fs_ ,rstrText, '\0'); return *this; }

	FileStream& operator >> (bool& flag)		{ flag = GetUInt8() != 0; return *this; }

	// for char arrays only
	template<class TArray>
		void Read(TArray& array)				{ file_.Read(array, array_count(array)); }

	void Read(char* buf, uint32 size)			{ file_.Read(buf, size); }

	void GetString(string& buf, uint32 size);

/*	// writing
	FileStream& operator << (uint32 val)			{ PutUInt32(val); return *this; }
	FileStream& operator << (int32 val)			{ PutUInt32(val); return *this; }

	FileStream& operator << (uint16 val)			{ PutUInt16(val); return *this; }
	FileStream& operator << (int16 val)			{ PutUInt16(val); return *this; }

	FileStream& operator << (uint8 val)			{ fs_.put(val); return *this; }
	FileStream& operator << (int8 val)			{ fs_.put(val); return *this; }

	FileStream& operator << (const string& rstrText)	{ fs_.write(rstrText.c_str(), rstrText.length() + 1); return *this; }

	FileStream& operator << (bool flag)			{ fs_.put(flag ? '\xff' : '\0'); return *this; }

	// for char arrays only
	template<class TArray>
		void Write(TArray& array)				{ fs_.write(array, array_count(array)); }

	void WriteData(const Offset* pcuData, uint32 size)
	{ for (uint32 i= 0; i < size; ++i) *this << pcuData[i]; }

	void ZeroOut(uint32 size);
*/
//	bool operator ! ()							{ return !fs_; }
/*
	void WPosition(int32 offset, ios_base::seekdir dir= ios_base::beg)
	{ fs_.seekp(offset, dir); }

	Offset WPosition()
	{ Offset pos= fs_.tellp(); if (pos == -1) throw 1; return pos; }
*/
	enum seekdir { cur, beg, end };
	void RPosition(int32 offset, seekdir dir= cur);

	void RPosition(int32 offset_base, int32 offset, bool dummy);

	Offset RPosition();

//	void PadToWord()
//	{ Offset pos= WPosition(); if (pos & 1) fs_.put('\0'); }

	uint32 GetUInt32(uint8* external_buff= 0)
	{
		uint8 vuchBuf[4];
		uint8* buf= external_buff ? external_buff : vuchBuf;
//		fs_.read(reinterpret_cast<char*>(buf), array_count(vuchBuf));
		file_.Read(buf, array_count(vuchBuf));
		if (big_endian_)
			return buf[3] | (uint32(buf[2]) << 8) | (uint32(buf[1]) << 16) | (uint32(buf[0]) << 24);
		else
			return buf[0] | (uint32(buf[1]) << 8) | (uint32(buf[2]) << 16) | (uint32(buf[3]) << 24);
	}

	uint16 GetUInt16()
	{
		uint8 vuchBuf[2];
//		fs_.read(reinterpret_cast<char*>(vuchBuf), array_count(vuchBuf));
		file_.Read(vuchBuf, array_count(vuchBuf));
		if (big_endian_)
			return vuchBuf[1] | (uint16(vuchBuf[0]) << 8);
		else
			return vuchBuf[0] | (uint16(vuchBuf[1]) << 8);
	}

	uint8 GetUInt8()
	{
		uint8 vuchBuf[1];
//		fs_.read(reinterpret_cast<char*>(vuchBuf), array_count(vuchBuf));
		file_.Read(vuchBuf, array_count(vuchBuf));
		return vuchBuf[0];
	}

/*
	void PutUInt32(uint32 val)
	{
		char vchBuf[4]= { val >> 24, val >> 16, val >> 8, val };
		fs_.write(vchBuf, array_count(vchBuf));
	}

	void PutUInt16(uint16 val)
	{
		char vchBuf[2]= { val >> 8, val };
		fs_.write(vchBuf, array_count(vchBuf));
	}
*/

private:
//	fstream fs_;
	CFile file_;
	bool big_endian_;

	// no copy allowed
	void operator = (const FileStream&);
	FileStream(const FileStream&);
};


#endif	// USE_MEM_MAP_FILE =============================================================


#endif // _file_stream_h_

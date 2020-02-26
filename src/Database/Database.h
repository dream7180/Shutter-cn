/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// Database.h: interface for the Database class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DATABASE_H__5570585E_478B_4F87_A849_D321107D102F__INCLUDED_)
#define AFX_DATABASE_H__5570585E_478B_4F87_A849_D321107D102F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "../MemMappedFile.h"
#include "StorageFile.h"
#include <boost/noncopyable.hpp>
#include "StdFile.h"


class Database : boost::noncopyable
{
public:
	Database(bool use_index= true);
	virtual ~Database();

	// open db file
	bool Open(const String& db_path, uint32 db_version, bool read_only);

	// find offset to the record
	uint64 FindRecord(const String& key, String& key_found);

	// append record; return it's position (offset)
	uint64 Append(const String& key, const std::vector<uint8>& record);
	// append record (without a key; it will not be indexed)
	uint64 Append(const std::vector<uint8>& record);

	// in-place update
	void Update(uint64 offset, const String& key, const std::vector<uint8>& record);
	void Update(uint64 offset, const std::vector<uint8>& record);

	// read record
	void Read(uint64 offset, std::vector<uint8>& record);

	// read record's key (returns record's size)
	uint32 ReadKey(uint64 offset, String& key);

	// read record's size
	uint32 ReadRecordSize(uint64 offset);

	// try to read next record
	bool ReadNext(std::vector<uint8>& record, uint64* offset= 0);

	// return offset to the next record (or 0)
	uint64 NextRecordOffset();

	bool IsOpen() const;

	// close db files
	void Close();

	// flush data to the disk
	void Flush();

	// delete record (currently index only is removed)
	void DeleteRecord(uint64 offset);

	// return name of index file
	static String GetIndexName(const String& db_file_name);

private:

	class Index : public std::vector<uint64>
	{
	public:
		Index() : db_(0), modified_(false)
		{}

		~Index();

		// read index in
		bool Read(const String& file, bool read_only);

		void Insert(const String& key, uint64 offset);

		// write index file if index has been modified
		void Flush();

		bool IsOpen() const;

		std::vector<uint64>::iterator Find(const String& key, String& key_found);

		void SetDb(Database* db)		 { db_ = db; }

		void Close()					 { file_index_.Close(); }

		void Remove(uint64 offset);

	private:
		Database* db_;
		//String cur_key_;
		StdFile file_index_;
		bool modified_;
	};

	StorageFile file_db_;
	bool read_only_;
	Index index_;
	bool use_index_;
	CCriticalSection lock_;

	friend class DatabaseLocker;

	// lock/unlock modify 'lock_' critical section;
	// this locking is actually only used in a derived class
	void Lock();
	void Unlock();
};


class DatabaseLocker
{
public:
	DatabaseLocker(Database& db) : db_(db)
	{
		db_.Lock();
	}

	~DatabaseLocker()
	{
		db_.Unlock();
	}

private:
	Database& db_;
};

#endif // !defined(AFX_DATABASE_H__5570585E_478B_4F87_A849_D321107D102F__INCLUDED_)

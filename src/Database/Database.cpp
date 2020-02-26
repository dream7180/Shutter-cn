/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// Database.cpp: implementation of the Database class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Database.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Database::Database(bool use_index)
{
	read_only_ = true;
	index_.SetDb(this);
	use_index_ = use_index;
}

Database::~Database()
{}


bool Database::IsOpen() const
{
	if (read_only_)
		return file_db_.IsOpen();//hack: index_.IsOpen();
	else
		return file_db_.IsOpen() && index_.IsOpen();
}


bool Database::Open(const String& db_path, uint32 db_version, bool read_only)
{
	if (!file_db_.Open(db_path, read_only))
		return false;

	read_only_ = read_only;

	file_db_.GoToBegin();

	const uint32 db_header= 0xf74b694d;		// magic value for header

	uint32 header= 0;
	uint32 version= 0;
	if (!file_db_.ReadHeader(header, version) || header != db_header || version != db_version)
	{
		if (read_only)
		{
			file_db_.Close();
			return false;
		}

		file_db_.Close();
		::DeleteFile(db_path.c_str());
		::DeleteFile(GetIndexName(db_path).c_str());

		if (!file_db_.Open(db_path, read_only))
			return false;

		file_db_.GoToBegin();
		file_db_.WriteHeader(db_header, db_version);
	}

	if (use_index_ && !index_.Read(GetIndexName(db_path), read_only))
		return false;

	return true;
}


String Database::GetIndexName(const String& db_file_name)
{
	return db_file_name + _T(".ix");
}


// update record
void Database::Update(uint64 offset, const String& key, const std::vector<uint8>& record)
{
	// verification
	String existing;
	uint32 size= ReadKey(offset, existing);

	if (key != existing)	// has to have the same key
		throw 4;
	if (size < record.size())	// has to fit in the old place
		throw 5;

	file_db_.GoTo(offset);

	ASSERT(!record.empty());
	file_db_.Write(&record.front(), record.size());
}

// update record without the key
void Database::Update(uint64 offset, const std::vector<uint8>& record)
{
	const uint32 size= ReadRecordSize(offset);

	if (size < record.size())	// has to fit in the old place
		throw 5;

	file_db_.GoTo(offset);

	ASSERT(!record.empty());
	file_db_.Write(&record.front(), record.size());
}


// append new record
uint64 Database::Append(const String& key, const std::vector<uint8>& record)
{
	ASSERT(use_index_);

	uint64 offset= Append(record);

	if (!key.empty())
		index_.Insert(key, offset);
	//	index_.Flush();

	return offset;
}


uint64 Database::Append(const std::vector<uint8>& record)
{
	uint64 offset= file_db_.GoToEnd();
	ASSERT(offset < 0xffffffff);

	ASSERT(!record.empty());
	file_db_.Write(&record.front(), record.size());

	return offset;
}


// read record
void Database::Read(uint64 offset, std::vector<uint8>& record)
{
	file_db_.ReadRecord(offset, record);
}


// return offset to the next record (or 0)
uint64 Database::NextRecordOffset()
{
	return file_db_.Probe();
}


bool Database::ReadNext(std::vector<uint8>& record, uint64* offset/*= 0*/)
{
	if (uint64 pos= file_db_.Probe())
	{
//TRACE(L"next rec: %x\n", int(pos));
		file_db_.ReadRecord(pos, record);

		if (offset)
			*offset = pos;

		return true;
	}
	return false;
}


uint32 Database::ReadKey(uint64 offset, String& key)
{
	ASSERT(use_index_);
	return file_db_.ReadKeyAt(offset, key);
}


uint32 Database::ReadRecordSize(uint64 offset)
{
	return file_db_.ReadRecordSize(offset);
}


Database::Index::~Index()
{
	try
	{
		Flush();
	}
	catch (...)
	{}
}


// add new index preserving sort order
void Database::Index::Insert(const String& key, uint64 offset)
{
	ASSERT(offset < 0xffffffff);
	String found_key;
	insert(Find(key, found_key), offset);
	modified_ = true;
}


void Database::Index::Flush()
{
	if (modified_)
	{
		uint32 size= this->size() * sizeof(value_type);
		file_index_.SeekToBegin();
		file_index_.Write(reinterpret_cast<const char*>(&front()), size);
		file_index_.Flush();
		file_index_.SetLength(size);
		modified_ = false;
	}
}


bool Database::Index::IsOpen() const
{
	return file_index_.IsOpen();
//	return file_index_.file_ != CFile::file_null;
}


// read index in
bool Database::Index::Read(const String& file, bool read_only)
{
#ifdef _UNICODE
	// make sure index file is accessible from all user accounts
	SECURITY_DESCRIPTOR dacl;
	if (::InitializeSecurityDescriptor(&dacl, SECURITY_DESCRIPTOR_REVISION))
		::SetFileSecurity(file.c_str(), DACL_SECURITY_INFORMATION, &dacl);
	//	::SetNamedSecurityInfo(file.c_str(), SE_FILE_OBJECT, DACL_SECURITY_INFORMATION, 0, 0, &dacl, 0);
#endif

	UINT flags= 0;
	if (read_only)
		flags |= CFile::modeRead | CFile::shareDenyNone | CFile::typeBinary;
	else
		flags |= CFile::modeReadWrite | CFile::shareDenyWrite | CFile::modeCreate | CFile::modeNoTruncate | CFile::typeBinary;

	if (!file_index_.Open(file.c_str(), flags))
	{
#ifdef _DEBUG
		TCHAR text[200];
		wsprintf(text, L"cannot open index: %d %x", ::GetLastError(), ::GetLastError());
		::MessageBox(0, text, L"EP", MB_OK);
#endif
		return false;
	}

	DWORD len= static_cast<DWORD>(file_index_.GetLength());

	resize(len / sizeof(value_type));

	if (!empty())
		file_index_.Read(&front(), len);

	return true;
}


struct CmpKey	// comparing records with a key (for binary search)
{
	CmpKey(Database& db, String& key) : db_(&db), key_(&key)
	{}

	bool operator () (uint64 offset, const String& key) const
	{
		db_->ReadKey(offset, *key_);
		return key < *key_;
	}

	Database* db_;
	String* key_;
};


std::vector<uint64>::iterator Database::Index::Find(const String& key, String& key_found)
{
	CmpKey cmp(*db_, key_found);

	// find offset to the record
	return lower_bound(begin(), end(), key, cmp);
}


uint64 Database::FindRecord(const String& key, String& key_found)
{
	if (!IsOpen())
		throw String(_T("Image database is not opened"));

	std::vector<uint64>::iterator it= index_.Find(key, key_found);

	if (it != index_.end())
	{
		ReadKey(*it, key_found);
		return *it;
	}

	return 0;
}


void Database::Close()
{
	file_db_.Close();
	if (use_index_)
	{
		index_.Flush();
		index_.Close();
	}
}


void Database::Flush()
{
	if (use_index_)
		index_.Flush();
}


void Database::DeleteRecord(uint64 offset)
{
	file_db_.DeleteRecord(offset);
	index_.Remove(offset);
}


void Database::Index::Remove(uint64 offset)
{
	erase(find(begin(), end(), offset));
}


void Database::Lock()
{
	lock_.Lock();
}

void Database::Unlock()
{
	lock_.Unlock();
}

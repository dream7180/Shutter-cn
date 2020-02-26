/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// StorageFile.h: interface for the StorageFile class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_STORAGEFILE_H__1D8018A4_9BE6_4356_AFF8_4970A71D5665__INCLUDED_)
#define AFX_STORAGEFILE_H__1D8018A4_9BE6_4356_AFF8_4970A71D5665__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "StdFile.h"


class StorageFile
{
public:
	StorageFile();
	virtual ~StorageFile();

	bool Open(const String& db_path, bool read_only);

	void Close();

	void Write(const void* data, uint32 size);

	uint64 GoToEnd();
	void GoToBegin();
	void GoTo(uint64 offset);

	void ReadRecord(uint64 offset, std::vector<uint8>& record);
	// a key (as in database parlance) is a unique record identifier; here a string to be read
	uint32 ReadKeyAt(uint64 offset, String& key);
	uint32 ReadRecordSize(uint64 offset);

	void DeleteRecord(uint64 offset);

	bool IsOpen() const;

	// find out if there's a record at current file position
	uint64 Probe();

	bool ReadHeader(uint32& header, uint32& version);
	void WriteHeader(uint32 header, uint32 version);

private:
	// stdio replacement (buggy)
	//StdFile file_;
	CFile file_;

	enum { REC_ID= '!cer', REC_DEL= '!led' };
};

#endif // !defined(AFX_STORAGEFILE_H__1D8018A4_9BE6_4356_AFF8_4970A71D5665__INCLUDED_)

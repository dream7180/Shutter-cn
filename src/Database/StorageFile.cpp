/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

//
// StorageFile.cpp: implementation of the StorageFile: simple binary database
//


#include "stdafx.h"
#include "StorageFile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


StorageFile::StorageFile()
{
}

StorageFile::~StorageFile()
{
}

#include <aclapi.h>

// give read/write/deleta access to Everyone
bool set_access_rights(const TCHAR* path)
{
	bool done= false;

	PSID everyone= NULL;
	SID_IDENTIFIER_AUTHORITY SIDAuthWorld= SECURITY_WORLD_SID_AUTHORITY;
	SID_IDENTIFIER_AUTHORITY SIDAuthNT= SECURITY_NT_AUTHORITY;

	// Create a well-known SID for the Everyone group
	if (!AllocateAndInitializeSid(&SIDAuthWorld, 1, SECURITY_WORLD_RID, 0, 0, 0, 0, 0, 0, 0, &everyone))
	{
		printf("AllocateAndInitializeSid Error %u\n", GetLastError());
		goto Cleanup;
	}

	// Initialize an EXPLICIT_ACCESS structure for an ACE
	// The ACE will allow Everyone read access to the key
	EXPLICIT_ACCESS ea[2];
	ZeroMemory(&ea, 2 * sizeof(EXPLICIT_ACCESS));
	ea[0].grfAccessPermissions = GENERIC_WRITE | GENERIC_READ | DELETE | READ_CONTROL;
	ea[0].grfAccessMode = SET_ACCESS;
	ea[0].grfInheritance= NO_INHERITANCE;
	ea[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
	ea[0].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
	ea[0].Trustee.ptstrName = reinterpret_cast<LPTSTR>(everyone);

	// Create a SID for the BUILTIN\Administrators group
	PSID admin= 0;
	if (!AllocateAndInitializeSid(&SIDAuthNT, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &admin))
	{
		printf("AllocateAndInitializeSid Error %u\n", GetLastError());
		goto Cleanup;
	}

	// Initialize an EXPLICIT_ACCESS structure for an ACE
	// The ACE will allow the Administrators group full access to the file
	ea[1].grfAccessPermissions = KEY_ALL_ACCESS;
	ea[1].grfAccessMode = SET_ACCESS;
	ea[1].grfInheritance = NO_INHERITANCE;
	ea[1].Trustee.TrusteeForm = TRUSTEE_IS_SID;
	ea[1].Trustee.TrusteeType = TRUSTEE_IS_GROUP;
	ea[1].Trustee.ptstrName = reinterpret_cast<LPTSTR>(admin);

	// Create a new ACL that contains the new ACEs
	ACL* acl= 0;
	DWORD res= SetEntriesInAcl(2, ea, NULL, &acl);
	if (res != ERROR_SUCCESS)
	{
		printf("SetEntriesInAcl Error %u\n", GetLastError());
		goto Cleanup;
	}

	// Initialize a security descriptor
	SECURITY_DESCRIPTOR* sd= static_cast<SECURITY_DESCRIPTOR*>(LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH));
	if (sd == 0)
	{
		printf("LocalAlloc Error %u\n", GetLastError());
		goto Cleanup;
	}

	if (!InitializeSecurityDescriptor(sd, SECURITY_DESCRIPTOR_REVISION))
	{
		printf("InitializeSecurityDescriptor Error %u\n", GetLastError());
		goto Cleanup;
	}

	// Add the ACL to the security descriptor
	if (!SetSecurityDescriptorDacl(sd,
		TRUE,     // bDaclPresent flag
		acl,
		FALSE))   // not a default DACL
	{
		printf("SetSecurityDescriptorDacl Error %u\n", GetLastError());
		goto Cleanup;
	}

	// Initialize a security attributes structure
	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof (SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = sd;
	sa.bInheritHandle = FALSE;

	if (::SetFileSecurity(path, DACL_SECURITY_INFORMATION, sd))
		done = true;

Cleanup:

	if (everyone)
		FreeSid(everyone);
	if (admin)
		FreeSid(admin);
	if (acl)
		LocalFree(acl);
	if (sd)
		LocalFree(sd);

	return done;
}


bool StorageFile::Open(const String& db_path, bool read_only)
{
//#ifdef _UNICODE
//	// make sure db file is accessible from all user accounts
//	SECURITY_DESCRIPTOR dacl;
//	if (::InitializeSecurityDescriptor(&dacl, SECURITY_DESCRIPTOR_REVISION))
//		::SetFileSecurity(db_path.c_str(), DACL_SECURITY_INFORMATION, &dacl);
//	//	::SetNamedSecurityInfo(db_path.c_str(), SE_FILE_OBJECT, DACL_SECURITY_INFORMATION, 0, 0, &dacl, 0);
//#endif

	UINT flags= /*shareExclusive*/ /*| CFile::typeBinary*/ 0;

	if (read_only)
		flags |= CFile::modeRead | CFile::shareDenyNone | CFile::typeBinary;
	else
		flags |= CFile::shareDenyWrite | CFile::modeReadWrite | CFile::modeCreate | CFile::modeNoTruncate | CFile::typeBinary;

	if (!file_.Open(db_path.c_str(), flags))
	{
#ifdef _DEBUG
		TCHAR text[200];
		wsprintf(text, L"未能打开 db: %d %x", ::GetLastError(), ::GetLastError());
		::MessageBox(0, text, L"EP", MB_OK);
#endif
		return false;
	}

#ifdef _UNICODE
	// make sure db file is accessible from all user accounts
	bool rights_set= set_access_rights(db_path.c_str());
#endif

	return true;
}


void StorageFile::Close()
{
	file_.Close();
}


void StorageFile::Write(const void* data, uint32 size)
{
	uint32 id= REC_ID;
	file_.Write(&id, sizeof id);		// identifier
	file_.Write(&size, sizeof size);	// record's size
	if (size > 0)
		file_.Write(data, size);		// record's data

	// flushing, that was used to improve cache integrity, is *very* costly, so it's commented out
//	file_.Flush();
}


uint64 StorageFile::GoToEnd()
{
	return file_.SeekToEnd();
}


void StorageFile::GoToBegin()
{
	file_.SeekToBegin();
}


uint32 StorageFile::ReadKeyAt(uint64 offset, String& key)
{
	file_.Seek(static_cast<LONGLONG>(offset), CFile::begin);

	uint32 id= 0;
	file_.Read(&id, sizeof id);
	if (id != REC_ID)
		throw 3;

	uint32 size= 0;
	file_.Read(&size, sizeof size);

	uint32 len= 0;
	file_.Read(&len, sizeof len);

	key.resize(len);
	// illegal
	file_.Read(const_cast<TCHAR*>(key.data()), len * sizeof TCHAR);

	return size;
}


bool StorageFile::IsOpen() const
{
// for StdFile
//	return file_.IsOpen();
	return file_.m_hFile != CFile::hFileNull;
}


void StorageFile::ReadRecord(uint64 offset, std::vector<uint8>& record)
{
	file_.Seek(static_cast<LONGLONG>(offset), CFile::begin);

	uint32 id= 0;
	if (file_.Read(&id, sizeof id) != sizeof id || id != REC_ID)
		throw 1;

	uint32 size= 0;
	if (file_.Read(&size, sizeof size) != sizeof size)
		throw 1;

	record.resize(size);

	if (size > 0)
		if (file_.Read(&record.front(), size) != size)
			throw 1;
}


uint32 StorageFile::ReadRecordSize(uint64 offset)
{
	file_.Seek(static_cast<LONGLONG>(offset), CFile::begin);

	uint32 id= 0;
	if (file_.Read(&id, sizeof id) != sizeof id || id != REC_ID)
		throw 2;

	uint32 size= 0;
	if (file_.Read(&size, sizeof size) != sizeof size)
		throw 2;

	return size;
}


uint64 StorageFile::Probe()	// probing current position: is there a record there?
{
	ULONGLONG pos= file_.GetPosition();
	if (pos == 0)
		return 0;

	uint32 id= 0;
	if (file_.Read(&id, sizeof id) != sizeof id || id != REC_ID)
		return 0;

	return pos;
}


bool StorageFile::ReadHeader(uint32& header, uint32& version)
{
	if (file_.Read(&header, sizeof header) != sizeof header)
		return false;

	if (file_.Read(&version, sizeof version) != sizeof version)
		return false;

	return true;
}


void StorageFile::WriteHeader(uint32 header, uint32 version)
{
	file_.Write(&header, sizeof header);
	file_.Write(&version, sizeof version);
//test: file_.SetLength(0x7f000000);
}


void StorageFile::DeleteRecord(uint64 offset)
{
	file_.Seek(static_cast<LONGLONG>(offset), CFile::begin);

	uint32 id= 0;
	if (file_.Read(&id, sizeof id) != sizeof id || id != REC_ID)
		throw 2;

	file_.Seek(static_cast<LONGLONG>(offset), CFile::begin);

	id = REC_DEL;
	file_.Write(&id, sizeof id);
}


void StorageFile::GoTo(uint64 offset)
{
	file_.Seek(static_cast<LONGLONG>(offset), CFile::begin);
}

// =================================================================================================
// ADOBE SYSTEMS INCORPORATED
// Copyright 2010 Adobe Systems Incorporated
// All Rights Reserved
//
// NOTICE: Adobe permits you to use, modify, and distribute this file in accordance with the terms
// of the Adobe license agreement accompanying it.
// =================================================================================================

#include "public/include/XMP_Environment.h"	// ! This must be the first include.

#include "source/Host_IO.hpp"
#include "source/XMP_LibUtils.hpp"
#include "source/UnicodeConversions.hpp"

#if XMP_WinBuild
	#pragma warning ( disable : 4800 )	// forcing value to bool 'true' or 'false' (performance warning)
#endif

// =================================================================================================
// Host_IO implementations for Windows
// ===================================

#if ! XMP_WinBuild
	#error "This is the Windows implementation of Host_IO."
#endif

// =================================================================================================
// File operations
// =================================================================================================

// =================================================================================================
// Host_IO::Exists
// ===============

bool Host_IO::Exists ( const char* filePath )
{
	std::string wideName;
	const size_t utf8Len = strlen(filePath);
	const size_t maxLen = 2 * (utf8Len+1);

	wideName.reserve ( maxLen );
	wideName.assign ( maxLen, ' ' );
	int wideLen = MultiByteToWideChar ( CP_UTF8, 0, filePath, -1, (LPWSTR)wideName.data(), (int)maxLen );
	if ( wideLen == 0 ) return false;

	DWORD attrs = GetFileAttributes ( (LPCWSTR)wideName.data() );
	return ( attrs != INVALID_FILE_ATTRIBUTES);

}

// =================================================================================================
// Host_IO::Create
// ===============

bool Host_IO::Create ( const char* filePath )
{
	if ( Host_IO::Exists ( filePath ) ) {
		if ( Host_IO::GetFileMode ( filePath ) == kFMode_IsFile ) return false;
		XMP_Throw ( "Host_IO::Create, path exists but is not a file", kXMPErr_InternalFailure );
	}

	std::string wideName;
	const size_t utf8Len = strlen(filePath);
	const size_t maxLen = 2 * (utf8Len+1);

	wideName.reserve ( maxLen );
	wideName.assign ( maxLen, ' ' );
	int wideLen = MultiByteToWideChar ( CP_UTF8, 0, filePath, -1, (LPWSTR)wideName.data(), (int)maxLen );
	if ( wideLen == 0 ) XMP_Throw ( "Host_IO::Create, cannot convert path", kXMPErr_InternalFailure );;

	Host_IO::FileRef fileHandle;
	fileHandle = CreateFileW ( (LPCWSTR)wideName.data(), (GENERIC_READ | GENERIC_WRITE), 0, 0, CREATE_ALWAYS,
							   (FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS), 0 );
	if ( fileHandle == INVALID_HANDLE_VALUE ) XMP_Throw ( "Host_IO::Create, cannot create file", kXMPErr_InternalFailure );;

	CloseHandle ( fileHandle );
	return true;

}	// Host_IO::Create

// =================================================================================================
// FillXMPTime
// ===========

static void FillXMPTime ( const SYSTEMTIME & winTime, XMP_DateTime * xmpTime )
{

	// Ignore the fractional seconds for consistency with UNIX and to avoid false newness even on
	// Windows. Some other sources of time only resolve to seconds, we don't want 25.3 looking
	// newer than 25.

	xmpTime->year = winTime.wYear;
	xmpTime->month = winTime.wMonth;
	xmpTime->day = winTime.wDay;
	xmpTime->hasDate = true;

	xmpTime->hour = winTime.wHour;
	xmpTime->minute = winTime.wMinute;
	xmpTime->second = winTime.wSecond;
	xmpTime->nanoSecond = 0;	// See note above; winTime.wMilliseconds * 1000*1000;
	xmpTime->hasTime = true;

	xmpTime->tzSign = kXMP_TimeIsUTC;
	xmpTime->tzHour = 0;
	xmpTime->tzMinute = 0;
	xmpTime->hasTimeZone = true;

}

// =================================================================================================
// Host_IO::GetModifyDate
// ======================

bool Host_IO::GetModifyDate ( const char* filePath, XMP_DateTime * modifyDate )
{
	BOOL ok;
	Host_IO::FileRef fileHandle;
	
	try {	// Host_IO::Open should not throw - fix after CS6.
		fileHandle = Host_IO::Open ( filePath, Host_IO::openReadOnly );
		if ( fileHandle == Host_IO::noFileRef ) return false;
	} catch ( ... ) {
		return false;
	}
	
	FILETIME binTime;
	ok = GetFileTime ( fileHandle, 0, 0, &binTime );
	Host_IO::Close ( fileHandle );
	if ( ! ok ) return false;
	
	SYSTEMTIME utcTime;
	ok = FileTimeToSystemTime ( &binTime, &utcTime );
	if ( ! ok ) return false;
	
	FillXMPTime ( utcTime, modifyDate );
	return true;

}	// Host_IO::GetModifyDate

// =================================================================================================
// ConjureDerivedPath
// ==================

static std::string ConjureDerivedPath ( const char* basePath )
{
	std::string tempPath = basePath;
	tempPath += "._nn_";
	char * indexPart = (char*) tempPath.c_str() + strlen(basePath) + 2;

	for ( char ten = '0'; ten <= '9' ; ++ten ) {
		indexPart[0] = ten;
		for ( char one = '0'; one <= '9' ; ++one ) {
			indexPart[1] = one;
			if ( ! Host_IO::Exists ( tempPath.c_str() ) ) return tempPath;
		}
	}

	return "";

}	// ConjureDerivedPath

// =================================================================================================
// Host_IO::CreateTemp
// ===================

std::string Host_IO::CreateTemp ( const char* sourcePath )
{
	std::string tempPath = ConjureDerivedPath ( sourcePath );
	if ( tempPath.empty() ) XMP_Throw ( "Host_IO::CreateTemp, cannot create temp file path", kXMPErr_InternalFailure );
	XMP_Assert ( ! Host_IO::Exists ( tempPath.c_str() ) );

	Host_IO::Create ( tempPath.c_str() );
	return tempPath;

}	// Host_IO::CreateTemp

// =================================================================================================
// Host_IO::Open
// =============

Host_IO::FileRef Host_IO::Open ( const char* filePath, bool readOnly )
{
	if ( ! Host_IO::Exists ( filePath ) ) return Host_IO::noFileRef;

	DWORD access = GENERIC_READ;	// Assume read mode.
	DWORD share  = FILE_SHARE_READ;

	if ( ! readOnly ) {
		access |= GENERIC_WRITE;
		share = 0;
	}

	std::string wideName;
	const size_t utf8Len = strlen(filePath);
	const size_t maxLen = 2 * (utf8Len+1);

	wideName.reserve ( maxLen );
	wideName.assign ( maxLen, ' ' );
	int wideLen = MultiByteToWideChar ( CP_UTF8, 0, filePath, -1, (LPWSTR)wideName.data(), (int)maxLen );
	if ( wideLen == 0 ) XMP_Throw ( "Host_IO::Open, MultiByteToWideChar failure", kXMPErr_ExternalFailure );

	Host_IO::FileRef fileHandle;
	fileHandle = CreateFileW ( (LPCWSTR)wideName.data(), access, share, 0, OPEN_EXISTING,
							   (FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS), 0 );
	if ( fileHandle == INVALID_HANDLE_VALUE ) XMP_Throw ( "Host_IO::Open, CreateFileW failure", kXMPErr_ExternalFailure );

	return fileHandle;

}	// Host_IO::Open

// =================================================================================================
// Host_IO::Close
// ==============

void Host_IO::Close ( Host_IO::FileRef fileHandle )
{
	if ( fileHandle == Host_IO::noFileRef ) return;

	BOOL ok = CloseHandle ( fileHandle );
	if ( ! ok ) XMP_Throw ( "Host_IO::Close, CloseHandle failure", kXMPErr_ExternalFailure );

}	// Host_IO::Close

// =================================================================================================
// Host_IO::SwapData
// =================

void Host_IO::SwapData ( const char* sourcePath, const char* destPath )
{

	// For lack of a better approach, do a 3-way rename.

	std::string thirdPath = ConjureDerivedPath ( sourcePath );
	if ( thirdPath.empty() ) XMP_Throw ( "Cannot create temp file path", kXMPErr_InternalFailure );
	XMP_Assert ( ! Host_IO::Exists ( thirdPath.c_str() ) );

	Host_IO::Rename ( sourcePath, thirdPath.c_str() );
	
	try {
		Host_IO::Rename ( destPath, sourcePath );
	} catch ( ... ) {
		Host_IO::Rename ( thirdPath.c_str(), sourcePath );
		throw;
	}
	
	try {
		Host_IO::Rename ( thirdPath.c_str(), destPath );
	} catch ( ... ) {
		Host_IO::Rename ( sourcePath, destPath );
		Host_IO::Rename ( thirdPath.c_str(), sourcePath );
		throw;
	}

}	// Host_IO::SwapData

// =================================================================================================
// Host_IO::Rename
// ===============

void Host_IO::Rename ( const char* oldPath, const char* newPath )
{
	if ( Host_IO::Exists ( newPath ) ) XMP_Throw ( "Host_IO::Rename, new path exists", kXMPErr_InternalFailure );

	std::string wideOldPath, wideNewPath;
	size_t utf8Len = strlen(oldPath);
	if ( utf8Len < strlen(newPath) ) utf8Len = strlen(newPath);
	const size_t maxLen = 2 * (utf8Len+1);
	int wideLen;

	wideOldPath.reserve ( maxLen );
	wideOldPath.assign ( maxLen, ' ' );
	wideLen = MultiByteToWideChar ( CP_UTF8, 0, oldPath, -1, (LPWSTR)wideOldPath.data(), (int)maxLen );
	if ( wideLen == 0 ) XMP_Throw ( "Host_IO::Rename, MultiByteToWideChar failure", kXMPErr_ExternalFailure );

	wideNewPath.reserve ( maxLen );
	wideNewPath.assign ( maxLen, ' ' );
	wideLen = MultiByteToWideChar ( CP_UTF8, 0, newPath, -1, (LPWSTR)wideNewPath.data(), (int)maxLen );
	if ( wideLen == 0 ) XMP_Throw ( "Host_IO::Rename, MultiByteToWideChar failure", kXMPErr_ExternalFailure );

	BOOL ok = MoveFileW ( (LPCWSTR)wideOldPath.data(), (LPCWSTR)wideNewPath.data() );
	if ( ! ok ) XMP_Throw ( "Host_IO::Rename, MoveFileW failure", kXMPErr_ExternalFailure );

}	// Host_IO::Rename

// =================================================================================================
// Host_IO::Delete
// ===============

void Host_IO::Delete ( const char* filePath )
{
	if ( ! Host_IO::Exists ( filePath ) ) return;

	std::string wideName;
	const size_t utf8Len = strlen(filePath);
	const size_t maxLen = 2 * (utf8Len+1);

	wideName.reserve ( maxLen );
	wideName.assign ( maxLen, ' ' );
	int wideLen = MultiByteToWideChar ( CP_UTF8, 0, filePath, -1, (LPWSTR)wideName.data(), (int)maxLen );
	if ( wideLen == 0 ) XMP_Throw ( "Host_IO::Delete, MultiByteToWideChar failure", kXMPErr_ExternalFailure );

	BOOL ok = DeleteFileW ( (LPCWSTR)wideName.data() );
	if ( ! ok ) {
		DWORD errCode = GetLastError();
		if ( errCode != ERROR_FILE_NOT_FOUND ) {
			XMP_Throw ( "Host_IO::Delete, DeleteFileW failure", kXMPErr_ExternalFailure );
		}
	}

}	// Host_IO::Delete

// =================================================================================================
// Host_IO::Seek
// =============

XMP_Int64 Host_IO::Seek ( Host_IO::FileRef fileHandle, XMP_Int64 offset, SeekMode mode )
{
	DWORD method;
	switch ( mode ) {
		case kXMP_SeekFromStart :
			method = FILE_BEGIN;
			break;
		case kXMP_SeekFromCurrent :
			method = FILE_CURRENT;
			break;
		case kXMP_SeekFromEnd :
			method = FILE_END;
			break;
		default :
			XMP_Throw ( "Invalid seek mode", kXMPErr_InternalFailure );
			break;
	}

	LARGE_INTEGER seekOffset, newPos;
	seekOffset.QuadPart = offset;

	BOOL ok = SetFilePointerEx ( fileHandle, seekOffset, &newPos, method );
	if ( ! ok ) XMP_Throw ( "Host_IO::Seek, SetFilePointerEx failure", kXMPErr_ExternalFailure );

	return newPos.QuadPart;

}	// Host_IO::Seek

// =================================================================================================
// Host_IO::Read
// =============

#define TwoGB (XMP_Uns32)(2*1024*1024*1024UL)

XMP_Uns32 Host_IO::Read ( Host_IO::FileRef fileHandle, void * buffer, XMP_Uns32 count )
{
	if ( count >= TwoGB ) XMP_Throw ( "Host_IO::Read, request too large", kXMPErr_EnforceFailure );

	DWORD bytesRead;
	BOOL ok = ReadFile ( fileHandle, buffer, count, &bytesRead, 0 );
	if ( ! ok ) XMP_Throw ( "Host_IO::Read, ReadFile failure", kXMPErr_ExternalFailure );

	return bytesRead;

}	// Host_IO::Read

// =================================================================================================
// Host_IO::Write
// ==============

void Host_IO::Write ( Host_IO::FileRef fileHandle, const void * buffer, XMP_Uns32 count )
{
	if ( count >= TwoGB ) XMP_Throw ( "Host_IO::Write, request too large", kXMPErr_EnforceFailure );

	DWORD bytesWritten;
	BOOL ok = WriteFile ( fileHandle, buffer, count, &bytesWritten, 0 );
	if ( (! ok) || (bytesWritten != count) ) XMP_Throw ( "Host_IO::Write, WriteFile failure", kXMPErr_ExternalFailure );

}	// Host_IO::Write

// =================================================================================================
// Host_IO::Length
// ===============

XMP_Int64 Host_IO::Length ( Host_IO::FileRef fileHandle )
{
	LARGE_INTEGER length;
	BOOL ok = GetFileSizeEx ( fileHandle, &length );
	if ( ! ok ) XMP_Throw ( "Host_IO::Length, GetFileSizeEx failure", kXMPErr_ExternalFailure );

	return length.QuadPart;

}	// Host_IO::Length

// =================================================================================================
// Host_IO::SetEOF
// ===============

void Host_IO::SetEOF ( Host_IO::FileRef fileHandle, XMP_Int64 length )
{
	LARGE_INTEGER winLength;
	winLength.QuadPart = length;

	BOOL ok = SetFilePointerEx ( fileHandle, winLength, 0, FILE_BEGIN );
	if ( ! ok ) XMP_Throw ( "Host_IO::SetEOF, SetFilePointerEx failure", kXMPErr_ExternalFailure );
	ok = SetEndOfFile ( fileHandle );
	if ( ! ok ) XMP_Throw ( "Host_IO::SetEOF, SetEndOfFile failure", kXMPErr_ExternalFailure );

}	// Host_IO::SetEOF

// =================================================================================================
// Folder operations
// =================================================================================================

// =================================================================================================
// Host_IO::GetFileMode
// ====================

static DWORD kOtherAttrs = (FILE_ATTRIBUTE_DEVICE | FILE_ATTRIBUTE_OFFLINE);

Host_IO::FileMode Host_IO::GetFileMode ( const char * path )
{
	std::string utf16;	// GetFileAttributes wants native UTF-16.
	ToUTF16Native ( (UTF8Unit*)path, strlen(path), &utf16 );
	utf16.append ( 2, '\0' );	// Make sure there are at least 2 final zero bytes.

	// ! A shortcut is seen as a file, we would need extra code to recognize it and find the target.

	DWORD fileAttrs = GetFileAttributesW ( (LPCWSTR) utf16.c_str() );
	if ( fileAttrs == INVALID_FILE_ATTRIBUTES ) return kFMode_DoesNotExist;	// ! Any failure turns into does-not-exist.

	if ( fileAttrs & FILE_ATTRIBUTE_DIRECTORY ) return kFMode_IsFolder;
	if ( fileAttrs & kOtherAttrs ) return kFMode_IsOther;
	return kFMode_IsFile;

}	// Host_IO::GetFileMode

// =================================================================================================
// Host_IO::GetChildMode
// =====================

Host_IO::FileMode Host_IO::GetChildMode ( const char * parentPath, const char * childName )
{
	std::string fullPath = parentPath;
	fullPath += '\\';
	fullPath += childName;

	return GetFileMode ( fullPath.c_str() );

}	// Host_IO::GetChildMode

// =================================================================================================
// Host_IO::OpenFolder
// ===================

Host_IO::FolderRef Host_IO::OpenFolder ( const char* folderPath )
{

	switch ( Host_IO::GetFileMode ( folderPath ) ) {

		case Host_IO::kFMode_IsFolder :
		{
			WIN32_FIND_DATAW childInfo;
			std::string findPath = folderPath;
			// Looking for all children of that folder, add * as search criteria
			findPath += findPath[findPath.length() - 1] == '\\' ? "*" : "\\*";

			std::string utf16;	// FindFirstFile wants native UTF-16.
			ToUTF16Native ( (UTF8Unit*)findPath.c_str(), findPath.size(), &utf16 );
			utf16.append ( 2, '\0' );	// Make sure there are at least 2 final zero bytes.

			Host_IO::FolderRef folder = FindFirstFileW ( (LPCWSTR) utf16.c_str(), &childInfo );
			if ( folder == noFolderRef ) XMP_Throw ( "Host_IO::OpenFolder - FindFirstFileW failed", kXMPErr_ExternalFailure );
			// The first child should be ".", which we want to ignore anyway.
			XMP_Assert ( (folder == noFolderRef) || (childInfo.cFileName[0] == '.') );

			return folder;
		}

		case Host_IO::kFMode_DoesNotExist :
			return Host_IO::noFolderRef;

		default :
			XMP_Throw ( "Host_IO::OpenFolder, path is not a folder", kXMPErr_ExternalFailure );

	}

	XMP_Throw ( "Host_IO::OpenFolder, should not get here", kXMPErr_InternalFailure );

}	// Host_IO::OpenFolder

// =================================================================================================
// Host_IO::CloseFolder
// ====================

void Host_IO::CloseFolder ( Host_IO::FolderRef folder )
{
	if ( folder == noFolderRef ) return;

	BOOL ok = FindClose ( folder );
	if ( ! ok ) XMP_Throw ( "Host_IO::CloseFolder, FindClose failure", kXMPErr_ExternalFailure );

}	// Host_IO::CloseFolder

// =================================================================================================
// Host_IO::GetNextChild
// =====================

bool Host_IO::GetNextChild ( Host_IO::FolderRef folder, std::string* childName )
{
	bool found;
	WIN32_FIND_DATAW childInfo;

	if ( folder == Host_IO::noFolderRef ) return false;

	do {	// Ignore all children with names starting in '.'. This covers ., .., .DS_Store, etc.
		found = (bool) FindNextFile ( folder, &childInfo );
	} while ( found && (childInfo.cFileName[0] == '.') );
	if ( ! found ) return false;

	if ( childName != 0 ) {
		size_t len16 = 0;	// The cFileName field is native UTF-16.
		while ( childInfo.cFileName[len16] != 0 ) ++len16;
		FromUTF16Native ( (UTF16Unit*)childInfo.cFileName, len16, childName );
	}

	return true;

}	// Host_IO::GetNextChild

// =================================================================================================

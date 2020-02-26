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

#include <cstring>

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

// =================================================================================================
// Host_IO implementations for POSIX
// =================================

#if (! XMP_MacBuild) & (! XMP_UNIXBuild)
	#error "This is the POSIX implementation of Host_IO for Mac and general UNIX."
#endif

// =================================================================================================
// ======================================   File operations   ======================================
// =================================================================================================

// Make sure off_t is 64 bits and signed.
static char check_off_t_size [ (sizeof(off_t) == 8) ? 1 : -1 ];
// *** No std::numeric_limits?  static char check_off_t_sign [ std::numeric_limits<off_t>::is_signed ? -1 : 1 ];

// =================================================================================================
// Host_IO::Exists
// ===============

bool Host_IO::Exists ( const char* filePath )
{
	struct stat info;
	int err = stat ( filePath, &info );
	return (err == 0);
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

	mode_t mode = (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	int refNum = open ( filePath, (O_CREAT | O_EXCL | O_RDWR), mode );	// *** Include O_EXLOCK?
	if ( refNum == -1 ) XMP_Throw ( "Host_IO::Create, cannot create file", kXMPErr_InternalFailure );
	close ( refNum );
	return true;

}	// Host_IO::Create

// =================================================================================================
// ConvertPosixDateTime
// ====================

static void ConvertPosixDateTime ( const time_t & osTime, XMP_DateTime * xmpTime )
{

	struct tm posixUTC;
	gmtime_r ( &osTime, &posixUTC );

	xmpTime->year = posixUTC.tm_year + 1900;
	xmpTime->month = posixUTC.tm_mon + 1;
	xmpTime->day = posixUTC.tm_mday;
	xmpTime->hasDate = true;

	xmpTime->hour = posixUTC.tm_hour;
	xmpTime->minute = posixUTC.tm_min;
	xmpTime->second = posixUTC.tm_sec;
	xmpTime->nanoSecond = 0;	// The time_t resolution is only to seconds.
	xmpTime->hasTime = true;

	xmpTime->tzSign = kXMP_TimeIsUTC;
	xmpTime->tzHour = 0;
	xmpTime->tzMinute = 0;
	xmpTime->hasTimeZone = true;

}	// ConvertPosixDateTime

// =================================================================================================
// Host_IO::GetModifyDate
// ======================

bool Host_IO::GetModifyDate ( const char* filePath, XMP_DateTime* modifyDate )
{

	struct stat info;
	int err = stat ( filePath, &info );
	if ( err != 0 ) return false;
	if ( (! S_ISREG(info.st_mode)) && (! S_ISDIR(info.st_mode)) ) return false;
	
	if ( modifyDate != 0 ) ConvertPosixDateTime ( info.st_mtime, modifyDate );
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

	int flags = (readOnly ? O_RDONLY : O_RDWR);	// *** Include O_EXLOCK?

	int refNum = open ( filePath, flags, ( S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP ) );
	if ( refNum == -1 ) XMP_Throw ( "Host_IO::Open, open failure", kXMPErr_ExternalFailure );


	if ( ! readOnly ) {
		// A root user might be able to open a write-protected file w/o complaint.
		struct stat info;
		if ( fstat ( refNum, &info ) == -1 ) XMP_Throw ( "Host_IO::Open, fstat failed.", kXMPErr_ExternalFailure );
		if ( 0 == (info.st_mode & S_IWUSR) ) XMP_Throw ( "Host_IO::Open, file is write proected", kXMPErr_ExternalFailure );
	}

	return refNum;

}	// Host_IO::Open

// =================================================================================================
// Host_IO::Close
// ==============

void Host_IO::Close ( Host_IO::FileRef refNum )
{
	if ( refNum == Host_IO::noFileRef ) return;

	int err = close ( refNum );
	if ( err != 0 ) XMP_Throw ( "Host_IO::Close, close failure", kXMPErr_ExternalFailure );

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
	int err = rename ( oldPath, newPath );
	if ( err != 0 ) XMP_Throw ( "Host_IO::Rename, rename failure", kXMPErr_ExternalFailure );

}	// Host_IO::Rename

// =================================================================================================
// Host_IO::Delete
// ===============

void Host_IO::Delete ( const char* filePath )
{
	int err;

	switch ( Host_IO::GetFileMode ( filePath ) ) {

		case Host_IO::kFMode_DoesNotExist :
			return;

		case Host_IO::kFMode_IsFile :
			err = unlink ( filePath );
			if ( err != 0 ) XMP_Throw ( "Host_IO::Delete, unlink failure", kXMPErr_ExternalFailure );
			return;

		case Host_IO::kFMode_IsFolder :
			err = rmdir ( filePath );
			if ( err != 0 ) XMP_Throw ( "Host_IO::Delete, rmdir failure", kXMPErr_ExternalFailure );
			return;

		case Host_IO::kFMode_IsOther :
			XMP_Throw ( "Host_IO::Delete, can't delete 'other' file", kXMPErr_ExternalFailure );

	}

}	// Host_IO::Delete

// =================================================================================================
// Host_IO::Seek
// =============

XMP_Int64 Host_IO::Seek ( Host_IO::FileRef refNum, XMP_Int64 offset, SeekMode mode )
{
	int posMode;
	switch ( mode ) {
		case kXMP_SeekFromStart :
			posMode = SEEK_SET;
			break;
		case kXMP_SeekFromCurrent :
			posMode = SEEK_CUR;
			break;
		case kXMP_SeekFromEnd :
			posMode = SEEK_END;
			break;
		default :
			XMP_Throw ( "Host_IO::Seek, Invalid seek mode", kXMPErr_InternalFailure );
			break;
	}

	XMP_Int64 newPos = (XMP_Int64) lseek ( refNum, offset, mode );
	if ( newPos == -1 ) XMP_Throw ( "Host_IO::Seek, lseek failure", kXMPErr_ExternalFailure );

	return newPos;

}	// Host_IO::Seek

// =================================================================================================
// Host_IO::Read
// =============

#define TwoGB (XMP_Uns32)(2*1024*1024*1024UL)

XMP_Uns32 Host_IO::Read ( Host_IO::FileRef refNum, void * buffer, XMP_Uns32 count )
{
	if ( count >= TwoGB ) XMP_Throw ( "Host_IO::Read, request too large", kXMPErr_EnforceFailure );

	ssize_t bytesRead = read ( refNum, buffer, count );
	if ( bytesRead == -1 ) XMP_Throw ( "Host_IO::Read, read failure", kXMPErr_ExternalFailure );

	return bytesRead;

}	// Host_IO::Read

// =================================================================================================
// Host_IO::Write
// ==============

void Host_IO::Write ( Host_IO::FileRef refNum, const void * buffer, XMP_Uns32 count )
{
	if ( count >= TwoGB ) XMP_Throw ( "Host_IO::Write, request too large", kXMPErr_EnforceFailure );

	ssize_t bytesWritten = write ( refNum, buffer, count );
	if ( bytesWritten != count ) XMP_Throw ( "Host_IO::Write, write failure", kXMPErr_ExternalFailure );

}	// Host_IO::Write

// =================================================================================================
// Host_IO::Length
// ===============

XMP_Int64 Host_IO::Length ( Host_IO::FileRef refNum )
{
	off_t currPos = lseek ( refNum, 0, kXMP_SeekFromCurrent );
	off_t length  = lseek ( refNum, 0, kXMP_SeekFromEnd );
	if ( (currPos == -1) || (length == -1) ) XMP_Throw ( "Host_IO::Length, lseek failure", kXMPErr_ExternalFailure );
	(void) lseek ( refNum, currPos, kXMP_SeekFromStart );

	return length;

}	// Host_IO::Length

// =================================================================================================
// Host_IO::SetEOF
// ===============

void Host_IO::SetEOF ( Host_IO::FileRef refNum, XMP_Int64 length )
{
	int err = ftruncate ( refNum, length );
	if ( err != 0 ) XMP_Throw ( "Host_IO::SetEOF, ftruncate failure", kXMPErr_ExternalFailure );

}	// Host_IO::SetEOF

// =================================================================================================
// =====================================   Folder operations   =====================================
// =================================================================================================

// =================================================================================================
// Host_IO::GetFileMode
// ====================

Host_IO::FileMode Host_IO::GetFileMode ( const char * path )
{
	struct stat fileInfo;

	int err = stat ( path, &fileInfo );
	if ( err != 0 ) return kFMode_DoesNotExist;	// ! Any failure turns into does-not-exist.

	// ! The target of a symlink is properly recognized, not the symlink itself. A Mac alias is
	// ! seen as a file, we would need extra code to recognize it and find the target.

	if ( S_ISREG ( fileInfo.st_mode ) ) return kFMode_IsFile;
	if ( S_ISDIR ( fileInfo.st_mode ) ) return kFMode_IsFolder;
	return kFMode_IsOther;

}	// Host_IO::GetFileMode

// =================================================================================================
// Host_IO::GetChildMode
// =====================

Host_IO::FileMode Host_IO::GetChildMode ( const char * parentPath, const char * childName )
{
	std::string fullPath = parentPath;
	fullPath += '/';
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
			Host_IO::FolderRef folder = opendir ( folderPath );
			if ( folder == noFolderRef ) XMP_Throw ( "Host_IO::OpenFolder, opendir failed", kXMPErr_ExternalFailure );
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

	int err = closedir ( folder );
	if ( err != 0 ) XMP_Throw ( "Host_IO::CloseFolder, closedir failed", kXMPErr_ExternalFailure );

}	// Host_IO::CloseFolder

// =================================================================================================
// Host_IO::GetNextChild
// =====================

bool Host_IO::GetNextChild ( Host_IO::FolderRef folder, std::string* childName )
{
	struct dirent childInfo;
	struct dirent * result;

	if ( folder == Host_IO::noFolderRef ) return false;

	while ( true ) {
		// Ignore all children with names starting in '.'. This covers ., .., .DS_Store, etc.
		// ! On AIX readdir_r returns 9 instead of 0 for normal termination.
		int err = readdir_r ( folder, &childInfo, &result );	// ! Use the thread-dafe form.
		if ( err == 9 ) return false;	// Tolerable should some other UNIX return 9.
		if ( err != 0 ) XMP_Throw ( "Host_IO::GetNextChild, readdir_r failed", kXMPErr_ExternalFailure );
		if ( result == 0 ) return false;
		if ( childInfo.d_name[0] != '.' ) break;
	}

	if ( childName != 0 ) *childName = childInfo.d_name;
	return true;

}	// Host_IO::GetNextChild

// =================================================================================================

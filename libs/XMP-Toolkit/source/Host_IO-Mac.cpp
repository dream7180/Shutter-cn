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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

// =================================================================================================
// Host_IO implementations for Macintosh
// =====================================

#if ! XMP_MacBuild
	#error "This is the Mac implementation of Host_IO."
#endif

// =================================================================================================
// File operations
// =================================================================================================

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

	SInt8 perm = ( readOnly ? fsRdPerm : fsRdWrPerm );
	OSErr err;

	HFSUniStr255 forkName;
	err = FSGetDataForkName ( &forkName );
	if ( err != noErr ) XMP_Throw ( "Host_IO::Open, FSGetDataForkName failure", kXMPErr_ExternalFailure );

	FSRef fileRef;
	err = FSPathMakeRef ( (XMP_Uns8*)filePath, &fileRef, 0 );
	if ( err != noErr ) XMP_Throw ( "Host_IO::Open, FSPathMakeRef failure", kXMPErr_ExternalFailure );

	FSIORefNum refNum;
	err = FSOpenFork ( &fileRef, forkName.length, forkName.unicode, perm, &refNum );
	if ( err != noErr ) XMP_Throw ( "Host_IO::Open, FSOpenFork failure", kXMPErr_ExternalFailure );

	return refNum;

}	// Host_IO::Open

// =================================================================================================
// Host_IO::Close
// ==============

void Host_IO::Close ( Host_IO::FileRef refNum )
{
	if ( refNum == Host_IO::noFileRef ) return;

	OSErr err = FSCloseFork ( refNum );
	if ( err != noErr ) XMP_Throw ( "Host_IO::Close, FSCloseFork failure", kXMPErr_ExternalFailure );

}	// Host_IO::Close

// =================================================================================================
// CopyOtherForks
// ==============
//
// Support for SwapData to workaround the way FSExchangeObjects works. All that SwapData wants to do
// is swap the data fork contents between the original and temp files. But FSExchangeObjects swaps
// the entire file system object. SwapData calls CopyOtherForks before FSExchangeObjects to get all
// of the other forks into the temp file.

static void CopyOtherForks ( FSRef & sourceFSRef, FSRef & destFSRef )
{
	OSErr err;

	CatPositionRec catPos;
	HFSUniStr255 dataName, forkName;
	SInt64 forkSize;

	XMP_Uns8 buffer [64*1024];
	ByteCount bytesRead;

	err = FSGetDataForkName ( &dataName );
	if ( err != noErr ) XMP_Throw ( "Host_IO::CopyOtherForks, FSGetDataForkName failure", kXMPErr_ExternalFailure );
	size_t dataNameBytes = dataName.length * sizeof(UniChar);

	catPos.initialize = 0;
	while ( true ) {

		err = FSIterateForks ( &sourceFSRef, &catPos, &forkName, &forkSize, 0 );
		if ( (err != noErr) || (forkName.length == 0) ) break;
		if ( forkSize == 0 ) continue;
		if ( (forkName.length == dataName.length) &&
			 (memcmp ( &forkName.unicode[0], &dataName.unicode[0], dataNameBytes ) == 0) ) continue;

		err = FSCreateFork ( &destFSRef, forkName.length, forkName.unicode );
		if ( (err != noErr) && (err != errFSForkExists) ) continue;

		FSIORefNum sourceRefNum, destRefNum;
		err = FSOpenFork ( &sourceFSRef, forkName.length, forkName.unicode, fsRdPerm, &sourceRefNum );
		if ( err == noErr ) err = FSOpenFork ( &destFSRef, forkName.length, forkName.unicode, fsWrPerm, &destRefNum );
		if ( err != noErr ) continue;

		err = FSSetForkPosition ( sourceRefNum, fsFromStart, 0 );	// Sanity in case already open.
		if ( err == noErr ) err = FSSetForkPosition ( destRefNum, fsFromStart, 0 );
		if ( err == noErr ) err = FSSetForkSize ( destRefNum, fsFromStart, 0 );
		if ( err != noErr ) continue;

		while ( true ) {
			err = FSReadFork ( sourceRefNum, fsAtMark, 0, sizeof(buffer), buffer, &bytesRead );
			if ( (bytesRead == 0) || ((err != noErr) && (err != eofErr)) ) break;
			err = FSWriteFork ( destRefNum, fsAtMark, 0, bytesRead, buffer, 0 );
			if ( err != noErr ) break;
		}

	}

}	// CopyOtherForks

// =================================================================================================
// Host_IO::SwapData
// =================

void Host_IO::SwapData ( const char* sourcePath, const char* destPath )
{
	OSErr err;
	FSRef sourceRef, destRef;

	err = FSPathMakeRef ( (XMP_Uns8*)sourcePath, &sourceRef, 0 );
	if ( err == noErr ) err = FSPathMakeRef ( (XMP_Uns8*)destPath, &destRef, 0 );
	if ( err != noErr ) XMP_Throw ( "Host_IO::SwapData, FSPathMakeRef failure", kXMPErr_ExternalFailure );

	CopyOtherForks ( sourceRef, destRef );	// Would be nice to swap just the data fork.

	err = FSExchangeObjects ( &sourceRef, &destRef );
	if ( err == noErr ) return;
	
	// FSExchangeObjects will fail sometimes for remote volumes. Try a 3-way rename.

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
	int err = rename ( oldPath, newPath );	// *** Better to use an FS function?
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
	UInt16 posMode;
	switch ( mode ) {
		case kXMP_SeekFromStart :
			posMode = fsFromStart;
			break;
		case kXMP_SeekFromCurrent :
			posMode = fsFromMark;
			break;
		case kXMP_SeekFromEnd :
			posMode = fsFromLEOF;
			break;
		default :
			XMP_Throw ( "Host_IO::Seek, Invalid seek mode", kXMPErr_InternalFailure );
			break;
	}

	OSErr err;
	XMP_Int64 newPos;

	err = FSSetForkPosition ( refNum, posMode, offset );
	if ( err != noErr ) XMP_Throw ( "Host_IO::Seek, FSSetForkPosition failure", kXMPErr_ExternalFailure );

	err = FSGetForkPosition ( refNum, &newPos );
	if ( err != noErr ) XMP_Throw ( "Host_IO::Seek, FSGetForkPosition failure", kXMPErr_ExternalFailure );

	return newPos;

}	// Host_IO::Seek

// =================================================================================================
// Host_IO::Read
// =============

#define TwoGB (XMP_Uns32)(2*1024*1024*1024UL)

XMP_Uns32 Host_IO::Read ( Host_IO::FileRef refNum, void * buffer, XMP_Uns32 count )
{
	if ( count >= TwoGB ) XMP_Throw ( "Host_IO::Read, request too large", kXMPErr_EnforceFailure );

	ByteCount bytesRead;
	OSErr err = FSReadFork ( refNum, fsAtMark, 0, count, buffer, &bytesRead );
	if ( (err != noErr) && (err != eofErr) ) {
		// ! FSReadFork returns eofErr for a normal encounter with the end of file.
		XMP_Throw_Verbose ( "Host_IO::Read, FSReadFork failure",err, kXMPErr_ExternalFailure );
	}

	return bytesRead;

}	// Host_IO::Read

// =================================================================================================
// Host_IO::Write
// ==============

void Host_IO::Write ( Host_IO::FileRef refNum, const void * buffer, XMP_Uns32 count )
{
	if ( count >= TwoGB ) XMP_Throw ( "Host_IO::Write, request too large", kXMPErr_EnforceFailure );

	ByteCount bytesWritten;
	OSErr err = FSWriteFork ( refNum, fsAtMark, 0, count, buffer, &bytesWritten );
	if ( (err != noErr) | (bytesWritten != (ByteCount)count) ) XMP_Throw_Verbose ( "Host_IO::Write, FSWriteFork failure",err, kXMPErr_ExternalFailure );

}	// Host_IO::Write

// =================================================================================================
// Host_IO::Length
// ===============

XMP_Int64 Host_IO::Length ( Host_IO::FileRef refNum )
{
	XMP_Int64 length;
	OSErr err = FSGetForkSize ( refNum, &length );
	if ( err != noErr ) XMP_Throw ( "Host_IO::Length, FSGetForkSize failure", kXMPErr_ExternalFailure );

	return length;

}	// Host_IO::Length

// =================================================================================================
// Host_IO::SetEOF
// ===============

void Host_IO::SetEOF ( Host_IO::FileRef refNum, XMP_Int64 length )
{
	OSErr err = FSSetForkSize ( refNum, fsFromStart, length );
	if ( err != noErr ) XMP_Throw ( "Host_IO::SetEOF, FSSetForkSize failure", kXMPErr_ExternalFailure );

}	// Host_IO::SetEOF

// =================================================================================================
// Folder operations, both Mac and UNIX use POSIX services.
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
		int err = readdir_r ( folder, &childInfo, &result );	// ! Use the thread-dafe form.
		if ( err != 0 ) XMP_Throw ( "Host_IO::GetNextChild, readdir_r failed", kXMPErr_ExternalFailure );
		if ( result == 0 ) return false;
		if ( childInfo.d_name[0] != '.' ) break;
	}

	if ( childName != 0 ) *childName = childInfo.d_name;
	return true;

}	// Host_IO::GetNextChild

// =================================================================================================

// =================================================================================================
// ADOBE SYSTEMS INCORPORATED
// Copyright 2010 Adobe Systems Incorporated
// All Rights Reserved
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in accordance with the terms
// of the Adobe license agreement accompanying it.
// =================================================================================================

#include "public/include/XMP_Environment.h"	// ! XMP_Environment.h must be the first included header.

#include "public/include/XMP_Const.h"
#include "public/include/XMP_IO.hpp"

#include "source/XMP_LibUtils.hpp"
#include "source/XMPFiles_IO.hpp"
#include "source/XIO.hpp"

// =================================================================================================
// XMPFiles_IO::New_XMPFiles_IO
// ============================

XMPFiles_IO* XMPFiles_IO::New_XMPFiles_IO ( const char* filePath, bool readOnly )
{
	Host_IO::FileRef hostFile = Host_IO::noFileRef;
	
	switch ( Host_IO::GetFileMode ( filePath ) ) {
		case Host_IO::kFMode_IsFile:
			hostFile = Host_IO::Open ( filePath, readOnly );
			break;
		case Host_IO::kFMode_DoesNotExist:
			break;
		default:
			XMP_Throw ( "New_XMPFiles_IO, path must be a file or not exist", kXMPErr_BadParam );
	}
	if ( hostFile == Host_IO::noFileRef ) return 0;

	Host_IO::Rewind ( hostFile );	// Make sure offset really is 0.

	XMPFiles_IO* newFile = new XMPFiles_IO ( hostFile, filePath, readOnly );
	return newFile;

}	// XMPFiles_IO::New_XMPFiles_IO

// =================================================================================================
// XMPFiles_IO::XMPFiles_IO
// ========================

XMPFiles_IO::XMPFiles_IO ( Host_IO::FileRef hostFile, const char* _filePath, bool _readOnly )
	: readOnly(_readOnly), filePath(_filePath), fileRef(hostFile), currOffset(0), isTemp(false), derivedTemp(0)
{
	XMP_Assert ( this->fileRef != Host_IO::noFileRef );

	this->currLength = Host_IO::Length ( this->fileRef );

}	// XMPFiles_IO::XMPFiles_IO

// =================================================================================================
// XMPFiles_IO::~XMPFiles_IO
// =========================

XMPFiles_IO::~XMPFiles_IO()
{

	try {
		if ( this->derivedTemp != 0 ) this->DeleteTemp();
		if ( this->fileRef != Host_IO::noFileRef ) Host_IO::Close ( this->fileRef );
		if ( this->isTemp && (! this->filePath.empty()) ) Host_IO::Delete ( this->filePath.c_str() );
	} catch ( ... ) {
		// All of the above is fail-safe cleanup, ignore problems.
	}

}	// XMPFiles_IO::~XMPFiles_IO

// =================================================================================================
// XMPFiles_IO::operator=
// ======================

void XMPFiles_IO::operator= ( const XMP_IO& in )
{

	XMP_Throw ( "No assignment for XMPFiles_IO", kXMPErr_InternalFailure );

};	// XMPFiles_IO::operator=

// =================================================================================================
// XMPFiles_IO::Read
// =================

XMP_Uns32 XMPFiles_IO::Read ( void* buffer, XMP_Uns32 count, bool readAll /* = false */ )
{
	XMP_Assert ( this->fileRef != Host_IO::noFileRef );
	XMP_Assert ( this->currOffset == Host_IO::Offset ( this->fileRef ) );
	XMP_Assert ( this->currLength == Host_IO::Length ( this->fileRef ) );
	XMP_Assert ( this->currOffset <= this->currLength );

	if ( count > (this->currLength - this->currOffset) ) {
		if ( readAll ) XMP_Throw ( "XMPFiles_IO::Read, not enough data", kXMPErr_EnforceFailure );
		count = (XMP_Uns32) (this->currLength - this->currOffset);
	}

	XMP_Uns32 amountRead = Host_IO::Read ( this->fileRef, buffer, count );
	XMP_Enforce ( amountRead == count );

	this->currOffset += amountRead;
	return amountRead;

}	// XMPFiles_IO::Read

// =================================================================================================
// XMPFiles_IO::Write
// ==================

void XMPFiles_IO::Write ( const void* buffer, XMP_Uns32 count )
{
	XMP_Assert ( this->fileRef != Host_IO::noFileRef );
	XMP_Assert ( this->currOffset == Host_IO::Offset ( this->fileRef ) );
	XMP_Assert ( this->currLength == Host_IO::Length ( this->fileRef ) );
	XMP_Assert ( this->currOffset <= this->currLength );

	try {
		Host_IO::Write ( this->fileRef, buffer, count );
	} catch ( ... ) {
		// Make sure the internal state reflects partial writes.
		this->currOffset = Host_IO::Offset ( this->fileRef );
		this->currLength = Host_IO::Length ( this->fileRef );
		throw;
	}

	this->currOffset += count;
	if ( this->currOffset > this->currLength ) this->currLength = this->currOffset;

}	// XMPFiles_IO::Write

// =================================================================================================
// XMPFiles_IO::Seek
// =================

XMP_Int64 XMPFiles_IO::Seek ( XMP_Int64 offset, SeekMode mode )
{
	XMP_Assert ( this->fileRef != Host_IO::noFileRef );
	XMP_Assert ( this->currOffset == Host_IO::Offset ( this->fileRef ) );
	XMP_Assert ( this->currLength == Host_IO::Length ( this->fileRef ) );

	XMP_Int64 newOffset = offset;
	if ( mode == kXMP_SeekFromCurrent ) {
		newOffset += this->currOffset;
	} else if ( mode == kXMP_SeekFromEnd ) {
		newOffset += this->currLength;
	}
	XMP_Enforce ( newOffset >= 0 );

	if ( newOffset <= this->currLength ) {
		this->currOffset = Host_IO::Seek ( this->fileRef, offset, mode );
	} else if ( this->readOnly ) {
		XMP_Throw ( "XMPFiles_IO::Seek, read-only seek beyond EOF", kXMPErr_EnforceFailure );
	} else {
		Host_IO::SetEOF ( this->fileRef, newOffset );	// Extend a file open for writing.
		this->currLength = newOffset;
		this->currOffset = Host_IO::Seek ( this->fileRef, 0, kXMP_SeekFromEnd );
	}

	XMP_Assert ( this->currOffset == newOffset );
	return this->currOffset;

}	// XMPFiles_IO::Seek

// =================================================================================================
// XMPFiles_IO::Length
// ===================

XMP_Int64 XMPFiles_IO::Length()
{
	XMP_Assert ( this->fileRef != Host_IO::noFileRef );
	XMP_Assert ( this->currOffset == Host_IO::Offset ( this->fileRef ) );
	XMP_Assert ( this->currLength == Host_IO::Length ( this->fileRef ) );

	return this->currLength;

}	// XMPFiles_IO::Length

// =================================================================================================
// XMPFiles_IO::Truncate
// =====================

void XMPFiles_IO::Truncate ( XMP_Int64 length )
{
	XMP_Assert ( this->fileRef != Host_IO::noFileRef );
	XMP_Assert ( this->currOffset == Host_IO::Offset ( this->fileRef ) );
	XMP_Assert ( this->currLength == Host_IO::Length ( this->fileRef ) );

	XMP_Enforce ( length <= this->currLength );
	Host_IO::SetEOF ( this->fileRef, length );

	this->currLength = length;
	if ( this->currOffset > this->currLength ) this->currOffset = this->currLength;

	// ! Seek to the expected offset, some versions of Host_IO::SetEOF implicitly seek to EOF.
	Host_IO::Seek ( this->fileRef, this->currOffset, kXMP_SeekFromStart );
	XMP_Assert ( this->currOffset == Host_IO::Offset ( this->fileRef ) );

}	// XMPFiles_IO::Truncate

// =================================================================================================
// XMPFiles_IO::DeriveTemp
// =======================

XMP_IO* XMPFiles_IO::DeriveTemp()
{
	XMP_Assert ( this->fileRef != Host_IO::noFileRef );

	if ( this->derivedTemp != 0 ) return this->derivedTemp;

	if ( this->readOnly ) {
		XMP_Throw ( "XMPFiles_IO::DeriveTemp, can't derive from read-only", kXMPErr_InternalFailure );
	}

	std::string tempPath = Host_IO::CreateTemp ( this->filePath.c_str() );

	XMPFiles_IO* newTemp = XMPFiles_IO::New_XMPFiles_IO ( tempPath.c_str(), Host_IO::openReadWrite );
	if ( newTemp == 0 ) {
		Host_IO::Delete ( tempPath.c_str() );
		XMP_Throw ( "XMPFiles_IO::DeriveTemp, can't open temp file", kXMPErr_InternalFailure );
	}

	newTemp->isTemp = true;
	this->derivedTemp = newTemp;

	return this->derivedTemp;

}	// XMPFiles_IO::DeriveTemp

// =================================================================================================
// XMPFiles_IO::AbsorbTemp
// =======================

void XMPFiles_IO::AbsorbTemp()
{
	XMP_Assert ( this->fileRef != Host_IO::noFileRef );

	XMPFiles_IO* temp = this->derivedTemp;
	if ( temp == 0 ) {
		XMP_Throw ( "XMPFiles_IO::AbsorbTemp, no temp to absorb", kXMPErr_InternalFailure );
	}
	XMP_Assert ( temp->isTemp );

	this->Close();
	temp->Close();

	Host_IO::SwapData ( this->filePath.c_str(), temp->filePath.c_str() );
	this->DeleteTemp();

	this->fileRef = Host_IO::Open ( this->filePath.c_str(), Host_IO::openReadWrite );
	this->currLength = Host_IO::Length ( this->fileRef );
	this->currOffset = 0;

}	// XMPFiles_IO::AbsorbTemp

// =================================================================================================
// XMPFiles_IO::DeleteTemp
// =======================

void XMPFiles_IO::DeleteTemp()
{
	XMPFiles_IO* temp = this->derivedTemp;

	if ( temp != 0 ) {

		if ( temp->fileRef != Host_IO::noFileRef ) {
			Host_IO::Close ( temp->fileRef );
			temp->fileRef = Host_IO::noFileRef;
		}

		if ( ! temp->filePath.empty() ) {
			Host_IO::Delete ( temp->filePath.c_str() );
			temp->filePath.erase();
		}

		delete temp;
		this->derivedTemp = 0;

	}

}	// XMPFiles_IO::DeleteTemp

// =================================================================================================
// XMPFiles_IO::Close
// ==================

void XMPFiles_IO::Close()
{

	if ( this->fileRef != Host_IO::noFileRef ) {
		Host_IO::Close ( this->fileRef );
		this->fileRef = Host_IO::noFileRef;
	}

}	// XMPFiles_IO::Close

// =================================================================================================

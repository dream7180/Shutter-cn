#ifndef __XMPFiles_IO_hpp__
#define __XMPFiles_IO_hpp__ 1

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

#include "source/Host_IO.hpp"

#include <string>

// =================================================================================================

class XMPFiles_IO : public XMP_IO
{
public:

	// Implementation class for I/O inside XMPFiles, uses host O/S file services. All of the common
	// functions behave as described for XMP_IO. Use openReadOnly and openReadWrite constants from
	// Host_IO for the readOnly parameter to the constructors.

	static XMPFiles_IO* New_XMPFiles_IO ( const char* filePath, bool readOnly );

	XMPFiles_IO ( Host_IO::FileRef hostFile, const char* filePath, bool readOnly );

	XMP_Uns32 Read ( void* buffer, XMP_Uns32 count, bool readAll = false );
	void Write ( const void* buffer, XMP_Uns32 count );

	XMP_Int64 Seek ( XMP_Int64 offset, SeekMode mode );

	XMP_Int64 Length();
	void Truncate ( XMP_Int64 length );

	XMP_IO* DeriveTemp();
	void AbsorbTemp();
	void DeleteTemp();

	void Close();	// Not part of XMP_IO, added here to let errors propagate.

	virtual ~XMPFiles_IO();

private:

	bool readOnly;
	std::string filePath;
	Host_IO::FileRef fileRef;
	XMP_Int64 currOffset, currLength;

	bool isTemp;
	XMPFiles_IO* derivedTemp;

	XMPFiles_IO() : fileRef(Host_IO::noFileRef), isTemp(false), derivedTemp(0) {};	// Hidden on purpose.

	// The copy constructor and assignment operators are private to prevent client use. Allowing
	// them would require shared I/O state between XMPFiles_IO objects.
	XMPFiles_IO ( const XMPFiles_IO & original );
	void operator= ( const XMP_IO& in );
	void operator= ( const XMPFiles_IO& in );

};	// XMPFiles_IO

// =================================================================================================

#endif	// __XMPFiles_IO_hpp__

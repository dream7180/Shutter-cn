/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// PhotoAttrAccess.cpp: implementation of the PhotoAttrAccess class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PhotoAttrAccess.h"
#include "PhotoInfo.h"
#include "Markers.h"
#include "ReadMarker.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////

namespace {
	const int g_MARKER= 4;		// 2 + 2
	const int g_DESC_SIZE= g_MARKER + sizeof(PhotoAttr);
	const int g_BYTES_TO_ADD= g_DESC_SIZE;
	const int g_APP_MARKER_SIZE= g_BYTES_TO_ADD - 2;
}


uint32 PhotoAttrAccess::GetAppMarkerSize()
{
	return g_APP_MARKER_SIZE;
}


bool PhotoAttrAccess::Open(bool silent)
{
	if (!photo_.CreateWriteView(path_.c_str(), 0))
	{
		if (!silent)
		{
			String msg= _T("Photograph '") + path_ + _T("'\ncannot be opened for writing.");
			parent_->MessageBox(msg.c_str(), 0, MB_OK);
		}
		return false;
	}

	data_.Reset(photo_.GetBaseAddrChr(), photo_.GetFileSize(), 0);

	if (ReadMarker(data_) != MARK_SOI)		// not a JPEG image?
		return false;

	uint16 marker= 0;
	int before_marker_pos= 0;

	for (;;)
	{
		before_marker_pos = data_.GetPos();

		marker = ReadMarker(data_);

		if (marker < MARK_APP0 || marker >= MARK_APP6)
			break;

		if ((marker < MARK_APP0 || marker > MARK_APP15))// && marker != MARK_COM)
			return false;

		uint16 len= data_.GetUInt16();
		if (len < 2)
			return false;		// bogus data--wrong size

		data_ += len - 2;		// skip marker data
	}

	if (marker != MARK_APP6)				// not an app6 marker?
	{
		data_.SetPos(before_marker_pos);

		int offset= data_.GetPos();

		// insert app6 marker and space for description

		if (!photo_.CreateWriteView(path_.c_str(), g_BYTES_TO_ADD))
		{
//			string msg= "Cannot open photograph\n'" + inf.path_ + "'\nfor writing.";
//			parent_->MessageBox(msg.c_str(), 0, MB_OK);
			return false;
		}

		data_.Reset(photo_.GetBaseAddrChr(), photo_.GetFileSize(), offset);

		// move file contents to make space
		int length= photo_.GetFileSize() - offset;
		memmove(data_.GetPtr() + g_BYTES_TO_ADD, data_.GetPtr(), length);

		// add app6 marker
		data_.PutUInt16(MARK_APP6);
		data_.PutUInt16(g_APP_MARKER_SIZE);

		PhotoAttr pa;
		// copy structure
		memcpy(data_.GetPtr(), &pa, sizeof pa);

		data_.Reset(photo_.GetBaseAddrChr(), photo_.GetFileSize(), offset);
		marker = data_.GetUInt16();
	}

	uint16 data_size= data_.GetUInt16();
	if (data_size < g_APP_MARKER_SIZE)
		return false;

	PhotoAttr* attr= reinterpret_cast<PhotoAttr*>(data_.GetPtr());

	if (!attr->IsValid())
		return false;

	return true;
}


void PhotoAttrAccess::Touch()
{
	photo_.Touch();
}

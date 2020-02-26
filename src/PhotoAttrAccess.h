/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// PhotoAttrAccess.h: interface for the PhotoAttrAccess class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PHOTOATTRACCESS_H__7C94D244_5AC2_4A1F_9AFC_1D118E1D10C8__INCLUDED_)
#define AFX_PHOTOATTRACCESS_H__7C94D244_5AC2_4A1F_9AFC_1D118E1D10C8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "PhotoAttr.h"
#include "MemMappedFile.h"
#include "MemPointer.h"
class PhotoInfo;


// PhotoAttr access object: writing (modifying) photo's attrib (description and flags)
//
class PhotoAttrAccess
{
public:
	PhotoAttrAccess(const String& path, CWnd* parent) : path_(path), parent_(parent)
	{}
	PhotoAttrAccess(const TCHAR* path, CWnd* parent) : path_(path), parent_(parent)
	{}

	bool Open(bool silent);
	void Close()				{ photo_.CloseFile(); data_.Reset(0, 0, 0); }

	PhotoAttr* GetAttribPtr()	{ return reinterpret_cast<PhotoAttr*>(data_.GetPtr()); }

	static uint32 GetAppMarkerSize();

	void Touch();

private:
	String path_;
	MemMappedFile photo_;
	MemPointer data_;
	CWnd* parent_;
};


#endif // !defined(AFX_PHOTOATTRACCESS_H__7C94D244_5AC2_4A1F_9AFC_1D118E1D10C8__INCLUDED_)

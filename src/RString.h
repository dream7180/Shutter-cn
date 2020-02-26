/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// RString.h: interface for the RString class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_RSTRING_H__4A834A2B_2B48_4563_9A78_9483307C1F75__INCLUDED_)
#define AFX_RSTRING_H__4A834A2B_2B48_4563_9A78_9483307C1F75__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class RString : public CString
{
public:
	RString();
	RString(int rsrc_id);
	virtual ~RString();

	void Format(UINT format_id, ...);
	bool LoadString(int rsrc_id);

	void Replace(TCHAR old, TCHAR copy);

	RString SubStr(int index);

	const TCHAR* CStr()			{ const TCHAR* p= *static_cast<CString*>(this); return p; }
	operator const TCHAR* ()	{ return CStr(); }
};

#endif // !defined(AFX_RSTRING_H__4A834A2B_2B48_4563_9A78_9483307C1F75__INCLUDED_)

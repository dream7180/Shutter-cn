/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// ResizeWnd.h: interface for the ResizeWnd class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_RESIZEWND_H__649034A3_E6D8_4D1B_AEBF_BA9F65FBD7A1__INCLUDED_)
#define AFX_RESIZEWND_H__649034A3_E6D8_4D1B_AEBF_BA9F65FBD7A1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class ResizeWnd
{
public:
	ResizeWnd() {}
	virtual ~ResizeWnd() {}

	virtual int GetPaneHeight()= 0;
	virtual void ResizePane(int height)= 0;
	virtual void Resizing(bool start);
};

#endif // !defined(AFX_RESIZEWND_H__649034A3_E6D8_4D1B_AEBF_BA9F65FBD7A1__INCLUDED_)

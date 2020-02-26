/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// FrameRect.h: interface for the CFrameRect class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FRAMERECT_H__55DEE9CB_40CD_4783_8C11_DD5D155E6358__INCLUDED_)
#define AFX_FRAMERECT_H__55DEE9CB_40CD_4783_8C11_DD5D155E6358__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Frame.h"


class CFrameRect : public CFrame
{
public:
	CFrameRect(const CRect& location_rect, int prec);
	CFrameRect();
	virtual ~CFrameRect();

	virtual void Draw(CDC* dc, const CPrnUserParams& Params, CDC* dc_real= NULL);
};

#endif // !defined(AFX_FRAMERECT_H__55DEE9CB_40CD_4783_8C11_DD5D155E6358__INCLUDED_)

/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// FramePrepareDC.h: interface for the CFramePrepareDC class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FRAMEPREPAREDC_H__1DD2E7F9_8AC7_4082_916E_D07C9F9D9D85__INCLUDED_)
#define AFX_FRAMEPREPAREDC_H__1DD2E7F9_8AC7_4082_916E_D07C9F9D9D85__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CPrnUserParams;
class CFrame;


class CFramePrepareDC
{
public:
	CFramePrepareDC(CFrame& frame, const CPrnUserParams& params, CDC &dc);
	~CFramePrepareDC();

};

#endif // !defined(AFX_FRAMEPREPAREDC_H__1DD2E7F9_8AC7_4082_916E_D07C9F9D9D85__INCLUDED_)

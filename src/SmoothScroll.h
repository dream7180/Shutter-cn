/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// SmoothScroll.h: interface for the CSmoothScroll class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SMOOTHSCROLL_H__E07B0DBA_9371_42AD_8850_46B8778D6785__INCLUDED_)
#define AFX_SMOOTHSCROLL_H__E07B0DBA_9371_42AD_8850_46B8778D6785__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CSmoothScroll
{
public:
	CSmoothScroll(CWnd* wnd);

	void Hide(CRect rect, COLORREF rgb_back);

private:
	CWnd* wnd_;
};

#endif // !defined(AFX_SMOOTHSCROLL_H__E07B0DBA_9371_42AD_8850_46B8778D6785__INCLUDED_)

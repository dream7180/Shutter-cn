// Rect.h: interface for the MRect class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_RECT_H__FD9A0FCC_C1A6_4C48_898B_ABA86330FDED__INCLUDED_)
#define AFX_RECT_H__FD9A0FCC_C1A6_4C48_898B_ABA86330FDED__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

struct MRect : public RECT
{
public:
	MRect(int l, int t, int r, int b)
	{ left = l; top = t; right = r; bottom = b; }

	MRect() {}

	~MRect() {}

	int Width() const		{ return right - left; }
	int Height() const		{ return bottom - top; }
};

#endif // !defined(AFX_RECT_H__FD9A0FCC_C1A6_4C48_898B_ABA86330FDED__INCLUDED_)

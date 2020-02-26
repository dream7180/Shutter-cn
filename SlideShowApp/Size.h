// Size.h: interface for the MSize class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SIZE_H__D91F30C9_84BC_4C61_A60D_C9DA19035C69__INCLUDED_)
#define AFX_SIZE_H__D91F30C9_84BC_4C61_A60D_C9DA19035C69__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class MSize
{
public:
	MSize() {}
	MSize(int cx, int cy) : cx(cx), cy(cy) {}

	int cx;
	int cy;
};

#endif // !defined(AFX_SIZE_H__D91F30C9_84BC_4C61_A60D_C9DA19035C69__INCLUDED_)

// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_)
#define AFX_STDAFX_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#define _WIN32_WINNT 0x0400

#include <windows.h>
#include <vfw.h>
#include <stdio.h>
#include "../AutoPtr.h"
#include "Size.h"
#include "DC.h"
#include "Rect.h"
#include "Bitmap.h"
#include <assert.h>
#include <tchar.h>

#define ASSERT(x)	assert(x)

template<class Arr>
inline int array_count(const Arr& array)	{ return sizeof array / sizeof array[0]; }


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__A9DB83DB_A9FD_11D0_BFD1_444553540000__INCLUDED_)

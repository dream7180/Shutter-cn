/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__0C5D6B59_B044_44A1_9DAC_9B54CCCAF11F__INCLUDED_)
#define AFX_STDAFX_H__0C5D6B59_B044_44A1_9DAC_9B54CCCAF11F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#define _CRT_SECURE_NO_DEPRECATE
#define _SCL_SECURE_NO_WARNINGS
//#define _HAS_ITERATOR_DEBUGGING		0

#pragma warning(disable: 4786)  // identifier was truncated to '255' characters in the browser information

#include <vector>
//#include <unordered_map>
#include <string>
#include <list>
#include <map>
#include <functional>
#include <algorithm>
#include <numeric>
#include <sstream>
#include <iomanip>
#include <memory>
#include <fstream>
#include <unordered_set> //hash_set>
#include <unordered_map> //hash_map>
#include <utility>

// x86 build targetting WinXP needs 0501 below, or else things in the UI start falling apart (like rebar InsertBand)
//#define WINVER 0x0501
//#define WINVER 0x0600		// Vista
#define WINVER 0x0600		// Vista
#define _WIN32_IE 0x0700	// IE 7

#define __VSSYM32_H__	// force inclusion of vssym32.h instead of tmschema.h, even in WIN 0x0501

#pragma warning(disable: 4018)  // signed/unsigned mismatch

#pragma pack(8)

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxole.h>
#include <afxpriv.h>
#include <afxmt.h>

#undef CATCH_ALL

#include <VsStyle.h>

#include <ShlObj.h>
#include <ComDef.h>
#include <shlwapi.h>

#ifdef _DEBUG
	#undef DEBUG_NEW
	#define DEBUG_NEW new(__FILE__, __LINE__)
#endif


#ifdef USE_GDI_PLUS
	#define GDIPVER 0x0110
	#include <GdiPlus.h>
#endif


#ifndef WS_EX_COMPOSITED
  #define WS_EX_COMPOSITED        0x02000000L
#endif


//using namespace std;

typedef unsigned __int8		uint8;
typedef unsigned __int16	uint16;
typedef unsigned __int32	uint32;
typedef unsigned __int64	uint64;

typedef __int8				int8;
typedef __int16				int16;
typedef __int32				int32;
typedef __int64				int64;


template<class Arr>
inline int array_count(const Arr& array)	{ return sizeof array / sizeof array[0]; }

// simple auto ptr that allows assignment (unlike std::auto_ptr)
#include "AutoPtr.h"


#ifdef _UNICODE

typedef std::wstring String;
typedef std::wostringstream oStringstream;
typedef std::wistringstream iStringstream;

#else

typedef std::string String;
typedef std::ostringstream oStringstream;
typedef std::istringstream iStringstream;

#endif


#ifndef LVS_EX_DOUBLEBUFFER
#define LVS_EX_DOUBLEBUFFER     0x00010000
#endif


#define PACKVERSION(major,minor) MAKELONG(minor,major)

// define to use reference-counted ptr in place of raw pointers to PhotoInfo
#define PHOTO_INFO_SMART_PTR

// undefine those macros so std functions are visible
#undef min
#undef max

#undef MAX
#define MAX(a,b)	((a) > (b) ? (a) : (b))
#undef MIN
#define MIN(a,b)	((a) < (b) ? (a) : (b))

#ifndef CS_DROPSHADOW
#define CS_DROPSHADOW       0x00020000
#endif

// to build boost libs:
// debug
// b2 architecture=x86 address-model=64 link=static threading=multi variant=debug --with-date_time --with-thread --with-system stage
// release
// b2 architecture=x86 address-model=64 link=static runtime-link=static threading=multi variant=release --with-date_time --with-thread --with-system stage

#define BOOST_CHRONO_HEADER_ONLY 1
#define BOOST_CHRONO_NO_LIB 1

//#define BOOST_SYSTEM_NO_LIB 1

#include <boost/function.hpp>
#include <boost/bind.hpp>

// bjam link=static runtime-link=static threading=multi release
// bjam link=static runtime-link=shared threading=multi debug
//#define BOOST_SIGNALS_NO_LIB 1
#include <boost/signals2.hpp>

// threading lib uses date_time lib
//#define BOOST_THREAD_NO_LIB 1
//#define BOOST_DATE_TIME_NO_LIB 1
#include <boost/date_time.hpp>
#include <boost/thread/thread.hpp>

//#include <boost/chrono.hpp>

typedef boost::posix_time::ptime DateTime;
typedef boost::posix_time::time_duration TimeDuration;

//#include <afxdhtml.h>

// local copy of intrusive_ptr (incompatible with boost) working with public base 'counter_base'
#include "intrusive_ptr.h"

//#define nullptr 0

#endif // !defined(AFX_STDAFX_H__0C5D6B59_B044_44A1_9DAC_9B54CCCAF11F__INCLUDED_)

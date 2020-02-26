/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//extern String GetFormattedDateTime(const CTime& date, CDC& dc, int horz_space)
//{
//	String str;
//	SYSTEMTIME time;
//	if (date.GetAsSystemTime(time))
//	{
//		const int MAX= 200;
//		TCHAR label_date[MAX + 2]= { 0 };
//		::GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &time, 0, label_date, MAX);
//
//		TCHAR label_time[MAX + 2]= { 0 };
//		::GetTimeFormat(LOCALE_USER_DEFAULT, TIME_NOSECONDS, &time, 0, label_time, MAX);
//
//		str = label_date;
//		str += _T("  ");
//		str += label_time;
//
//		if (dc.GetTextExtent(str.c_str()).cx > horz_space)
//			str = label_date;	// use just date without time
//	}
//	return str;
//}

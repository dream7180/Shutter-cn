/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#include "stdafx.h"
#include "DateTimeUtils.h"
#include "StringConversions.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static String GetFormattedDateTime(const SYSTEMTIME& time, CDC& dc, int horz_space)
{
	String str;

	const int MAX= 200;
	TCHAR label_date[MAX + 2]= { 0 };
	::GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &time, 0, label_date, MAX);

	TCHAR label_time[MAX + 2]= { 0 };
	::GetTimeFormat(LOCALE_USER_DEFAULT, TIME_NOSECONDS, &time, 0, label_time, MAX);

	str = label_date;
	str += _T("  ");
	str += label_time;

	if (dc.GetTextExtent(str.c_str()).cx > horz_space)
		str = label_date;	// use just date without time

	return str;
}


String GetFormattedDateTime(DateTime date, CDC& dc, int horz_space)
{
	if (date.is_not_a_date_time())
		return L"?";
	else
		return GetFormattedDateTime(DateTimeToSytemTime(date), dc, horz_space);
}


//String GetFormattedDateTime(const CTime& date, CDC& dc, int horz_space)
//{
//	String str;
//	SYSTEMTIME time;
//	if (date.GetAsSystemTime(time))
//		str = GetFormattedDateTime(time, dc, horz_space);
//	return str;
//}


// read EXIF date/time (like "2012:08:15 08:41:03")

DateTime ExifDateToDateTime(const String& date_time)
{
	try
	{
		if (date_time.length() >= 19 && date_time[0] != _T(' '))
			if (const TCHAR* p= date_time.c_str())
				return DateTime(
					boost::gregorian::date(_ttoi(p), _ttoi(p + 5), _ttoi(p + 8)),
					TimeDuration(_ttoi(p + 11), _ttoi(p + 14), _ttoi(p + 17))
				);
	}
	catch (...)
	{}

	return DateTime();
}


std::string DateTimeToExifDate(DateTime tm)
{
	if (tm.is_not_a_date_time())
		return "0000:00:00 00:00:00";

	auto ymd= tm.date().year_month_day();
	auto t= tm.time_of_day();

	char date_time[200];
	wsprintfA(date_time, "%04d:%02d:%02d %02d:%02d:%02d", int(ymd.year), int(ymd.month), int(ymd.day), int(t.hours()), int(t.minutes()), int(t.seconds()));
	return date_time;
}


String DateTimeFmt(SYSTEMTIME time, const TCHAR* dt_separator)
{
	oStringstream ost;

	const int MAX= 100;
	TCHAR label_date[MAX + 2]= { 0 };
	::GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &time, 0, label_date, MAX);

	TCHAR label_time[MAX + 2]= { 0 };
	::GetTimeFormat(LOCALE_USER_DEFAULT, 0/*TIME_NOSECONDS*/, &time, 0, label_time, MAX);

	ost << label_date;
	if (dt_separator && *dt_separator)
		ost << dt_separator;
	ost << label_time;

	return ost.str();
}


DateTime SytemTimeToDateTime(const SYSTEMTIME& tm)
{
	// milliseconds
	const auto adjustor= TimeDuration::ticks_per_second() / 1000;

	DateTime dt(
		boost::gregorian::date(tm.wYear, tm.wMonth, tm.wDay),
		TimeDuration(tm.wHour, tm.wMinute, tm.wSecond, tm.wMilliseconds * adjustor));

	return dt;
}


DateTime ToDateTime(int year, int month, int day, int hour, int minute, int second)
{
	DateTime dt(
		boost::gregorian::date(year, month, day),
		TimeDuration(hour, minute, second));

	return dt;
}


SYSTEMTIME DateTimeToSytemTime(DateTime tm)
{
//{
//FILETIME ftime;
//ftime.dwHighDateTime = 0;
//ftime.dwLowDateTime = 0;
//auto dtm= FileTimeToDateTime(ftime);
//std::string s= DateTimeToExifDate(dtm);
//ftime.dwLowDateTime = 1;
//dtm= FileTimeToDateTime(ftime);
//s= DateTimeToExifDate(dtm);
//s.length();
//}
	SYSTEMTIME time;

	if (tm.is_not_a_date_time())
	{
		memset(&time, 0, sizeof time);
		return time;
	}

	auto d= tm.date();
	time.wYear = d.year();
	time.wMonth = d.month();
	time.wDay = d.day();
	time.wDayOfWeek = d.day_of_week();

	auto t= tm.time_of_day();
	time.wHour = t.hours();
	time.wMinute = t.minutes();
	time.wSecond = t.seconds();

	const auto adjustor= TimeDuration::ticks_per_second() / 1000;
	time.wMilliseconds = static_cast<WORD>(t.fractional_seconds() / adjustor);

	return time;
}


String DateTimeFmt(DateTime tm, const TCHAR* dt_separator)
{
	if (tm.is_not_a_date_time())
		return L"-";

	SYSTEMTIME time= DateTimeToSytemTime(tm);
	return DateTimeFmt(time, dt_separator);
}


//String DateTimeFmt(CTime tm, const TCHAR* dt_separator)
//{
//	SYSTEMTIME time;
//
//	if (tm.GetAsSystemTime(time))
//		return DateTimeFmt(time, dt_separator);
//	else
//		return String(L"-");
//}


String DateFmt(const SYSTEMTIME& time, DateFmtType fmt)
{
	const int MAX= 100;
	TCHAR date[MAX + 2]= { 0 };
	TCHAR fmt_buf[MAX + 2]= { 0 };

	switch (fmt)
	{
	case DateFmt_Medium:
		{
			CTime now= CTime::GetCurrentTime();
			SYSTEMTIME t1;
			if (now.GetAsSystemTime(t1) && t1.wYear == time.wYear && t1.wMonth == time.wMonth && t1.wDay == time.wDay)
				return _T("今日");

			now -= CTimeSpan(1, 0, 0, 0);	// go back one day
			if (now.GetAsSystemTime(t1) && t1.wYear == time.wYear && t1.wMonth == time.wMonth && t1.wDay == time.wDay)
				return _T("昨日");

			TCHAR* format= fmt_buf;
			if (::GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SLONGDATE, fmt_buf, MAX) > 0)
			{
				if (_tcsncmp(_T("dddd"), format, 4) == 0)
				{
					format += 4;	// skip 'dddd' (day of week)

					while (*format == ' ' || *format == ',' || *format == ';' || *format == '-')
						format++;
				}

				::GetDateFormat(LOCALE_USER_DEFAULT, 0, &time, format, date, MAX);
			}
			else
				::GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &time, 0, date, MAX);
		}
		break;

	case DateFmt_Short:
		::GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &time, 0, date, MAX);
		break;

	case DateFmt_Long:
		::GetDateFormat(LOCALE_USER_DEFAULT, DATE_LONGDATE, &time, 0, date, MAX);
		break;

	case DateFmt_Year:
		::GetDateFormat(LOCALE_USER_DEFAULT, 0, &time, _T("yyyy"), date, MAX);
		break;

	case DateFmt_YearMonth:
		//DWORD ver= ::GetVersion();
		//if ((ver & DWORD(0x80000000)) != 0 || LOWORD(ver) < 5)
		//{
		//	oStringstream ost;
		//	ost << int(time.wYear) << _T(" ") << int(time.wMonth);
		//	return ost.str();
		//}
		//else
			::GetDateFormat(LOCALE_USER_DEFAULT, DATE_YEARMONTH, &time, 0, date, MAX);
		break;
	}

	return date;
}


//String DateFmt(CTime date_time, DateFmtType fmt)
//{
//	SYSTEMTIME time;
//	if (date_time.GetAsSystemTime(time))
//		return DateFmt(time, fmt);
//	else
//		return _T("?");
//}


String DateFmt(DateTime date_time, DateFmtType fmt)
{
	if (date_time.is_not_a_date_time())
		return L"";

	auto time= DateTimeToSytemTime(date_time);
	return DateFmt(time, fmt);
}


int GetDateTimeMilliseconds(DateTime tm)
{
	if (tm.is_not_a_date_time())
		return 0;

	const auto adjustor= TimeDuration::ticks_per_second() / 1000;
	return static_cast<int>(tm.time_of_day().fractional_seconds() / adjustor);
}


FILETIME DateTimeToFileTime(DateTime pt, bool to_utc)
{
	// extract the date from boost::posix_time to SYSTEMTIME
	SYSTEMTIME st;
	boost::gregorian::date::ymd_type ymd = pt.date().year_month_day();

	st.wYear = ymd.year;
	st.wMonth = ymd.month;
	st.wDay = ymd.day;
	st.wDayOfWeek = pt.date().day_of_week();

	// Now extract the hour/min/second field from time_duration
	boost::posix_time::time_duration td = pt.time_of_day();
	st.wHour = static_cast<WORD>(td.hours());
	st.wMinute = static_cast<WORD>(td.minutes());
	st.wSecond = static_cast<WORD>(td.seconds());

	// Although ptime has a fractional second field, SYSTEMTIME millisecond
	// field is 16 bit, and will not store microsecond. We will treat this
	// field separately later.
	st.wMilliseconds = 0;

	// Convert SYSTEMTIME to FILETIME structure
	FILETIME ft;
	SystemTimeToFileTime(&st, &ft);

	// Now we are almost done. The FILETIME has date, and time. It is only missing fractional second.

	// Extract the raw FILETIME into a 64 bit integer.
	boost::uint64_t _100nsSince1601 = ft.dwHighDateTime;
	_100nsSince1601 <<= 32;
	_100nsSince1601 |= ft.dwLowDateTime;

	// Add in the fractional second, which is in microsecond * 10 to get 100s of nanosecond
	_100nsSince1601 += td.fractional_seconds() * 10;

	// Now put the time back inside filetime.
	ft.dwHighDateTime = _100nsSince1601 >> 32;
	ft.dwLowDateTime = _100nsSince1601 & 0x00000000FFFFFFFF;

	if (to_utc)	// transform from local to UTC time?
	{
		FILETIME utc;
		if (!LocalFileTimeToFileTime(&ft, &utc))
			CFileException::ThrowOsError(::GetLastError());

		return utc;
	}

	return ft;
}


void SetFileCreationModificationTime(const TCHAR* path, DateTime tm)
{
	auto time= DateTimeToFileTime(tm, true);

	auto file= ::CreateFile(path, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file == INVALID_HANDLE_VALUE)
		CFileException::ThrowOsError(::GetLastError(), path);

	if (!::SetFileTime(file, &time, nullptr, &time))
	{
		auto err= ::GetLastError();
		::CloseHandle(file);
		CFileException::ThrowOsError(err, path);
	}

	::CloseHandle(file);
}


DateTime FileTimeToDateTime(FILETIME ftime)
{
	return boost::posix_time::from_ftime<DateTime>(ftime);
}


double DateTimeToFraction(DateTime dt)
{
	if (dt.is_not_a_date_time())
		return 0.0;

	//dt.time_count();
	double d= dt.date().day_number();
	double div= double(TimeDuration::ticks_per_second());
	double t= dt.time_of_day().ticks() / div / (24 * 60 * 60);

	return d + t;
}


TimeDuration CreateTimeDuration(int days, int hours, int minutes, int seconds)
{
	long long d= days;
	d *= 24 * 60 * 60;

	long long h= hours;
	h *= 60 * 60;

	long long m= minutes;
	m *= 60;

	long long fr= d + h + m + seconds;

	return TimeDuration(0, 0, 0, fr * TimeDuration::ticks_per_second());
}


DateTime AdjustDateTime(DateTime tm, TimeDuration span)
{
	if (tm.is_not_a_date_time())
		return tm;

	tm += span;		// this may throw already

	auto ymd= tm.date().year_month_day();
	if (ymd.year < 1601)	// limit of FILETIME
		throw std::invalid_argument("Invalid date below year 1601");

	if (ymd.year > 9999)	// EXIF field limit
		throw std::invalid_argument("Invalid date above year 9999");

	return tm;
}


DateTime FromISOString(const std::wstring& str)
{
	return boost::posix_time::from_iso_string(WStr2UTF8(str));
}


std::wstring ToISOString(DateTime dt)
{
	auto t= to_iso_string(dt);
	std::wstring w;
	MultiByteToWideString(t, w);
	return w;
}

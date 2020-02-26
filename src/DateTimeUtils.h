/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

// Date & Time util functions


String GetFormattedDateTime(DateTime date, CDC& dc, int horz_space);

// read EXIF date/time (like "2012:08:15 08:41:03") and return it as DateTime struct
// doesn't throw; DateTime may be 'empty'
DateTime ExifDateToDateTime(const String& date_time);

std::string DateTimeToExifDate(DateTime tm);

String DateTimeFmt(DateTime tm, const TCHAR* dt_separator= L"  ");

SYSTEMTIME DateTimeToSytemTime(DateTime tm);
DateTime SytemTimeToDateTime(const SYSTEMTIME& tm);
DateTime ToDateTime(int year, int month, int day, int hour, int minute, int second);

enum DateFmtType { DateFmt_Short, DateFmt_Long, DateFmt_Year, DateFmt_YearMonth, DateFmt_Medium };

String DateFmt(const SYSTEMTIME& time, DateFmtType fmt);
String DateFmt(DateTime date_time, DateFmtType fmt);

int GetDateTimeMilliseconds(DateTime tm);

void SetFileCreationModificationTime(const TCHAR* path, DateTime tm);

DateTime FileTimeToDateTime(FILETIME ftime);

// return date/time as a fractional number
double DateTimeToFraction(DateTime dt);

TimeDuration CreateTimeDuration(int days, int hours, int minutes, int seconds);

DateTime AdjustDateTime(DateTime tm, TimeDuration span);

// parse date/time in ISO representation
DateTime FromISOString(const std::wstring& str);

// serialize date/time as ISO string
std::wstring ToISOString(DateTime dt);

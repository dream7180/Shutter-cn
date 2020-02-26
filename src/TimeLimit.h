/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#ifndef _time_limit_h_
#define _time_limit_h_

#ifdef DEMO	// ======================================================

#define TIME_LIMIT	CTime(2004, 3, 1, 0, 0, 0)


#define CHECK_TIME_LIMIT	\
	{ \
		if (CTime::GetCurrentTime() > TIME_LIMIT) \
		{ \
			AfxMessageBox(_T("This beta copy of ExifPro has expired.")); \
			::exit(0); \
		} \
	} \



#else	// release ==================================================

	#define CHECK_TIME_LIMIT

#endif	// ==========================================================


#endif // _time_limit_h_

/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/


#define w2a(const wchar_t* str, char* out)		\
		{	int _convert = (lstrlenA(str) + 1),	\
			AfxA2WHelper(reinterpret_cast<wchar_t*>(alloca(_convert * 2)), out, _convert); \
		}

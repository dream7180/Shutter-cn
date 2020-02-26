/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#define SHOW_STACK_INFO


#ifdef SHOW_STACK_INFO

inline __declspec(naked) DWORD __stack_ptr()	{ DWORD d; __asm mov d, esp; return d; }

#define STACK_INFO							\
{											\
	MEMORY_BASIC_INFORMATION mi;			\
	BYTE* page= __stack_ptr();				\
	::VirtualQuery(page, &mi, sizeof mi);	\
}											\


#define xSTACK_INFO							\
{											\
	MEMORY_BASIC_INFORMATION mi;			\
	BYTE* page;							\
	_asm mov page, esp;					\
	::VirtualQuery(page, &mi, sizeof mi);	\
	CString str;							\
	str.Format(_T("ESP %p   base %p   alloc base %p   height %d"), page, mi.BaseAddress, mi.AllocationBase, (char*)mi.BaseAddress - (char*)mi.AllocationBase);	\
	AfxMessageBox(str, MB_OK);	 			\
}											\


#else	/////////////////////////////////////

#define STACK_INFO


#endif

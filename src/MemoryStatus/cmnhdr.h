/*____________________________________________________________________________

   ExifPro Image Viewer

   Copyright (C) 2000-2015 Michael Kowalski
____________________________________________________________________________*/

#ifndef CMNHDR_H
#define CMNHDR_H

/******************************************************************************
Module name: CmnHdr.h
Written by: Jeffrey Richter
Notices: Copyright (c) 1995-1998 Jeffrey Richter
Purpose: Common header file containing handy macros and definitions used
         throughout all the applications in the book.
******************************************************************************/


/* Disable ridiculous warnings so that the code */
/* compiles cleanly using warning level 4.      */

/* nonstandard extension 'single line comment' was used */
#pragma warning(disable: 4001)

// nonstandard extension used : nameless struct/union
#pragma warning(disable: 4201)

// unreferenced inline function has been removed
#pragma warning(disable: 4514)

// unreferenced formal parameter
#pragma warning(disable: 4100)

// nonstandard extension used : bit field types other than int
#pragma warning(disable: 4214)

// Note: Creating precompiled header 
#pragma warning(disable: 4699)

// indirection to slightly different base types
#pragma warning(disable: 4057)

// named type definition in parentheses
#pragma warning(disable: 4115)

// nonstandard extension used : benign typedef redefinition
#pragma warning(disable: 4209)

// 'function' : function not expanded
#pragma warning(disable: 4710)


//////////////////////// Windows Version Build Option /////////////////////////


//#define _WIN32_WINNT 0x0400


///////////////////////////// STRICT Build Option /////////////////////////////


// Force all EXEs/DLLs to use STRICT type checking.
#ifndef STRICT
#define STRICT
#endif

/////////////////////////// CPU Portability Macros ////////////////////////////


// If no CPU platform was specified, default to the current platform.
#if !defined(_ALPHA_) && !defined(_X86_)
   #if defined(_M_IX86)
      #define _X86_
   #endif
   #if defined(_M_ALPHA)
      #define _ALPHA_
   #endif
#endif


//////////////////////////// Unicode Build Option /////////////////////////////


// If we are not compiling for an x86 CPU, we always compile using Unicode.
#ifndef _X86_
#define UNICODE
#endif


// To compile using Unicode on the x86 CPU, uncomment the line below.
//#define UNICODE

// When using Unicode Win32 functions, use Unicode C-Runtime functions too.
//#ifdef UNICODE
//#define _UNICODE
//#endif


////////////////////////////// chINRANGE Macro ////////////////////////////////


// This macro returns TRUE if a number is between two others
#define chINRANGE(low, Num, High) (((low) <= (Num)) && ((Num) <= (High)))


//////////////////////////////// chDIMOF Macro ////////////////////////////////


// This macro evaluates to the number of elements in an array. 
#define chDIMOF(Array) (sizeof(Array) / sizeof(Array[0]))


///////////////////////////// chBEGINTHREADEX Macro ///////////////////////////


// Create a chBEGINTHREADEX macro that calls the C run-time's
// _beginthreadex function. The C run-time library doesn't
// want to have any reliance on Win32 data types such as
// HANDLE. This means that a Win32 programmer needs to cast
// the return value to a HANDLE. This is terribly inconvenient,
// so I have created this macro to perform the casting.
typedef unsigned (__stdcall *PTHREAD_START) (void *);

#define chBEGINTHREADEX(lpsa, cbStack, lpStartAddr, \
   lpvThreadParm, fdwCreate, lpIDThread)          \
      ((HANDLE)_beginthreadex(                    \
         (void *) (lpsa),                         \
         (unsigned) (cbStack),                    \
         (PTHREAD_START) (lpStartAddr),           \
         (void *) (lpvThreadParm),                \
         (unsigned) (fdwCreate),                  \
         (unsigned *) (lpIDThread)))


//////////////////////////// Assert/Verify Macros /////////////////////////////


#define chFAIL(szMSG) {                                                   \
      MessageBox(GetActiveWindow(), szMSG,                                \
         __TEXT("Assertion Failed"), MB_OK | MB_ICONERROR);               \
      DebugBreak();                                                       \
   }

// Put up an assertion failure message box.
#define chDEBUG_ASSERTFAIL(file,line,expr) {                                    \
      TCHAR sz[128];                                                      \
      wsprintf(sz, __TEXT("File %hs, line %d : %hs"), file, line, expr);  \
      chFAIL(sz);                                                         \
   }

// Put up a message box if an assertion fails in a debug build.
#ifdef _DEBUG
#define chDEBUG_ASSERT(x) if (!(x)) chDEBUG_ASSERTFAIL(__FILE__, __LINE__, #x)
#else
#define chDEBUG_ASSERT(x)
#endif

// Assert in debug builds, but don't remove the code in retail builds.
#ifdef _DEBUG
#define chVERIFY(x) chDEBUG_ASSERT(x)
#else
#define chVERIFY(x) (x)
#endif


/////////////////////////// chHANDLE_DLGMSG Macro /////////////////////////////


// The normal HANDLE_MSG macro in WINDOWSX.H does not work properly for dialog
// boxes because a DlgProc returns a BOOL instead of an LRESULT (like
// WndProcs). This chHANDLE_DLGMSG macro corrects the problem:
#define chHANDLE_DLGMSG(hwnd, message, fn)                           \
   case (message): return (SetDlgMsgResult(hwnd, uMsg,               \
      HANDLE_##message((hwnd), (wParam), (lParam), (fn))))


/////////////////////////// Quick MessageBox Macro ////////////////////////////


#define chMB(s) {                                                    \
      TCHAR szTMP[128];                                              \
      GetModuleFileName(NULL, szTMP, chDIMOF(szTMP));                \
      MessageBox(GetActiveWindow(), s, szTMP, MB_OK);                \
   }


///////////////////////////// Zero Variable Macro /////////////////////////////


// Zero out a structure. If fInitSize is TRUE, initialize 
// the first int to the size of the structure. Many structures 
// like WNDCLASSEX and STARTUPINFO require that their first 
// member be set to the size of the structure itself.
#define chINITSTRUCT(structure, fInitSize)                           \
   (ZeroMemory(&(structure), sizeof(structure)),                     \
   fInitSize ? (*(int*) &(structure) = sizeof(structure)) : 0)


//////////////////////// Dialog Box Icon Setting Macro ////////////////////////


// Associates an icon with a window.
#define chSETDLGICONS(hwnd, idiLarge, idiSmall)                            \
   {                                                                       \
      SendMessage(hwnd, WM_SETICON, TRUE,  (LPARAM)                        \
         LoadIcon(GetWindowInstance(hwnd), MAKEINTRESOURCE(idiLarge)));    \
      SendMessage(hwnd, WM_SETICON, FALSE, (LPARAM)                        \
         LoadIcon(GetWindowInstance(hwnd), MAKEINTRESOURCE(idiSmall)));    \
   }
    

///////////////////////////// UNICODE Check Macro /////////////////////////////


#ifdef UNICODE

#define chWARNIFUNICODEUNDERWIN95()                                        \
   if (GetWindowsDirectoryW(NULL, 0) <= 0)                                 \
      MessageBoxA(NULL, "This operating system doesn't support Unicode.",  \
         NULL, MB_OK)

#else

#define chWARNIFUNICODEUNDERWIN95()

#endif


///////////////////////// Pragma message helper macro /////////////////////////


/* 
When the compiler sees a line like this:
#pragma chMSG(Fix this later)

it outputs a line like this:
C:\Document\AdvWin\Code\Sysinfo.06\..\CmnHdr.H(296):Fix this later

You can easily jump directly to this line and examine the surrounding code.
*/
#define chSTR(x)       #x
#define chSTR2(x)   chSTR(x)
#define chMSG(desc) message(__FILE__ "(" chSTR2(__LINE__) "):" #desc)


//////////////////////////// Message Cracker Macros ///////////////////////////
// WindowsX.H doesn't have these message cracker macros


/* BOOL Cls_OnCopyData(HWND hwnd, HWND hwndFrom, PCOPYDATASTRUCT pcds) */
#define HANDLE_WM_COPYDATA(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (HWND)(wParam), \
     (PCOPYDATASTRUCT)lParam), 0L)
#define FORWARD_WM_COPYDATA(hwnd, hwndFrom, pcds, fn) \
    (BOOL)(UINT)(DWORD)(fn)((hwnd), WM_COPYDATA, \
      (WPARAM)(hwndFrom), (LPARAM)(pcds))


/* void Cls_OnHotKey(HWND hwnd, int idHotKey, UINT fuModifiers, UINT vk) */
#define HANDLE_WM_HOTKEY(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (int)(wParam), (UINT)LOWORD(lParam), (UINT)HIWORD(lParam)), 0L)
#define FORWARD_WM_HOTKEY(hwnd, idHotKey, fuModifiers, vk, fn) \
    (void)(fn)((hwnd), WM_HOTKEY, (WPARAM)(idHotKey), MAKELPARAM((fuModifiers), (vk)))


// WindowsX.H defines HANDLE_WM_NCCALCSIZE incorrectly
/*
#undef HANDLE_WM_NCCALCSIZE
#define HANDLE_WM_NCCALCSIZE(hwnd, wParam, lParam, fn) \
    (LRESULT)(DWORD)(UINT)(fn)((hwnd), (BOOL)(wParam), (NCCALCSIZE_PARAMS *)(lParam))
*/

///////////////////////////////// End of File /////////////////////////////////

#endif
